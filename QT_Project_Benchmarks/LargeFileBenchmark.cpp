/**
 * @file LargeFileBenchmark.cpp
 * @brief Implements the explicitly invoked large-file performance executable.
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTextStream>
#include <QTimer>
#include <algorithm>
#include <functional>
#include <utility>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <Psapi.h>
#endif

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogCacheReadService.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

namespace
{
/**
 * @brief Returns the current process working-set size.
 * @return Working-set size in bytes, or zero when unavailable.
 */
[[nodiscard]] auto get_working_set_bytes() -> quint64
{
    quint64 bytes = 0;
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS counters{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != FALSE)
    {
        bytes = counters.WorkingSetSize;
    }
#endif
    return bytes;
}

/**
 * @class BenchmarkRunner
 * @brief Owns the asynchronous benchmark state and composes the measured production services.
 */
class BenchmarkRunner final
{
    public:
        /**
         * @brief Constructs the isolated benchmark composition.
         * @param dataset_path Unique dataset copy without an existing cache generation.
         * @param results_path CSV output path.
         * @param source_state Commit and working-tree state stored with the measurements.
         * @param storage_root Temporary root used for history and persistent cache files.
         * @param include_message_sort Whether the known-expensive message sort is measured.
         * @param completion Callback invoked with the final process exit code.
         */
        BenchmarkRunner(QString dataset_path, QString results_path, QString source_state,
                        QString storage_root, bool include_message_sort,
                        std::function<void(int)> completion)
            : m_dataset_path(std::move(dataset_path)),
              m_results_path(std::move(results_path)),
              m_source_state(std::move(source_state)),
              m_storage_root(std::move(storage_root)),
              m_include_message_sort(include_message_sort),
              m_completion(std::move(completion)),
              m_profile(LogParsingProfile::create_default(
                  QStringLiteral("{timestamp} {level} {message} {app_name}"),
                  QStringLiteral("Large-file benchmark"))),
              m_ingest(m_profile),
              m_filters(&m_views),
              m_cache_catalog(QDir(m_storage_root).filePath(QStringLiteral("index-v2"))),
              m_cache_reader(&m_cache_catalog, &m_views),
              m_history(QDir(m_storage_root).filePath(QStringLiteral("history.sqlite"))),
              m_history_writer(m_history.get_database_path()),
              m_pages(&m_history, &m_views),
              m_queries(&m_filters, &m_pages, &m_views),
              m_imports(m_profile, &m_ingest, &m_views, &m_history, &m_history_writer, &m_pages,
                        &m_queries, nullptr, &m_cache_catalog)
        {
            m_history.set_cache_read_service(&m_cache_reader);
            QObject::connect(&m_imports, &LogImportCoordinator::finished,
                             [this](const QUuid& view_id, const QString& file_path) {
                                 handle_finished(view_id, file_path);
                             });
            QObject::connect(
                &m_imports, &LogImportCoordinator::error,
                [this](const QUuid& view_id, const QString& file_path, const QString& message) {
                    handle_error(view_id, file_path, message);
                });
        }

        /** @brief Starts the fresh-import phase on the next event-loop iteration. */
        auto start() -> void
        {
            QTimer::singleShot(0, &m_imports, [this] { start_phase(); });
        }

    private:
        struct Result {
                QString phase;
                QString meaning;
                double duration_ms{0.0};
                qint64 entry_count{0};
                quint64 working_set_bytes{0};
                bool includes_throughput{false};
        };

        /** @brief Starts the currently selected import phase. */
        auto start_phase() -> void
        {
            const QString phase = m_scenario == QStringLiteral("fresh_import")
                                      ? QStringLiteral("Erstimport")
                                      : QStringLiteral("Cache-Treffer");
            qInfo().noquote() << QStringLiteral("Starte Messphase: %1").arg(phase);
            m_timer.start();
            m_active_view = m_imports.import_file_async(m_dataset_path, m_profile, 10'000);
            if (m_active_view.isNull())
            {
                m_completion(2);
            }
        }

        /**
         * @brief Records a completed import and advances to the next phase.
         * @param view_id Completed view.
         * @param file_path Completed source path.
         */
        auto handle_finished(const QUuid& view_id, const QString& file_path) -> void
        {
            const bool matches_active =
                view_id == m_active_view && QFileInfo(file_path).absoluteFilePath() ==
                                                QFileInfo(m_dataset_path).absoluteFilePath();
            if (matches_active)
            {
                qint64 entry_count = 0;
                const LogPageState* state = m_pages.get_page_state(view_id);
                if (state != nullptr)
                {
                    entry_count = state->get_total_entries();
                }
                const bool fresh_import = m_scenario == QStringLiteral("fresh_import");
                const QString phase = fresh_import ? QStringLiteral("Erstimport")
                                                   : QStringLiteral("Cache-Treffer");
                const QString meaning =
                    fresh_import
                        ? QStringLiteral("Datei vollständig einlesen, analysieren und speichern")
                        : QStringLiteral("Bereits aufgebauten Cache öffnen und erste Seite laden");
                const double duration_ms =
                    static_cast<double>(m_timer.nsecsElapsed()) / 1000000.0;
                m_results.append(Result{phase, meaning, duration_ms, entry_count,
                                        get_working_set_bytes(), true});
                qInfo().noquote()
                    << QStringLiteral("Messphase abgeschlossen: %1 (%2 s)")
                           .arg(phase, QString::number(duration_ms / 1000.0, 'f', 3));
                m_active_view = QUuid();

                if (m_scenario == QStringLiteral("fresh_import"))
                {
                    m_scenario = QStringLiteral("cache_hit");
                    QTimer::singleShot(0, &m_imports, [this] { start_phase(); });
                }
                else
                {
                    const bool interactions_measured = run_interaction_benchmarks(view_id);
                    const bool written = interactions_measured && write_results();
                    m_imports.shutdown();
                    m_completion(written ? 0 : 3);
                }
            }
        }

        /**
         * @brief Measures one synchronous interaction against the loaded cache view.
         * @param view_id View used by the operation.
         * @param phase Human-readable phase name.
         * @param meaning Human-readable description of the measured work.
         * @param operation Operation returning whether the query or page load succeeded.
         * @return True when the measured operation succeeded.
         */
        auto measure_interaction(const QUuid& view_id, const QString& phase,
                                 const QString& meaning,
                                 const std::function<bool()>& operation) -> bool
        {
            qInfo().noquote() << QStringLiteral("Starte Messphase: %1").arg(phase);
            QElapsedTimer timer;
            timer.start();
            const bool succeeded = operation();
            qint64 entry_count = 0;
            const LogPageState* state = m_pages.get_page_state(view_id);
            if (state != nullptr)
            {
                entry_count = state->get_total_entries();
            }
            if (succeeded)
            {
                const double duration_ms =
                    static_cast<double>(timer.nsecsElapsed()) / 1000000.0;
                m_results.append(Result{phase, meaning, duration_ms, entry_count,
                                        get_working_set_bytes(), false});
                qInfo().noquote()
                    << QStringLiteral("Messphase abgeschlossen: %1 (%2 s)")
                           .arg(phase, QString::number(duration_ms / 1000.0, 'f', 3));
            }
            return succeeded;
        }

        /**
         * @brief Applies one explicit sort order to the active result set.
         * @param view_id View whose query is updated.
         * @param column Sortable model column selected by the user-facing path.
         * @param order Requested sort direction.
         * @return True when the updated query was loaded.
         */
        auto apply_sort(const QUuid& view_id, int column, Qt::SortOrder order) -> bool
        {
            return m_queries.set_sort(view_id, column, order);
        }

        /**
         * @brief Measures paging, sorting, filtering, plain search, and regular-expression search.
         * @param view_id Fully loaded cache-backed view.
         * @return True when every interaction completed successfully.
         */
        auto run_interaction_benchmarks(const QUuid& view_id) -> bool
        {
            const LogPageState* state = m_pages.get_page_state(view_id);
            const qsizetype total_pages = state != nullptr ? state->get_total_pages() : 1;
            const qsizetype middle_page = std::max<qsizetype>(1, total_pages / 2);
            bool succeeded = state != nullptr;

            if (succeeded && total_pages > 1)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Zweite Seite öffnen"),
                    QStringLiteral("Von Seite 1 auf die direkt benachbarte Seite 2 wechseln"),
                    [this, view_id] { return m_pages.set_current_page(view_id, 2); });
            }
            if (succeeded && total_pages > 1)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Erste Seite wieder öffnen"),
                    QStringLiteral("Von Seite 2 zur ersten Ergebnisseite zurückkehren"),
                    [this, view_id] { return m_pages.set_current_page(view_id, 1); });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Mittlere Seite öffnen"),
                    QStringLiteral("Von der ersten auf die mittlere Ergebnisseite wechseln"),
                    [this, view_id, middle_page] {
                        return m_pages.set_current_page(view_id, middle_page);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Letzte Seite öffnen"),
                    QStringLiteral("Von der mittleren auf die letzte Ergebnisseite wechseln"),
                    [this, view_id, total_pages] {
                        return m_pages.set_current_page(view_id, total_pages);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Zeitstempel aufsteigend sortieren"),
                    QStringLiteral("Alle Einträge nach dem ältesten Zeitstempel zuerst sortieren"),
                    [this, view_id] {
                        return apply_sort(view_id, LogModel::Timestamp, Qt::AscendingOrder);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Anwendung sortieren"),
                    QStringLiteral("Alle Einträge alphabetisch nach Anwendung sortieren"),
                    [this, view_id] {
                        return apply_sort(view_id, LogModel::AppName, Qt::AscendingOrder);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Log-Level sortieren"),
                    QStringLiteral("Alle Einträge alphabetisch nach Log-Level sortieren"),
                    [this, view_id] {
                        return apply_sort(view_id, LogModel::Level, Qt::AscendingOrder);
                    });
            }
            if (succeeded && m_include_message_sort)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Nachricht sortieren"),
                    QStringLiteral("Alle Einträge alphabetisch nach Nachricht sortieren"),
                    [this, view_id] {
                        return apply_sort(view_id, LogModel::Message, Qt::AscendingOrder);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Standardsortierung wiederherstellen"),
                    QStringLiteral("Neueste Zeitstempel wieder zuerst anzeigen"),
                    [this, view_id] {
                        return apply_sort(view_id, LogModel::Timestamp, Qt::DescendingOrder);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Anwendungsfilter setzen"),
                    QStringLiteral("Nur Logeinträge der Anwendung App01 laden"),
                    [this, view_id] {
                        return m_queries.set_app_name(view_id, QStringLiteral("App01"));
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Anwendungsfilter entfernen"),
                    QStringLiteral("Wieder Logeinträge aller Anwendungen laden"),
                    [this, view_id] { return m_queries.set_app_name(view_id, QString()); });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Log-Level-Filter setzen"),
                    QStringLiteral("Nur Logeinträge mit Level ERROR laden"),
                    [this, view_id] {
                        return m_queries.set_log_levels(view_id,
                                                        QSet<QString>{QStringLiteral("ERROR")});
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Log-Level-Filter entfernen"),
                    QStringLiteral("Wieder Logeinträge aller Level laden"),
                    [this, view_id] { return m_queries.set_log_levels(view_id, {}); });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Textsuche ausführen"),
                    QStringLiteral("Nach dem Text scenario=large-file in Meldungen suchen"),
                    [this, view_id] {
                        return m_queries.set_search(view_id, QStringLiteral("scenario=large-file"),
                                                    SearchField::Message, false);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Textsuche entfernen"),
                    QStringLiteral("Textsuche leeren und ungefilterte erste Seite laden"),
                    [this, view_id] {
                        return m_queries.set_search(view_id, QString(), SearchField::Message, false);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Regex-Suche ausführen"),
                    QStringLiteral("Regulären Ausdruck über alle vier Millionen Meldungen prüfen"),
                    [this, view_id] {
                        return m_queries.set_search(view_id,
                                                    QStringLiteral("Benchmark entry 0{3}[0-9]{7}"),
                                                    SearchField::Message, true);
                    });
            }
            if (succeeded)
            {
                succeeded = measure_interaction(
                    view_id, QStringLiteral("Regex-Suche entfernen"),
                    QStringLiteral("Regulären Ausdruck leeren und erste Seite neu laden"),
                    [this, view_id] {
                        return m_queries.set_search(view_id, QString(), SearchField::Message, false);
                    });
            }
            return succeeded;
        }

        /**
         * @brief Stops the benchmark when the active import fails.
         * @param view_id Failed view.
         * @param file_path Failed source path.
         * @param message Failure description.
         */
        auto handle_error(const QUuid& view_id, const QString& file_path,
                          const QString& message) -> void
        {
            Q_UNUSED(file_path);
            qCritical().noquote() << message;
            if (view_id == m_active_view)
            {
                m_completion(4);
            }
        }

        /**
         * @brief Writes machine-readable CSV data and a human-readable Markdown report.
         * @return True when both result files were opened and written.
         */
        [[nodiscard]] auto write_results() const -> bool
        {
            QFile output(m_results_path);
            const QFileInfo output_info(m_results_path);
            const QString report_path = output_info.dir().filePath(
                QStringLiteral("%1.md").arg(output_info.completeBaseName()));
            QFile report(report_path);
            const bool csv_has_content = output_info.exists() && output_info.size() > 0;
            const QFileInfo report_info(report_path);
            const bool report_has_content = report_info.exists() && report_info.size() > 0;
            const bool csv_opened = output.open(QIODevice::WriteOnly | QIODevice::Text |
                                                QIODevice::Append);
            const bool report_opened = report.open(QIODevice::WriteOnly | QIODevice::Text |
                                                   QIODevice::Append);
            const bool opened = csv_opened && report_opened;
            if (opened)
            {
                QTextStream stream(&output);
                QTextStream report_stream(&report);
                if (!csv_has_content)
                {
                    stream
                        << "Zeitpunkt (UTC nach ISO 8601);Git-Commit;Dateigröße (Bytes);Messphase;"
                           "Dauer (Millisekunden);Verarbeitete Logeinträge;"
                           "Verarbeitete Daten pro Sekunde (Mebibyte);"
                           "Arbeitsspeicher am Phasenende (Mebibyte)\n";
                }
                const qint64 bytes = QFileInfo(m_dataset_path).size();
                const double dataset_mebibytes = static_cast<double>(bytes) / 1048576.0;
                const QString timestamp =
                    QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
                if (!report_has_content)
                {
                    report_stream << "# Large-File-Benchmark – Ergebnisse\n";
                }
                report_stream << "\n---\n\n## Messlauf " << timestamp << "\n\n"
                              << "- Quellstand: `" << m_source_state << "`\n"
                              << "- Nachrichtensortierung: "
                              << (m_include_message_sort ? "enthalten" : "ausgelassen (opt-in)")
                              << "\n"
                              << "- Datensatzgröße: " << bytes << " Bytes ("
                              << QString::number(dataset_mebibytes, 'f', 2) << " Mebibyte)\n"
                              << "- Ergebnis-CSV: `" << output_info.fileName() << "`\n\n"
                              << "| Messphase | Bedeutung | Dauer | Verarbeitete Logeinträge | "
                                 "Verarbeitete Daten pro Sekunde | Arbeitsspeicher am Phasenende |\n"
                              << "|---|---|---:|---:|---:|---:|\n";
                for (const Result& result: m_results)
                {
                    const double seconds = static_cast<double>(result.duration_ms) / 1000.0;
                    const double throughput = seconds > 0.0
                                                  ? static_cast<double>(bytes) / 1048576.0 / seconds
                                                  : 0.0;
                    const QString throughput_value =
                        result.includes_throughput ? QString::number(throughput, 'f', 2) : QString();
                    stream << timestamp << ';' << m_source_state << ';' << bytes << ';'
                           << result.phase << ';'
                           << QString::number(result.duration_ms, 'f', 3) << ';'
                           << result.entry_count << ';'
                           << throughput_value << ';'
                           << QString::number(
                                  static_cast<double>(result.working_set_bytes) / 1048576.0, 'f', 2)
                           << '\n';
                    const QString report_throughput =
                        result.includes_throughput
                            ? QStringLiteral("%1 Mebibyte pro Sekunde")
                                  .arg(QString::number(throughput, 'f', 2))
                            : QStringLiteral("–");
                    report_stream
                        << "| " << result.phase << " | " << result.meaning << " | "
                        << QString::number(seconds, 'f', 3) << " s | " << result.entry_count
                        << " | " << report_throughput << " | "
                        << QString::number(
                               static_cast<double>(result.working_set_bytes) / 1048576.0, 'f', 2)
                        << " Mebibyte |\n";
                }
                report_stream
                    << "\n## Bedeutung der Einheiten\n\n"
                    << "- **Mebibyte pro Sekunde:** Wie viele Daten pro Sekunde verarbeitet "
                       "wurden. Ein Mebibyte entspricht 1.048.576 Bytes. Ein höherer Wert ist "
                       "besser.\n"
                    << "- **Arbeitsspeicher am Phasenende:** Physischer Speicher, den der "
                       "Benchmark-Prozess beim Abschluss der jeweiligen Phase belegt hat. "
                       "Ein niedrigerer Wert ist besser.\n"
                    << "- **Erstimport:** Misst den vollständigen Weg ohne vorhandenen Cache.\n"
                    << "- **Cache-Treffer:** Misst das erneute Öffnen derselben Datei über den "
                       "bereits erzeugten persistenten Cache.\n"
                    << "- **Leeres Feld bei Daten pro Sekunde:** Für Filter, Suche und Seitenwechsel "
                       "ist eine Dateidatenrate fachlich nicht sinnvoll; dort zählt ausschließlich "
                       "die gemessene Dauer.\n";
            }
            return opened;
        }

        QString m_dataset_path;
        QString m_results_path;
        QString m_source_state;
        QString m_storage_root;
        bool m_include_message_sort{false};
        std::function<void(int)> m_completion;
        QString m_scenario{QStringLiteral("fresh_import")};
        LogParsingProfile m_profile;
        LogIngestController m_ingest;
        ViewRegistry m_views;
        FilterCoordinator m_filters;
        LogCacheCatalog m_cache_catalog;
        LogCacheReadService m_cache_reader;
        LogHistoryService m_history;
        HistoryWriteService m_history_writer;
        LogPageCoordinator m_pages;
        LogQueryController m_queries;
        LogImportCoordinator m_imports;
        QElapsedTimer m_timer;
        QUuid m_active_view;
        QVector<Result> m_results;
};
}  // namespace

/**
 * @brief Runs the explicitly requested performance benchmark.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Zero on successful measurement and result creation.
 */
auto main(int argc, char* argv[]) -> int
{
    QCoreApplication application(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({
        {{QStringLiteral("dataset")}, QStringLiteral("Unique dataset copy."),
         QStringLiteral("path")},
        {{QStringLiteral("results")}, QStringLiteral("CSV output path."), QStringLiteral("path")},
        {{QStringLiteral("git-commit")}, QStringLiteral("Commit identifier."),
         QStringLiteral("hash")},
        {{QStringLiteral("source-state")}, QStringLiteral("Commit and working-tree state."),
         QStringLiteral("description")},
        {{QStringLiteral("storage-root")}, QStringLiteral("Temporary benchmark storage root."),
         QStringLiteral("path")},
        {{QStringLiteral("include-message-sort")},
         QStringLiteral("Include the known-expensive message sorting scenario.")},
    });
    parser.process(application);

    int exit_code = 1;
    const QString dataset_path = parser.value(QStringLiteral("dataset"));
    const QString results_path = parser.value(QStringLiteral("results"));
    const QString storage_root = parser.value(QStringLiteral("storage-root"));
    const bool include_message_sort = parser.isSet(QStringLiteral("include-message-sort"));
    QString source_state = parser.value(QStringLiteral("source-state"));
    if (source_state.isEmpty())
    {
        source_state = parser.value(QStringLiteral("git-commit"));
    }
    if (!dataset_path.isEmpty() && !results_path.isEmpty() && !storage_root.isEmpty() &&
        QFileInfo::exists(dataset_path) && QDir().mkpath(storage_root))
    {
        BenchmarkRunner runner(dataset_path, results_path, source_state, storage_root,
                               include_message_sort,
                               [&application](int code) { application.exit(code); });
        runner.start();
        exit_code = application.exec();
    }
    return exit_code;
}

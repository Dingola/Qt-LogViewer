#include "Qt-LogViewer/Presenters/LogImportTabPresenterTest.h"

#include <QFile>
#include <QFileInfo>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Presenters/LogImportTabPresenter.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/App/LogImportWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogImportTabPresenterTest.cpp
 * @brief Implements tests for the temporary import-tab workflow.
 */

/**
 * @brief Creates isolated settings, import services, and a tab container.
 */
void LogImportTabPresenterTest::SetUp()
{
    m_temp_dir = new QTemporaryDir();
    ASSERT_TRUE(m_temp_dir->isValid());
    m_settings = new LogViewerSettings(m_temp_dir->filePath(QStringLiteral("settings.ini")),
                                       QSettings::IniFormat);
    ASSERT_TRUE(m_settings->set_log_parsing_profiles({m_profile}));
    m_controller = new LogViewerController(m_profile);
    m_tab_widget = new LogTabWidget();
    m_tab_widget->setup_default_behavior();
    m_presenter = new LogImportTabPresenter(
        *m_settings, *m_controller->get_preview_service(), *m_controller->get_import_coordinator(),
        *m_tab_widget,
        [this](const QUuid& view_id) {
            auto* widget = new LogViewWidget(m_tab_widget);
            widget->set_view_id(view_id);
            widget->set_model(m_controller->get_log_model(view_id));
            return widget;
        },
        nullptr, nullptr);
}

/**
 * @brief Destroys the isolated import workflow and temporary files.
 */
void LogImportTabPresenterTest::TearDown()
{
    delete m_presenter;
    m_presenter = nullptr;
    delete m_tab_widget;
    m_tab_widget = nullptr;
    delete m_controller;
    m_controller = nullptr;
    delete m_settings;
    m_settings = nullptr;
    delete m_temp_dir;
    m_temp_dir = nullptr;
}

/**
 * @brief Writes complete records to an isolated log file.
 * @param file_name File name relative to the temporary directory.
 * @param records Complete records to write.
 * @return Absolute path of the created file, or an empty path on failure.
 */
auto LogImportTabPresenterTest::create_log_file(const QString& file_name,
                                                const QStringList& records) const -> QString
{
    const QString file_path = m_temp_dir->filePath(file_name);
    QFile file(file_path);
    QString result;

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        for (const QString& record: records)
        {
            stream << record << '\n';
        }
        stream.flush();
        file.close();
        result = file_path;
    }

    return result;
}

/**
 * @test Verifies that cancelling removes the temporary import tab.
 */
TEST_F(LogImportTabPresenterTest, CancelsImportTab)
{
    const QString file_path =
        create_log_file(QStringLiteral("cancel.log"),
                        {QStringLiteral("2024-01-01 12:00:00 INFO CancelMessage CancelApp")});
    ASSERT_FALSE(file_path.isEmpty());

    m_presenter->show_import_tab(LogFileInfo(file_path));
    auto* import_widget = qobject_cast<LogImportWidget*>(m_tab_widget->currentWidget());
    ASSERT_NE(import_widget, nullptr);

    emit import_widget->cancel_requested();

    EXPECT_EQ(m_tab_widget->count(), 0);
}

/**
 * @test Verifies that a confirmed import replaces its temporary tab with a new log view.
 */
TEST_F(LogImportTabPresenterTest, ReplacesImportTabWithNewView)
{
    const QString file_path =
        create_log_file(QStringLiteral("new-view.log"),
                        {QStringLiteral("2024-01-01 12:00:00 INFO NewMessage NewApp")});
    ASSERT_FALSE(file_path.isEmpty());

    m_presenter->show_import_tab(LogFileInfo(file_path));
    auto* import_widget = qobject_cast<LogImportWidget*>(m_tab_widget->currentWidget());
    ASSERT_NE(import_widget, nullptr);
    QPushButton* import_button =
        import_widget->findChild<QPushButton*>(QStringLiteral("pushButtonImport"));
    ASSERT_NE(import_button, nullptr);
    ASSERT_TRUE(import_button->isEnabled());

    import_button->click();

    ASSERT_EQ(m_tab_widget->count(), 1);
    LogViewWidget* log_view_widget = m_tab_widget->current_log_view();
    ASSERT_NE(log_view_widget, nullptr);
    EXPECT_FALSE(log_view_widget->get_view_id().isNull());
    EXPECT_EQ(m_controller->get_all_view_ids().size(), 1);
}

/**
 * @test Verifies that a confirmed file is submitted to the selected existing view.
 */
TEST_F(LogImportTabPresenterTest, ImportsIntoExistingView)
{
    const QString first_file =
        create_log_file(QStringLiteral("existing.log"),
                        {QStringLiteral("2024-01-01 12:00:00 INFO ExistingMessage ExistingApp")});
    const QString added_file =
        create_log_file(QStringLiteral("added.log"),
                        {QStringLiteral("2024-01-01 12:01:00 INFO AddedMessage AddedApp")});
    ASSERT_FALSE(first_file.isEmpty());
    ASSERT_FALSE(added_file.isEmpty());
    const QUuid target_view = m_controller->load_log_file(first_file, m_profile);
    ASSERT_FALSE(target_view.isNull());
    auto* target_widget = new LogViewWidget(m_tab_widget);
    target_widget->set_view_id(target_view);
    target_widget->set_model(m_controller->get_log_model(target_view));
    m_tab_widget->add_log_view_tab(target_widget, QStringLiteral("Existing"), false);

    m_presenter->show_import_tab(LogFileInfo(added_file), target_view);
    auto* import_widget = qobject_cast<LogImportWidget*>(m_tab_widget->currentWidget());
    ASSERT_NE(import_widget, nullptr);
    QPushButton* import_button =
        import_widget->findChild<QPushButton*>(QStringLiteral("pushButtonImport"));
    ASSERT_NE(import_button, nullptr);
    ASSERT_TRUE(import_button->isEnabled());

    import_button->click();

    ASSERT_EQ(m_tab_widget->count(), 1);
    ASSERT_NE(m_tab_widget->current_log_view(), nullptr);
    EXPECT_EQ(m_tab_widget->current_log_view()->get_view_id(), target_view);
    EXPECT_TRUE(m_controller->get_view_file_paths(target_view)
                    .contains(QFileInfo(added_file).absoluteFilePath()));
}

/**
 * @test Verifies that a closed target leaves the import tab available and reports the problem.
 */
TEST_F(LogImportTabPresenterTest, KeepsImportTabWhenTargetViewWasClosed)
{
    const QString first_file =
        create_log_file(QStringLiteral("closed-target.log"),
                        {QStringLiteral("2024-01-01 12:00:00 INFO ExistingMessage ExistingApp")});
    const QString added_file =
        create_log_file(QStringLiteral("pending.log"),
                        {QStringLiteral("2024-01-01 12:01:00 INFO PendingMessage PendingApp")});
    ASSERT_FALSE(first_file.isEmpty());
    ASSERT_FALSE(added_file.isEmpty());
    const QUuid target_view = m_controller->load_log_file(first_file, m_profile);
    ASSERT_FALSE(target_view.isNull());
    auto* target_widget = new LogViewWidget(m_tab_widget);
    target_widget->set_view_id(target_view);
    target_widget->set_model(m_controller->get_log_model(target_view));
    m_tab_widget->add_log_view_tab(target_widget, QStringLiteral("Closing"), false);
    m_presenter->show_import_tab(LogFileInfo(added_file), target_view);
    auto* import_widget = qobject_cast<LogImportWidget*>(m_tab_widget->currentWidget());
    ASSERT_NE(import_widget, nullptr);
    QPushButton* import_button =
        import_widget->findChild<QPushButton*>(QStringLiteral("pushButtonImport"));
    ASSERT_NE(import_button, nullptr);
    ASSERT_TRUE(import_button->isEnabled());
    ASSERT_TRUE(m_tab_widget->remove_view_tab_by_id(target_view));
    ASSERT_TRUE(m_controller->remove_view(target_view));
    QSignalSpy status_spy(m_presenter, &LogImportTabPresenter::status_message_requested);

    import_button->click();

    EXPECT_EQ(m_tab_widget->count(), 1);
    EXPECT_EQ(m_tab_widget->currentWidget(), import_widget);
    ASSERT_EQ(status_spy.count(), 1);
    EXPECT_TRUE(
        status_spy.first().at(0).toString().contains(QStringLiteral("no longer available")));
}

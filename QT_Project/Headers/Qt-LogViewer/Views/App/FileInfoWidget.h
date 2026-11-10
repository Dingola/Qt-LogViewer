#pragma once

#include <QDateTime>
#include <QVector>
#include <QWidget>

class QComboBox;
class QLabel;
class QLineEdit;
class QToolButton;

/**
 * @file FileInfoWidget.h
 * @brief Declares the presentation-only file inspector widget.
 */

/**
 * @struct FileInfoSnapshot
 * @brief Contains display-ready metadata for one source or an aggregate view.
 */
struct FileInfoSnapshot {
        /** @brief Selector label and displayed file name. */
        QString display_name;
        /** @brief Absolute source path or aggregate description. */
        QString path;
        /** @brief Human-readable source size. */
        QString size;
        /** @brief Localized last-modification time. */
        QString modified;
        /** @brief Localized recognized-record count. */
        QString record_count;
        /** @brief Earliest and latest recognized timestamps. */
        QString timestamp_range;
        /** @brief Detected application values. */
        QString applications;
        /** @brief Detected log-level values. */
        QString log_levels;
        /** @brief Selected parsing-profile name. */
        QString parsing_profile;
        /** @brief Captured parse-success information. */
        QString parse_success_rate;
        /** @brief Safely detected encoding information. */
        QString encoding;
        /** @brief Safely detected newline convention. */
        QString line_endings;
        /** @brief Current cache lifecycle description. */
        QString cache_state;
        /** @brief Human-readable cache storage size. */
        QString cache_size;
        /** @brief Localized cache access time. */
        QString cache_last_access;
        /** @brief Current view-level live-tail state. */
        QString live_tail_state;
        /** @brief Last accepted live append time. */
        QString last_live_entry;
        /** @brief Whether source-specific file-system actions are available. */
        bool local_path_available{false};
};

/**
 * @class FileInfoWidget
 * @brief Displays aggregate and per-source metadata supplied by a presenter.
 */
class FileInfoWidget final: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs an initially empty file inspector.
         * @param parent Optional parent widget.
         */
        explicit FileInfoWidget(QWidget* parent = nullptr);

        /**
         * @brief Replaces the available aggregate and per-file snapshots.
         * @param snapshots Aggregate snapshot followed by source snapshots.
         * @param selected_index Index to select after replacement.
         */
        auto set_snapshots(const QVector<FileInfoSnapshot>& snapshots,
                           qsizetype selected_index = 0) -> void;

        /** @brief Shows the no-active-file state. */
        auto clear() -> void;

        /** @brief Copies every displayed property of the current snapshot. */
        auto copy_current_summary() -> void;

    signals:
        /**
         * @brief Requests display of another snapshot.
         * @param index Index in the current snapshot collection.
         */
        void snapshot_selected(qsizetype index);

    private:
        /**
         * @brief Displays one retained snapshot.
         * @param index Index in the retained snapshot collection.
         */
        auto show_snapshot(qsizetype index) -> void;

        /**
         * @brief Adds one labeled, selectable value row.
         * @param label User-facing row label.
         * @return Created read-only value field.
         */
        [[nodiscard]] auto add_value_row(const QString& label) -> QLineEdit*;

        /** @brief Updates contextual action states and icons. */
        auto refresh_actions() -> void;

        /**
         * @brief Copies text and shows temporary confirmation on an action.
         * @param text Text to place on the clipboard.
         * @param button Activated action button.
         */
        auto copy_text(const QString& text, QToolButton* button) -> void;

        /**
         * @brief Builds a readable multiline representation of the current snapshot.
         * @return Complete displayed summary, or an empty value without a snapshot.
         */
        [[nodiscard]] auto build_summary_text() const -> QString;

        QComboBox* m_source_selector{nullptr};       ///< Aggregate/per-source selector.
        QLabel* m_empty_label{nullptr};              ///< Guidance shown without active sources.
        QLabel* m_title_label{nullptr};              ///< Prominent source heading.
        QLabel* m_subtitle_label{nullptr};           ///< Compact source summary.
        QWidget* m_content_widget{nullptr};          ///< Scrollable structured content.
        QToolButton* m_copy_all_button{nullptr};     ///< Complete-summary copy action.
        QToolButton* m_copy_path_button{nullptr};    ///< Source-path copy action.
        QToolButton* m_open_folder_button{nullptr};  ///< Explorer action.
        QVector<QLineEdit*> m_value_labels;          ///< Ordered read-only metadata fields.
        QVector<FileInfoSnapshot> m_snapshots;       ///< Currently selectable snapshots.
        qsizetype m_current_index{-1};               ///< Currently displayed snapshot index.
};

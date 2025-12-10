#include "Qt-LogViewer/Views/App/LogViewWidget.h"

#include <QAbstractProxyModel>
#include <QItemSelectionModel>
#include <QLayout>
#include <QMenu>
#include <QToolButton>

#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Services/UiUtils.h"
#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"
#include "Qt-LogViewer/Views/App/FilesInViewWidgetAction.h"
#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "ui_LogViewWidget.h"

/**
 * @brief Constructs a LogViewWidget object.
 *
 * Initializes UI components and connects internal signals to be forwarded outward.
 *
 * @param parent The parent widget, or nullptr.
 */
LogViewWidget::LogViewWidget(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LogViewWidget),
      m_view_id(QUuid()),
      m_files_menu(nullptr),
      m_view_file_paths()
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->verticalLayout->setContentsMargins(9, 0, 9, 0);

    // Forward filter signals
    connect(ui->logFilterWidget, &LogFilterWidget::app_filter_changed, this,
            &LogViewWidget::app_filter_changed);
    connect(ui->logFilterWidget, &LogFilterWidget::log_level_filter_changed, this,
            &LogViewWidget::log_level_filter_changed);

    // Forward current row changes in the table view
    QItemSelectionModel* selection_model = ui->logTableView->selectionModel();

    if (selection_model != nullptr)
    {
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this,
                &LogViewWidget::current_row_changed);
    }

    setup_files_menu();
}

/**
 * @brief Destroys the LogViewWidget object.
 *
 * Cleans up allocated UI resources.
 */
LogViewWidget::~LogViewWidget()
{
    delete ui;
}

/**
 * @brief Sets the unique view id associated with this widget.
 *
 * Emits `view_id_changed` if the id changes.
 *
 * @param view_id The new view id.
 */
auto LogViewWidget::set_view_id(const QUuid& view_id) -> void
{
    bool changed = (m_view_id != view_id);
    m_view_id = view_id;

    if (changed)
    {
        emit view_id_changed(m_view_id);
    }
}

/**
 * @brief Gets the unique view id associated with this widget.
 *
 * @return The current view id (may be null QUuid if not set).
 */
auto LogViewWidget::get_view_id() const -> QUuid
{
    QUuid value = m_view_id;
    return value;
}

/**
 * @brief Sets the model on the internal log table view.
 *
 * Reconnects the selection model's currentRowChanged signal.
 *
 * @param model The model to assign.
 */
auto LogViewWidget::set_model(QAbstractItemModel* model) -> void
{
    ui->logTableView->setModel(model);
    QItemSelectionModel* selection_model = ui->logTableView->selectionModel();

    if (selection_model != nullptr)
    {
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this,
                &LogViewWidget::current_row_changed);
    }

    auto* proxy_model = qobject_cast<QAbstractProxyModel*>(ui->logTableView->model());
    if (proxy_model != nullptr)
    {
        auto* sort_proxy = qobject_cast<LogSortFilterProxyModel*>(proxy_model->sourceModel());
        if (sort_proxy != nullptr)
        {
            connect(sort_proxy, &LogSortFilterProxyModel::file_visibility_changed, this,
                    [this](const QString&) { refresh_files_menu_states(); });
            connect(sort_proxy, &LogSortFilterProxyModel::show_only_changed, this,
                    [this](const QString&) { refresh_files_menu_states(); });
        }
    }
}

/**
 * @brief Convenience method: resizes columns in the internal table view.
 */
auto LogViewWidget::auto_resize_columns() -> void
{
    ui->logTableView->auto_resize_columns();
}

/**
 * @brief Sets the application names for the filter widget.
 * @param app_names Set of application names.
 */
auto LogViewWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    ui->logFilterWidget->set_app_names(app_names);
}

/**
 * @brief Selects a specific application name in the filter widget if present.
 * @param app_name Application name to select.
 */
auto LogViewWidget::set_current_app_name_filter(const QString& app_name) -> void
{
    ui->logFilterWidget->set_current_app_name_filter(app_name);
}

/**
 * @brief Sets available log level names for creating level items.
 * @param log_levels List of log level names.
 */
auto LogViewWidget::set_available_log_levels(const QVector<QString>& log_levels) -> void
{
    ui->logFilterWidget->set_available_log_levels(log_levels);
}

/**
 * @brief Sets which log levels are currently checked.
 * @param levels Set of log level names to check.
 */
auto LogViewWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    ui->logFilterWidget->set_log_levels(levels);
}

/**
 * @brief Updates per-log-level counts displayed in the filter items.
 * @param level_counts Map of level name to count.
 */
auto LogViewWidget::set_log_level_counts(const QMap<QString, int>& level_counts) -> void
{
    ui->logFilterWidget->set_log_level_counts(level_counts);
}

/**
 * @brief Shows or hides the filter widget area.
 *
 * @param visible True to show, false to hide.
 */
auto LogViewWidget::set_filter_widget_visible(bool visible) -> void
{
    ui->logFilterWidget->setVisible(visible);
}

/**
 * @brief Resets all filter related UI elements (apps + levels).
 *
 * Clears application names, log levels, counts.
 */
auto LogViewWidget::clear_filters() -> void
{
    ui->logFilterWidget->set_app_names({});
    ui->logFilterWidget->set_available_log_levels({});
    ui->logFilterWidget->set_log_levels({});
    ui->logFilterWidget->set_log_level_counts({});
}

/**
 * @brief Returns the currently selected application name.
 * @return Current application name.
 */
auto LogViewWidget::get_current_app_name() const -> QString
{
    QString value = ui->logFilterWidget->get_current_app_name();
    return value;
}

/**
 * @brief Returns the set of currently selected log levels.
 * @return Set of selected log level names.
 */
auto LogViewWidget::get_current_log_levels() const -> QSet<QString>
{
    QSet<QString> levels = ui->logFilterWidget->get_current_log_levels();
    return levels;
}

/**
 * @brief Returns a pointer to the internal LogFilterWidget.
 * @return Pointer to LogFilterWidget.
 */
auto LogViewWidget::get_filter_widget() const -> LogFilterWidget*
{
    LogFilterWidget* widget = ui->logFilterWidget;
    return widget;
}

/**
 * @brief Returns a pointer to the internal LogTableView.
 * @return Pointer to LogTableView.
 */
auto LogViewWidget::get_table_view() const -> LogTableView*
{
    LogTableView* widget = ui->logTableView;
    return widget;
}

/**
 * @brief Updates the list of file paths loaded in the current view.
 *
 * Rebuilds the "Files in View" menu to reflect current per-file actions.
 *
 * @param file_paths Vector of absolute file paths currently loaded in this view.
 */
auto LogViewWidget::set_view_file_paths(const QVector<QString>& file_paths) -> void
{
    m_view_file_paths = file_paths;
    rebuild_files_menu();
}

/**
 * @brief Creates the "Files in View" tool button and attaches the dropdown menu.
 *
 * The button is inserted into the top-level layout and configured for instant popup.
 */
auto LogViewWidget::setup_files_menu() -> void
{
    if (ui->filesInViewToolButton != nullptr)
    {
        ui->filesInViewToolButton->setText(tr("Files..."));
        ui->filesInViewToolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        ui->filesInViewToolButton->setPopupMode(QToolButton::InstantPopup);
        ui->filesInViewToolButton->setAutoRaise(true);
    }

    if (m_files_menu == nullptr)
    {
        m_files_menu = new QMenu(ui->filesInViewToolButton);
        m_files_menu->setObjectName(QStringLiteral("filesInViewMenu"));
        m_files_menu->setProperty("variant", "inline-menu");
        ui->filesInViewToolButton->setMenu(m_files_menu);
    }

    rebuild_files_menu();
}

/**
 * @brief Rebuilds the "Files in View" dropdown menu from `m_view_file_paths`.
 *
 * Adds a "Show All Files" action and one per-file row with inline actions
 * (Show Only, Hide, Remove).
 */
auto LogViewWidget::rebuild_files_menu() -> void
{
    const bool menu_available = (m_files_menu != nullptr);

    if (menu_available)
    {
        m_files_menu->clear();

        auto* show_all_action = new QAction(tr("Show All Files"), m_files_menu);
        m_files_menu->addAction(show_all_action);
        connect(show_all_action, &QAction::triggered, this,
                [this]() { emit show_only_file_requested(QString()); });

        const bool has_files = !m_view_file_paths.isEmpty();

        if (!has_files)
        {
            auto* none = new QAction(tr("No files in view"), m_files_menu);
            none->setEnabled(false);
            m_files_menu->addAction(none);
            m_files_menu->close();
        }
        else
        {
            // Determine current file filter state from proxy
            QString show_only_path;
            QSet<QString> hidden_paths;

            auto* proxy_model = qobject_cast<QAbstractProxyModel*>(ui->logTableView->model());
            if (proxy_model != nullptr)
            {
                auto* sort_proxy =
                    qobject_cast<LogSortFilterProxyModel*>(proxy_model->sourceModel());
                if (sort_proxy != nullptr)
                {
                    show_only_path = sort_proxy->get_show_only_file_path();
                    hidden_paths = sort_proxy->get_hidden_file_paths();
                }
            }

            m_files_menu->addSeparator();

            for (const QString& path: m_view_file_paths)
            {
                auto* widget_action = new FilesInViewWidgetAction(m_files_menu);
                widget_action->set_file_path(path);

                // Effective hidden: explicitly hidden OR hidden by active show-only on another file
                const bool hidden_effective = hidden_paths.contains(path) ||
                                              (!show_only_path.isEmpty() && path != show_only_path);
                widget_action->set_hidden_effective(hidden_effective);

                connect(
                    widget_action, &FilesInViewWidgetAction::show_only_requested, this,
                    [this](const QString& file_path) { emit show_only_file_requested(file_path); });

                connect(widget_action, &FilesInViewWidgetAction::toggle_visibility_requested, this,
                        [this](const QString& file_path) {
                            emit toggle_visibility_requested(file_path);
                        });

                connect(
                    widget_action, &FilesInViewWidgetAction::remove_requested, this,
                    [this](const QString& file_path) { emit remove_file_requested(file_path); });

                m_files_menu->addAction(widget_action);
            }

            // Ensure the freshly built menu reflects any runtime changes from proxies afterwards
            refresh_files_menu_states();
        }
    }
}

/**
 * @brief Refreshes the "Files in View" menu rows' visibility toggle presentation
 *        based on the current proxy state. Private by design.
 */
auto LogViewWidget::refresh_files_menu_states() -> void
{
    // Determine current file filter state from proxy
    QString show_only_path;
    QSet<QString> hidden_paths;

    auto* proxy_lvl1 = qobject_cast<QAbstractProxyModel*>(ui->logTableView->model());
    if (proxy_lvl1 != nullptr)
    {
        auto* sort_proxy = qobject_cast<LogSortFilterProxyModel*>(proxy_lvl1->sourceModel());
        if (sort_proxy != nullptr)
        {
            show_only_path = sort_proxy->get_show_only_file_path();
            hidden_paths = sort_proxy->get_hidden_file_paths();
        }
    }

    if (m_files_menu != nullptr)
    {
        const QList<QAction*> actions = m_files_menu->actions();
        for (QAction* act: actions)
        {
            auto* widget_action = qobject_cast<FilesInViewWidgetAction*>(act);
            if (widget_action != nullptr)
            {
                const QString path = widget_action->get_file_path();
                const bool hidden_effective = (hidden_paths.contains(path)) ||
                                              (!show_only_path.isEmpty() && path != show_only_path);

                widget_action->set_hidden_effective(hidden_effective);
            }
        }
    }
}

/**
 * @brief Handles language change and other UI change events.
 * @param event The change event.
 */
auto LogViewWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        if (ui->filesInViewToolButton != nullptr)
        {
            ui->filesInViewToolButton->setText(tr("Files..."));
        }
    }

    QWidget::changeEvent(event);
}

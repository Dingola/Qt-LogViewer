/**
 * @file StartPageWidget.cpp
 * @brief Implementation of the StartPageWidget for the application start page.
 */

#include "Qt-LogViewer/Views/App/StartPageWidget.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QDateTime>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QMenu>
#include <QModelIndex>
#include <QPushButton>
#include <QSize>
#include <QStackedWidget>
#include <QVariant>

#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Views/Shared/EmptyStateWidget.h"
#include "Qt-LogViewer/Views/Shared/TwoLineItemDelegate.h"
#include "ui_StartPageWidget.h"

namespace
{
constexpr int k_brand_icon_size_px = 40;
constexpr auto k_brand_icon_path = ":/Resources/Icons/App/AppIcon.svg";

/**
 * @brief Creates the shared two-line presentation used by recent-item lists.
 * @param parent Owner of the delegate.
 * @param include_status Whether unavailable-item status text should be rendered.
 * @return Configured delegate using the recent-item semantic roles.
 */
[[nodiscard]] auto make_recent_item_delegate(QObject* parent,
                                             bool include_status) -> TwoLineItemDelegate*
{
    TwoLineItemDelegate::Roles roles;
    roles.title = to_role_id(RecentItemRole::Title);
    roles.secondary = to_role_id(RecentItemRole::SecondaryText);
    roles.metadata = to_role_id(RecentItemRole::LastUsed);
    roles.icon = to_role_id(RecentItemRole::Icon);
    roles.status = include_status ? to_role_id(RecentItemRole::StatusText) : -1;

    auto* result = new TwoLineItemDelegate(roles, parent);
    result->set_metadata_formatter([](const QVariant& value) {
        QString text;
        const QDateTime date_time = value.toDateTime();
        if (date_time.isValid())
        {
            text = date_time.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        }
        return text;
    });
    return result;
}
}  // namespace

/**
 * @brief Construct the StartPageWidget UI wrapper.
 * @param parent Parent widget.
 */
StartPageWidget::StartPageWidget(QWidget* parent): QWidget(parent), ui(new Ui::StartPageWidget)
{
    ui->setupUi(this);
    setup_branding();
    setup_table_views();
    setup_empty_states();
    setup_connections();
    update_buttons_state();
}

/**
 * @brief Configures the Koala brand mark shown beside the start-page welcome heading.
 */
auto StartPageWidget::setup_branding() -> void
{
    const QIcon brand_icon(QString::fromLatin1(k_brand_icon_path));
    ui->brandIconLabel->setPixmap(
        brand_icon.pixmap(QSize(k_brand_icon_size_px, k_brand_icon_size_px)));
    ui->brandIconLabel->setAccessibleName(tr("Qt-LogViewer Koala logo"));
}

/**
 * @brief Destroy the StartPageWidget.
 */
StartPageWidget::~StartPageWidget()
{
    delete ui;
}

/**
 * @brief Assign the model for the recent files table view.
 * @param model QAbstractItemModel exposing file presentation, status, and enabled roles.
 */
auto StartPageWidget::set_recent_files_model(QAbstractItemModel* model) -> void
{
    ui->recentFilesView->setModel(model);

    if (auto* lv = qobject_cast<QListView*>(ui->recentFilesView); lv != nullptr)
    {
        lv->setItemDelegate(make_recent_item_delegate(lv, true));
        lv->setUniformItemSizes(true);
        lv->setSpacing(4);
    }

    QAbstractItemModel* files_model = ui->recentFilesView->model();
    if (files_model != nullptr && files_model->rowCount() > 0)
    {
        QModelIndex first_enabled;
        for (int row = 0; row < files_model->rowCount() && !first_enabled.isValid(); ++row)
        {
            const QModelIndex candidate = files_model->index(row, 0);
            if (candidate.flags().testFlag(Qt::ItemIsEnabled))
            {
                first_enabled = candidate;
            }
        }
        if (first_enabled.isValid())
        {
            ui->recentFilesView->selectionModel()->setCurrentIndex(
                first_enabled, QItemSelectionModel::ClearAndSelect);
        }
    }

    if (ui->recentFilesView->selectionModel() != nullptr)
    {
        connect(ui->recentFilesView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                [this]() { update_buttons_state(); });
        connect(files_model, &QAbstractItemModel::modelReset, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
        connect(files_model, &QAbstractItemModel::rowsInserted, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
        connect(files_model, &QAbstractItemModel::rowsRemoved, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
    }

    update_buttons_state();
    update_empty_states();
}

/**
 * @brief Assign the model for the recent sessions table view.
 * @param model QAbstractItemModel exposing roles: name, last_opened, id.
 */
auto StartPageWidget::set_recent_sessions_model(QAbstractItemModel* model) -> void
{
    ui->recentSessionsView->setModel(model);

    if (auto* lv = qobject_cast<QListView*>(ui->recentSessionsView); lv != nullptr)
    {
        lv->setItemDelegate(make_recent_item_delegate(lv, false));
        lv->setUniformItemSizes(true);
        lv->setSpacing(4);
    }

    QAbstractItemModel* session_model = ui->recentSessionsView->model();
    if (session_model != nullptr && session_model->rowCount() > 0)
    {
        const QModelIndex first = session_model->index(0, 0);
        ui->recentSessionsView->selectionModel()->setCurrentIndex(
            first, QItemSelectionModel::ClearAndSelect);
    }

    if (ui->recentSessionsView->selectionModel() != nullptr)
    {
        connect(ui->recentSessionsView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, [this]() { update_buttons_state(); });
        connect(session_model, &QAbstractItemModel::modelReset, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
        connect(session_model, &QAbstractItemModel::rowsInserted, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
        connect(session_model, &QAbstractItemModel::rowsRemoved, this, [this]() {
            update_buttons_state();
            update_empty_states();
        });
    }

    update_buttons_state();
    update_empty_states();
}

/**
 * @brief Configure sensible defaults for the table views.
 *
 * Applies:
 * - selectionBehavior: SelectRows
 * - selectionMode: SingleSelection
 * - editTriggers: NoEditTriggers
 * - alternatingRowColors: true
 */
auto StartPageWidget::setup_table_views() -> void
{
    auto* files = ui->recentFilesView;
    auto* sessions = ui->recentSessionsView;

    if (files != nullptr)
    {
        files->setSelectionBehavior(QAbstractItemView::SelectRows);
        files->setSelectionMode(QAbstractItemView::SingleSelection);
        files->setEditTriggers(QAbstractItemView::NoEditTriggers);
        files->setAlternatingRowColors(true);
        files->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(files, &QWidget::customContextMenuRequested, this,
                &StartPageWidget::show_recent_file_context_menu);

        if (auto* lv = qobject_cast<QListView*>(files); lv != nullptr)
        {
            lv->setUniformItemSizes(true);
            lv->setSpacing(4);
        }
    }

    if (sessions != nullptr)
    {
        sessions->setSelectionBehavior(QAbstractItemView::SelectRows);
        sessions->setSelectionMode(QAbstractItemView::SingleSelection);
        sessions->setEditTriggers(QAbstractItemView::NoEditTriggers);
        sessions->setAlternatingRowColors(true);

        if (auto* lv = qobject_cast<QListView*>(sessions); lv != nullptr)
        {
            lv->setUniformItemSizes(true);
            lv->setSpacing(4);
        }
    }
}

/**
 * @brief Presents metadata-only cleanup actions for the recent file under the cursor.
 * @param position Viewport-local context-menu position.
 */
auto StartPageWidget::show_recent_file_context_menu(const QPoint& position) -> void
{
    QListView* view = ui->recentFilesView;
    QAbstractItemModel* model = view != nullptr ? view->model() : nullptr;
    const QModelIndex context_index = view != nullptr ? view->indexAt(position) : QModelIndex();
    bool has_unavailable_items = false;

    if (model != nullptr)
    {
        for (int row = 0; row < model->rowCount() && !has_unavailable_items; ++row)
        {
            has_unavailable_items = !model->index(row, 0).flags().testFlag(Qt::ItemIsEnabled);
        }
    }

    QMenu menu(this);
    QAction* remove_action = menu.addAction(tr("Remove from Recent Files"));
    remove_action->setEnabled(context_index.isValid());
    QAction* cleanup_action = menu.addAction(tr("Remove Unavailable Files..."));
    cleanup_action->setEnabled(has_unavailable_items);

    QAction* selected_action = nullptr;
    if (view != nullptr && (context_index.isValid() || has_unavailable_items))
    {
        selected_action = menu.exec(view->viewport()->mapToGlobal(position));
    }

    if (selected_action == remove_action && model != nullptr)
    {
        const QString file_path =
            model->data(context_index, to_role_id(RecentItemRole::Identity)).toString();
        if (!file_path.isEmpty())
        {
            emit remove_recent_file_requested(file_path);
        }
    }
    else if (selected_action == cleanup_action)
    {
        emit remove_unavailable_recent_files_requested();
    }
}

/** @brief Creates compact empty states that reuse the start page's existing
 * actions. */
auto StartPageWidget::setup_empty_states() -> void
{
    m_recent_files_stack = new QStackedWidget(ui->recentFilesView->parentWidget());
    m_recent_files_stack->setObjectName(QStringLiteral("recentFilesStack"));
    ui->verticalLayout->replaceWidget(ui->recentFilesView, m_recent_files_stack);
    ui->recentFilesView->setParent(m_recent_files_stack);
    m_recent_files_stack->addWidget(ui->recentFilesView);

    m_recent_files_empty_state = new EmptyStateWidget(m_recent_files_stack);
    m_recent_files_empty_state->setObjectName(QStringLiteral("recentFilesEmptyState"));
    m_recent_files_empty_state->set_density(EmptyStateWidget::Density::Compact);
    m_recent_files_empty_state->set_alignment(Qt::AlignCenter);
    m_recent_files_empty_state->set_title(tr("No recent log files"));
    m_recent_files_empty_state->set_description(
        tr("Files you open will appear here for quick access."));
    m_recent_files_empty_state->set_drop_text(tr("Drop log files anywhere in this window"));
    m_recent_files_empty_state->set_drop_space_reserved(true);
    m_recent_files_empty_state->set_drop_state(EmptyStateWidget::DropState::Available);
    auto* open_file_action = new QAction(tr("Open Log File..."), this);
    connect(open_file_action, &QAction::triggered, this, &StartPageWidget::open_log_file_requested);
    m_recent_files_empty_state->add_action(open_file_action, EmptyStateWidget::ActionRole::Primary);
    m_recent_files_stack->addWidget(m_recent_files_empty_state);

    m_recent_sessions_stack = new QStackedWidget(ui->recentSessionsView->parentWidget());
    m_recent_sessions_stack->setObjectName(QStringLiteral("recentSessionsStack"));
    ui->verticalLayout_2->replaceWidget(ui->recentSessionsView, m_recent_sessions_stack);
    ui->recentSessionsView->setParent(m_recent_sessions_stack);
    m_recent_sessions_stack->addWidget(ui->recentSessionsView);

    m_recent_sessions_empty_state = new EmptyStateWidget(m_recent_sessions_stack);
    m_recent_sessions_empty_state->setObjectName(QStringLiteral("recentSessionsEmptyState"));
    m_recent_sessions_empty_state->set_density(EmptyStateWidget::Density::Compact);
    m_recent_sessions_empty_state->set_alignment(Qt::AlignCenter);
    m_recent_sessions_empty_state->set_title(tr("No saved sessions"));
    m_recent_sessions_empty_state->set_description(
        tr("Saved sessions let you return to the same files and views later."));
    m_recent_sessions_empty_state->set_drop_text(tr("Drop log files anywhere in this window"));
    m_recent_sessions_empty_state->set_drop_space_reserved(true);
    auto* open_session_action = new QAction(tr("Open Session..."), this);
    connect(open_session_action, &QAction::triggered, this,
            &StartPageWidget::open_session_requested);
    m_recent_sessions_empty_state->add_action(open_session_action,
                                              EmptyStateWidget::ActionRole::Primary);
    m_recent_sessions_stack->addWidget(m_recent_sessions_empty_state);

    update_empty_states();
}

/**
 * @brief Wire button clicks to signals.
 */
auto StartPageWidget::setup_connections() -> void
{
    connect(ui->openFileButton, &QPushButton::clicked, this,
            &StartPageWidget::open_log_file_requested);

    connect(ui->openSelectedFileButton, &QPushButton::clicked, this, [this]() {
        const QString path = get_selected_recent_file_path();
        if (!path.isEmpty())
        {
            emit open_recent_file_requested(path);
        }
    });

    connect(ui->clearRecentFilesButton, &QPushButton::clicked, this,
            &StartPageWidget::clear_recent_files_requested);

    connect(ui->openSessionButton, &QPushButton::clicked, this,
            &StartPageWidget::open_session_requested);

    connect(ui->openSelectedSessionButton, &QPushButton::clicked, this, [this]() {
        const QString id = get_selected_recent_session_id();
        if (!id.isEmpty())
        {
            emit open_recent_session_requested(id);
        }
    });

    connect(ui->reopenSessionButton, &QPushButton::clicked, this,
            &StartPageWidget::reopen_last_session_requested);

    connect(ui->deleteSessionButton, &QPushButton::clicked, this, [this]() {
        const QString id = get_selected_recent_session_id();
        if (!id.isEmpty())
        {
            emit delete_session_requested(id);
        }
    });

    connect(ui->recentFilesView, &QListView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid() && index.flags().testFlag(Qt::ItemIsEnabled) &&
            ui->recentFilesView->model() != nullptr)
        {
            const QVariant file_path_data =
                ui->recentFilesView->model()->data(index, to_role_id(RecentItemRole::Identity));
            const QString file_path = file_path_data.toString();
            if (!file_path.isEmpty())
            {
                emit open_recent_file_requested(file_path);
            }
        }
    });

    connect(ui->recentSessionsView, &QListView::doubleClicked, this,
            [this](const QModelIndex& index) {
                if (index.isValid() && ui->recentSessionsView->model() != nullptr)
                {
                    const QVariant session_id_data = ui->recentSessionsView->model()->data(
                        index, to_role_id(RecentItemRole::Identity));
                    const QString session_id = session_id_data.toString();
                    if (!session_id.isEmpty())
                    {
                        emit open_recent_session_requested(session_id);
                    }
                }
            });
}

/**
 * @brief Helper to read selected file_path from recentFilesView.
 * @return Selected file path or empty.
 */
auto StartPageWidget::get_selected_recent_file_path() const -> QString
{
    QString path;

    const auto* view = ui->recentFilesView;
    if (view != nullptr)
    {
        const QItemSelectionModel* files_selection_model = view->selectionModel();
        if (files_selection_model != nullptr)
        {
            const QModelIndex index = files_selection_model->currentIndex();
            if (index.isValid() && index.flags().testFlag(Qt::ItemIsEnabled) &&
                view->model() != nullptr)
            {
                const QVariant v = view->model()->data(index, to_role_id(RecentItemRole::Identity));
                path = v.toString();
            }
        }
    }

    return path;
}

/**
 * @brief Helper to read selected session id from recentSessionsView.
 * @return Selected session id or empty.
 */
auto StartPageWidget::get_selected_recent_session_id() const -> QString
{
    QString id;

    const auto* view = ui->recentSessionsView;
    if (view != nullptr)
    {
        const QItemSelectionModel* sel = view->selectionModel();
        if (sel != nullptr)
        {
            const QModelIndex index = sel->currentIndex();
            if (index.isValid() && view->model() != nullptr)
            {
                const QVariant v = view->model()->data(index, to_role_id(RecentItemRole::Identity));
                id = v.toString();
            }
        }
    }

    return id;
}

/**
 * @brief Update enabled state of action buttons based on models and selection.
 */
auto StartPageWidget::update_buttons_state() -> void
{
    bool files_has_items = false;
    bool files_has_selection = false;

    auto* files_model = ui->recentFilesView->model();
    files_has_items = (files_model != nullptr) && (files_model->rowCount() > 0);

    const QItemSelectionModel* files_selection_model = ui->recentFilesView->selectionModel();
    const QModelIndex selected_file_index =
        files_selection_model != nullptr ? files_selection_model->currentIndex() : QModelIndex();
    files_has_selection = files_selection_model != nullptr &&
                          files_selection_model->hasSelection() &&
                          selected_file_index.flags().testFlag(Qt::ItemIsEnabled);

    bool sessions_has_items = false;
    bool sessions_has_selection = false;

    auto* session_model = ui->recentSessionsView->model();
    sessions_has_items = (session_model != nullptr) && (session_model->rowCount() > 0);

    const QItemSelectionModel* session_mselection_model = ui->recentSessionsView->selectionModel();
    sessions_has_selection =
        (session_mselection_model != nullptr) && session_mselection_model->hasSelection();

    const bool enable_open_selected_file = files_has_items && files_has_selection;
    const bool enable_clear_recent_files = files_has_items;

    const bool enable_open_selected_session = sessions_has_items && sessions_has_selection;
    const bool enable_delete_session = sessions_has_items && sessions_has_selection;

    ui->openSelectedFileButton->setEnabled(enable_open_selected_file);
    ui->clearRecentFilesButton->setEnabled(enable_clear_recent_files);
    ui->openSelectedSessionButton->setEnabled(enable_open_selected_session);
    ui->deleteSessionButton->setEnabled(enable_delete_session);

    const bool enable_reopen_last_session = sessions_has_items;
    ui->reopenSessionButton->setEnabled(enable_reopen_last_session);
}

/** @brief Shows recent lists only while their assigned models contain rows. */
auto StartPageWidget::update_empty_states() -> void
{
    const QAbstractItemModel* files_model = ui->recentFilesView->model();
    const bool files_have_items = files_model != nullptr && files_model->rowCount() > 0;
    if (m_recent_files_stack != nullptr)
    {
        m_recent_files_stack->setCurrentWidget(files_have_items
                                                   ? static_cast<QWidget*>(ui->recentFilesView)
                                                   : m_recent_files_empty_state);
    }
    ui->openSelectedFileButton->setVisible(files_have_items);
    ui->clearRecentFilesButton->setVisible(files_have_items);

    const QAbstractItemModel* sessions_model = ui->recentSessionsView->model();
    const bool sessions_have_items = sessions_model != nullptr && sessions_model->rowCount() > 0;
    if (m_recent_sessions_stack != nullptr)
    {
        m_recent_sessions_stack->setCurrentWidget(
            sessions_have_items ? static_cast<QWidget*>(ui->recentSessionsView)
                                : m_recent_sessions_empty_state);
    }
    ui->openSelectedSessionButton->setVisible(sessions_have_items);
    ui->deleteSessionButton->setVisible(sessions_have_items);
}

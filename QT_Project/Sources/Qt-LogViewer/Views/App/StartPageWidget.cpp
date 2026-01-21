/**
 * @file StartPageWidget.cpp
 * @brief Implementation of the StartPageWidget for the application start page.
 */

#include "Qt-LogViewer/Views/App/StartPageWidget.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QDateTime>
#include <QListView>
#include <QModelIndex>
#include <QPushButton>
#include <QVariant>

#include "Qt-LogViewer/Models/RecentLogFilesModel.h"
#include "Qt-LogViewer/Models/RecentSessionsModel.h"
#include "Qt-LogViewer/Views/Shared/RecentItemDelegate.h"
#include "ui_StartPageWidget.h"

/**
 * @brief Construct the StartPageWidget UI wrapper.
 * @param parent Parent widget.
 */
StartPageWidget::StartPageWidget(QWidget* parent): QWidget(parent), ui(new Ui::StartPageWidget)
{
    ui->setupUi(this);
    setup_table_views();
    setup_connections();
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
 * @param model QAbstractItemModel exposing roles: file_path, file_name, app_name, last_opened.
 */
auto StartPageWidget::set_recent_files_model(QAbstractItemModel* model) -> void
{
    if (ui->recentFilesView != nullptr)
    {
        ui->recentFilesView->setModel(model);

        if (auto* lv = qobject_cast<QListView*>(ui->recentFilesView); lv != nullptr)
        {
            lv->setItemDelegate(new RecentItemDelegate(RecentLogFilesModel::FileNameRole,
                                                       RecentLogFilesModel::FilePathRole,
                                                       RecentLogFilesModel::LastOpenedRole, lv));
            lv->setUniformItemSizes(true);
            lv->setSpacing(4);
        }
    }
}

/**
 * @brief Assign the model for the recent sessions table view.
 * @param model QAbstractItemModel exposing roles: name, last_opened, id.
 */
auto StartPageWidget::set_recent_sessions_model(QAbstractItemModel* model) -> void
{
    if (ui->recentSessionsView != nullptr)
    {
        ui->recentSessionsView->setModel(model);

        if (auto* lv = qobject_cast<QListView*>(ui->recentSessionsView); lv != nullptr)
        {
            lv->setItemDelegate(new RecentItemDelegate(RecentSessionsModel::NameRole,
                                                       RecentSessionsModel::IdRole,
                                                       RecentSessionsModel::LastOpenedRole, lv));
            lv->setUniformItemSizes(true);
            lv->setSpacing(4);
        }
    }
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
 * @brief Wire button clicks to signals.
 */
auto StartPageWidget::setup_connections() -> void
{
    if (ui->openFileButton != nullptr)
    {
        connect(ui->openFileButton, &QPushButton::clicked, this,
                [this]() { emit open_log_file_requested(); });
    }

    if (ui->openSelectedFileButton != nullptr)
    {
        connect(ui->openSelectedFileButton, &QPushButton::clicked, this, [this]() {
            const QString path = get_selected_recent_file_path();
            if (!path.isEmpty())
            {
                emit open_recent_file_requested(path);
            }
        });
    }

    if (ui->clearRecentFilesButton != nullptr)
    {
        connect(ui->clearRecentFilesButton, &QPushButton::clicked, this,
                [this]() { emit clear_recent_files_requested(); });
    }

    if (ui->openSessionButton != nullptr)
    {
        connect(ui->openSessionButton, &QPushButton::clicked, this, [this]() {
            const QString id = get_selected_recent_session_id();
            if (!id.isEmpty())
            {
                emit open_session_requested(id);
            }
        });
    }

    if (ui->openSelectedSessionButton != nullptr)
    {
        connect(ui->openSelectedSessionButton, &QPushButton::clicked, this, [this]() {
            const QString id = get_selected_recent_session_id();
            if (!id.isEmpty())
            {
                emit open_recent_session_requested(id);
            }
        });
    }

    if (ui->deleteSessionButton != nullptr)
    {
        connect(ui->deleteSessionButton, &QPushButton::clicked, this, [this]() {
            const QString id = get_selected_recent_session_id();
            if (!id.isEmpty())
            {
                emit delete_session_requested(id);
            }
        });
    }
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
        const QItemSelectionModel* sel = view->selectionModel();
        if (sel != nullptr)
        {
            const QModelIndex index = sel->currentIndex();
            if (index.isValid() && view->model() != nullptr)
            {
                const QVariant v = view->model()->data(index, Qt::UserRole + 1);
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
                const int id_role = Qt::UserRole + 3;
                const QVariant v = view->model()->data(index, id_role);
                id = v.toString();
            }
        }
    }

    return id;
}

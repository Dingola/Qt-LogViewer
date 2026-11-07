/**
 * @file RecentItemsMenuBinder.cpp
 * @brief Implements the reusable recent-items menu binding.
 */

#include "Qt-LogViewer/Views/Shared/RecentItemsMenuBinder.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QModelIndex>
#include <utility>

/**
 * @brief Constructs a binder for a menu and model.
 * @param menu Menu receiving generated actions.
 * @param model Flat model providing recent-item data.
 * @param roles Model roles consumed by the binder.
 * @param parent Optional QObject parent.
 */
RecentItemsMenuBinder::RecentItemsMenuBinder(QMenu* menu, QAbstractItemModel* model,
                                             RecentItemsMenuRoles roles, QObject* parent)
    : QObject(parent), m_menu(menu), m_model(model), m_roles(roles)
{
    if (m_menu != nullptr)
    {
        connect(m_menu, &QMenu::aboutToShow, this, &RecentItemsMenuBinder::refresh);
    }
    if (m_model != nullptr)
    {
        connect(m_model, &QAbstractItemModel::modelReset, this, &RecentItemsMenuBinder::refresh);
        connect(m_model, &QAbstractItemModel::rowsInserted, this, &RecentItemsMenuBinder::refresh);
        connect(m_model, &QAbstractItemModel::rowsRemoved, this, &RecentItemsMenuBinder::refresh);
        connect(m_model, &QAbstractItemModel::dataChanged, this, &RecentItemsMenuBinder::refresh);
        connect(m_model, &QAbstractItemModel::layoutChanged, this, &RecentItemsMenuBinder::refresh);
    }
    refresh();
}

/**
 * @brief Sets the maximum number of model items represented in the menu.
 * @param maximum Maximum visible items; negative values are treated as zero.
 */
auto RecentItemsMenuBinder::set_maximum_visible_items(int maximum) -> void
{
    m_maximum_visible_items = qMax(0, maximum);
    refresh();
}

/**
 * @brief Sets optional text shown as a disabled action for an empty model.
 * @param text Empty-state action text, or an empty string to omit it.
 */
auto RecentItemsMenuBinder::set_empty_text(const QString& text) -> void
{
    m_empty_text = text;
    refresh();
}

/**
 * @brief Sets optional text for a clear action managed by the binder.
 * @param text Clear-action text, or an empty string to omit it.
 */
auto RecentItemsMenuBinder::set_clear_text(const QString& text) -> void
{
    m_clear_text = text;
    refresh();
}

/**
 * @brief Sets a format used to combine disabled item text and status.
 * @param format Two-placeholder format receiving the item text and status text.
 */
auto RecentItemsMenuBinder::set_disabled_text_format(const QString& format) -> void
{
    m_disabled_text_format = format;
    refresh();
}

/**
 * @brief Sets persistent application-owned actions appended after generated items.
 * @param actions Footer actions in display order; null pointers are ignored.
 */
auto RecentItemsMenuBinder::set_footer_actions(const QList<QAction*>& actions) -> void
{
    m_footer_actions = actions;
    refresh();
}

/**
 * @brief Immediately rebuilds the menu from the current model state.
 */
auto RecentItemsMenuBinder::refresh() -> void
{
    bool contains_disabled_items = false;
    if (m_menu != nullptr)
    {
        m_menu->clear();
        const int model_rows = m_model != nullptr ? m_model->rowCount() : 0;
        const int visible_rows = qMin(model_rows, m_maximum_visible_items);

        for (int row = 0; row < model_rows; ++row)
        {
            const QModelIndex index = m_model->index(row, 0);
            const bool enabled = m_roles.enabled_role >= 0
                                     ? index.data(m_roles.enabled_role).toBool()
                                     : index.flags().testFlag(Qt::ItemIsEnabled);
            contains_disabled_items = contains_disabled_items || !enabled;
        }

        for (int row = 0; row < visible_rows; ++row)
        {
            const QModelIndex index = m_model->index(row, 0);
            QString text = index.data(m_roles.text_role).toString();
            const QVariant identity = index.data(m_roles.identity_role);
            const QString tooltip = index.data(m_roles.tooltip_role).toString();
            const QString status =
                m_roles.status_role >= 0 ? index.data(m_roles.status_role).toString() : QString();
            const QIcon icon = qvariant_cast<QIcon>(index.data(m_roles.icon_role));
            const bool enabled = m_roles.enabled_role >= 0
                                     ? index.data(m_roles.enabled_role).toBool()
                                     : index.flags().testFlag(Qt::ItemIsEnabled);
            if (!enabled && !status.isEmpty() && !m_disabled_text_format.isEmpty())
            {
                text = m_disabled_text_format.arg(text, status);
            }

            QAction* action = new QAction(icon, text, m_menu);
            action->setEnabled(enabled);
            action->setToolTip(tooltip);
            action->setWhatsThis(tooltip);
            action->setStatusTip(status);
            m_menu->addAction(action);
            connect(action, &QAction::triggered, this,
                    [this, identity]() { emit item_activated(identity); });
        }

        if (model_rows == 0 && !m_empty_text.isEmpty())
        {
            QAction* empty_action = m_menu->addAction(m_empty_text);
            empty_action->setEnabled(false);
        }

        const bool has_footer = !m_footer_actions.isEmpty() || !m_clear_text.isEmpty();
        if (has_footer && (visible_rows > 0 || !m_empty_text.isEmpty()))
        {
            m_menu->addSeparator();
        }

        for (QAction* footer_action: std::as_const(m_footer_actions))
        {
            if (footer_action != nullptr)
            {
                m_menu->addAction(footer_action);
            }
        }

        if (!m_clear_text.isEmpty())
        {
            QAction* clear_action = m_menu->addAction(m_clear_text);
            clear_action->setEnabled(model_rows > 0);
            connect(clear_action, &QAction::triggered, this,
                    &RecentItemsMenuBinder::clear_requested);
        }
    }
    emit contains_disabled_items_changed(contains_disabled_items);
}

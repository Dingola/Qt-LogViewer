#pragma once

#include <QAbstractItemModel>
#include <QList>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>

class QAction;

/**
 * @file RecentItemsMenuBinder.h
 * @brief Declares a reusable binding between an item model and a recent-items menu.
 */

/**
 * @struct RecentItemsMenuRoles
 * @brief Configures the model roles consumed by `RecentItemsMenuBinder`.
 */
struct RecentItemsMenuRoles {
        int text_role{Qt::DisplayRole};     ///< Role containing the action label.
        int identity_role{Qt::UserRole};    ///< Role containing the activation identity.
        int enabled_role{-1};               ///< Optional boolean role; flags are used if unset.
        int tooltip_role{Qt::ToolTipRole};  ///< Optional tooltip role.
        int icon_role{Qt::DecorationRole};  ///< Optional icon role.
        int status_role{-1};                ///< Optional short status role.
};

/**
 * @class RecentItemsMenuBinder
 * @brief Mirrors a flat item model into a QMenu and emits semantic user intent.
 */
class RecentItemsMenuBinder: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a binder for a menu and model.
         * @param menu Menu receiving generated actions.
         * @param model Flat model providing recent-item data.
         * @param roles Model roles consumed by the binder.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemsMenuBinder(QMenu* menu, QAbstractItemModel* model,
                                       RecentItemsMenuRoles roles, QObject* parent = nullptr);

        /**
         * @brief Sets the maximum number of model items represented in the menu.
         * @param maximum Maximum visible items; negative values are treated as zero.
         */
        auto set_maximum_visible_items(int maximum) -> void;

        /**
         * @brief Sets optional text shown as a disabled action for an empty model.
         * @param text Empty-state action text, or an empty string to omit it.
         */
        auto set_empty_text(const QString& text) -> void;

        /**
         * @brief Sets optional text for a clear action managed by the binder.
         * @param text Clear-action text, or an empty string to omit it.
         */
        auto set_clear_text(const QString& text) -> void;

        /**
         * @brief Sets a format used to combine disabled item text and status.
         * @param format Two-placeholder format receiving the item text and status text.
         */
        auto set_disabled_text_format(const QString& format) -> void;

        /**
         * @brief Sets persistent application-owned actions appended after generated items.
         * @param actions Footer actions in display order; null pointers are ignored.
         */
        auto set_footer_actions(const QList<QAction*>& actions) -> void;

        /**
         * @brief Immediately rebuilds the menu from the current model state.
         */
        auto refresh() -> void;

    signals:
        /**
         * @brief Emitted when a generated item action is activated.
         * @param identity Application-owned identity supplied by the model.
         */
        void item_activated(const QVariant& identity);

        /** @brief Emitted when the optional clear action is activated. */
        void clear_requested();

        /**
         * @brief Reports whether the represented model currently contains disabled items.
         * @param contains_disabled_items True when at least one represented item is disabled.
         */
        void contains_disabled_items_changed(bool contains_disabled_items);

    private:
        QPointer<QMenu> m_menu;
        QPointer<QAbstractItemModel> m_model;
        RecentItemsMenuRoles m_roles;
        int m_maximum_visible_items{10};
        QString m_empty_text;
        QString m_clear_text;
        QString m_disabled_text_format;
        QList<QAction*> m_footer_actions;
};

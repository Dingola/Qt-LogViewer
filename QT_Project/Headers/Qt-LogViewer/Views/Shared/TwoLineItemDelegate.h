#pragma once

#include <QMargins>
#include <QSize>
#include <QStyledItemDelegate>
#include <QVariant>
#include <functional>

/**
 * @file TwoLineItemDelegate.h
 * @brief Declares a configurable delegate for two-line list entries with optional metadata.
 */

/**
 * @class TwoLineItemDelegate
 * @brief Renders model-owned title, secondary, metadata, icon, and status values.
 */
class TwoLineItemDelegate: public QStyledItemDelegate
{
    public:
        /**
         * @struct Roles
         * @brief Identifies the model roles consumed by the delegate.
         */
        struct Roles {
                int title{Qt::DisplayRole};
                int secondary{-1};
                int metadata{-1};
                int icon{Qt::DecorationRole};
                int status{-1};
        };

        /**
         * @struct Layout
         * @brief Configures spacing, sizing, and text elision without domain assumptions.
         */
        struct Layout {
                QMargins content_margins{12, 8, 12, 8};
                QSize icon_size{16, 16};
                int row_spacing{2};
                int icon_spacing{8};
                int metadata_spacing{16};
                int badge_spacing{9};
                int badge_horizontal_padding{8};
                int badge_radius{4};
                Qt::TextElideMode title_elide_mode{Qt::ElideRight};
                Qt::TextElideMode secondary_elide_mode{Qt::ElideMiddle};
                Qt::TextElideMode metadata_elide_mode{Qt::ElideRight};
        };

        /**
         * @brief Converts a model value into visible metadata text.
         */
        using MetadataFormatter = std::function<QString(const QVariant&)>;

        /**
         * @brief Constructs a delegate using the supplied semantic model roles.
         * @param roles Model roles used for each visual element.
         * @param parent Optional QObject parent.
         */
        explicit TwoLineItemDelegate(Roles roles, QObject* parent = nullptr);

        /**
         * @brief Replaces the role mapping used for subsequent paints.
         * @param roles New semantic model roles.
         */
        auto set_roles(Roles roles) -> void;

        /**
         * @brief Replaces the visual layout configuration.
         * @param layout New spacing, sizing, and elision configuration.
         */
        auto set_layout(Layout layout) -> void;

        /**
         * @brief Sets the conversion applied to metadata values.
         * @param formatter Formatter supplied by the host application.
         */
        auto set_metadata_formatter(MetadataFormatter formatter) -> void;

        /**
         * @brief Paints a two-line model entry.
         * @param painter Painter used for rendering.
         * @param option Current item style and geometry.
         * @param index Model index to render.
         */
        auto paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const -> void override;

        /**
         * @brief Computes the preferred two-line item size.
         * @param option Current item style and geometry.
         * @param index Model index being measured.
         * @return Preferred item size including configured margins and spacing.
         */
        [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const -> QSize override;

    private:
        /**
         * @brief Formats a metadata model value for display.
         * @param value Raw metadata value.
         * @return Host-formatted metadata text or the value's string representation.
         */
        [[nodiscard]] auto format_metadata(const QVariant& value) const -> QString;

    private:
        Roles m_roles;
        Layout m_layout;
        MetadataFormatter m_metadata_formatter;
};

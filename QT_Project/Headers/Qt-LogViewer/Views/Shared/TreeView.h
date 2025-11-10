#pragma once

#include <QColor>
#include <QString>
#include <QTreeView>

/**
 * @file TreeView.h
 * @brief Declares the TreeView subclass which custom-paints branch connectors and indicator icons.
 *
 * Qt's stylesheet (QSS) cannot tint default branch images. This class overrides drawBranches()
 * to render:
 *  - Continuous vertical lines for all ancestor branches (no gaps caused by row margins).
 *  - A horizontal connector segment from the branch icon toward the item.
 *  - A tinted SVG indicator (open / closed) centered in the branch rectangle.
 *
 * Configurable via Q_PROPERTY for integration with QSS and dynamic runtime updates:
 *  - qproperty-tree_view_branch_icon_open:   QString path to open-branch SVG (default
 *    :/Resources/Icons/branch-open.svg)
 *  - qproperty-tree_view_branch_icon_closed: QString path to closed-branch SVG (default
 *    :/Resources/Icons/branch-closed.svg)
 *  - qproperty-tree_view_branch_icon_color:  QColor tint color (default #42a5f5)
 *  - qproperty-tree_view_branch_icon_px:     int square size of indicator; <=0 uses indentation()
 *  - qproperty-tree_view_branch_pen_width:   int connector line width; minimum enforced: 1
 *  - qproperty-tree_view_branch_hlength_px:  int horizontal connector length; <=0 uses
 *    indentation()/2
 *  - qproperty-tree_view_branch_top_vline:   bool whether to draw vertical segment above top-level
 *    items; default false
 *
 * Behavior details:
 *  - When a branch (node with children) is collapsed, no vertical connector is drawn below its
 * icon.
 *  - When expanded and not the last sibling, a vertical segment continues below the icon.
 *  - Ancestor vertical lines are drawn only if the ancestor has a following sibling.
 *
 * @see TreeView::drawBranches
 */

/**
 * @class TreeView
 * @brief QTreeView subclass providing custom branch connector rendering and SVG-based indicators.
 *
 * Use this class when you need consistent, theme-tinted branch icons and uninterrupted connector
 * lines regardless of item spacing. All appearance properties trigger a repaint when changed.
 */
class TreeView: public QTreeView
{
        Q_OBJECT
        Q_PROPERTY(QString tree_view_branch_icon_open READ get_tree_view_branch_icon_open WRITE
                       set_tree_view_branch_icon_open)
        Q_PROPERTY(QString tree_view_branch_icon_closed READ get_tree_view_branch_icon_closed WRITE
                       set_tree_view_branch_icon_closed)
        Q_PROPERTY(QColor tree_view_branch_icon_color READ get_tree_view_branch_icon_color WRITE
                       set_tree_view_branch_icon_color)
        Q_PROPERTY(int tree_view_branch_icon_px READ get_tree_view_branch_icon_px WRITE
                       set_tree_view_branch_icon_px)
        Q_PROPERTY(int tree_view_branch_pen_width READ get_tree_view_branch_pen_width WRITE
                       set_tree_view_branch_pen_width)
        Q_PROPERTY(int tree_view_branch_hlength_px READ get_tree_view_branch_hlength_px WRITE
                       set_tree_view_branch_hlength_px)
        Q_PROPERTY(bool tree_view_branch_top_vline READ get_tree_view_branch_top_vline WRITE
                       set_tree_view_branch_top_vline)

    public:
        /**
         * @brief Constructs the TreeView with default styling and resource paths.
         * @param parent Optional parent widget.
         *
         * Default values:
         *  - Open icon:    :/Resources/Icons/branch-open.svg
         *  - Closed icon:  :/Resources/Icons/branch-closed.svg
         *  - Icon color:   #42a5f5
         *  - Icon size:    <=0 (deferred to indentation())
         *  - Pen width:    2
         *  - Horizontal length: <=0 (deferred to indentation()/2)
         *  - Top vertical line: false
         */
        explicit TreeView(QWidget* parent = nullptr);

        /* ================= Setters ================= */

        /**
         * @brief Sets the SVG path used for an expanded (open) branch indicator.
         * @param path Resource or filesystem path to the SVG.
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_icon_open(const QString& path) -> void;

        /**
         * @brief Sets the SVG path used for a collapsed (closed) branch indicator.
         * @param path Resource or filesystem path to the SVG.
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_icon_closed(const QString& path) -> void;

        /**
         * @brief Sets the tint color applied to branch indicator SVGs and connector lines.
         * @param color The new color.
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_icon_color(const QColor& color) -> void;

        /**
         * @brief Sets the pixel size of the square indicator icon.
         * @param px Size in device-independent pixels; <=0 means use indentation().
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_icon_px(int px) -> void;

        /**
         * @brief Sets the pen width for connector lines.
         * @param px Desired width; values <1 are clamped to 1.
         *
         * Triggers repaint only if the (possibly clamped) value changes.
         */
        auto set_tree_view_branch_pen_width(int px) -> void;

        /**
         * @brief Sets the horizontal connector length (from icon center toward item).
         * @param px Length in pixels; <=0 uses indentation()/2.
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_hlength_px(int px) -> void;

        /**
         * @brief Enables or disables drawing a vertical segment above top-level branch icons.
         * @param enabled True to draw the top segment; false to start from icon center.
         *
         * Triggers repaint only if the value changes.
         */
        auto set_tree_view_branch_top_vline(bool enabled) -> void;

        /* ================= Getters ================= */

        /**
         * @brief Returns the current SVG path for the expanded (open) branch icon.
         * @return The SVG resource or file path.
         */
        [[nodiscard]] auto get_tree_view_branch_icon_open() const -> QString;

        /**
         * @brief Returns the current SVG path for the collapsed (closed) branch icon.
         * @return The SVG resource or file path.
         */
        [[nodiscard]] auto get_tree_view_branch_icon_closed() const -> QString;

        /**
         * @brief Returns the current tint color for indicators and connectors.
         * @return The QColor value.
         */
        [[nodiscard]] auto get_tree_view_branch_icon_color() const -> QColor;

        /**
         * @brief Returns the configured indicator icon size.
         * @return Size in pixels; <=0 means indentation() will be used at paint time.
         */
        [[nodiscard]] auto get_tree_view_branch_icon_px() const -> int;

        /**
         * @brief Returns the connector line pen width.
         * @return Width in pixels (>=1).
         */
        [[nodiscard]] auto get_tree_view_branch_pen_width() const -> int;

        /**
         * @brief Returns the horizontal connector length.
         * @return Length in pixels; <=0 means indentation()/2 will be used.
         */
        [[nodiscard]] auto get_tree_view_branch_hlength_px() const -> int;

        /**
         * @brief Returns whether top-level vertical segments are drawn above icons.
         * @return True if enabled; false otherwise.
         */
        [[nodiscard]] auto get_tree_view_branch_top_vline() const -> bool;

    protected:
        /**
         * @brief Paints branch connectors and indicator SVGs while preserving style sheet
         *        backgrounds.
         * @param painter Active painter.
         * @param rect The branch rectangle provided by QTreeView.
         * @param index The model index of the item whose branch is being drawn.
         *
         * Custom rules:
         *  - Collapsed branches omit the vertical line below the icon.
         *  - Expanded non-last siblings keep a vertical continuation segment.
         *  - Ancestor lines render only if the ancestor has a following sibling.
         */
        void drawBranches(QPainter* painter, const QRect& rect,
                          const QModelIndex& index) const override;

    private:
        /**
         * @brief Requests a repaint of the viewport after a visual property change.
         *
         * Safe no-op if viewport() is null (e.g., during destruction).
         */
        auto request_repaint() -> void;

    private:
        // Indicator SVGs (open/closed state)
        QString m_icon_open = QStringLiteral(":/Resources/Icons/branch-open.svg");
        QString m_icon_closed = QStringLiteral(":/Resources/Icons/branch-closed.svg");

        QColor m_icon_color = QColor("#42a5f5");
        int m_icon_px = -1;  // <= 0 -> use indentation()
        int m_pen_width = 2;
        int m_h_len_px = -1;       // <= 0 -> use indentation()/2
        bool m_top_vline = false;  // draw vertical segment above top-level items
};

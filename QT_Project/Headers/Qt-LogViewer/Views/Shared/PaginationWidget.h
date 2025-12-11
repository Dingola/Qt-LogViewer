#pragma once

#include <QHBoxLayout>
#include <QPainter>
#include <QToolButton>
#include <QWidget>
#include <utility>
#include <vector>

namespace Ui
{
class PaginationWidget;
}

/**
 * @class PaginationWidget
 * @brief Widget for displaying and controlling page navigation for paginated data.
 *
 * Shows previous/next arrows, a dynamic list of page buttons (with "..." for many pages),
 * and an input field for direct page jumps.
 */
class PaginationWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a PaginationWidget.
         * @param parent The parent widget, or nullptr.
         */
        explicit PaginationWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the PaginationWidget.
         */
        ~PaginationWidget() override;

        /**
         * @brief Sets the current page and total number of pages.
         * @param current_page The current page (1-based).
         * @param total_pages The total number of pages.
         */
        auto set_pagination(int current_page, int total_pages) -> void;

        /**
         * @brief Sets the maximum number of page buttons to display.
         *        This value represents the total visible button slots, including first/last and
         * ellipsis.
         * @param max_buttons The maximum number (minimum 3).
         */
        auto set_max_page_buttons(int max_buttons) -> void;

        /**
         * @brief Returns the current page (1-based).
         */
        [[nodiscard]] auto get_current_page() const -> int;

        /**
         * @brief Returns the total number of pages.
         */
        [[nodiscard]] auto get_total_pages() const -> int;

        /**
         * @brief Returns the number of items per page.
         */
        [[nodiscard]] auto get_items_per_page() const -> int;

    private:
        /**
         * @brief Internal display entry used to map a button slot to content.
         */
        struct page_entry {
                int page = -1;             ///< Page number (>=1) or -1 for ellipsis.
                bool is_ellipsis = false;  ///< True if this slot is the ellipsis "...".
                bool enabled = true;       ///< Enabled state for the button.
                bool checked = false;      ///< Checked state for the button (current page mark).
        };

        /**
         * @brief Ensures the page buttons pool exists and matches the current max size.
         *        If the pool needs to grow, new buttons are created and added once to the layout.
         */
        auto ensure_button_pool() -> void;

        /**
         * @brief Updates the button pool using the provided display model entries.
         *        Buttons outside the model range are hidden.
         * @param model The ordered list of entries to render.
         */
        auto apply_display_model(const std::vector<page_entry>& model) -> void;

        /**
         * @brief Builds the ordered display model for the current pagination state.
         *        The model length is <= m_max_page_buttons.
         * @return The model entries to render.
         */
        [[nodiscard]] auto build_display_model() const -> std::vector<page_entry>;

        /**
         * @brief Helper to create a single page button (pooled, reused).
         */
        [[nodiscard]] auto create_page_button() const -> QToolButton*;

        /**
         * @brief Updates the buttons and layout according to the current pagination state.
         */
        auto update_pagination() -> void;

        /**
         * @brief Updates the enabled/disabled state of controls.
         */
        auto update_controls_state() -> void;

        /**
         * @brief Adds display entries for the case of only one page.
         * @param model The model to append to.
         */
        auto add_single_page_entries(std::vector<page_entry>& model) const -> void;

        /**
         * @brief Adds display entries for the case where all pages fit without ellipsis.
         * @param model The model to append to.
         */
        auto add_simple_page_entries(std::vector<page_entry>& model) const -> void;

        /**
         * @brief Adds display entries for the case of max_page_buttons == 3.
         * @param model The model to append to.
         */
        auto add_three_page_entries(std::vector<page_entry>& model) const -> void;

        /**
         * @brief Adds display entries for the case of max_page_buttons == 4.
         * @param model The model to append to.
         */
        auto add_four_page_entries(std::vector<page_entry>& model) const -> void;

        /**
         * @brief Adds display entries for the general case (max_page_buttons > 4).
         * @param model The model to append to.
         */
        auto add_complex_page_entries(std::vector<page_entry>& model) const -> void;

        /**
         * @brief Calculates the range of middle page buttons for complex pagination.
         * @param total_pages The total number of pages.
         * @param max_btns The maximum number of buttons to display.
         * @param current_page The current page (1-based).
         * @return A pair containing the start and end indices for the middle buttons.
         */
        [[nodiscard]] auto calculate_middle_range(int total_pages, int max_btns,
                                                  int current_page) const -> std::pair<int, int>;

        /**
         * @brief Checks if the current page is the last page.
         * @return true if current page is the last page, false otherwise.
         */
        [[nodiscard]] auto is_last_page() const -> bool;

        /**
         * @brief Handles the paint event to enable QSS background and border styling.
         * @param event The paint event.
         */
        void paintEvent(QPaintEvent* event) override;

        /**
         * @brief Sends Leave to all page buttons and Enter to the one currently under the cursor.
         *        This resets stale hover highlights and applies hover to the correct target
         * immediately.
         */
        auto seed_hover_state() -> void;

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    signals:
        /**
         * @brief Emitted when a new page is selected.
         * @param page The selected page (1-based).
         */
        auto page_changed(int page) -> void;

        /**
         * @brief Emitted when the number of items per page changes.
         * @param items_per_page The new number of items per page.
         */
        auto items_per_page_changed(int items_per_page) -> void;

    private slots:
        auto onPrevClicked() -> void;
        auto onNextClicked() -> void;
        auto onPageButtonClicked() -> void;
        auto onJumpToPage() -> void;
        auto onItemsPerPageChanged(int index) -> void;

    private:
        Ui::PaginationWidget* ui;

        int m_current_page = 1;
        int m_total_pages = 1;
        int m_max_page_buttons = 7;

        std::vector<QToolButton*> m_page_buttons_pool;  ///< Reusable buttons, added once to layout.
};

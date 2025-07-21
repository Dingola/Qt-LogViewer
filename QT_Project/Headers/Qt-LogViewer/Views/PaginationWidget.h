#pragma once

#include <QHBoxLayout>
#include <QToolButton>
#include <QWidget>
#include <utility>

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
         * @param max_buttons The maximum number (e.g. 7).
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
         * @brief Dynamically creates the page buttons.
         */
        auto create_page_buttons() -> void;

        /**
         * @brief Helper to create a single page button.
         */
        [[nodiscard]] auto create_page_button(int page, bool enabled,
                                              bool checked) const -> QToolButton*;

        /**
         * @brief Updates the buttons and layout according to the current pagination state.
         */
        auto update_pagination() -> void;

        /**
         * @brief Updates the enabled/disabled state of controls.
         */
        auto update_controls_state() -> void;

        /**
         * @brief Creates the "..." button for pagination.
         * @return The created QToolButton.
         */
        [[nodiscard]] auto create_ellipsis_button() -> QToolButton*;

        /**
         * @brief Clears the page buttons layout.
         * Deletes all buttons and layout items in the page buttons widget.
         */
        auto clear_page_buttons_layout() -> void;

        /**
         * @brief Adds a single page button for the case of only one page.
         * @param layout The layout to add the button to.
         */
        auto add_single_page_button(QHBoxLayout* layout) -> void;

        /**
         * @brief Adds page buttons for the case where all pages fit without ellipsis.
         * @param layout The layout to add the buttons to.
         */
        auto add_simple_page_buttons(QHBoxLayout* layout) -> void;

        /**
         * @brief Adds page buttons for the case of max_page_buttons == 3.
         * @param layout The layout to add the buttons to.
         */
        auto add_three_page_buttons(QHBoxLayout* layout) -> void;

        /**
         * @brief Adds page buttons for the case of max_page_buttons == 4.
         * @param layout The layout to add the buttons to.
         */
        auto add_four_page_buttons(QHBoxLayout* layout) -> void;

        /**
         * @brief Adds page buttons for the general case (max_page_buttons > 4).
         * @param layout The layout to add the buttons to.
         */
        auto add_complex_page_buttons(QHBoxLayout* layout) -> void;

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
         * @brief Creates the last page button.
         * @return The created QToolButton for the last page.
         */
        [[nodiscard]] auto create_last_page_button() const -> QToolButton*;

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
};

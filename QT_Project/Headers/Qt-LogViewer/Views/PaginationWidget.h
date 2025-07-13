#pragma once

#include <QWidget>

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
        [[nodiscard]] auto create_page_button(int page, bool enabled, bool checked) -> QToolButton*;

        /**
         * @brief Updates the enabled/disabled state of controls.
         */
        auto update_controls_state() -> void;

    protected:
        /**
         * @brief Updates the buttons and layout according to the current pagination state.
         */
        auto update_pagination() -> void;

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

#include "Qt-LogViewer/Views/PaginationWidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

#include "ui_PaginationWidget.h"

/**
 * @brief Constructs a PaginationWidget and initializes the UI.
 * @param parent The parent widget, or nullptr.
 */
PaginationWidget::PaginationWidget(QWidget* parent): QWidget(parent), ui(new Ui::PaginationWidget)
{
    ui->setupUi(this);

    auto* layout = new QHBoxLayout(ui->pageButtonsWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    ui->pageButtonsWidget->setLayout(layout);

    ui->labelItemsPerPage->setText(tr("Per page"));

    ui->lineEditJumpTo->setValidator(new QIntValidator(1, 999999, this));

    // Set up comboBoxItemsPerPage
    ui->comboBoxItemsPerPage->setEditable(false);
    ui->comboBoxItemsPerPage->clear();
    ui->comboBoxItemsPerPage->addItem("25", 25);
    ui->comboBoxItemsPerPage->addItem("50", 50);
    ui->comboBoxItemsPerPage->addItem("100", 100);
    ui->comboBoxItemsPerPage->addItem("200", 200);
    ui->comboBoxItemsPerPage->setCurrentIndex(0);

    connect(ui->comboBoxItemsPerPage, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &PaginationWidget::onItemsPerPageChanged);

    connect(ui->buttonPrev, &QToolButton::clicked, this, &PaginationWidget::onPrevClicked);
    connect(ui->buttonNext, &QToolButton::clicked, this, &PaginationWidget::onNextClicked);
    connect(ui->buttonJumpPrev, &QToolButton::clicked, this, &PaginationWidget::onPrevClicked);
    connect(ui->buttonJumpNext, &QToolButton::clicked, this, &PaginationWidget::onNextClicked);
    connect(ui->lineEditJumpTo, &QLineEdit::editingFinished, this, &PaginationWidget::onJumpToPage);

    update_pagination();
    update_controls_state();
}

/**
 * @brief Destroys the PaginationWidget.
 */
PaginationWidget::~PaginationWidget()
{
    delete ui;
}

/**
 * @brief Sets the current page and total number of pages.
 * @param current_page The current page (1-based).
 * @param total_pages The total number of pages.
 */
auto PaginationWidget::set_pagination(int current_page, int total_pages) -> void
{
    if (total_pages < 1)
    {
        total_pages = 1;
    }

    if (current_page < 1)
    {
        current_page = 1;
    }

    if (current_page > total_pages)
    {
        current_page = total_pages;
    }

    if (m_current_page != current_page || m_total_pages != total_pages)
    {
        m_current_page = current_page;
        m_total_pages = total_pages;
        update_pagination();
    }
}

/**
 * @brief Sets the maximum number of page buttons to display.
 * @param max_buttons The maximum number (minimum 3).
 */
auto PaginationWidget::set_max_page_buttons(int max_buttons) -> void
{
    if (max_buttons < 3)
    {
        max_buttons = 3;
    }

    if (m_max_page_buttons != max_buttons)
    {
        m_max_page_buttons = max_buttons;
        update_pagination();
    }
}

/**
 * @brief Returns the current page (1-based).
 */
auto PaginationWidget::get_current_page() const -> int
{
    return m_current_page;
}

/**
 * @brief Returns the total number of pages.
 */
auto PaginationWidget::get_total_pages() const -> int
{
    return m_total_pages;
}

/**
 * @brief Returns the number of items per page.
 */
auto PaginationWidget::get_items_per_page() const -> int
{
    int index = ui->comboBoxItemsPerPage->currentIndex();
    return ui->comboBoxItemsPerPage->itemData(index).toInt();
}

/**
 * @brief Dynamically creates the page buttons.
 */
auto PaginationWidget::create_page_buttons() -> void
{
    clear_page_buttons_layout();
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(ui->pageButtonsWidget->layout());

    if (layout != nullptr)
    {
        if (m_total_pages == 1)
        {
            add_single_page_button(layout);
        }
        else if (m_total_pages <= m_max_page_buttons)
        {
            add_simple_page_buttons(layout);
        }
        else if (m_max_page_buttons == 3)
        {
            add_three_page_buttons(layout);
        }
        else if (m_max_page_buttons == 4)
        {
            add_four_page_buttons(layout);
        }
        else
        {
            add_complex_page_buttons(layout);
        }
    }
    else
    {
        qWarning() << "[PaginationWidget] pageButtonsWidget layout is nullptr!";
    }
}

/**
 * @brief Helper to create a single page button.
 * @param page The page number.
 * @param enabled Whether the button is enabled.
 * @param checked Whether the button is checked (current page).
 * @return The created QToolButton.
 */
auto PaginationWidget::create_page_button(int page, bool enabled,
                                          bool checked) const -> QToolButton*
{
    QToolButton* btn = new QToolButton(ui->pageButtonsWidget);
    btn->setText(QString::number(page));
    btn->setEnabled(enabled);
    btn->setCheckable(true);
    btn->setChecked(checked);
    btn->setAutoRaise(true);
    btn->setProperty("page", page);
    btn->setProperty("isCurrentPage", checked);

    connect(btn, &QToolButton::clicked, this, &PaginationWidget::onPageButtonClicked);

    return btn;
}

/**
 * @brief Updates the buttons and controls according to the current state.
 */
auto PaginationWidget::update_pagination() -> void
{
    qDebug() << "[PaginationWidget] update_pagination: current_page=" << m_current_page
             << "total_pages=" << m_total_pages << "max_buttons=" << m_max_page_buttons;

    create_page_buttons();
    update_controls_state();

    ui->lineEditJumpTo->blockSignals(true);
    ui->lineEditJumpTo->setText(QString::number(m_current_page));
    ui->lineEditJumpTo->blockSignals(false);
}

/**
 * @brief Updates the enabled/disabled state of controls.
 */
auto PaginationWidget::update_controls_state() -> void
{
    bool only_one_page = (m_total_pages == 1);

    if (ui->buttonPrev)
    {
        ui->buttonPrev->setEnabled(!only_one_page && m_current_page > 1);
    }

    if (ui->buttonNext)
    {
        ui->buttonNext->setEnabled(!only_one_page && m_current_page < m_total_pages);
    }

    if (ui->buttonJumpPrev)
    {
        ui->buttonJumpPrev->setEnabled(!only_one_page && m_current_page > 1);
    }

    if (ui->buttonJumpNext)
    {
        ui->buttonJumpNext->setEnabled(!only_one_page && m_current_page < m_total_pages);
    }

    if (ui->lineEditJumpTo)
    {
        ui->lineEditJumpTo->setEnabled(!only_one_page);
    }
}

/**
 * @brief Creates an ellipsis button for pagination.
 * @return A new QToolButton configured as an ellipsis button.
 */
auto PaginationWidget::create_ellipsis_button() -> QToolButton*
{
    QToolButton* button = new QToolButton(ui->pageButtonsWidget);
    button->setText("...");
    button->setEnabled(false);
    button->setAutoRaise(true);
    return button;
}

/**
 * @brief Clears the page buttons layout.
 * Deletes all buttons and layout items in the page buttons widget.
 */
auto PaginationWidget::clear_page_buttons_layout() -> void
{
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(ui->pageButtonsWidget->layout());

    while (QLayoutItem* child = (layout != nullptr ? layout->takeAt(0) : nullptr))
    {
        if (auto* button = qobject_cast<QToolButton*>(child->widget()))
        {
            delete button;
            button = nullptr;
        }

        delete child;
    }
}

/**
 * @brief Adds a single page button for the case of only one page.
 * @param layout The layout to add the button to.
 */
auto PaginationWidget::add_single_page_button(QHBoxLayout* layout) -> void
{
    layout->addWidget(create_page_button(1, false, true));
}

/**
 * @brief Adds page buttons for the case where all pages fit without ellipsis.
 * @param layout The layout to add the buttons to.
 */
auto PaginationWidget::add_simple_page_buttons(QHBoxLayout* layout) -> void
{
    for (int i = 1; i <= m_total_pages; ++i)
    {
        bool checked = (i == m_current_page);
        layout->addWidget(create_page_button(i, !checked, checked));
    }
}

/**
 * @brief Adds page buttons for the case of max_page_buttons == 3.
 * @param layout The layout to add the buttons to.
 */
auto PaginationWidget::add_three_page_buttons(QHBoxLayout* layout) -> void
{
    layout->addWidget(create_page_button(1, m_current_page != 1, m_current_page == 1));

    int middle_page = 0;
    if (m_current_page == 1)
    {
        middle_page = 2;
    }
    else if (is_last_page())
    {
        middle_page = m_total_pages - 1;
    }
    else
    {
        middle_page = m_current_page;
    }

    layout->addWidget(create_page_button(middle_page, middle_page != m_current_page,
                                         middle_page == m_current_page));
    layout->addWidget(create_last_page_button());
}

/**
 * @brief Adds page buttons for the case of max_page_buttons == 4.
 * @param layout The layout to add the buttons to.
 */
auto PaginationWidget::add_four_page_buttons(QHBoxLayout* layout) -> void
{
    layout->addWidget(create_page_button(1, m_current_page != 1, m_current_page == 1));

    if (m_current_page == 1 || m_current_page == 2)
    {
        layout->addWidget(create_page_button(2, m_current_page != 2, m_current_page == 2));
        layout->addWidget(create_ellipsis_button());
        layout->addWidget(
            create_page_button(m_total_pages, m_current_page != m_total_pages, is_last_page()));
    }
    else if (is_last_page() || m_current_page == (m_total_pages - 1))
    {
        layout->addWidget(create_ellipsis_button());
        layout->addWidget(create_page_button(m_total_pages - 1,
                                             m_current_page != (m_total_pages - 1),
                                             m_current_page == (m_total_pages - 1)));
        layout->addWidget(create_last_page_button());
    }
    else
    {
        int dist_to_start = m_current_page - 1;
        int dist_to_end = m_total_pages - m_current_page;
        if (dist_to_start <= dist_to_end)
        {
            layout->addWidget(create_page_button(m_current_page, false, true));
            layout->addWidget(create_ellipsis_button());
            layout->addWidget(create_last_page_button());
        }
        else
        {
            layout->addWidget(create_ellipsis_button());
            layout->addWidget(create_page_button(m_current_page, false, true));
            layout->addWidget(create_last_page_button());
        }
    }
}

/**
 * @brief Adds page buttons for the general case (max_page_buttons > 4).
 * @param layout The layout to add the buttons to.
 */
auto PaginationWidget::add_complex_page_buttons(QHBoxLayout* layout) -> void
{
    auto [start, end] = calculate_middle_range(m_total_pages, m_max_page_buttons, m_current_page);

    layout->addWidget(create_page_button(1, m_current_page != 1, m_current_page == 1));

    if (start > 2)
    {
        layout->addWidget(create_ellipsis_button());
    }

    for (int i = start; i <= end; ++i)
    {
        layout->addWidget(create_page_button(i, m_current_page != i, m_current_page == i));
    }

    if (end < m_total_pages - 1)
    {
        layout->addWidget(create_ellipsis_button());
    }

    layout->addWidget(create_last_page_button());
}

/**
 * @brief Calculates the range of middle page buttons for complex pagination.
 * @param total_pages The total number of pages.
 * @param max_btns The maximum number of buttons to display.
 * @param current_page The current page (1-based).
 * @return A pair containing the start and end indices for the middle buttons.
 */
auto PaginationWidget::calculate_middle_range(int total_pages, int max_btns,
                                              int current_page) const -> std::pair<int, int>
{
    int num_required = 2;  // first and last page
    int num_ellipsis = 2;
    int num_middle = max_btns - num_required - num_ellipsis;

    int start = 2;
    int end = total_pages - 1;

    if (total_pages <= max_btns)
    {
        // All pages fit, no ellipsis needed
        start = 2;
        end = total_pages - 1;
    }
    else
    {
        int left = current_page - num_middle / 2;
        int right = current_page + (num_middle - 1) / 2;

        if (left < 2)
        {
            right += (2 - left);
            left = 2;
        }
        if (right > total_pages - 1)
        {
            left -= (right - (total_pages - 1));
            right = total_pages - 1;
        }
        if (left < 2)
        {
            left = 2;
        }
        start = left;
        end = right;

        // Adjust ellipsis count
        if (start == 2)
        {
            num_ellipsis--;
        }
        if (end == total_pages - 1)
        {
            num_ellipsis--;
        }

        // If only one ellipsis is needed, allow more middle buttons
        if (num_ellipsis == 1)
        {
            num_middle = max_btns - num_required - 1;
            if (start == 2)
            {
                end = start + num_middle - 1;
            }
            else
            {
                start = end - num_middle + 1;
            }
        }
        // If no ellipsis is needed, show all pages
        if (num_ellipsis == 0)
        {
            start = 2;
            end = total_pages - 1;
        }
    }

    return {start, end};
}

/**
 * @brief Checks if the current page is the last page.
 * @return true if current page is the last page, false otherwise.
 */
auto PaginationWidget::is_last_page() const -> bool
{
    return m_current_page == m_total_pages;
}

/**
 * @brief Creates the last page button.
 * @return The created QToolButton for the last page.
 */
auto PaginationWidget::create_last_page_button() const -> QToolButton*
{
    return create_page_button(m_total_pages, !is_last_page(), is_last_page());
}

/**
 * @brief Slot: Go to previous page.
 */
auto PaginationWidget::onPrevClicked() -> void
{
    if (m_current_page > 1)
    {
        set_pagination(m_current_page - 1, m_total_pages);
        emit page_changed(m_current_page);
    }
}

/**
 * @brief Slot: Go to next page.
 */
auto PaginationWidget::onNextClicked() -> void
{
    if (m_current_page < m_total_pages)
    {
        set_pagination(m_current_page + 1, m_total_pages);
        emit page_changed(m_current_page);
    }
}

/**
 * @brief Slot: Page button clicked.
 */
auto PaginationWidget::onPageButtonClicked() -> void
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());

    int page = 0;

    if (button)
    {
        page = button->property("page").toInt();
    }

    if (page != m_current_page && page > 0)
    {
        set_pagination(page, m_total_pages);
        emit page_changed(m_current_page);
    }
}

/**
 * @brief Slot: Jump to page entered.
 */
auto PaginationWidget::onJumpToPage() -> void
{
    int page = ui->lineEditJumpTo->text().toInt();

    if (page < 1)
    {
        page = 1;
    }

    if (page > m_total_pages)
    {
        page = m_total_pages;
    }

    if (page != m_current_page)
    {
        set_pagination(page, m_total_pages);
        emit page_changed(m_current_page);
    }
}

/**
 * @brief Slot: Items per page changed.
 * Emits the items_per_page_changed signal with the new value.
 */
auto PaginationWidget::onItemsPerPageChanged(int index) -> void
{
    int value = ui->comboBoxItemsPerPage->itemData(index).toInt();
    emit items_per_page_changed(value);
}

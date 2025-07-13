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
    QHBoxLayout* layout = qobject_cast<QHBoxLayout*>(ui->pageButtonsWidget->layout());

    if (layout)
    {
        while (QLayoutItem* child = layout->takeAt(0))
        {
            if (auto* btn = qobject_cast<QToolButton*>(child->widget()))
            {
                btn->deleteLater();
            }

            delete child;
        }

        if (m_total_pages == 1)
        {
            QToolButton* btn = create_page_button(1, false, true);
            layout->addWidget(btn);
        }
        else
        {
            int max_btns = m_max_page_buttons;
            QVector<int> pages_to_show;

            if (m_total_pages <= max_btns)
            {
                for (int i = 1; i <= m_total_pages; ++i)
                {
                    pages_to_show.append(i);
                }
            }
            else
            {
                pages_to_show.append(1);

                int num_middle = max_btns - 2;
                int start = m_current_page - num_middle / 2;
                int end = m_current_page + num_middle / 2;

                if (start < 2)
                {
                    start = 2;
                    end = start + num_middle - 1;
                }

                if (end > m_total_pages - 1)
                {
                    end = m_total_pages - 1;
                    start = end - num_middle + 1;
                }

                if (start > 2)
                {
                    pages_to_show.append(-1);
                }

                for (int i = start; i <= end; ++i)
                {
                    pages_to_show.append(i);
                }

                if (end < m_total_pages - 1)
                {
                    pages_to_show.append(-1);
                }

                pages_to_show.append(m_total_pages);
            }

            for (int i = 0; i < pages_to_show.size(); ++i)
            {
                int page = pages_to_show[i];

                if (page == -1)
                {
                    QToolButton* btn = new QToolButton(ui->pageButtonsWidget);
                    btn->setText("...");
                    btn->setEnabled(false);
                    btn->setAutoRaise(true);
                    layout->addWidget(btn);
                }
                else
                {
                    bool checked = (page == m_current_page);
                    QToolButton* btn = create_page_button(page, !checked, checked);
                    layout->addWidget(btn);
                }
            }
        }
    }
}

/**
 * @brief Helper to create a single page button.
 * @param page The page number.
 * @param enabled Whether the button is enabled.
 * @param checked Whether the button is checked (current page).
 * @return The created QToolButton.
 */
auto PaginationWidget::create_page_button(int page, bool enabled, bool checked) -> QToolButton*
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
    QToolButton* btn = qobject_cast<QToolButton*>(sender());

    int page = 0;

    if (btn)
    {
        page = btn->property("page").toInt();
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

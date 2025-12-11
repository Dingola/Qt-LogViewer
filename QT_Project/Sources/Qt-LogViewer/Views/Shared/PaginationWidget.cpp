#include "Qt-LogViewer/Views/Shared/PaginationWidget.h"

#include <QCoreApplication>
#include <QCursor>
#include <QDebug>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QStyleOption>
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

    // Ensure hover tracking for correct hover behavior without mouse movement
    ui->pageButtonsWidget->setAttribute(Qt::WA_Hover, true);
    ui->pageButtonsWidget->setMouseTracking(true);

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

    // Create initial pool based on m_max_page_buttons
    ensure_button_pool();

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
 *        This value represents the total visible button slots, including first/last and ellipsis.
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
        ensure_button_pool();
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
    const int index = ui->comboBoxItemsPerPage->currentIndex();
    const int value = ui->comboBoxItemsPerPage->itemData(index).toInt();
    return value;
}

/**
 * @brief Ensures the page buttons pool exists and matches the current max size.
 *        If the pool needs to grow, new buttons are created and added once to the layout.
 */
auto PaginationWidget::ensure_button_pool() -> void
{
    auto* layout = qobject_cast<QHBoxLayout*>(ui->pageButtonsWidget->layout());

    if (layout == nullptr)
    {
        qWarning() << "[PaginationWidget] pageButtonsWidget layout is nullptr!";
    }
    else
    {
        const int required = m_max_page_buttons;
        const int current = static_cast<int>(m_page_buttons_pool.size());

        if (current < required)
        {
            const int to_create = required - current;
            for (int i = 0; i < to_create; ++i)
            {
                auto* btn = create_page_button();
                m_page_buttons_pool.push_back(btn);
                layout->addWidget(btn);
                btn->setVisible(false);
            }
        }
        else
        {
            for (auto* btn: m_page_buttons_pool)
            {
                btn->setVisible(false);
            }
        }
    }
}

/**
 * @brief Helper to create a single page button (pooled, reused).
 */
auto PaginationWidget::create_page_button() const -> QToolButton*
{
    auto* btn = new QToolButton(ui->pageButtonsWidget);
    btn->setAutoRaise(true);
    btn->setCheckable(true);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setAttribute(Qt::WA_Hover, true);
    btn->setMouseTracking(true);
    btn->setProperty("page", -1);
    btn->setProperty("isCurrentPage", false);
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

    ensure_button_pool();
    const auto model = build_display_model();
    apply_display_model(model);
    update_controls_state();

    ui->lineEditJumpTo->blockSignals(true);
    ui->lineEditJumpTo->setText(QString::number(m_current_page));
    ui->lineEditJumpTo->blockSignals(false);

    // Reseed hover to the correct button under the cursor after the model has been applied
    seed_hover_state();
}

/**
 * @brief Updates the enabled/disabled state of controls.
 */
auto PaginationWidget::update_controls_state() -> void
{
    const bool only_one_page = (m_total_pages == 1);

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
 * @brief Builds the ordered display model for the current pagination state.
 * @return The model entries to render.
 */
auto PaginationWidget::build_display_model() const -> std::vector<page_entry>
{
    std::vector<page_entry> model;

    if (m_total_pages == 1)
    {
        add_single_page_entries(model);
    }
    else if (m_total_pages <= m_max_page_buttons)
    {
        add_simple_page_entries(model);
    }
    else if (m_max_page_buttons == 3)
    {
        add_three_page_entries(model);
    }
    else if (m_max_page_buttons == 4)
    {
        add_four_page_entries(model);
    }
    else
    {
        add_complex_page_entries(model);
    }

    return model;
}

/**
 * @brief Applies the display model onto the pooled buttons.
 * @param model The ordered list of entries to render.
 */
auto PaginationWidget::apply_display_model(const std::vector<page_entry>& model) -> void
{
    const int count = static_cast<int>(model.size());
    const int pool = static_cast<int>(m_page_buttons_pool.size());

    for (int i = 0; i < pool; ++i)
    {
        if (i < count)
        {
            const auto& entry = model[static_cast<std::size_t>(i)];
            QToolButton* btn = m_page_buttons_pool[static_cast<std::size_t>(i)];

            if (entry.is_ellipsis)
            {
                btn->setText("...");
                btn->setEnabled(false);
                btn->setChecked(false);
                btn->setProperty("page", -1);
                btn->setProperty("isCurrentPage", false);
            }
            else
            {
                btn->setText(QString::number(entry.page));
                btn->setEnabled(true);  // active and inactive pages stay enabled
                btn->setChecked(entry.checked);
                btn->setProperty("page", entry.page);
                btn->setProperty("isCurrentPage", entry.checked);
            }

            // Force restyling so QSS picks up dynamic property changes immediately
            btn->style()->unpolish(btn);
            btn->style()->polish(btn);
            btn->update();

            btn->setVisible(true);
        }
        else
        {
            m_page_buttons_pool[static_cast<std::size_t>(i)]->setVisible(false);
        }
    }
}

/**
 * @brief Adds display entries for the case of only one page.
 * @param model The model to append to.
 */
auto PaginationWidget::add_single_page_entries(std::vector<page_entry>& model) const -> void
{
    page_entry entry;
    entry.page = 1;
    entry.is_ellipsis = false;
    entry.enabled = true;
    entry.checked = true;
    model.push_back(entry);
}

/**
 * @brief Adds display entries for the case where all pages fit without ellipsis.
 * @param model The model to append to.
 */
auto PaginationWidget::add_simple_page_entries(std::vector<page_entry>& model) const -> void
{
    for (int i = 1; i <= m_total_pages; ++i)
    {
        page_entry entry;
        entry.page = i;
        entry.is_ellipsis = false;
        entry.checked = (i == m_current_page);
        entry.enabled = true;
        model.push_back(entry);
    }
}

/**
 * @brief Adds display entries for the case of max_page_buttons == 3.
 * @param model The model to append to.
 */
auto PaginationWidget::add_three_page_entries(std::vector<page_entry>& model) const -> void
{
    page_entry first;
    first.page = 1;
    first.is_ellipsis = false;
    first.checked = (m_current_page == 1);
    first.enabled = true;
    model.push_back(first);

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

    page_entry middle;
    middle.page = middle_page;
    middle.is_ellipsis = false;
    middle.checked = (middle_page == m_current_page);
    middle.enabled = true;
    model.push_back(middle);

    page_entry last;
    last.page = m_total_pages;
    last.is_ellipsis = false;
    last.checked = is_last_page();
    last.enabled = true;
    model.push_back(last);
}

/**
 * @brief Adds display entries for the case of max_page_buttons == 4.
 * @param model The model to append to.
 */
auto PaginationWidget::add_four_page_entries(std::vector<page_entry>& model) const -> void
{
    page_entry first;
    first.page = 1;
    first.is_ellipsis = false;
    first.checked = (m_current_page == 1);
    first.enabled = true;
    model.push_back(first);

    if (m_current_page == 1 || m_current_page == 2)
    {
        page_entry two;
        two.page = 2;
        two.is_ellipsis = false;
        two.checked = (m_current_page == 2);
        two.enabled = true;
        model.push_back(two);

        page_entry dots;
        dots.is_ellipsis = true;
        dots.page = -1;
        dots.enabled = false;
        dots.checked = false;
        model.push_back(dots);

        page_entry last;
        last.page = m_total_pages;
        last.is_ellipsis = false;
        last.checked = is_last_page();
        last.enabled = true;
        model.push_back(last);
    }
    else if (is_last_page() || m_current_page == (m_total_pages - 1))
    {
        page_entry dots;
        dots.is_ellipsis = true;
        dots.page = -1;
        dots.enabled = false;
        dots.checked = false;
        model.push_back(dots);

        page_entry prev_last;
        prev_last.page = m_total_pages - 1;
        prev_last.is_ellipsis = false;
        prev_last.checked = (m_current_page == (m_total_pages - 1));
        prev_last.enabled = true;
        model.push_back(prev_last);

        page_entry last;
        last.page = m_total_pages;
        last.is_ellipsis = false;
        last.checked = is_last_page();
        last.enabled = true;
        model.push_back(last);
    }
    else
    {
        const int dist_to_start = m_current_page - 1;
        const int dist_to_end = m_total_pages - m_current_page;

        if (dist_to_start <= dist_to_end)
        {
            page_entry cur;
            cur.page = m_current_page;
            cur.is_ellipsis = false;
            cur.checked = true;
            cur.enabled = true;
            model.push_back(cur);

            page_entry dots;
            dots.is_ellipsis = true;
            dots.page = -1;
            dots.enabled = false;
            dots.checked = false;
            model.push_back(dots);

            page_entry last;
            last.page = m_total_pages;
            last.is_ellipsis = false;
            last.checked = is_last_page();
            last.enabled = true;
            model.push_back(last);
        }
        else
        {
            page_entry dots;
            dots.is_ellipsis = true;
            dots.page = -1;
            dots.enabled = false;
            dots.checked = false;
            model.push_back(dots);

            page_entry cur;
            cur.page = m_current_page;
            cur.is_ellipsis = false;
            cur.checked = true;
            cur.enabled = true;
            model.push_back(cur);

            page_entry last;
            last.page = m_total_pages;
            last.is_ellipsis = false;
            last.checked = is_last_page();
            last.enabled = true;
            model.push_back(last);
        }
    }
}

/**
 * @brief Adds display entries for the general case (max_page_buttons > 4).
 * @param model The model to append to.
 */
auto PaginationWidget::add_complex_page_entries(std::vector<page_entry>& model) const -> void
{
    const auto range = calculate_middle_range(m_total_pages, m_max_page_buttons, m_current_page);
    const int start = range.first;
    const int end = range.second;

    page_entry first;
    first.page = 1;
    first.is_ellipsis = false;
    first.checked = (m_current_page == 1);
    first.enabled = true;
    model.push_back(first);

    if (start > 2)
    {
        page_entry dots_left;
        dots_left.is_ellipsis = true;
        dots_left.page = -1;
        dots_left.enabled = false;
        dots_left.checked = false;
        model.push_back(dots_left);
    }

    for (int i = start; i <= end; ++i)
    {
        page_entry mid;
        mid.page = i;
        mid.is_ellipsis = false;
        mid.checked = (i == m_current_page);
        mid.enabled = true;
        model.push_back(mid);
    }

    if (end < m_total_pages - 1)
    {
        page_entry dots_right;
        dots_right.is_ellipsis = true;
        dots_right.page = -1;
        dots_right.enabled = false;
        dots_right.checked = false;
        model.push_back(dots_right);
    }

    page_entry last;
    last.page = m_total_pages;
    last.is_ellipsis = false;
    last.checked = is_last_page();
    last.enabled = true;
    model.push_back(last);
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
    int end;

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
    const bool last = (m_current_page == m_total_pages);
    return last;
}

/**
 * @brief Handles the paint event to enable QSS background and border styling.
 *        This ensures that custom QSS (e.g., background-color) is rendered.
 * @param event The paint event.
 */
void PaginationWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}

/**
 * @brief Handles change events to update the UI.
 * This method is called when the widget's properties change.
 * @param event The change event.
 */
auto PaginationWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->labelItemsPerPage->setText(tr("Per page"));
        update_pagination();
    }

    QWidget::changeEvent(event);
}

/**
 * @brief Sends Leave to all page buttons and Enter to the one currently under the cursor.
 *        This resets stale hover highlights and applies hover to the correct target immediately.
 */
auto PaginationWidget::seed_hover_state() -> void
{
    const auto buttons = ui->pageButtonsWidget->findChildren<QToolButton*>();

    // Reset stale hover on all pooled buttons
    for (auto* btn: buttons)
    {
        QEvent leave_ev(QEvent::Leave);
        QCoreApplication::sendEvent(btn, &leave_ev);
    }

    const QPoint global_pos = QCursor::pos();
    const QPoint local_pos = ui->pageButtonsWidget->mapFromGlobal(global_pos);
    if (ui->pageButtonsWidget->rect().contains(local_pos))
    {
        QWidget* child = ui->pageButtonsWidget->childAt(local_pos);
        auto* hovered_btn = qobject_cast<QToolButton*>(child);
        if (hovered_btn != nullptr)
        {
            const QPointF child_local = hovered_btn->mapFromGlobal(global_pos);
            QEnterEvent enter_ev(child_local, child_local, global_pos);
            QCoreApplication::sendEvent(hovered_btn, &enter_ev);
        }
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
    auto* button = qobject_cast<QToolButton*>(sender());

    int page = 0;
    if (button != nullptr)
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
    const int value = ui->comboBoxItemsPerPage->itemData(index).toInt();
    emit items_per_page_changed(value);
}

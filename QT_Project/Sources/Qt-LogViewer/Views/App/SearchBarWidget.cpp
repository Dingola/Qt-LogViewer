#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

#include <QEvent>
#include <QFontMetrics>
#include <QLineEdit>
#include <QResizeEvent>
#include <QString>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QToolButton>

#include "Qt-LogViewer/Services/UiUtils.h"
#include "ui_SearchBarWidget.h"

namespace
{
constexpr auto k_search_placeholder_text = QT_TRANSLATE_NOOP("SearchBarWidget", "Search...");
}  // namespace

/**
 * @brief Constructs a SearchBarWidget object.
 *
 * Initializes the search bar widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
SearchBarWidget::SearchBarWidget(QWidget* parent): QWidget(parent), ui(new Ui::SearchBarWidget)
{
    ui->setupUi(this);

    ui->comboBoxSearchArea->addItems(QStringList()
                                     << "All Fields" << "Message" << "Level" << "AppName");

    ui->lineEditSearch->setPlaceholderText(tr(k_search_placeholder_text));
    QIcon search_icon(
        UiUtils::colored_svg_icon(":/Resources/Icons/magnifying-glass.svg", QColor("#42a5f5")));
    ui->lineEditSearch->addAction(search_icon, QLineEdit::LeadingPosition);

    // Save original margins (may be zero).
    m_original_text_margins = ui->lineEditSearch->textMargins();

    // Create clear button and hook behavior.
    init_clear_button();
    update_clear_button_visibility(ui->lineEditSearch->text());

    // Connect search button and text changes (existing behavior preserved).
    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            [=] { style()->polish(ui->lineEditSearch); });
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, [this] {
        emit search_requested(get_search_text(), get_search_field(), get_use_regex());
    });
    connect(ui->lineEditSearch, &QLineEdit::returnPressed, this, [this] {
        emit search_requested(get_search_text(), get_search_field(), get_use_regex());
    });
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [this](const QString& new_text) {
        emit search_text_changed(get_search_text());
        update_clear_button_visibility(new_text);
        if (get_use_live_search())
        {
            emit search_requested(get_search_text(), get_search_field(), get_use_regex());
        }
    });
    connect(ui->comboBoxSearchArea, &QComboBox::currentTextChanged, this,
            [this] { emit search_field_changed(get_search_field()); });
    connect(ui->checkBoxRegEx, &QCheckBox::toggled, this,
            [this] { emit regex_toggled(get_use_regex()); });
    connect(ui->checkBoxLiveSearch, &QCheckBox::toggled, this, [this] {
        emit live_search_toggled(get_use_live_search());
        if (get_use_live_search())
        {
            emit search_requested(get_search_text(), get_search_field(), get_use_regex());
        }
    });
}

/**
 * @brief Destroys the SearchBarWidget object.
 *
 * Cleans up any resources used by the search bar widget.
 */
SearchBarWidget::~SearchBarWidget()
{
    delete ui;
}

/**
 * @brief Creates and configures the clear button as a child of the line edit.
 */
auto SearchBarWidget::init_clear_button() -> void
{
    m_clear_button = new QToolButton(ui->lineEditSearch);
    m_clear_button->setObjectName(QStringLiteral("searchClearButton"));
    m_clear_button->setCursor(Qt::ArrowCursor);
    m_clear_button->setFocusPolicy(Qt::NoFocus);
    m_clear_button->setAutoRaise(true);
    m_clear_button->setToolTip(tr("Clear"));

    update_clear_button_icon();
    m_clear_button->setIconSize(QSize(m_clear_icon_px, m_clear_icon_px));
    m_clear_button->installEventFilter(this);

    connect(m_clear_button, &QToolButton::clicked, this, [this] {
        ui->lineEditSearch->clear();
        emit search_text_changed(get_search_text());
        if (get_use_live_search())
        {
            emit search_requested(get_search_text(), get_search_field(), get_use_regex());
        }
        ui->lineEditSearch->setFocus();
    });

    m_clear_button->setVisible(false);
}

/**
 * @brief Shows/hides and repositions the clear button based on current text.
 * @param text The current line edit text.
 */
auto SearchBarWidget::update_clear_button_visibility(const QString& text) -> void
{
    bool should_show = (!text.isEmpty());
    m_clear_button->setVisible(should_show);
    if (should_show)
    {
        ensure_text_right_margin();
        position_clear_button();
    }
    else
    {
        ui->lineEditSearch->setTextMargins(m_original_text_margins);
        style()->polish(ui->lineEditSearch);
    }
}

/**
 * @brief Ensures the line edit has a right margin large enough for the clear button.
 */
auto SearchBarWidget::ensure_text_right_margin() -> void
{
    QMargins current = ui->lineEditSearch->textMargins();
    int needed_right = m_clear_button->width() + m_clear_button_padding;
    int right = current.right();
    int left = current.left();
    int top = current.top();
    int bottom = current.bottom();

    if (right < needed_right)
    {
        ui->lineEditSearch->setTextMargins(left, top, needed_right, bottom);
    }
}

/**
 * @brief Positions the clear button after the visible text without overlapping it.
 *
 * Calculates the text end position by measuring the text width and adding the offset
 * for any leading action icons (search icon). The button is clamped to stay within bounds.
 */
auto SearchBarWidget::position_clear_button() -> void
{
    if (!(m_clear_button == nullptr || !m_clear_button->isVisible()))
    {
        const QString text = ui->lineEditSearch->text();
        QFontMetrics fm(ui->lineEditSearch->font());
        const int text_width = fm.horizontalAdvance(text);

        // Get content rect from style
        QStyleOptionFrame opt;
        opt.initFrom(ui->lineEditSearch);
        const QRect contents = ui->lineEditSearch->style()->subElementRect(
            QStyle::SE_LineEditContents, &opt, ui->lineEditSearch);

        // Find the leading action button(s) to calculate their total width.
        // QLineEdit creates internal QToolButtons for actions added via addAction().
        int leading_action_width = 0;
        const QList<QToolButton*> action_buttons = ui->lineEditSearch->findChildren<QToolButton*>();
        for (QToolButton* btn: action_buttons)
        {
            // Skip our own clear button
            if (btn == m_clear_button)
            {
                continue;
            }
            // Check if button is on the left side (leading position)
            if (btn->x() < contents.center().x())
            {
                leading_action_width += btn->width();
            }
        }

        // Text starts after the content rect left edge plus any leading action icons
        const int text_start_x = contents.left() + leading_action_width;
        const int text_end_x = text_start_x + text_width;

        const int button_w = m_clear_button->sizeHint().width();
        const int guard = 4;
        const int widget_w = ui->lineEditSearch->width();
        const int max_x = widget_w - button_w - guard;

        int desired = text_end_x + m_clear_button_padding;

        int x = desired;
        if (x > max_x)
        {
            x = max_x;
        }
        if (x < text_start_x)
        {
            x = text_start_x;
        }

        const int h = m_clear_button->sizeHint().height();
        const int widget_h = ui->lineEditSearch->height();
        const int y = (widget_h - h) / 2;

        m_clear_button->setGeometry(x, y, button_w, h);
    }
}

/**
 * @brief Updates the clear button icon using the current color and size.
 */
auto SearchBarWidget::update_clear_button_icon() -> void
{
    const QSize icon_sz(m_clear_icon_px, m_clear_icon_px);
    QIcon icon(
        UiUtils::colored_svg_icon(":/Resources/Icons/close.svg", m_clear_icon_color, icon_sz));
    m_clear_button->setIcon(icon);
    m_clear_button->setIconSize(icon_sz);
}

/**
 * @brief Gets the normal (non-hover) clear icon color.
 * @return The current color.
 */
[[nodiscard]] auto SearchBarWidget::get_clear_icon_color() const -> QColor
{
    QColor result = m_clear_icon_color;
    return result;
}

/**
 * @brief Sets the normal (non-hover) clear icon color and refreshes the icon.
 * @param color The new color to use.
 */
auto SearchBarWidget::set_clear_icon_color(const QColor& color) -> void
{
    if (m_clear_icon_color != color)
    {
        m_clear_icon_color = color;
        if (m_clear_button != nullptr)
        {
            update_clear_button_icon();
        }
    }
}

/**
 * @brief Gets the hover/pressed clear icon color.
 * @return The current hover color.
 */
[[nodiscard]] auto SearchBarWidget::get_clear_icon_color_hover() const -> QColor
{
    QColor result = m_clear_icon_color_hover;
    return result;
}

/**
 * @brief Sets the hover/pressed clear icon color.
 * @param color The new hover color to use.
 */
auto SearchBarWidget::set_clear_icon_color_hover(const QColor& color) -> void
{
    if (m_clear_icon_color_hover != color)
    {
        m_clear_icon_color_hover = color;
        if (m_clear_button != nullptr)
        {
            update_clear_button_icon();
        }
    }
}

/**
 * @brief Gets the clear icon square size in pixels.
 * @return The icon size.
 */
[[nodiscard]] auto SearchBarWidget::get_clear_icon_px() const -> int
{
    int result = m_clear_icon_px;
    return result;
}

/**
 * @brief Sets the clear icon size in pixels and updates layout.
 * @param px The new size (values < 1 are clamped to 1).
 */
auto SearchBarWidget::set_clear_icon_px(int px) -> void
{
    int clamped = px;
    if (clamped < 1)
    {
        clamped = 1;
    }

    if (m_clear_icon_px != clamped)
    {
        m_clear_icon_px = clamped;
        if (m_clear_button != nullptr)
        {
            update_clear_button_icon();
            m_clear_button->setIconSize(QSize(m_clear_icon_px, m_clear_icon_px));
            position_clear_button();
            ensure_text_right_margin();
        }
    }
}

/**
 * @brief Gets the padding between text end and clear button.
 * @return The padding in pixels.
 */
[[nodiscard]] auto SearchBarWidget::get_clear_button_padding() const -> int
{
    int result = m_clear_button_padding;
    return result;
}

/**
 * @brief Sets the padding between text end and clear button and updates layout.
 * @param px The new padding in pixels (values < 0 are clamped to 0).
 */
auto SearchBarWidget::set_clear_button_padding(int px) -> void
{
    int clamped = px;
    if (clamped < 0)
    {
        clamped = 0;
    }

    if (m_clear_button_padding != clamped)
    {
        m_clear_button_padding = clamped;
        if (m_clear_button != nullptr)
        {
            position_clear_button();
            ensure_text_right_margin();
        }
    }
}

/**
 * @brief Handles change events to update the UI and clear button.
 * @param event The change event.
 */
auto SearchBarWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->lineEditSearch->setPlaceholderText(tr(k_search_placeholder_text));
        if (m_clear_button != nullptr)
        {
            m_clear_button->setToolTip(tr("Clear"));
        }
    }
    else
    {
        if (event != nullptr &&
            (event->type() == QEvent::StyleChange || event->type() == QEvent::FontChange))
        {
            if (m_clear_button != nullptr)
            {
                update_clear_button_icon();
                position_clear_button();
                ensure_text_right_margin();
            }
        }
    }

    QWidget::changeEvent(event);
}

/**
 * @brief Ensures clear button is repositioned when the widget resizes.
 * @param event The resize event.
 */
auto SearchBarWidget::resizeEvent(QResizeEvent* event) -> void
{
    QWidget::resizeEvent(event);
    position_clear_button();
}

/**
 * @brief Handles hover/press events for the clear button to apply hover color.
 * @param watched The watched object (expected: clear button).
 * @param event The event to handle.
 * @return false to propagate (no consumption here).
 */
auto SearchBarWidget::eventFilter(QObject* watched, QEvent* event) -> bool
{
    bool handled = false;
    if (watched == m_clear_button && m_clear_button != nullptr)
    {
        bool enter = (event->type() == QEvent::Enter);
        bool leave = (event->type() == QEvent::Leave);
        bool press = (event->type() == QEvent::MouseButtonPress);
        bool release = (event->type() == QEvent::MouseButtonRelease);

        if (enter || press)
        {
            const QSize icon_sz(m_clear_icon_px, m_clear_icon_px);
            QIcon icon_hover(UiUtils::colored_svg_icon(":/Resources/Icons/close.svg",
                                                       m_clear_icon_color_hover, icon_sz));
            m_clear_button->setIcon(icon_hover);
            m_clear_button->setIconSize(icon_sz);
        }
        else
        {
            if (leave || release)
            {
                update_clear_button_icon();
            }
        }
    }

    return handled;
}

/**
 * @brief Sets the available search fields in the combo box.
 * @param fields The list of search fields.
 */
auto SearchBarWidget::set_search_fields(const QStringList& fields) -> void
{
    ui->comboBoxSearchArea->clear();
    ui->comboBoxSearchArea->addItems(fields);
}

/**
 * @brief Sets the placeholder text for the search line edit.
 * @param text The placeholder text.
 */
auto SearchBarWidget::set_search_placeholder(const QString& text) -> void
{
    ui->lineEditSearch->setPlaceholderText(text);
}

/**
 * @brief Returns the current search text.
 * @return The search text.
 */
auto SearchBarWidget::get_search_text() const -> QString
{
    QString result = ui->lineEditSearch->text();
    return result;
}

/**
 * @brief Returns the currently selected search field.
 * @return The search field.
 */
auto SearchBarWidget::get_search_field() const -> QString
{
    QString result = ui->comboBoxSearchArea->currentText();
    return result;
}

/**
 * @brief Returns whether regex is enabled.
 * @return True if regex is enabled, false otherwise.
 */
auto SearchBarWidget::get_use_regex() const -> bool
{
    bool result = ui->checkBoxRegEx->isChecked();
    return result;
}

/**
 * @brief Returns whether live search is enabled.
 * @return True if live search is enabled, false otherwise.
 */
auto SearchBarWidget::get_use_live_search() const -> bool
{
    bool result = ui->checkBoxLiveSearch->isChecked();
    return result;
}

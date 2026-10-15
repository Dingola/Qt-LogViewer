#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFontMetrics>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QSize>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

#include "QtWidgetsCommonLib/Utils/UiUtils.h"
#include "ui_SearchBarWidget.h"

using QtWidgetsCommonLib::UiUtils;

namespace
{
constexpr auto k_search_placeholder_text = QT_TRANSLATE_NOOP("SearchBarWidget", "Search...");
constexpr auto k_all_fields_text = QT_TRANSLATE_NOOP("SearchBarWidget", "All Fields");
constexpr auto k_message_text = QT_TRANSLATE_NOOP("SearchBarWidget", "Message");
constexpr auto k_level_text = QT_TRANSLATE_NOOP("SearchBarWidget", "Level");
constexpr auto k_app_name_text = QT_TRANSLATE_NOOP("SearchBarWidget", "AppName");
/** @brief Quiet period after typing before a live search is executed. */
constexpr int k_live_search_delay_ms = 300;
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

    m_live_search_timer = new QTimer(this);
    m_live_search_timer->setSingleShot(true);
    m_live_search_timer->setInterval(k_live_search_delay_ms);
    connect(m_live_search_timer, &QTimer::timeout, this, &SearchBarWidget::request_search_if_valid);

    qRegisterMetaType<SearchField>("SearchField");
    populate_default_search_fields();

    ui->lineEditSearch->setPlaceholderText(tr(k_search_placeholder_text));
    QIcon search_icon(
        UiUtils::colored_svg_icon(":/Resources/Icons/magnifying-glass.svg", QColor("#42a5f5")));
    ui->lineEditSearch->addAction(search_icon, QLineEdit::LeadingPosition);

    // Save original margins (may be zero).
    m_original_text_margins = ui->lineEditSearch->textMargins();

    // Create clear button and hook behavior.
    init_clear_button();
    update_clear_button_visibility(ui->lineEditSearch->text());

    connect(ui->pushButtonSearch, &QPushButton::clicked, this,
            &SearchBarWidget::request_search_if_valid);
    connect(ui->lineEditSearch, &QLineEdit::returnPressed, this,
            &SearchBarWidget::request_search_if_valid);
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [this](const QString& new_text) {
        emit search_text_changed(get_search_text());
        update_clear_button_visibility(new_text);
        const bool valid = validate_search_pattern();
        if (get_use_live_search() && valid)
        {
            m_live_search_timer->start();
        }
        else
        {
            m_live_search_timer->stop();
        }
    });
    connect(ui->comboBoxSearchArea, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [this](int) {
                m_live_search_timer->stop();
                emit search_field_changed(get_search_field());
            });
    connect(ui->checkBoxRegEx, &QCheckBox::toggled, this, [this] {
        m_live_search_timer->stop();
        validate_search_pattern();
        emit regex_toggled(get_use_regex());
    });
    connect(ui->checkBoxLiveSearch, &QCheckBox::toggled, this, [this] {
        emit live_search_toggled(get_use_live_search());
        if (get_use_live_search())
        {
            request_search_if_valid();
        }
        else
        {
            m_live_search_timer->stop();
        }
    });
    validate_search_pattern();
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
 * @brief Populates the default translated search fields.
 */
auto SearchBarWidget::populate_default_search_fields() -> void
{
    QVector<QPair<QString, SearchField>> fields;
    fields.append({tr(k_all_fields_text), SearchField::AllFields});
    fields.append({tr(k_message_text), SearchField::Message});
    fields.append({tr(k_level_text), SearchField::Level});
    fields.append({tr(k_app_name_text), SearchField::AppName});

    m_use_default_search_fields = true;
    set_search_fields_internal(fields);
}

/**
 * @brief Applies the given search fields while preserving selection if possible.
 * @param fields The fields to apply.
 */
auto SearchBarWidget::set_search_fields_internal(const QVector<QPair<QString, SearchField>>& fields)
    -> void
{
    SearchField selected_field = SearchField::AllFields;

    if (ui->comboBoxSearchArea->count() > 0)
    {
        selected_field = get_search_field();
    }

    m_search_fields = fields;

    const bool previous = ui->comboBoxSearchArea->blockSignals(true);
    ui->comboBoxSearchArea->clear();

    int selected_index = -1;
    int index = 0;

    for (const auto& field: fields)
    {
        ui->comboBoxSearchArea->addItem(field.first, QVariant::fromValue(field.second));

        if ((selected_index < 0) && (field.second == selected_field))
        {
            selected_index = index;
        }

        ++index;
    }

    if (selected_index < 0 && ui->comboBoxSearchArea->count() > 0)
    {
        selected_index = 0;
    }

    if (selected_index >= 0)
    {
        ui->comboBoxSearchArea->setCurrentIndex(selected_index);
    }

    ui->comboBoxSearchArea->blockSignals(previous);
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
 * @brief Validates the current regex and updates the inline validation presentation.
 * @return True when searching with the current controls is valid.
 */
auto SearchBarWidget::validate_search_pattern() -> bool
{
    bool valid = true;
    QString error_message;

    if (get_use_regex() && !get_search_text().isEmpty())
    {
        const QRegularExpression expression(get_search_text());
        valid = expression.isValid();
        if (!valid)
        {
            const qsizetype error_offset = expression.patternErrorOffset();
            if (error_offset >= 0)
            {
                error_message =
                    tr("Invalid regular expression at position %1: %2")
                        .arg(QLocale().toString(error_offset + 1), expression.errorString());
            }
            else
            {
                error_message = tr("Invalid regular expression: %1").arg(expression.errorString());
            }
        }
    }

    const QString validation_state = valid ? QStringLiteral("valid") : QStringLiteral("invalid");
    if (ui->lineEditSearch->property("validationState").toString() != validation_state)
    {
        ui->lineEditSearch->setProperty("validationState", validation_state);
        ui->lineEditSearch->style()->unpolish(ui->lineEditSearch);
        ui->lineEditSearch->style()->polish(ui->lineEditSearch);
    }
    ui->lineEditSearch->setToolTip(error_message);
    ui->lineEditSearch->setAccessibleDescription(error_message);
    ui->labelRegexError->setText(error_message);
    ui->labelRegexError->setToolTip(error_message);
    ui->labelRegexError->setVisible(!valid);
    ui->pushButtonSearch->setEnabled(valid);
    m_search_pattern_valid = valid;

    return valid;
}

/** @brief Emits a search request only when the current pattern is valid. */
auto SearchBarWidget::request_search_if_valid() -> void
{
    m_live_search_timer->stop();
    const bool valid = validate_search_pattern();
    if (valid)
    {
        emit search_requested(get_search_text(), get_search_field(), get_use_regex());
    }
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

        if (m_use_default_search_fields)
        {
            populate_default_search_fields();
        }

        if (m_clear_button != nullptr)
        {
            m_clear_button->setToolTip(tr("Clear"));
        }
        validate_search_pattern();
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
auto SearchBarWidget::set_search_fields(const QVector<QPair<QString, SearchField>>& fields) -> void
{
    m_use_default_search_fields = false;
    set_search_fields_internal(fields);
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
auto SearchBarWidget::get_search_field() const -> SearchField
{
    SearchField result = SearchField::AllFields;
    const QVariant data = ui->comboBoxSearchArea->currentData();

    if (data.isValid() && data.canConvert<SearchField>())
    {
        result = data.value<SearchField>();
    }

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
 *
 * Enabled live searches are debounced while text is being entered.
 *
 * @return True if live search is enabled, false otherwise.
 */
auto SearchBarWidget::get_use_live_search() const -> bool
{
    bool result = ui->checkBoxLiveSearch->isChecked();
    return result;
}

/** @brief Focuses the search input and selects its current text. */
auto SearchBarWidget::focus_search_input() -> void
{
    ui->lineEditSearch->setFocus(Qt::ShortcutFocusReason);
    ui->lineEditSearch->selectAll();
}

/** @brief Clears the search text and immediately applies the empty search. */
auto SearchBarWidget::clear_search() -> void
{
    m_live_search_timer->stop();
    ui->lineEditSearch->clear();
    request_search_if_valid();
}

/**
 * @brief Reports whether the current text is a valid search pattern.
 * @return True for plain text, disabled regex mode, or a valid regular expression.
 */
auto SearchBarWidget::has_valid_search_pattern() const noexcept -> bool
{
    const bool result = m_search_pattern_valid;
    return result;
}

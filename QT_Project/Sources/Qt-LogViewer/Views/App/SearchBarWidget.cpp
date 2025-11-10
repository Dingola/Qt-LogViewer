#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

#include <QString>

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

    // Connect search button and text changes
    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            [=] { style()->polish(ui->lineEditSearch); });
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, [this] {
        emit search_requested(get_search_text(), get_search_field(), get_use_regex());
    });
    connect(ui->lineEditSearch, &QLineEdit::returnPressed, this, [this] {
        emit search_requested(get_search_text(), get_search_field(), get_use_regex());
    });
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [this] {
        emit search_text_changed(get_search_text());
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
 * @brief Sets the available search fields in the combo box.
 *
 * @param fields The list of search fields.
 */
auto SearchBarWidget::set_search_fields(const QStringList& fields) -> void
{
    ui->comboBoxSearchArea->clear();
    ui->comboBoxSearchArea->addItems(fields);
}

/**
 * @brief Sets the placeholder text for the search line edit.
 *
 * @param text The placeholder text.
 */
auto SearchBarWidget::set_search_placeholder(const QString& text) -> void
{
    ui->lineEditSearch->setPlaceholderText(text);
}

/**
 * @brief Returns the current search text.
 *
 * @return The search text.
 */
auto SearchBarWidget::get_search_text() const -> QString
{
    return ui->lineEditSearch->text();
}

/**
 * @brief Returns the currently selected search field.
 *
 * @return The search field.
 */
auto SearchBarWidget::get_search_field() const -> QString
{
    return ui->comboBoxSearchArea->currentText();
}

/**
 * @brief Returns whether regex is enabled.
 *
 * @return True if regex is enabled, false otherwise.
 */
auto SearchBarWidget::get_use_regex() const -> bool
{
    return ui->checkBoxRegEx->isChecked();
}

/**
 * @brief Returns whether live search is enabled.
 *
 * @return True if live search is enabled, false otherwise.
 */
auto SearchBarWidget::get_use_live_search() const -> bool
{
    return ui->checkBoxLiveSearch->isChecked();
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto SearchBarWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->lineEditSearch->setPlaceholderText(tr(k_search_placeholder_text));
    }

    QWidget::changeEvent(event);
}
/**
 * @file LogImportWidget.cpp
 * @brief Implements parsing-profile selection and bounded log preview.
 */

#include "Qt-LogViewer/Views/App/LogImportWidget.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMetaType>
#include <QModelIndex>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSet>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStringList>
#include <QStyle>
#include <QTableView>
#include <QTimer>
#include <QVariant>
#include <algorithm>
#include <utility>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Services/LogParseOutcome.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "QtWidgetsCommonLib/Layouts/FlowLayout.h"
#include "ui_LogImportWidget.h"

namespace
{
constexpr int ProfileNameMaximumWidth = 520;
constexpr int ProfileFormatMaximumWidth = 960;

/**
 * @brief Applies the complete wrapped text height for a label's current width.
 * @param label Label whose height is synchronized.
 */
auto update_wrapped_hint_height(QLabel& label) -> void
{
    constexpr auto UpdatingHeightProperty = "_logViewerUpdatingWrappedHeight";
    if (!label.property(UpdatingHeightProperty).toBool())
    {
        label.setProperty(UpdatingHeightProperty, true);
        const int width = std::min(label.width(), label.maximumWidth());
        label.setMinimumHeight(0);
        label.setMaximumHeight(QWIDGETSIZE_MAX);
        if (width > 0)
        {
            const int required_height = label.heightForWidth(width);
            if (required_height > 0)
            {
                label.setFixedHeight(required_height);
            }
        }
        label.setProperty(UpdatingHeightProperty, false);
    }
}

/**
 * @brief Synchronizes a wrapped hint's height whenever Qt changes its width.
 */
class WrappedHintHeightFilter final: public QObject
{
    public:
        /**
         * @brief Constructs a filter owned by the observed label.
         * @param parent Label that owns this filter.
         */
        explicit WrappedHintHeightFilter(QObject* parent): QObject(parent) {}

    protected:
        /**
         * @brief Updates the complete text height after resize and show events.
         * @param watched Object receiving the event.
         * @param event Event delivered by Qt.
         * @return Result of the base event-filter implementation.
         */
        bool eventFilter(QObject* watched, QEvent* event) override
        {
            if (event->type() == QEvent::Resize || event->type() == QEvent::Show)
            {
                auto* label = qobject_cast<QLabel*>(watched);
                if (label != nullptr)
                {
                    update_wrapped_hint_height(*label);
                }
            }
            return QObject::eventFilter(watched, event);
        }
};

/**
 * @brief Supplies Qt's width-dependent height contract for the library flow layout.
 */
class ImportFieldLayout final: public QtWidgetsCommonLib::FlowLayout
{
    public:
        /** @brief Constructs the left-aligned field layout with compact spacing. */
        ImportFieldLayout(): FlowLayout(nullptr, 0, 6, 4, RowAlignment::Left)
        {
            setContentsMargins(8, 4, 0, 4);
        }

        /** @brief Reports width-dependent height support. @return Always true. */
        bool hasHeightForWidth() const override
        {
            return true;
        }

        /**
         * @brief Measures all wrapped rows using the library's line-break rule.
         * @param width Available outer layout width.
         * @return Required height including top and bottom margins.
         */
        int heightForWidth(int width) const override
        {
            const QMargins margins = contentsMargins();
            int x = margins.left();
            int height = margins.top() + margins.bottom();
            int row_height = 0;
            for (int index = 0; index < count(); ++index)
            {
                const QSize size = itemAt(index)->sizeHint();
                if (x + size.width() > width - 1 && row_height > 0)
                {
                    height += row_height + vertical_spacing();
                    x = margins.left();
                    row_height = 0;
                }
                x += size.width() + horizontal_spacing();
                row_height = std::max(row_height, size.height());
            }
            return height + row_height;
        }

        /**
         * @brief Prevents compression below the wrapped row height.
         * @param width Available outer layout width.
         * @return Required minimum height for that width.
         */
        int minimumHeightForWidth(int width) const override
        {
            return heightForWidth(width);
        }
};

enum RawRecordColumns
{
    StatusColumn,
    LineColumn,
    RawRecordTextColumn,
    RawRecordColumnCount
};

enum ParsedFieldColumn
{
    FieldColumn,
    ValueColumn,
    ParsedFieldColumnCount
};

const QString PreviewRemainderField{QStringLiteral("_log_viewer_preview_remainder")};

/**
 * @brief Creates a non-editable preview-table item.
 * @param text Text displayed by the item.
 * @return Newly allocated item owned by the preview model after insertion.
 */
[[nodiscard]] auto create_preview_item(const QString& text) -> QStandardItem*
{
    auto* item = new QStandardItem(text);
    item->setEditable(false);
    item->setToolTip(text);
    return item;
}

/**
 * @brief Converts a parsed QVariant into readable preview text.
 * @param value Parsed value supplied by QtRecordParser.
 * @return Human-readable value preserving date-time precision.
 */
[[nodiscard]] auto format_preview_value(const QVariant& value) -> QString
{
    QString text;

    if (value.metaType().id() == QMetaType::QDateTime)
    {
        text = value.toDateTime().toString(Qt::ISODateWithMs);
    }
    else
    {
        text = value.toString();
    }

    return text;
}

/**
 * @brief Checks whether a profile supplies the field required by the LogViewer model.
 * @param profile Profile whose placeholder-based format is inspected.
 * @return True when the format contains the required message placeholder.
 */
[[nodiscard]] auto contains_required_message_field(const LogParsingProfile& profile) -> bool
{
    return profile.get_configuration().format.contains(QStringLiteral("{message}"));
}

/**
 * @brief Checks whether a profile has the display name required for persistence.
 * @param profile Profile whose trimmed display name is inspected.
 * @return True when the profile has a non-empty display name.
 */
[[nodiscard]] auto contains_profile_name(const LogParsingProfile& profile) -> bool
{
    return !profile.get_name().trimmed().isEmpty();
}

/**
 * @brief Removes an unfinished whitespace-only suffix following the final placeholder.
 * @param profile User-selected profile to adapt for bounded preview parsing.
 * @return Profile copy without trailing whitespace, or the unchanged profile when not applicable.
 */
[[nodiscard]] auto create_trimmed_preview_profile(const LogParsingProfile& profile)
    -> LogParsingProfile
{
    LogParsingProfile preview_profile = profile;
    QtRecordParser::ParserConfiguration configuration = profile.get_configuration();
    const QRegularExpression placeholder_pattern(QStringLiteral(R"(\{[A-Za-z_][A-Za-z0-9_]*\})"));
    QRegularExpressionMatchIterator placeholders =
        placeholder_pattern.globalMatch(configuration.format);
    qsizetype final_placeholder_end = -1;

    while (placeholders.hasNext())
    {
        final_placeholder_end = placeholders.next().capturedEnd();
    }

    if (final_placeholder_end >= 0)
    {
        const QString trailing_literal = configuration.format.mid(final_placeholder_end);

        if (!trailing_literal.isEmpty() && trailing_literal.trimmed().isEmpty())
        {
            configuration.format.truncate(final_placeholder_end);
            preview_profile = profile.with_configuration(std::move(configuration));
        }
    }

    return preview_profile;
}

/**
 * @brief Creates a preview-only profile accepting content after the current format prefix.
 * @param profile Profile whose current format is treated as a record prefix.
 * @return Profile copy containing an internal trailing remainder placeholder.
 */
[[nodiscard]] auto create_prefix_preview_profile(const LogParsingProfile& profile)
    -> LogParsingProfile
{
    QtRecordParser::ParserConfiguration configuration = profile.get_configuration();

    if (!configuration.format.contains(QStringLiteral("{%1}").arg(PreviewRemainderField)))
    {
        configuration.format.append(QStringLiteral("{%1}").arg(PreviewRemainderField));
    }

    return profile.with_configuration(std::move(configuration));
}

/**
 * @brief Checks whether at least one generic parser result succeeded.
 * @param outcomes Preview outcomes in source order.
 * @return True when any record matched the evaluated parser configuration.
 */
[[nodiscard]] auto contains_successful_parse(const QVector<LogParseOutcome>& outcomes) -> bool
{
    bool successful_parse_found = false;

    for (const LogParseOutcome& outcome: outcomes)
    {
        if (!successful_parse_found && outcome.parse_result.succeeded())
        {
            successful_parse_found = true;
        }
    }

    return successful_parse_found;
}

/**
 * @brief Returns the buttons that insert fields into the profile format.
 * @param ui Generated user interface containing the field buttons.
 * @return Field buttons in their visual order.
 */
[[nodiscard]] auto get_field_buttons(Ui::LogImportWidget& ui) -> QList<QPushButton*>
{
    return {
        ui.pushButtonInsertTimestamp, ui.pushButtonInsertLevel,    ui.pushButtonInsertMessage,
        ui.pushButtonInsertAppName,   ui.pushButtonInsertFile,     ui.pushButtonInsertLine,
        ui.pushButtonInsertFunction,  ui.pushButtonInsertCategory, ui.pushButtonInsertCustomField};
}

/**
 * @brief Limits a hint while Qt calculates its wrapped height.
 * @param label Hint label whose current text and stylesheet determine the preferred width.
 * @param maximum_width Maximum width allowed by the surrounding form.
 */
auto update_hint_width(QLabel& label, int maximum_width = QWIDGETSIZE_MAX) -> void
{
    label.ensurePolished();
    label.setMaximumWidth(QWIDGETSIZE_MAX);
    label.setWordWrap(false);
    const int natural_width = label.sizeHint().width();
    label.setWordWrap(true);
    QSizePolicy size_policy = label.sizePolicy();
    size_policy.setHeightForWidth(true);
    label.setSizePolicy(size_policy);
    label.setMaximumWidth(std::min(natural_width, maximum_width));
    update_wrapped_hint_height(label);
    label.updateGeometry();
}

/**
 * @brief Returns the maximum width from the format label through the format editor.
 * @param ui Generated interface containing the form controls.
 * @return Maximum width shared by form hints and import actions.
 */
[[nodiscard]] auto get_import_form_maximum_width(const Ui::LogImportWidget& ui) -> int
{
    const int label_width =
        std::max({ui.labelProfile->sizeHint().width(), ui.labelProfileName->sizeHint().width(),
                  ui.labelFormat->sizeHint().width()});
    return label_width + std::max(0, ui.gridLayout->horizontalSpacing()) +
           ProfileFormatMaximumWidth;
}

/**
 * @brief Aligns form hints and actions with the bounded format row.
 * @param ui Generated interface whose width constraints are updated.
 */
auto update_import_form_widths(Ui::LogImportWidget& ui) -> void
{
    const int maximum_width = get_import_form_maximum_width(ui);
    update_hint_width(*ui.labelFieldHelp, maximum_width);
    update_hint_width(*ui.labelValidation, maximum_width);

    const int action_spacing = std::max(0, ui.horizontalLayoutImportActions->spacing());
    const int action_width = ui.pushButtonCancel->sizeHint().width() +
                             ui.pushButtonImport->sizeHint().width() + action_spacing;
    ui.horizontalSpacerImportActionsLeading->changeSize(
        std::max(0, maximum_width - action_width), 0, QSizePolicy::Preferred, QSizePolicy::Minimum);
    ui.horizontalLayoutImportActions->invalidate();
}

/**
 * @brief Reparents all widgets contained in a layout item.
 * @param item Layout item whose widget tree is moved.
 * @param parent New widget parent for the contained controls.
 */
auto reparent_layout_widgets(QLayoutItem& item, QWidget& parent) -> void
{
    QWidget* widget = item.widget();
    if (widget != nullptr)
    {
        widget->setParent(&parent);
    }

    QLayout* layout = item.layout();
    if (layout != nullptr)
    {
        for (int index = 0; index < layout->count(); ++index)
        {
            reparent_layout_widgets(*layout->itemAt(index), parent);
        }
    }
}

/**
 * @brief Wraps the import form in a scroll area to avoid clipping on small screens.
 * @param ui Generated interface whose import content is placed in the scroll area.
 */
auto configure_scrollable_content(Ui::LogImportWidget& ui) -> void
{
    auto* content = new QWidget();
    content->setObjectName(QStringLiteral("importScrollContent"));
    auto* content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(0, 0, 0, 0);
    while (ui.verticalLayoutMain->count() > 0)
    {
        QLayoutItem* item = ui.verticalLayoutMain->takeAt(0);
        reparent_layout_widgets(*item, *content);
        content_layout->addItem(item);
    }
    content_layout->setStretch(2, 1);

    auto* scroll_area = new QScrollArea();
    scroll_area->setObjectName(QStringLiteral("importScrollArea"));
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidgetResizable(true);
    scroll_area->setWidget(content);
    content->setAutoFillBackground(false);
    ui.verticalLayoutMain->insertWidget(0, scroll_area, 1);
}
}  // namespace

/**
 * @brief Constructs a log import widget for one file.
 * @param file_path File whose records are previewed and later imported.
 * @param settings Application settings containing reusable parsing profiles.
 * @param controller Controller used for bounded, side-effect-free preview parsing.
 * @param parent Parent widget responsible for ownership.
 */
LogImportWidget::LogImportWidget(QString file_path, LogViewerSettings& settings,
                                 const LogViewerController& controller, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LogImportWidget),
      m_settings(settings),
      m_controller(controller),
      m_raw_records_model(new QStandardItemModel(this)),
      m_parsed_fields_model(new QStandardItemModel(this)),
      m_preview_timer(new QTimer(this)),
      m_file_path(std::move(file_path))
{
    ui->setupUi(this);

    ui->comboBoxProfile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->widgetProfileSelection->setMaximumWidth(ProfileNameMaximumWidth);
    ui->widgetProfileSelection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->lineEditProfileName->setMaximumWidth(ProfileNameMaximumWidth);
    ui->lineEditFormat->setMaximumWidth(ProfileFormatMaximumWidth);
    // Keep the shared profile/name column comfortably wide while allowing it to shrink.
    ui->gridLayout->addItem(
        new QSpacerItem(ProfileNameMaximumWidth, 0, QSizePolicy::Preferred, QSizePolicy::Fixed), 0,
        1);
    ui->labelProfileMode->setProperty("variant", QStringLiteral("profile-mode"));
    ui->labelProfileMode->setVisible(false);
    ui->lineEditFilePath->setText(m_file_path);
    ui->lineEditFilePath->setToolTip(m_file_path);
    ui->tableViewRawRecords->setModel(m_raw_records_model);
    ui->tableViewRawRecords->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewRawRecords->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableViewRawRecords->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableViewRawRecords->setWordWrap(false);
    ui->tableViewParsedFields->setModel(m_parsed_fields_model);
    ui->tableViewParsedFields->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewParsedFields->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableViewParsedFields->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableViewParsedFields->setWordWrap(false);

    m_raw_records_model->setColumnCount(RawRecordColumnCount);
    m_raw_records_model->setHorizontalHeaderLabels({tr("Status"), tr("Line"), tr("Raw Record")});
    m_parsed_fields_model->setColumnCount(ParsedFieldColumnCount);
    m_parsed_fields_model->setHorizontalHeaderLabels({tr("Field"), tr("Value")});

    QHeaderView* raw_header = ui->tableViewRawRecords->horizontalHeader();
    raw_header->setSectionsMovable(true);
    raw_header->setSectionResizeMode(QHeaderView::Interactive);
    raw_header->resizeSection(StatusColumn, 80);
    raw_header->resizeSection(LineColumn, 70);
    raw_header->resizeSection(RawRecordTextColumn, 520);

    QHeaderView* parsed_header = ui->tableViewParsedFields->horizontalHeader();
    parsed_header->setSectionsMovable(true);
    parsed_header->setSectionResizeMode(FieldColumn, QHeaderView::Interactive);
    parsed_header->setSectionResizeMode(ValueColumn, QHeaderView::Stretch);
    parsed_header->resizeSection(FieldColumn, 150);
    ui->splitterPreview->setSizes({520, 440});

    while (ui->gridLayoutFields->count() > 0)
    {
        delete ui->gridLayoutFields->takeAt(0);
    }
    ui->gridLayout->removeItem(ui->gridLayoutFields);
    delete ui->gridLayoutFields;
    ui->gridLayoutFields = nullptr;
    auto* field_layout = new ImportFieldLayout();
    field_layout->setObjectName(QStringLiteral("flowLayoutFields"));
    ui->gridLayout->addLayout(field_layout, 4, 1, 1, 2);
    for (QPushButton* field_button: get_field_buttons(*ui))
    {
        field_button->setProperty("variant", QStringLiteral("field-token"));
        field_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        field_layout->addWidget(field_button);
    }

    ui->pushButtonNewProfile->setProperty("variant", QStringLiteral("profile-action"));
    ui->pushButtonCancel->setProperty("variant", QStringLiteral("dialog-action"));
    ui->pushButtonCancel->setProperty("actionRole", QStringLiteral("secondary"));
    ui->pushButtonImport->setProperty("variant", QStringLiteral("dialog-action"));
    ui->pushButtonImport->setProperty("actionRole", QStringLiteral("primary"));
    ui->labelValidation->setVisible(false);
    ui->labelPreviewSummary->setVisible(false);
    ui->gridLayout->setAlignment(ui->labelFieldHelp, Qt::AlignTop);
    ui->gridLayout->setAlignment(ui->labelValidation, Qt::AlignTop);
    ui->gridLayout->setAlignment(Qt::AlignTop);
    ui->verticalLayout->setAlignment(ui->labelPreviewSummary, Qt::AlignTop);
    for (QLabel* hint: {ui->labelFieldHelp, ui->labelValidation, ui->labelPreviewSummary})
    {
        hint->installEventFilter(new WrappedHintHeightFilter(hint));
    }
    configure_scrollable_content(*ui);

    m_preview_timer->setInterval(350);
    m_preview_timer->setSingleShot(true);

    connect(ui->comboBoxProfile, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &LogImportWidget::select_profile);
    connect(ui->pushButtonNewProfile, &QPushButton::clicked, this,
            &LogImportWidget::begin_new_profile);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &LogImportWidget::cancel_requested);
    connect(ui->pushButtonImport, &QPushButton::clicked, this, &LogImportWidget::request_import);
    connect(ui->lineEditProfileName, &QLineEdit::textChanged, this,
            &LogImportWidget::update_widget_state);
    connect(ui->lineEditFormat, &QLineEdit::textChanged, this,
            &LogImportWidget::update_widget_state);
    connect(ui->spinBoxPreviewRecords, qOverload<int>(&QSpinBox::valueChanged), this,
            [this](int) { schedule_preview_refresh(); });
    connect(m_preview_timer, &QTimer::timeout, this, &LogImportWidget::refresh_preview);
    connect(ui->tableViewRawRecords->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &LogImportWidget::show_selected_outcome);
    connect(ui->pushButtonInsertTimestamp, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(LogField::Timestamp); });
    connect(ui->pushButtonInsertLevel, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(LogField::Level); });
    connect(ui->pushButtonInsertMessage, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(LogField::Message); });
    connect(ui->pushButtonInsertAppName, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(LogField::AppName); });
    connect(ui->pushButtonInsertFile, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(QStringLiteral("file")); });
    connect(ui->pushButtonInsertLine, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(QStringLiteral("line")); });
    connect(ui->pushButtonInsertFunction, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(QStringLiteral("function")); });
    connect(ui->pushButtonInsertCategory, &QPushButton::clicked, this,
            [this]() { insert_field_placeholder(QStringLiteral("category")); });
    connect(ui->pushButtonInsertCustomField, &QPushButton::clicked, this,
            &LogImportWidget::insert_custom_field_placeholder);

    load_profiles();
    update_import_form_widths(*ui);
}

/**
 * @brief Destroys the widget and its generated user interface.
 */
LogImportWidget::~LogImportWidget()
{
    delete ui;
}

/**
 * @brief Returns the file selected for this import.
 * @return File path supplied during construction.
 */
auto LogImportWidget::get_file_path() const noexcept -> const QString&
{
    return m_file_path;
}

/**
 * @brief Returns the profile confirmed by the user.
 * @return Confirmed profile after an import request, or std::nullopt beforehand.
 */
auto LogImportWidget::get_selected_profile() const -> std::optional<LogParsingProfile>
{
    return m_selected_profile;
}

/**
 * @brief Loads persisted profiles and initializes existing- or new-profile mode.
 */
auto LogImportWidget::load_profiles() -> void
{
    QString error;
    m_profiles = m_settings.get_log_parsing_profiles(&error);

    const QSignalBlocker profile_selection_blocker(ui->comboBoxProfile);
    ui->comboBoxProfile->clear();
    for (const LogParsingProfile& profile: m_profiles)
    {
        ui->comboBoxProfile->addItem(profile.get_name(),
                                     profile.get_id().toString(QUuid::WithoutBraces));
    }

    if (m_profiles.isEmpty())
    {
        begin_new_profile();
    }
    else
    {
        ui->comboBoxProfile->setCurrentIndex(0);
        select_profile(0);
    }

    if (!error.isEmpty())
    {
        set_validation_message(tr("Stored profiles could not be loaded: %1").arg(error));
    }
}

/**
 * @brief Displays the profile selected by combo-box index.
 * @param index Index in the persisted profile collection.
 */
auto LogImportWidget::select_profile(int index) -> void
{
    if (index >= 0 && index < m_profiles.size())
    {
        const LogParsingProfile& profile = m_profiles.at(index);
        m_creating_profile = false;
        ui->labelProfileMode->setVisible(false);
        ui->labelProfileName->setText(tr("Name"));
        ui->labelFormat->setText(tr("Format"));
        ui->lineEditProfileName->setReadOnly(true);
        ui->lineEditFormat->setReadOnly(true);
        ui->lineEditProfileName->setText(profile.get_name());
        ui->lineEditFormat->setText(profile.get_configuration().format);
        update_widget_state();
        refresh_preview();
    }
}

/**
 * @brief Clears the editor and enables creation of a new profile.
 */
auto LogImportWidget::begin_new_profile() -> void
{
    m_creating_profile = true;
    m_preview_contains_successful_record = false;
    ui->labelProfileMode->setVisible(true);
    ui->labelProfileName->setText(tr("Name *"));
    ui->labelFormat->setText(tr("Format *"));
    ui->comboBoxProfile->setCurrentIndex(-1);
    ui->lineEditProfileName->setReadOnly(false);
    ui->lineEditFormat->setReadOnly(false);
    ui->lineEditProfileName->clear();
    ui->lineEditFormat->clear();
    update_widget_state();
    show_raw_preview();
    ui->lineEditProfileName->setFocus();
}

/**
 * @brief Parses the configured number of records and refreshes the preview table.
 */
auto LogImportWidget::refresh_preview() -> void
{
    QString error;
    const std::optional<LogParsingProfile> profile = create_current_profile(&error);
    m_preview_matches_complete_record = false;
    m_showing_partial_preview = false;

    if (profile.has_value())
    {
        QVector<LogParseOutcome> outcomes = m_controller.preview_log_file(
            m_file_path, profile.value(), ui->spinBoxPreviewRecords->value());
        m_preview_matches_complete_record = contains_successful_parse(outcomes);

        if (!m_preview_matches_complete_record)
        {
            const LogParsingProfile trimmed_profile =
                create_trimmed_preview_profile(profile.value());

            if (trimmed_profile.get_configuration().format != profile->get_configuration().format)
            {
                outcomes = m_controller.preview_log_file(m_file_path, trimmed_profile,
                                                         ui->spinBoxPreviewRecords->value());
            }

            if (!contains_successful_parse(outcomes))
            {
                const LogParsingProfile prefix_profile =
                    create_prefix_preview_profile(trimmed_profile);
                outcomes = m_controller.preview_log_file(m_file_path, prefix_profile,
                                                         ui->spinBoxPreviewRecords->value());
            }

            m_showing_partial_preview = contains_successful_parse(outcomes);
        }

        m_showing_raw_preview = false;
        populate_preview(outcomes);

        if (!m_preview_contains_successful_record)
        {
            error =
                tr("The format did not match any preview record. It must describe the complete "
                   "record, including spaces and separators.");
        }
        else if (m_showing_partial_preview)
        {
            error =
                tr("The entered prefix is recognized, but the format does not yet describe a "
                   "complete record.");
        }
        else if (!contains_profile_name(profile.value()))
        {
            error =
                tr("The preview matches, but a profile name is required before it can be "
                   "imported.");
        }
        else if (!contains_required_message_field(profile.value()))
        {
            error =
                tr("The preview matches, but the format must contain the required {message} "
                   "field before it can be imported.");
        }
    }
    else
    {
        m_preview_contains_successful_record = false;

        if (m_creating_profile)
        {
            show_raw_preview();
        }
        else
        {
            m_preview_outcomes.clear();
            m_raw_records_model->removeRows(0, m_raw_records_model->rowCount());
            m_parsed_fields_model->removeRows(0, m_parsed_fields_model->rowCount());
            set_preview_summary(QString());
        }
    }

    set_validation_message(error);
    update_import_button(profile, error);
}

/**
 * @brief Shows raw source records while no usable profile format is available.
 */
auto LogImportWidget::show_raw_preview() -> void
{
    const LogParsingProfile raw_profile = LogParsingProfile::create_default(
        QStringLiteral("{raw_record}"), QStringLiteral("Raw preview"));
    const QVector<LogParseOutcome> outcomes =
        m_controller.preview_log_file(m_file_path, raw_profile, ui->spinBoxPreviewRecords->value());

    m_showing_raw_preview = true;
    m_showing_partial_preview = false;
    m_preview_matches_complete_record = false;
    populate_preview(outcomes);

    for (int row = 0; row < m_raw_records_model->rowCount(); ++row)
    {
        QStandardItem* status_item = m_raw_records_model->item(row, StatusColumn);

        if (status_item != nullptr)
        {
            status_item->setText(tr("Raw"));
        }
    }

    m_preview_contains_successful_record = false;
    set_preview_summary(
        tr("Showing %1 raw record(s). Enter a name and format, then refresh the preview.")
            .arg(outcomes.size()));
    update_import_button(std::nullopt,
                         tr("Enter a valid format and profile name before importing."));
}

/**
 * @brief Restarts the delayed preview refresh after an editor change.
 */
auto LogImportWidget::schedule_preview_refresh() -> void
{
    m_preview_timer->start();
}

/**
 * @brief Inserts a parser placeholder at the current format-editor cursor position.
 * @param field_id Stable or custom field identifier without braces.
 */
auto LogImportWidget::insert_field_placeholder(const QString& field_id) -> void
{
    if (m_creating_profile && !field_id.isEmpty())
    {
        ui->lineEditFormat->insert(QStringLiteral("{%1}").arg(field_id));
        ui->lineEditFormat->setFocus();
    }
}

/**
 * @brief Requests a custom field name and inserts its placeholder when valid.
 */
auto LogImportWidget::insert_custom_field_placeholder() -> void
{
    bool accepted = false;
    const QString field_id =
        QInputDialog::getText(this, tr("Custom Field"), tr("Field identifier:"), QLineEdit::Normal,
                              QString(), &accepted)
            .trimmed();
    const QRegularExpression identifier_pattern(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));

    if (accepted && identifier_pattern.match(field_id).hasMatch())
    {
        insert_field_placeholder(field_id);
    }
    else if (accepted)
    {
        set_validation_message(
            tr("Field identifiers may contain letters, numbers and underscores and must not "
               "start with a number."));
    }
}

/**
 * @brief Validates and stores the selected profile before requesting the import.
 */
auto LogImportWidget::request_import() -> void
{
    QString error;
    std::optional<LogParsingProfile> profile = create_current_profile(&error);

    if (profile.has_value() && !contains_profile_name(profile.value()))
    {
        error = tr("Enter a profile name.");
        profile.reset();
    }

    if (profile.has_value() && !contains_required_message_field(profile.value()))
    {
        error = tr("The format must contain the required {message} field.");
        profile.reset();
    }

    if (profile.has_value() && !m_preview_contains_successful_record)
    {
        error = tr("Refresh the preview and verify that the format matches at least one record.");
        profile.reset();
    }

    if (profile.has_value() && !m_preview_matches_complete_record)
    {
        error = tr("Complete the format before importing.");
        profile.reset();
    }

    if (profile.has_value() && m_creating_profile)
    {
        QVector<LogParsingProfile> profiles = m_profiles;
        profiles.append(profile.value());

        if (!m_settings.set_log_parsing_profiles(profiles))
        {
            error = tr("The new profile could not be saved.");
            profile.reset();
        }
    }

    if (profile.has_value())
    {
        m_selected_profile = std::move(profile);
        emit import_requested();
    }
    else
    {
        set_validation_message(error);
        update_import_button(profile, error);
    }
}

/**
 * @brief Updates editor state, validation text and import-button availability.
 */
auto LogImportWidget::update_widget_state() -> void
{
    QString error;
    const std::optional<LogParsingProfile> profile = create_current_profile(&error);
    m_preview_contains_successful_record = false;
    m_preview_matches_complete_record = false;
    m_showing_partial_preview = false;

    if (profile.has_value())
    {
        if (m_creating_profile && !contains_profile_name(profile.value()))
        {
            error = tr("The preview can be evaluated, but a profile name is required for import.");
        }
        else if (!contains_required_message_field(profile.value()))
        {
            error = tr("The preview can be evaluated, but {message} is required for import.");
        }
        else
        {
            error = tr("Refresh the preview to validate the changed profile.");
        }
    }

    for (QPushButton* field_button: get_field_buttons(*ui))
    {
        field_button->setEnabled(m_creating_profile);
    }

    set_validation_message(error);
    update_import_button(profile, error);

    if (m_creating_profile)
    {
        schedule_preview_refresh();
    }
}

/**
 * @brief Displays or hides the current profile-validation message.
 * @param message Message to display, or an empty string to hide the hint.
 */
auto LogImportWidget::set_validation_message(const QString& message) -> void
{
    ui->labelValidation->setText(message);
    ui->labelValidation->setVisible(!message.isEmpty());
    update_import_form_widths(*ui);
}

/**
 * @brief Displays or hides the current preview summary.
 * @param message Summary to display, or an empty string to hide the hint.
 */
auto LogImportWidget::set_preview_summary(const QString& message) -> void
{
    ui->labelPreviewSummary->setText(message);
    ui->labelPreviewSummary->setVisible(!message.isEmpty());
    update_hint_width(*ui->labelPreviewSummary);
}

/**
 * @brief Updates import availability and explains a disabled state through a tooltip.
 * @param profile Current valid preview profile, or std::nullopt when none is available.
 * @param unavailable_reason Current parser or preview validation message.
 */
auto LogImportWidget::update_import_button(const std::optional<LogParsingProfile>& profile,
                                           const QString& unavailable_reason) -> void
{
    QStringList requirements;

    if (!profile.has_value())
    {
        requirements.append(unavailable_reason.isEmpty()
                                ? tr("Select or create a valid parsing profile.")
                                : unavailable_reason);
    }
    else
    {
        if (m_creating_profile && !contains_profile_name(profile.value()))
        {
            requirements.append(tr("Enter a profile name."));
        }

        if (!contains_required_message_field(profile.value()))
        {
            requirements.append(tr("Add the required {message} field."));
        }

        if (!m_preview_contains_successful_record)
        {
            requirements.append(unavailable_reason.isEmpty()
                                    ? tr("Match at least one preview record.")
                                    : unavailable_reason);
        }
        else if (!m_preview_matches_complete_record)
        {
            requirements.append(tr("Complete the format so it matches the entire record."));
        }
    }

    ui->pushButtonImport->setEnabled(requirements.isEmpty());
    ui->pushButtonImport->setToolTip(requirements.join(QLatin1Char('\n')));
    const bool import_ready = requirements.isEmpty();
    if (ui->labelPreviewSummary->property("importReady").toBool() != import_ready)
    {
        ui->labelPreviewSummary->setProperty("importReady", import_ready);
        ui->labelPreviewSummary->style()->unpolish(ui->labelPreviewSummary);
        ui->labelPreviewSummary->style()->polish(ui->labelPreviewSummary);
        ui->labelPreviewSummary->update();
    }
}

/**
 * @brief Creates the profile represented by the current controls.
 * @param error_message Optional destination for parser validation errors.
 * @return Current profile, or std::nullopt when the controls do not describe a valid one.
 */
auto LogImportWidget::create_current_profile(QString* error_message) const
    -> std::optional<LogParsingProfile>
{
    QString error;
    std::optional<LogParsingProfile> profile;

    if (m_creating_profile)
    {
        const QString name = ui->lineEditProfileName->text().trimmed();
        const QString format = ui->lineEditFormat->text();

        profile = LogParsingProfile::create_default(format, name);
    }
    else
    {
        const int index = ui->comboBoxProfile->currentIndex();
        if (index >= 0 && index < m_profiles.size())
        {
            profile = m_profiles.at(index);
        }
        else
        {
            error = tr("Select a parsing profile.");
        }
    }

    if (profile.has_value() && !profile->is_valid())
    {
        error = profile->get_validation_error();
        profile.reset();
    }

    if (error_message != nullptr)
    {
        *error_message = error;
    }

    return profile;
}

/**
 * @brief Replaces the raw-record list and retains its corresponding parse outcomes.
 * @param outcomes Successful and failed parse outcomes in source order.
 */
auto LogImportWidget::populate_preview(const QVector<LogParseOutcome>& outcomes) -> void
{
    m_preview_outcomes = outcomes;
    m_raw_records_model->removeRows(0, m_raw_records_model->rowCount());
    m_parsed_fields_model->removeRows(0, m_parsed_fields_model->rowCount());
    qsizetype successful_count = 0;

    for (const LogParseOutcome& outcome: outcomes)
    {
        QList<QStandardItem*> row;

        if (outcome.parse_result.succeeded())
        {
            ++successful_count;
        }

        const QString line_number =
            outcome.line_number >= 0 ? QString::number(outcome.line_number) : QString();

        QString status = tr("Failed");

        if (outcome.parse_result.succeeded())
        {
            status = m_showing_partial_preview ? tr("Partial") : tr("Parsed");
        }

        row.append(create_preview_item(status));
        row.append(create_preview_item(line_number));
        row.append(create_preview_item(outcome.raw_record));
        m_raw_records_model->appendRow(row);
    }

    const qsizetype failed_count = outcomes.size() - successful_count;
    m_preview_contains_successful_record = successful_count > 0;

    if (outcomes.isEmpty())
    {
        const bool file_available = QFileInfo(m_file_path).isReadable();
        set_preview_summary(file_available ? tr("No non-empty records are available for preview.")
                                           : tr("The log file is not readable."));
    }
    else
    {
        const QString summary =
            m_showing_partial_preview
                ? tr("%1 of %2 records recognized as a partial format; %3 failed.")
                      .arg(successful_count)
                      .arg(outcomes.size())
                      .arg(failed_count)
                : tr("%1 of %2 records parsed successfully; %3 failed.")
                      .arg(successful_count)
                      .arg(outcomes.size())
                      .arg(failed_count);
        set_preview_summary(summary);
    }

    ui->tableViewRawRecords->resizeColumnToContents(RawRecordTextColumn);

    if (!outcomes.isEmpty())
    {
        ui->tableViewRawRecords->setCurrentIndex(
            m_raw_records_model->index(0, RawRecordTextColumn));
    }
}

/**
 * @brief Displays parsed values or failure details for one selected raw record.
 * @param current Current row in the raw-record table.
 */
auto LogImportWidget::show_selected_outcome(const QModelIndex& current) -> void
{
    m_parsed_fields_model->removeRows(0, m_parsed_fields_model->rowCount());

    if (current.isValid() && current.row() >= 0 && current.row() < m_preview_outcomes.size())
    {
        const LogParseOutcome& outcome = m_preview_outcomes.at(current.row());

        auto append_field = [this](const QString& field, const QString& value) {
            m_parsed_fields_model->appendRow(
                {create_preview_item(field), create_preview_item(value)});
        };

        if (m_showing_raw_preview)
        {
            append_field(tr("Preview"),
                         tr("Build a format with the fields above to parse this record."));
        }
        else if (!outcome.parse_result.succeeded())
        {
            if (!outcome.parse_result.error_field.isEmpty())
            {
                append_field(tr("Field"), outcome.parse_result.error_field);
            }

            append_field(tr("Error"), outcome.parse_result.error_message);
        }
        else
        {
            const QVector<QPair<QString, QString>> standard_fields{
                {LogField::Timestamp, tr("Timestamp")},
                {LogField::Level, tr("Level")},
                {LogField::Message, tr("Message")},
                {LogField::AppName, tr("Application")},
                {QStringLiteral("file"), tr("File")},
                {QStringLiteral("line"), tr("Line")},
                {QStringLiteral("function"), tr("Function")},
                {QStringLiteral("category"), tr("Category")}};
            QSet<QString> displayed_fields;

            for (const auto& standard_field: standard_fields)
            {
                displayed_fields.insert(standard_field.first);
                append_field(
                    standard_field.second,
                    format_preview_value(outcome.parse_result.record.value(standard_field.first)));
            }

            for (auto iterator = outcome.parse_result.record.values.cbegin();
                 iterator != outcome.parse_result.record.values.cend(); ++iterator)
            {
                if (!displayed_fields.contains(iterator.key()) &&
                    iterator.key() != PreviewRemainderField)
                {
                    append_field(iterator.key(), format_preview_value(iterator.value()));
                }
            }
        }
    }

    ui->tableViewParsedFields->resizeColumnToContents(ValueColumn);
}

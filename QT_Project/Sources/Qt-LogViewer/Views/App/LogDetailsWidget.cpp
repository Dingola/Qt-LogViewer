/**
 * @file LogDetailsWidget.cpp
 * @brief Implements the structured inspector for one selected log entry.
 */

#include "Qt-LogViewer/Views/App/LogDetailsWidget.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QLocale>
#include <QMenu>
#include <QMetaType>
#include <QPointer>
#include <QResizeEvent>
#include <QSet>
#include <QShortcut>
#include <QStyle>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolButton>
#include <algorithm>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogLevelMapping.h"
#include "Qt-LogViewer/Services/DesktopServices.h"
#include "Qt-LogViewer/Views/Shared/EmptyStateWidget.h"
#include "QtWidgetsCommonLib/Utils/UiUtils.h"
#include "ui_LogDetailsWidget.h"

namespace
{
constexpr auto k_copy_icon_path = ":/Resources/Icons/copy.svg";
constexpr auto k_copy_confirmed_icon_path = ":/Resources/Icons/check-light.svg";
constexpr auto k_open_folder_icon_path = ":/Resources/Icons/folder-open-external.svg";
constexpr int k_copy_feedback_duration_ms = 900;
constexpr int k_compact_details_width = 840;

/**
 * @brief Returns the field identifiers already represented by dedicated controls.
 * @return Built-in field identifier set.
 */
[[nodiscard]] auto dedicated_field_ids() -> const QSet<QString>&
{
    static const QSet<QString> field_ids{LogField::Timestamp, LogField::Level,
                                         LogField::Message,   LogField::AppName,
                                         LogField::FilePath,  LogField::InsertionOrder};
    return field_ids;
}
}  // namespace

/**
 * @brief Constructs an initially empty log-details inspector.
 * @param parent Parent widget.
 */
LogDetailsWidget::LogDetailsWidget(QWidget* parent): QWidget(parent), ui(new Ui::LogDetailsWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->tableWidgetFields->verticalHeader()->hide();
    ui->tableWidgetFields->horizontalHeader()->setSectionResizeMode(0,
                                                                    QHeaderView::ResizeToContents);
    ui->tableWidgetFields->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidgetFields->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->detailsSplitter->setStretchFactor(0, 3);
    ui->detailsSplitter->setStretchFactor(1, 2);
    update_details_layout();
    setup_empty_state();
    connect_actions();
    update_action_texts();
    update_action_icons();
    clear_log_entry();
}

/** @brief Destroys the generated user interface. */
LogDetailsWidget::~LogDetailsWidget()
{
    delete ui;
}

/**
 * @brief Presents one complete log entry.
 * @param entry Entry to inspect.
 */
auto LogDetailsWidget::set_log_entry(const LogEntry& entry) -> void
{
    m_entry = entry;
    refresh_entry();
    ui->detailsStateStack->setCurrentWidget(ui->detailsPage);
}

/** @brief Clears the selection and displays the embedded empty state. */
auto LogDetailsWidget::clear_log_entry() -> void
{
    m_entry.reset();
    ui->plainTextEditMessage->clear();
    ui->plainTextEditRaw->clear();
    ui->lineEditPath->clear();
    ui->tableWidgetFields->setRowCount(0);
    refresh_actions();
    ui->detailsStateStack->setCurrentWidget(ui->emptyPage);
}

/**
 * @brief Reports whether an entry is currently presented.
 * @return True when the inspector contains a selected entry.
 */
auto LogDetailsWidget::has_log_entry() const noexcept -> bool
{
    const bool result = m_entry.has_value();
    return result;
}

/** @brief Copies all details of the current entry and shows action feedback. */
auto LogDetailsWidget::copy_current_details() -> void
{
    if (m_entry.has_value())
    {
        copy_text(build_complete_details_text());
        show_copy_feedback(ui->toolButtonCopyAll);
    }
}

/**
 * @brief Returns no preferred extent so the inspector cannot resize its dock.
 * @return An empty preferred size.
 */
auto LogDetailsWidget::sizeHint() const -> QSize
{
    const QSize result;
    return result;
}

/**
 * @brief Returns no minimum extent so restored dock sizes remain authoritative.
 * @return An empty minimum size.
 */
auto LogDetailsWidget::minimumSizeHint() const -> QSize
{
    const QSize result;
    return result;
}

/**
 * @brief Returns the normal action icon tint.
 * @return Configured icon color.
 */
auto LogDetailsWidget::get_action_icon_color() const -> QColor
{
    return m_action_icon_color;
}

/**
 * @brief Sets the normal action icon tint.
 * @param color New icon color.
 */
auto LogDetailsWidget::set_action_icon_color(const QColor& color) -> void
{
    if (m_action_icon_color != color)
    {
        m_action_icon_color = color;
        update_action_icons();
    }
}

/**
 * @brief Returns the hovered action icon tint.
 * @return Configured hover color.
 */
auto LogDetailsWidget::get_action_icon_color_hover() const -> QColor
{
    return m_action_icon_color_hover;
}

/**
 * @brief Sets the hovered action icon tint.
 * @param color New hover color.
 */
auto LogDetailsWidget::set_action_icon_color_hover(const QColor& color) -> void
{
    if (m_action_icon_color_hover != color)
    {
        m_action_icon_color_hover = color;
        update_action_icons();
    }
}

/**
 * @brief Returns the square action icon size.
 * @return Icon size in pixels.
 */
auto LogDetailsWidget::get_action_icon_px() const noexcept -> int
{
    return m_action_icon_px;
}

/**
 * @brief Sets the square action icon size.
 * @param size Icon size in pixels.
 */
auto LogDetailsWidget::set_action_icon_px(int size) -> void
{
    const int bounded_size = qMax(1, size);
    if (m_action_icon_px != bounded_size)
    {
        m_action_icon_px = bounded_size;
        update_action_icons();
    }
}

/**
 * @brief Retranslates generated and runtime-created text after language changes.
 * @param event Delivered widget event.
 */
auto LogDetailsWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        m_empty_state->set_title(tr("No log entry selected"));
        m_empty_state->set_description(
            tr("Select a row in the active log view to inspect its complete details."));
        update_action_texts();
        refresh_entry();
    }
    QWidget::changeEvent(event);
}

/**
 * @brief Adapts the inspector sections to the available dock width.
 * @param event Resize event delivered by Qt.
 */
auto LogDetailsWidget::resizeEvent(QResizeEvent* event) -> void
{
    QWidget::resizeEvent(event);
    update_details_layout();
}

/** @brief Creates the compact no-selection presentation inside the UI stack. */
auto LogDetailsWidget::setup_empty_state() -> void
{
    m_empty_state = new EmptyStateWidget(ui->emptyPage);
    m_empty_state->setObjectName(QStringLiteral("logDetailsEmptyState"));
    m_empty_state->set_density(EmptyStateWidget::Density::Compact);
    m_empty_state->set_title(tr("No log entry selected"));
    m_empty_state->set_description(
        tr("Select a row in the active log view to inspect its complete details."));
    ui->verticalLayoutEmpty->addWidget(m_empty_state);
}

/** @brief Connects copy, folder, and field-table actions. */
auto LogDetailsWidget::connect_actions() -> void
{
    connect(ui->toolButtonCopyMessage, &QToolButton::clicked, this, [this] {
        if (m_entry.has_value())
        {
            copy_text(m_entry->get_message());
            show_copy_feedback(ui->toolButtonCopyMessage);
        }
    });
    connect(ui->toolButtonCopyRaw, &QToolButton::clicked, this, [this] {
        if (m_entry.has_value())
        {
            copy_text(m_entry->get_raw_record());
            show_copy_feedback(ui->toolButtonCopyRaw);
        }
    });
    connect(ui->toolButtonCopyAll, &QToolButton::clicked, this,
            &LogDetailsWidget::copy_current_details);
    connect(ui->toolButtonCopyPath, &QToolButton::clicked, this, [this] {
        copy_text(ui->lineEditPath->text());
        show_copy_feedback(ui->toolButtonCopyPath);
    });
    connect(ui->toolButtonOpenFolder, &QToolButton::clicked, this, [this] {
        if (m_entry.has_value())
        {
            const QFileInfo source_file(m_entry->get_file_info().get_file_path());
            const bool opened = DesktopServices::open_folder(source_file.absolutePath());
            Q_UNUSED(opened);
        }
    });
    connect(ui->tableWidgetFields, &QTableWidget::customContextMenuRequested, this,
            &LogDetailsWidget::show_fields_context_menu);
    connect(ui->tableWidgetFields, &QTableWidget::cellDoubleClicked, this,
            [this](int, int) { copy_selected_fields(); });

    auto* copy_shortcut = new QShortcut(QKeySequence::Copy, ui->tableWidgetFields);
    connect(copy_shortcut, &QShortcut::activated, this, &LogDetailsWidget::copy_selected_fields);
}

/** @brief Updates translated text for runtime-configured controls. */
auto LogDetailsWidget::update_action_texts() -> void
{
    ui->toolButtonCopyAll->setText(tr("Copy details"));
    ui->toolButtonCopyAll->setToolTip(tr("Copy all details"));
    ui->toolButtonCopyAll->setAccessibleName(tr("Copy all details"));
    ui->toolButtonCopyPath->setToolTip(tr("Copy source path"));
    ui->toolButtonCopyPath->setAccessibleName(tr("Copy source path"));
    ui->toolButtonOpenFolder->setToolTip(tr("Open containing folder"));
    ui->toolButtonOpenFolder->setAccessibleName(tr("Open containing folder"));
}

/** @brief Switches the details splitter between wide and compact arrangements. */
auto LogDetailsWidget::update_details_layout() -> void
{
    if (ui != nullptr)
    {
        const bool compact = width() < k_compact_details_width;
        const Qt::Orientation desired_orientation = compact ? Qt::Vertical : Qt::Horizontal;
        const bool was_compact = ui->toolButtonCopyAll->property("compact").toBool();
        ui->toolButtonCopyAll->setProperty("compact", compact);
        ui->toolButtonCopyAll->setToolButtonStyle(compact ? Qt::ToolButtonIconOnly
                                                          : Qt::ToolButtonTextBesideIcon);
        if (was_compact != compact)
        {
            ui->toolButtonCopyAll->style()->unpolish(ui->toolButtonCopyAll);
            ui->toolButtonCopyAll->style()->polish(ui->toolButtonCopyAll);
        }
        if (ui->detailsSplitter->orientation() != desired_orientation)
        {
            ui->detailsSplitter->setOrientation(desired_orientation);
            if (desired_orientation == Qt::Horizontal)
            {
                ui->detailsSplitter->setSizes({600, 400});
            }
            else
            {
                ui->detailsSplitter->setSizes({140, 110});
            }
        }
    }
}

/** @brief Applies the current entry to every visible details section. */
auto LogDetailsWidget::refresh_entry() -> void
{
    if (m_entry.has_value())
    {
        const LogEntry& entry = m_entry.value();
        const QString level = entry.get_level().trimmed();
        const QString application = entry.get_app_name().trimmed();
        const QString file_path = entry.get_file_info().get_file_path();
        const QFileInfo file_info(file_path);
        const bool has_file_path = !file_path.isEmpty();
        const QDateTime timestamp = entry.get_timestamp();
        const QString timestamp_text =
            timestamp.isValid()
                ? tr("%1 at %2")
                      .arg(QLocale().toString(timestamp.date(), QLocale::ShortFormat),
                           timestamp.time().toString(QStringLiteral("HH:mm:ss.zzz")))
                : tr("Unknown time");

        const std::optional<SimpleCppLogger::LogLevel> severity = LogLevels::from_text(level);
        const QString severity_style = severity.has_value()
                                           ? LogLevels::to_text(severity.value()).toLower()
                                           : QStringLiteral("unknown");
        ui->labelSeverityBadge->setText(level.isEmpty() ? tr("UNKNOWN") : level.toUpper());
        ui->labelSeverityBadge->setProperty("severity", severity_style);
        ui->labelSeverityBadge->style()->unpolish(ui->labelSeverityBadge);
        ui->labelSeverityBadge->style()->polish(ui->labelSeverityBadge);
        ui->labelTimestampValue->setText(timestamp_text);
        ui->labelTimestampValue->setToolTip(timestamp.toString(Qt::ISODateWithMs));
        ui->labelApplicationValue->setText(application.isEmpty() ? tr("Unknown application")
                                                                 : application);
        ui->labelApplicationValue->setToolTip(ui->labelApplicationValue->text());
        ui->plainTextEditMessage->setPlainText(entry.get_message());
        ui->plainTextEditRaw->setPlainText(entry.get_raw_record());
        ui->labelFileValue->setText(file_info.fileName());
        ui->labelFileValue->setToolTip(file_info.fileName());
        ui->lineEditPath->setText(file_path);
        ui->lineEditPath->setToolTip(file_path);
        ui->labelFileValue->setVisible(has_file_path);
        ui->framePathField->setVisible(has_file_path);
        ui->toolButtonOpenFolder->setVisible(has_file_path);

        const bool has_source_line = entry.get_source_line() >= 0;
        const bool has_byte_range = entry.get_byte_offset() >= 0 && entry.get_byte_length() >= 0;
        QStringList source_meta;
        if (has_source_line)
        {
            source_meta.append(tr("Line %1").arg(QLocale().toString(entry.get_source_line())));
        }
        if (has_byte_range)
        {
            source_meta.append(tr("%1 bytes").arg(QLocale().toString(entry.get_byte_length())));
        }
        QString byte_range;
        if (has_byte_range)
        {
            const qint64 last_byte = entry.get_byte_length() > 0
                                         ? entry.get_byte_offset() + entry.get_byte_length() - 1
                                         : entry.get_byte_offset();
            byte_range = tr("%1 – %2 (%3 bytes)")
                             .arg(QLocale().toString(entry.get_byte_offset()))
                             .arg(QLocale().toString(last_byte))
                             .arg(QLocale().toString(entry.get_byte_length()));
        }
        ui->labelSourceMetaValue->setText(source_meta.join(QStringLiteral("  ·  ")));
        ui->labelSourceMetaValue->setToolTip(byte_range);
        ui->labelSourceMetaValue->setVisible(!source_meta.isEmpty());
        const bool has_source_details = has_file_path || has_source_line || has_byte_range;
        ui->sourceSection->setVisible(has_source_details);
        refresh_additional_fields();
        ui->metadataSectionCard->setVisible(has_source_details ||
                                            ui->tableWidgetFields->rowCount() > 0);
    }
    refresh_actions();
}

/** @brief Rebuilds the additional-field table from the current entry. */
auto LogDetailsWidget::refresh_additional_fields() -> void
{
    ui->tableWidgetFields->setRowCount(0);
    if (m_entry.has_value())
    {
        const LogEntry::ParsedFields& fields = m_entry->get_parsed_fields();
        for (auto iterator = fields.constBegin(); iterator != fields.constEnd(); ++iterator)
        {
            if (!dedicated_field_ids().contains(iterator.key()))
            {
                const int row = ui->tableWidgetFields->rowCount();
                ui->tableWidgetFields->insertRow(row);
                auto* name_item = new QTableWidgetItem(format_field_label(iterator.key()));
                name_item->setData(Qt::UserRole, iterator.key());
                auto* value_item = new QTableWidgetItem(format_field_value(iterator.value()));
                name_item->setToolTip(iterator.key());
                value_item->setToolTip(value_item->text());
                ui->tableWidgetFields->setItem(row, 0, name_item);
                ui->tableWidgetFields->setItem(row, 1, value_item);
            }
        }
    }
    ui->additionalFieldsSection->setVisible(ui->tableWidgetFields->rowCount() > 0);
}

/** @brief Updates enabled states and tooltips derived from the current entry. */
auto LogDetailsWidget::refresh_actions() -> void
{
    const bool has_entry = m_entry.has_value();
    const QString message = has_entry ? m_entry->get_message() : QString();
    const QString raw_record = has_entry ? m_entry->get_raw_record() : QString();
    const QString file_path = has_entry ? m_entry->get_file_info().get_file_path() : QString();
    const QFileInfo file_info(file_path);
    const bool has_folder = !file_path.isEmpty() && QDir(file_info.absolutePath()).exists();

    ui->toolButtonCopyAll->setEnabled(has_entry);
    ui->toolButtonCopyMessage->setEnabled(!message.isEmpty());
    ui->toolButtonCopyRaw->setEnabled(!raw_record.isEmpty());
    ui->toolButtonCopyPath->setEnabled(!file_path.isEmpty());
    ui->toolButtonOpenFolder->setEnabled(has_folder);
    ui->detailsTabWidget->setTabEnabled(ui->detailsTabWidget->indexOf(ui->rawTab),
                                        !raw_record.isEmpty());
    if (raw_record.isEmpty() && ui->detailsTabWidget->currentWidget() == ui->rawTab)
    {
        ui->detailsTabWidget->setCurrentWidget(ui->detailsTab);
    }
}

/** @brief Rebuilds QSS-tinted copy and external-folder icons. */
auto LogDetailsWidget::update_action_icons() -> void
{
    if (ui != nullptr)
    {
        const QSize icon_size(m_action_icon_px, m_action_icon_px);
        QIcon copy_icon = QtWidgetsCommonLib::UiUtils::colored_svg_icon(
            QString::fromLatin1(k_copy_icon_path), m_action_icon_color, icon_size);
        copy_icon.addPixmap(
            QtWidgetsCommonLib::UiUtils::colored_svg_icon(QString::fromLatin1(k_copy_icon_path),
                                                          m_action_icon_color_hover, icon_size),
            QIcon::Active);
        QIcon folder_icon = QtWidgetsCommonLib::UiUtils::colored_svg_icon(
            QString::fromLatin1(k_open_folder_icon_path), m_action_icon_color, icon_size);
        folder_icon.addPixmap(
            QtWidgetsCommonLib::UiUtils::colored_svg_icon(
                QString::fromLatin1(k_open_folder_icon_path), m_action_icon_color_hover, icon_size),
            QIcon::Active);

        const QList<QToolButton*> copy_buttons{ui->toolButtonCopyAll, ui->toolButtonCopyMessage,
                                               ui->toolButtonCopyPath, ui->toolButtonCopyRaw};
        for (QToolButton* button: copy_buttons)
        {
            button->setIcon(copy_icon);
            button->setIconSize(icon_size);
        }
        ui->toolButtonOpenFolder->setIcon(folder_icon);
        ui->toolButtonOpenFolder->setIconSize(icon_size);
    }
}

/**
 * @brief Briefly marks a copy button as successfully activated.
 * @param button Copy button receiving visual confirmation.
 */
auto LogDetailsWidget::show_copy_feedback(QToolButton* button) -> void
{
    if (button != nullptr)
    {
        const QSize icon_size(m_action_icon_px, m_action_icon_px);
        button->setIcon(QtWidgetsCommonLib::UiUtils::colored_svg_icon(
            QString::fromLatin1(k_copy_confirmed_icon_path), QColor(Qt::white), icon_size));
        button->setProperty("copyConfirmed", true);
        if (button == ui->toolButtonCopyAll)
        {
            button->setText(tr("Copied"));
        }
        button->style()->unpolish(button);
        button->style()->polish(button);

        const QPointer<QToolButton> guarded_button(button);
        QTimer::singleShot(k_copy_feedback_duration_ms, this, [this, guarded_button] {
            if (guarded_button != nullptr)
            {
                guarded_button->setProperty("copyConfirmed", false);
                if (guarded_button == ui->toolButtonCopyAll)
                {
                    update_action_texts();
                }
                guarded_button->style()->unpolish(guarded_button);
                guarded_button->style()->polish(guarded_button);
                update_action_icons();
            }
        });
    }
}

/** @brief Opens a context menu for copying selected additional fields. */
auto LogDetailsWidget::show_fields_context_menu(const QPoint& position) -> void
{
    QMenu menu(this);
    QAction* copy_action = menu.addAction(tr("Copy selected fields"));
    copy_action->setEnabled(!ui->tableWidgetFields->selectedItems().isEmpty());
    connect(copy_action, &QAction::triggered, this, &LogDetailsWidget::copy_selected_fields);
    menu.exec(ui->tableWidgetFields->viewport()->mapToGlobal(position));
}

/** @brief Copies the selected additional field rows to the clipboard. */
auto LogDetailsWidget::copy_selected_fields() const -> void
{
    QSet<int> selected_rows;
    const QList<QTableWidgetItem*> selected_items = ui->tableWidgetFields->selectedItems();
    for (QTableWidgetItem* item: selected_items)
    {
        selected_rows.insert(item->row());
    }

    QList<int> ordered_rows = selected_rows.values();
    std::sort(ordered_rows.begin(), ordered_rows.end());
    QStringList lines;
    for (int row: ordered_rows)
    {
        const QTableWidgetItem* name_item = ui->tableWidgetFields->item(row, 0);
        const QTableWidgetItem* value_item = ui->tableWidgetFields->item(row, 1);
        if (name_item != nullptr && value_item != nullptr)
        {
            lines.append(QStringLiteral("%1: %2").arg(name_item->text(), value_item->text()));
        }
    }
    copy_text(lines.join(QLatin1Char('\n')));
}

/**
 * @brief Copies non-empty text to the system clipboard.
 * @param text Text to copy.
 */
auto LogDetailsWidget::copy_text(const QString& text) const -> void
{
    if (!text.isEmpty())
    {
        QApplication::clipboard()->setText(text);
    }
}

/**
 * @brief Builds a readable complete representation of the selected entry.
 * @return Complete details text, or an empty string without a selection.
 */
auto LogDetailsWidget::build_complete_details_text() const -> QString
{
    QString result;
    if (m_entry.has_value())
    {
        const LogEntry& entry = m_entry.value();
        QStringList lines;
        lines.append(tr("Timestamp: %1").arg(ui->labelTimestampValue->text()));
        lines.append(tr("Level: %1").arg(entry.get_level()));
        lines.append(tr("Application: %1").arg(entry.get_app_name()));
        lines.append(tr("Message: %1").arg(entry.get_message()));
        const QString file_path = entry.get_file_info().get_file_path();
        if (!file_path.isEmpty())
        {
            lines.append(tr("Source: %1").arg(file_path));
        }
        if (entry.get_source_line() >= 0)
        {
            lines.append(tr("Source line: %1").arg(entry.get_source_line()));
        }
        if (entry.get_byte_offset() >= 0 && entry.get_byte_length() >= 0)
        {
            lines.append(tr("Byte offset: %1").arg(entry.get_byte_offset()));
            lines.append(tr("Byte length: %1").arg(entry.get_byte_length()));
        }
        for (int row = 0; row < ui->tableWidgetFields->rowCount(); ++row)
        {
            const QTableWidgetItem* name_item = ui->tableWidgetFields->item(row, 0);
            const QTableWidgetItem* value_item = ui->tableWidgetFields->item(row, 1);
            if (name_item != nullptr && value_item != nullptr)
            {
                lines.append(QStringLiteral("%1: %2").arg(name_item->text(), value_item->text()));
            }
        }
        if (!entry.get_raw_record().isEmpty())
        {
            lines.append(QString());
            lines.append(tr("Raw record:"));
            lines.append(entry.get_raw_record());
        }
        result = lines.join(QLatin1Char('\n'));
    }
    return result;
}

/**
 * @brief Formats one parsed QVariant for display.
 * @param value Parsed field value.
 * @return Readable value text.
 */
auto LogDetailsWidget::format_field_value(const QVariant& value) -> QString
{
    QString result;
    if (value.metaType().id() == QMetaType::QDateTime)
    {
        result = value.toDateTime().toString(Qt::ISODateWithMs);
    }
    else if (value.metaType().id() == QMetaType::Bool)
    {
        result = value.toBool() ? tr("true") : tr("false");
    }
    else if (value.canConvert<QStringList>())
    {
        result = value.toStringList().join(QStringLiteral(", "));
    }
    else
    {
        result = value.toString();
    }
    return result;
}

/**
 * @brief Converts a stable field identifier to a readable fallback label.
 * @param field_id Stable parser field identifier.
 * @return Human-readable label.
 */
auto LogDetailsWidget::format_field_label(const QString& field_id) -> QString
{
    QString result = field_id;
    result.replace(QLatin1Char('_'), QLatin1Char(' '));
    if (!result.isEmpty())
    {
        result[0] = result.at(0).toUpper();
    }
    return result;
}

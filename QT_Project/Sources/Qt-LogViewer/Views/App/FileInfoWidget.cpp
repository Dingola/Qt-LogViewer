/**
 * @file FileInfoWidget.cpp
 * @brief Implements the presentation-only file inspector widget.
 */

#include "Qt-LogViewer/Views/App/FileInfoWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QScrollArea>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "Qt-LogViewer/Services/DesktopServices.h"
#include "QtWidgetsCommonLib/Utils/UiUtils.h"

namespace
{
/** @brief Number of value rows rendered by FileInfoWidget. */
constexpr qsizetype ValueCount = 16;
constexpr auto k_copy_icon_path = ":/Resources/Icons/copy.svg";
constexpr auto k_copy_confirmed_icon_path = ":/Resources/Icons/check-light.svg";
constexpr auto k_open_folder_icon_path = ":/Resources/Icons/folder-open-external.svg";
constexpr int k_copy_feedback_duration_ms = 900;
constexpr int k_action_icon_size = 16;
}  // namespace

/**
 * @brief Constructs the file inspector layout.
 * @param parent Optional parent widget.
 */
FileInfoWidget::FileInfoWidget(QWidget* parent)
    : QWidget(parent), m_source_selector(new QComboBox(this)), m_empty_label(new QLabel(this))
{
    setAttribute(Qt::WA_StyledBackground, true);
    auto* root_layout = new QVBoxLayout(this);
    m_source_selector->setObjectName(QStringLiteral("fileInfoSourceSelector"));
    root_layout->setContentsMargins(10, 10, 10, 10);
    root_layout->setSpacing(8);
    root_layout->addWidget(m_source_selector);

    m_empty_label->setText(tr("Open a log file to inspect its source information."));
    m_empty_label->setWordWrap(true);
    root_layout->addWidget(m_empty_label);

    m_content_widget = new QWidget(this);
    m_content_widget->setObjectName(QStringLiteral("fileInfoContent"));
    auto* content_layout = new QVBoxLayout(m_content_widget);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(10);

    auto* header = new QWidget(m_content_widget);
    header->setObjectName(QStringLiteral("fileInfoHeader"));
    auto* header_layout = new QHBoxLayout(header);
    header_layout->setContentsMargins(10, 8, 8, 8);
    auto* heading_layout = new QVBoxLayout();
    heading_layout->setSpacing(1);
    m_title_label = new QLabel(header);
    m_title_label->setObjectName(QStringLiteral("fileInfoTitle"));
    m_title_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_subtitle_label = new QLabel(header);
    m_subtitle_label->setObjectName(QStringLiteral("fileInfoSubtitle"));
    heading_layout->addWidget(m_title_label);
    heading_layout->addWidget(m_subtitle_label);
    header_layout->addLayout(heading_layout, 1);
    m_copy_all_button = new QToolButton(header);
    m_copy_all_button->setObjectName(QStringLiteral("fileInfoCopyAllButton"));
    m_copy_all_button->setProperty("fileInfoAction", true);
    m_copy_all_button->setToolTip(tr("Copy all file information"));
    header_layout->addWidget(m_copy_all_button);
    content_layout->addWidget(header);

    const QStringList row_labels = {tr("Name"),          tr("Path"),       tr("Size"),
                                    tr("Last modified"), tr("Records"),    tr("Time range"),
                                    tr("Applications"),  tr("Log levels"), tr("Parsing profile"),
                                    tr("Parse success"), tr("Encoding"),   tr("Line endings"),
                                    tr("Cache state"),   tr("Cache size"), tr("Cache last access"),
                                    tr("Live tail")};
    const QVector<QPair<QString, qsizetype>> sections = {{tr("Source"), 0},
                                                         {tr("Data"), 4},
                                                         {tr("Parsing"), 8},
                                                         {tr("Cache"), 12},
                                                         {tr("Live tail"), 15}};
    m_value_labels.reserve(ValueCount + 1);
    for (qsizetype section_index = 0; section_index < sections.size(); ++section_index)
    {
        auto* group = new QGroupBox(sections.at(section_index).first, m_content_widget);
        group->setProperty("fileInfoSection", true);
        auto* form = new QFormLayout(group);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        const qsizetype first = sections.at(section_index).second;
        const qsizetype last = section_index + 1 < sections.size()
                                   ? sections.at(section_index + 1).second
                                   : row_labels.size();
        for (qsizetype row_index = first; row_index < last; ++row_index)
        {
            m_value_labels.append(add_value_row(row_labels.at(row_index)));
            QLineEdit* field = m_value_labels.constLast();
            field->setObjectName(QStringLiteral("fileInfoValue%1").arg(m_value_labels.size() - 1));
            if (row_index == 1)
            {
                auto* path_container = new QWidget(group);
                path_container->setObjectName(QStringLiteral("fileInfoPathContainer"));
                auto* path_layout = new QHBoxLayout(path_container);
                path_layout->setContentsMargins(0, 0, 0, 0);
                path_layout->setSpacing(4);
                path_layout->addWidget(field, 1);
                m_copy_path_button = new QToolButton(path_container);
                m_copy_path_button->setObjectName(QStringLiteral("fileInfoCopyPathButton"));
                m_copy_path_button->setProperty("fileInfoAction", true);
                m_copy_path_button->setToolTip(tr("Copy full path"));
                m_open_folder_button = new QToolButton(path_container);
                m_open_folder_button->setObjectName(QStringLiteral("fileInfoOpenFolderButton"));
                m_open_folder_button->setProperty("fileInfoAction", true);
                m_open_folder_button->setToolTip(tr("Show in File Explorer"));
                path_layout->addWidget(m_copy_path_button);
                path_layout->addWidget(m_open_folder_button);
                form->addRow(row_labels.at(row_index) + QStringLiteral(":"), path_container);
            }
            else
            {
                form->addRow(row_labels.at(row_index) + QStringLiteral(":"), field);
            }
        }
        if (section_index + 1 == sections.size())
        {
            m_value_labels.append(add_value_row(tr("Last live entry")));
            m_value_labels.constLast()->setObjectName(
                QStringLiteral("fileInfoValue%1").arg(m_value_labels.size() - 1));
            form->addRow(tr("Last live entry:"), m_value_labels.constLast());
        }
        content_layout->addWidget(group);
    }

    content_layout->addStretch(1);

    auto* scroll_area = new QScrollArea(this);
    scroll_area->setObjectName(QStringLiteral("fileInfoScrollArea"));
    scroll_area->setWidgetResizable(true);
    scroll_area->setFrameShape(QFrame::NoFrame);
    scroll_area->setWidget(m_content_widget);
    root_layout->addWidget(scroll_area, 1);

    connect(m_source_selector, &QComboBox::currentIndexChanged, this, [this](int index) {
        const qsizetype selected_index = static_cast<qsizetype>(index);
        show_snapshot(selected_index);
        emit snapshot_selected(selected_index);
    });
    connect(m_copy_all_button, &QToolButton::clicked, this, &FileInfoWidget::copy_current_summary);
    connect(m_copy_path_button, &QToolButton::clicked, this, [this] {
        const bool valid = m_current_index >= 0 && m_current_index < m_snapshots.size();
        if (valid)
        {
            copy_text(m_snapshots.at(m_current_index).path, m_copy_path_button);
        }
    });
    connect(m_open_folder_button, &QToolButton::clicked, this, [this] {
        const bool valid = m_current_index >= 0 && m_current_index < m_snapshots.size();
        if (valid && m_snapshots.at(m_current_index).local_path_available)
        {
            const bool opened = DesktopServices::open_folder(
                QFileInfo(m_snapshots.at(m_current_index).path).absolutePath());
            Q_UNUSED(opened);
        }
    });
    clear();
}

/**
 * @brief Replaces all snapshots and selects one of them.
 * @param snapshots Aggregate snapshot followed by source snapshots.
 * @param selected_index Index to select after replacement.
 */
auto FileInfoWidget::set_snapshots(const QVector<FileInfoSnapshot>& snapshots,
                                   qsizetype selected_index) -> void
{
    m_snapshots = snapshots;
    m_source_selector->blockSignals(true);
    m_source_selector->clear();
    for (const FileInfoSnapshot& snapshot: m_snapshots)
    {
        m_source_selector->addItem(snapshot.display_name);
    }
    const qsizetype bounded_index =
        selected_index >= 0 && selected_index < m_snapshots.size() ? selected_index : 0;
    m_source_selector->setCurrentIndex(static_cast<int>(bounded_index));
    m_source_selector->blockSignals(false);
    m_source_selector->setVisible(m_snapshots.size() > 1);
    m_empty_label->setVisible(m_snapshots.isEmpty());
    m_content_widget->setVisible(!m_snapshots.isEmpty());
    show_snapshot(bounded_index);
}

/** @brief Clears the inspector and displays its explanatory empty state. */
auto FileInfoWidget::clear() -> void
{
    m_snapshots.clear();
    m_current_index = -1;
    m_source_selector->clear();
    m_source_selector->hide();
    m_content_widget->hide();
    m_empty_label->show();
    for (QLineEdit* label: m_value_labels)
    {
        label->clear();
    }
    refresh_actions();
}

/**
 * @brief Displays a retained snapshot.
 * @param index Index in the retained snapshot collection.
 */
auto FileInfoWidget::show_snapshot(qsizetype index) -> void
{
    const bool valid = index >= 0 && index < m_snapshots.size();
    if (valid)
    {
        m_current_index = index;
        const FileInfoSnapshot& value = m_snapshots.at(index);
        m_title_label->setText(value.display_name);
        m_subtitle_label->setText(tr("%1 records · %2").arg(value.record_count, value.size));
        const QStringList texts = {
            value.display_name,       value.path,           value.size,
            value.modified,           value.record_count,   value.timestamp_range,
            value.applications,       value.log_levels,     value.parsing_profile,
            value.parse_success_rate, value.encoding,       value.line_endings,
            value.cache_state,        value.cache_size,     value.cache_last_access,
            value.live_tail_state,    value.last_live_entry};
        for (qsizetype label_index = 0; label_index < m_value_labels.size(); ++label_index)
        {
            QLineEdit* label = m_value_labels.at(label_index);
            label->setText(texts.at(label_index));
            label->setToolTip(texts.at(label_index));
        }
        refresh_actions();
    }
}

/**
 * @brief Creates one selectable read-only metadata value.
 * @param label User-facing row label used as an accessible name.
 * @return Created read-only value field.
 */
auto FileInfoWidget::add_value_row(const QString& label) -> QLineEdit*
{
    auto* value = new QLineEdit(this);
    value->setReadOnly(true);
    value->setProperty("fileInfoField", true);
    value->setAccessibleName(label);
    return value;
}

/** @brief Copies every displayed property of the current snapshot. */
auto FileInfoWidget::copy_current_summary() -> void
{
    copy_text(build_summary_text(), m_copy_all_button);
}

/** @brief Updates contextual action states and theme-tinted icons. */
auto FileInfoWidget::refresh_actions() -> void
{
    const bool valid = m_current_index >= 0 && m_current_index < m_snapshots.size();
    const bool has_local_path = valid && m_snapshots.at(m_current_index).local_path_available;
    m_copy_all_button->setEnabled(valid);
    m_copy_path_button->setEnabled(has_local_path);
    m_open_folder_button->setEnabled(has_local_path);

    const QSize icon_size(k_action_icon_size, k_action_icon_size);
    const QColor normal_color(QStringLiteral("#b0bec5"));
    const QColor hover_color(QStringLiteral("#ffffff"));
    QIcon copy_icon = QtWidgetsCommonLib::UiUtils::colored_svg_icon(
        QString::fromLatin1(k_copy_icon_path), normal_color, icon_size);
    copy_icon.addPixmap(QtWidgetsCommonLib::UiUtils::colored_svg_icon(
                            QString::fromLatin1(k_copy_icon_path), hover_color, icon_size),
                        QIcon::Active);
    QIcon folder_icon = QtWidgetsCommonLib::UiUtils::colored_svg_icon(
        QString::fromLatin1(k_open_folder_icon_path), normal_color, icon_size);
    folder_icon.addPixmap(QtWidgetsCommonLib::UiUtils::colored_svg_icon(
                              QString::fromLatin1(k_open_folder_icon_path), hover_color, icon_size),
                          QIcon::Active);
    m_copy_all_button->setIcon(copy_icon);
    m_copy_path_button->setIcon(copy_icon);
    m_open_folder_button->setIcon(folder_icon);
}

/**
 * @brief Copies text and shows temporary confirmation on an action.
 * @param text Text to place on the clipboard.
 * @param button Activated action button.
 */
auto FileInfoWidget::copy_text(const QString& text, QToolButton* button) -> void
{
    if (!text.isEmpty() && button != nullptr)
    {
        QApplication::clipboard()->setText(text);
        button->setProperty("copyConfirmed", true);
        button->setIcon(QtWidgetsCommonLib::UiUtils::colored_svg_icon(
            QString::fromLatin1(k_copy_confirmed_icon_path), QColor(Qt::white),
            QSize(k_action_icon_size, k_action_icon_size)));
        button->style()->unpolish(button);
        button->style()->polish(button);
        QPointer<QToolButton> guarded_button(button);
        QTimer::singleShot(k_copy_feedback_duration_ms, this, [this, guarded_button] {
            if (guarded_button != nullptr)
            {
                guarded_button->setProperty("copyConfirmed", false);
                guarded_button->style()->unpolish(guarded_button);
                guarded_button->style()->polish(guarded_button);
                refresh_actions();
            }
        });
    }
}

/**
 * @brief Builds a readable multiline representation of the current snapshot.
 * @return Complete displayed summary, or an empty value without a snapshot.
 */
auto FileInfoWidget::build_summary_text() const -> QString
{
    QString result;
    const bool valid = m_current_index >= 0 && m_current_index < m_snapshots.size();
    if (valid)
    {
        const FileInfoSnapshot& value = m_snapshots.at(m_current_index);
        const QStringList lines = {tr("Name: %1").arg(value.display_name),
                                   tr("Path: %1").arg(value.path),
                                   tr("Size: %1").arg(value.size),
                                   tr("Last modified: %1").arg(value.modified),
                                   tr("Recognized records: %1").arg(value.record_count),
                                   tr("Time range: %1").arg(value.timestamp_range),
                                   tr("Applications: %1").arg(value.applications),
                                   tr("Log levels: %1").arg(value.log_levels),
                                   tr("Parsing profile: %1").arg(value.parsing_profile),
                                   tr("Parse success: %1").arg(value.parse_success_rate),
                                   tr("Encoding: %1").arg(value.encoding),
                                   tr("Line endings: %1").arg(value.line_endings),
                                   tr("Cache state: %1").arg(value.cache_state),
                                   tr("Cache size: %1").arg(value.cache_size),
                                   tr("Cache last access: %1").arg(value.cache_last_access),
                                   tr("Live tail: %1").arg(value.live_tail_state),
                                   tr("Last live entry: %1").arg(value.last_live_entry)};
        result = lines.join(QLatin1Char('\n'));
    }
    return result;
}

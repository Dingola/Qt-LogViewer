/**
 * @file LogDataStatusWidget.cpp
 * @brief Implements the compact active-view data status presentation.
 */

#include "Qt-LogViewer/Views/App/LogDataStatusWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QSizePolicy>
#include <QStyle>

#include "QtWidgetsCommonLib/Utils/NumberFormatUtils.h"

using QtWidgetsCommonLib::NumberFormatUtils;

namespace
{
constexpr qsizetype k_compact_entry_count_threshold = 1000000;

/**
 * @brief Adds a compact separator label to a data-status layout.
 * @param layout Layout receiving the separator.
 * @param parent Parent widget owning the separator.
 * @return Created separator label.
 */
[[nodiscard]] auto add_separator(QHBoxLayout* layout, QWidget* parent) -> QLabel*
{
    auto* separator = new QLabel(QStringLiteral("\u00b7"), parent);
    separator->setProperty("variant", QStringLiteral("separator"));
    layout->addWidget(separator);
    return separator;
}

/**
 * @brief Formats an entry count exactly below one million and compactly above it.
 * @param count Non-negative entry count.
 * @param locale Locale used for exact grouped values.
 * @return Readable exact or abbreviated count.
 */
[[nodiscard]] auto format_entry_count(qsizetype count, const QLocale& locale) -> QString
{
    QString formatted_count = locale.toString(count);
    if (count >= k_compact_entry_count_threshold)
    {
        formatted_count = NumberFormatUtils::format_number_abbreviated(static_cast<double>(count));
    }
    return formatted_count;
}
}  // namespace

/**
 * @brief Constructs an initially hidden data status widget.
 * @param parent Optional parent widget.
 */
LogDataStatusWidget::LogDataStatusWidget(QWidget* parent)
    : QWidget(parent),
      m_entries_label(new QLabel(this)),
      m_sources_label(new QLabel(this)),
      m_filters_label(new QLabel(this)),
      m_cache_label(new QLabel(this))
{
    setObjectName(QStringLiteral("LogDataStatusWidget"));
    setAccessibleName(tr("Data status"));
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 4, 0);
    layout->setSpacing(7);

    m_entries_label->setObjectName(QStringLiteral("labelEntryStatus"));
    m_entries_label->setAccessibleName(tr("Visible and total entries"));
    m_entries_label->setToolTip(tr("Entries matching active filters / all entries in this view"));
    layout->addWidget(m_entries_label);
    static_cast<void>(add_separator(layout, this));

    m_sources_label->setObjectName(QStringLiteral("labelSourceStatus"));
    m_sources_label->setAccessibleName(tr("Loaded sources"));
    layout->addWidget(m_sources_label);
    static_cast<void>(add_separator(layout, this));

    m_filters_label->setObjectName(QStringLiteral("labelFilterStatus"));
    m_filters_label->setAccessibleName(tr("Active filter categories"));
    m_filters_label->setToolTip(
        tr("Active application, level, search, and source filter categories"));
    layout->addWidget(m_filters_label);
    m_cache_separator = add_separator(layout, this);
    m_cache_separator->setObjectName(QStringLiteral("separatorCacheStatus"));

    m_cache_label->setObjectName(QStringLiteral("labelCacheStatus"));
    m_cache_label->setAccessibleName(tr("Index status"));
    layout->addWidget(m_cache_label);

    hide();
}

/**
 * @brief Updates all active-view status values and shows the widget.
 * @param visible_entries Entries matching the active filters.
 * @param total_entries All entries belonging to the active view.
 * @param source_count Number of loaded sources.
 * @param active_filter_count Number of active filter categories.
 * @param cache_status Current persistent-cache state represented as an index status.
 */
auto LogDataStatusWidget::set_status(qsizetype visible_entries, qsizetype total_entries,
                                     int source_count, int active_filter_count,
                                     LogCacheStatus cache_status) -> void
{
    const QLocale locale;
    const QString visible_count = format_entry_count(visible_entries, locale);
    const QString total_count = format_entry_count(total_entries, locale);
    m_entries_label->setText(tr("%1 visible / %2 total").arg(visible_count, total_count));
    m_entries_label->setToolTip(
        tr("%1 entries match active filters / %2 entries total")
            .arg(locale.toString(visible_entries), locale.toString(total_entries)));
    m_sources_label->setText(tr("%n source(s)", nullptr, source_count));
    m_sources_label->setToolTip(tr("%n log source(s) loaded in this view", nullptr, source_count));
    m_filters_label->setText(tr("%n filter(s) active", nullptr, active_filter_count));
    set_cache_status(cache_status);
    show();
}

/** @brief Clears and hides the status when no log view is active. */
auto LogDataStatusWidget::clear_status() -> void
{
    m_entries_label->clear();
    m_sources_label->clear();
    m_filters_label->clear();
    m_cache_label->clear();
    hide();
}

/**
 * @brief Updates the user-facing index label and its style property.
 * @param cache_status Cache state to present.
 */
auto LogDataStatusWidget::set_cache_status(LogCacheStatus cache_status) -> void
{
    QString state_name = QStringLiteral("unavailable");
    QString status_text;
    QString tool_tip;
    bool status_visible = false;

    switch (cache_status)
    {
    case LogCacheStatus::Current:
        state_name = QStringLiteral("current");
        status_text = tr("Index ready");
        tool_tip = tr("The index accelerates opening, filtering, and searching large log files");
        status_visible = true;
        break;
    case LogCacheStatus::Updating:
        state_name = QStringLiteral("updating");
        status_text = tr("Index is being built...");
        tool_tip = tr("The index is being prepared to accelerate filtering and searching");
        status_visible = true;
        break;
    case LogCacheStatus::Unavailable:
        break;
    }

    m_cache_label->setText(status_visible ? QStringLiteral("\u25cf %1").arg(status_text)
                                          : QString());
    m_cache_label->setToolTip(tool_tip);
    m_cache_label->setProperty("cacheState", state_name);
    m_cache_separator->setVisible(status_visible);
    m_cache_label->setVisible(status_visible);
    m_cache_label->style()->unpolish(m_cache_label);
    m_cache_label->style()->polish(m_cache_label);
}

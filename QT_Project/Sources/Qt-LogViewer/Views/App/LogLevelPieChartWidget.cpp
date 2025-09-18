/**
 * @file LogLevelPieChartWidget.cpp
 * @brief Implementation of LogLevelPieChartWidget using PieChart.
 */

#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"

#include <QHBoxLayout>
#include <QLabel>

/**
 * @brief Constructs the log level pie chart widget.
 *
 * Initializes the PieChart child widget, sets up the layout for centered alignment,
 * and applies the default colors for each log level.
 *
 * @param parent The parent widget.
 */
LogLevelPieChartWidget::LogLevelPieChartWidget(QWidget* parent)
    : QWidget(parent),
      m_trace_color("#b0bec5"),
      m_debug_color("#66bb6a"),
      m_info_color("#42a5f5"),
      m_warning_color("#ffb300"),
      m_error_color("#ef5350"),
      m_fatal_color("#ff1744"),
      m_pie_chart(new PieChart(this)),
      m_levels_widget(new QWidget(this)),
      m_levels_layout(new FlowLayout(m_levels_widget, 0, 12, 6))
{
    setAttribute(Qt::WA_StyledBackground, true);

    // Setup levels widget
    m_levels_widget->setObjectName("logLevelRowsWidget");
    m_levels_widget->setLayout(m_levels_layout);
    m_levels_layout->setContentsMargins(0, 0, 0, 0);
    m_levels_layout->setSpacing(6);
    m_levels_layout->set_row_alignment(FlowLayout::RowAlignment::CenterLeft);
    m_levels_layout->set_expand_to_show_all_rows(true);

    // Main layout
    auto* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(m_pie_chart, 1);
    main_layout->addWidget(m_levels_widget, 0);
    main_layout->setContentsMargins(24, 24, 24, 24);
    setLayout(main_layout);

    // Set default colors for log levels
    m_pie_chart->set_segment_color("trace", m_trace_color);
    m_pie_chart->set_segment_color("debug", m_debug_color);
    m_pie_chart->set_segment_color("info", m_info_color);
    m_pie_chart->set_segment_color("warning", m_warning_color);
    m_pie_chart->set_segment_color("error", m_error_color);
    m_pie_chart->set_segment_color("fatal", m_fatal_color);
}

/**
 * @brief Sets the log level counts and updates the chart.
 *
 * Passes the log level counts to the PieChart child widget for rendering.
 *
 * @param level_counts Map of log level to count.
 */
auto LogLevelPieChartWidget::set_log_level_counts(const QMap<QString, int>& level_counts) -> void
{
    qDebug() << "set_log_level_counts called with:" << level_counts;

    QMap<QString, int> normalized_counts;

    for (auto it = level_counts.constBegin(); it != level_counts.constEnd(); ++it)
    {
        normalized_counts[it.key().toLower()] = it.value();
    }

    m_level_counts = normalized_counts;
    m_pie_chart->set_values(normalized_counts);
    update_log_level_rows();
}

/**
 * @brief Gets the color for TRACE log level.
 * @return The color for TRACE.
 */
auto LogLevelPieChartWidget::get_trace_color() const -> QColor
{
    return m_trace_color;
}

/**
 * @brief Sets the color for TRACE log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_trace_color(const QColor& color) -> void
{
    m_trace_color = color;
    m_pie_chart->set_segment_color("trace", color);
    update_level_color_square("trace", color);
}

/**
 * @brief Gets the color for DEBUG log level.
 * @return The color for DEBUG.
 */
auto LogLevelPieChartWidget::get_debug_color() const -> QColor
{
    return m_debug_color;
}

/**
 * @brief Sets the color for DEBUG log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_debug_color(const QColor& color) -> void
{
    m_debug_color = color;
    m_pie_chart->set_segment_color("debug", color);
    update_level_color_square("debug", color);
}

/**
 * @brief Gets the color for INFO log level.
 * @return The color for INFO.
 */
auto LogLevelPieChartWidget::get_info_color() const -> QColor
{
    return m_info_color;
}

/**
 * @brief Sets the color for INFO log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_info_color(const QColor& color) -> void
{
    m_info_color = color;
    m_pie_chart->set_segment_color("info", color);
    update_level_color_square("info", color);
}

/**
 * @brief Gets the color for WARNING log level.
 * @return The color for WARNING.
 */
auto LogLevelPieChartWidget::get_warning_color() const -> QColor
{
    return m_warning_color;
}

/**
 * @brief Sets the color for WARNING log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_warning_color(const QColor& color) -> void
{
    m_warning_color = color;
    m_pie_chart->set_segment_color("warning", color);
    update_level_color_square("warning", color);
}

/**
 * @brief Gets the color for ERROR log level.
 * @return The color for ERROR.
 */
auto LogLevelPieChartWidget::get_error_color() const -> QColor
{
    return m_error_color;
}

/**
 * @brief Sets the color for ERROR log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_error_color(const QColor& color) -> void
{
    m_error_color = color;
    m_pie_chart->set_segment_color("error", color);
    update_level_color_square("error", color);
}

/**
 * @brief Gets the color for FATAL log level.
 * @return The color for FATAL.
 */
auto LogLevelPieChartWidget::get_fatal_color() const -> QColor
{
    return m_fatal_color;
}

/**
 * @brief Sets the color for FATAL log level.
 * @param color The new color.
 */
auto LogLevelPieChartWidget::set_fatal_color(const QColor& color) -> void
{
    m_fatal_color = color;
    m_pie_chart->set_segment_color("fatal", color);
    update_level_color_square("fatal", color);
}

/**
 * @brief Gets the inner radius as percent (0-100) of the outer radius.
 * @return The inner radius percent.
 */
auto LogLevelPieChartWidget::get_inner_radius_percent() const -> int
{
    return m_pie_chart->get_inner_radius_percent();
}

/**
 * @brief Sets the inner radius as percent (0-100) of the outer radius.
 * @param percent The inner radius percent.
 */
auto LogLevelPieChartWidget::set_inner_radius_percent(int percent) -> void
{
    m_pie_chart->set_inner_radius_percent(percent);
}

/**
 * @brief Gets the gap angle (in degrees) between segments.
 * @return The gap angle in degrees.
 */
auto LogLevelPieChartWidget::get_segment_gap_angle() const -> int
{
    return m_pie_chart->get_segment_gap_angle();
}

/**
 * @brief Sets the gap angle (in degrees) between segments.
 * @param degrees The gap angle in degrees.
 */
auto LogLevelPieChartWidget::set_segment_gap_angle(int degrees) -> void
{
    m_pie_chart->set_segment_gap_angle(degrees);
}

/**
 * @brief Updates the log level rows below the pie chart.
 *
 * Creates or updates a row for each log level present in m_level_counts.
 * Each row contains a colored square, the log level name, and the percentage value.
 */
auto LogLevelPieChartWidget::update_log_level_rows() -> void
{
    QLayoutItem* item = nullptr;

    while ((item = m_levels_layout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    m_level_row_widgets.clear();

    int total = 0;

    for (const auto& count: m_level_counts)
    {
        total += count;
    }

    if (total == 0)
    {
        m_levels_widget->setVisible(false);
    }
    else
    {
        m_levels_widget->setVisible(true);

        const QStringList levels = {"trace", "debug", "info", "warning", "error", "fatal"};
        const QMap<QString, QColor> color_map = {
            {"trace", m_trace_color},     {"debug", m_debug_color}, {"info", m_info_color},
            {"warning", m_warning_color}, {"error", m_error_color}, {"fatal", m_fatal_color}};

        for (const auto& level: levels)
        {
            if (m_level_counts.contains(level))
            {
                int count = m_level_counts.value(level, 0);
                int percent = static_cast<int>(std::round(100.0 * count / total));

                auto* row_widget = new QWidget(m_levels_widget);
                row_widget->setObjectName("logLevelRowWidget");
                auto* row_layout = new QHBoxLayout(row_widget);
                row_layout->setContentsMargins(0, 0, 0, 0);
                row_layout->setSpacing(8);

                auto* color_label = new QLabel(row_widget);
                color_label->setObjectName("logLevelColorSquare");
                color_label->setFixedSize(18, 18);
                color_label->setStyleSheet(
                    QString("background-color: %1;").arg(color_map.value(level).name()));

                auto* name_label = new QLabel(level.toUpper(), row_widget);
                name_label->setObjectName("logLevelNameLabel");

                auto* percent_label = new QLabel(QString("%1%").arg(percent), row_widget);
                percent_label->setObjectName("logLevelPercentLabel");

                color_label->setFixedHeight(name_label->sizeHint().height());
                color_label->setFixedWidth(name_label->sizeHint().height());

                row_layout->addWidget(color_label);
                row_layout->addWidget(name_label);
                row_layout->addWidget(percent_label);

                m_levels_layout->addWidget(row_widget);
                m_level_row_widgets[level] = row_widget;
            }
        }
    }
}

/**
 * @brief Updates the color square for a given log level row.
 * @param level The log level name (e.g., "trace").
 * @param color The color to apply.
 */
auto LogLevelPieChartWidget::update_level_color_square(const QString& level,
                                                       const QColor& color) -> void
{
    if (m_level_row_widgets.contains(level))
    {
        // Find the first QLabel child (the color square is always the first label)
        auto* row_widget = m_level_row_widgets[level];
        auto* color_label = row_widget->findChild<QLabel*>();

        if (color_label != nullptr)
        {
            color_label->setStyleSheet(
                QString("background-color: %1; border-radius: 3px;").arg(color.name()));
        }
    }
}

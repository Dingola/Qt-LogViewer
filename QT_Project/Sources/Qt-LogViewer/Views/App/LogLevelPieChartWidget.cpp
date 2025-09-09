/**
 * @file LogLevelPieChartWidget.cpp
 * @brief Implementation of LogLevelPieChartWidget using PieChart.
 */

#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"

#include <QHBoxLayout>

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
      m_fatal_color("#ff1744")
{
    setAttribute(Qt::WA_StyledBackground, true);

    m_pie_chart = new PieChart(this);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_pie_chart);
    layout->setContentsMargins(24, 24, 24, 24);
    setLayout(layout);

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
    m_level_counts = level_counts;
    m_pie_chart->set_values(level_counts);
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

/**
 * @file LogLevelPieChartWidget.h
 * @brief Widget for displaying log level distribution as a ring pie chart using PieChart.
 */

#pragma once

#include <QColor>
#include <QMap>
#include <QWidget>

#include "Qt-LogViewer/Views/PieChart.h"

/**
 * @class LogLevelPieChartWidget
 * @brief Displays the percentage distribution of log levels for a log file using PieChart.
 *
 * The colors for each log level can be set via properties. The widget draws only the
 * segments for the levels provided in level_counts. The chart is drawn as a ring (donut)
 * and gaps between segments can be configured.
 */
class LogLevelPieChartWidget: public QWidget
{
        Q_OBJECT
        Q_PROPERTY(QColor trace_color READ get_trace_color WRITE set_trace_color)
        Q_PROPERTY(QColor debug_color READ get_debug_color WRITE set_debug_color)
        Q_PROPERTY(QColor info_color READ get_info_color WRITE set_info_color)
        Q_PROPERTY(QColor warning_color READ get_warning_color WRITE set_warning_color)
        Q_PROPERTY(QColor error_color READ get_error_color WRITE set_error_color)
        Q_PROPERTY(QColor fatal_color READ get_fatal_color WRITE set_fatal_color)
        Q_PROPERTY(
            int inner_radius_percent READ get_inner_radius_percent WRITE set_inner_radius_percent)
        Q_PROPERTY(int segment_gap_angle READ get_segment_gap_angle WRITE set_segment_gap_angle)

    public:
        /**
         * @brief Constructs the log level pie chart widget.
         * @param parent The parent widget.
         */
        explicit LogLevelPieChartWidget(QWidget* parent = nullptr);

        /**
         * @brief Sets the log level counts and updates the chart.
         * @param level_counts Map of log level to count.
         */
        auto set_log_level_counts(const QMap<QString, int>& level_counts) -> void;

        // Color properties for each log level
        [[nodiscard]] auto get_trace_color() const -> QColor;
        auto set_trace_color(const QColor& color) -> void;
        [[nodiscard]] auto get_debug_color() const -> QColor;
        auto set_debug_color(const QColor& color) -> void;
        [[nodiscard]] auto get_info_color() const -> QColor;
        auto set_info_color(const QColor& color) -> void;
        [[nodiscard]] auto get_warning_color() const -> QColor;
        auto set_warning_color(const QColor& color) -> void;
        [[nodiscard]] auto get_error_color() const -> QColor;
        auto set_error_color(const QColor& color) -> void;
        [[nodiscard]] auto get_fatal_color() const -> QColor;
        auto set_fatal_color(const QColor& color) -> void;

        /**
         * @brief Gets the inner radius as percent (0-100) of the outer radius.
         */
        [[nodiscard]] auto get_inner_radius_percent() const -> int;

        /**
         * @brief Sets the inner radius as percent (0-100) of the outer radius.
         *        0 = full pie, 50 = half radius, 80 = thin ring.
         */
        auto set_inner_radius_percent(int percent) -> void;

        /**
         * @brief Gets the gap angle (in degrees) between segments.
         */
        [[nodiscard]] auto get_segment_gap_angle() const -> int;

        /**
         * @brief Sets the gap angle (in degrees) between segments.
         *        0 = no gap, 2-8 = small gap.
         */
        auto set_segment_gap_angle(int degrees) -> void;

    private:
        PieChart* m_pie_chart;
        QMap<QString, int> m_level_counts;
        QColor m_trace_color;
        QColor m_debug_color;
        QColor m_info_color;
        QColor m_warning_color;
        QColor m_error_color;
        QColor m_fatal_color;
};

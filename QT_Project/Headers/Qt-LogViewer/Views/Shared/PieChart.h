/**
 * @file PieChart.h
 * @brief Widget for displaying a ring pie chart with configurable segments and colors.
 */

#pragma once

#include <QColor>
#include <QMap>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QWidget>

/**
 * @class PieChart
 * @brief Displays a ring pie chart with segments for different categories.
 *
 * The chart is drawn as a donut (ring) and supports gaps between segments.
 * Colors and values for each segment can be set via properties and methods.
 */
class PieChart: public QWidget
{
        Q_OBJECT
        Q_PROPERTY(
            int inner_radius_percent READ get_inner_radius_percent WRITE set_inner_radius_percent)
        Q_PROPERTY(int segment_gap_angle READ get_segment_gap_angle WRITE set_segment_gap_angle)

    public:
        /**
         * @brief Constructs the PieChart widget.
         * @param parent The parent widget.
         */
        explicit PieChart(QWidget* parent = nullptr);

        /**
         * @brief Sets the values for each segment and updates the chart.
         * @param values Map of segment name to value.
         */
        auto set_values(const QMap<QString, int>& values) -> void;

        /**
         * @brief Sets the color for a segment.
         * @param segment The segment name.
         * @param color The color to use.
         */
        auto set_segment_color(const QString& segment, const QColor& color) -> void;

        /**
         * @brief Gets the color for a segment.
         * @param segment The segment name.
         * @return The color for the segment.
         */
        [[nodiscard]] auto get_segment_color(const QString& segment) const -> QColor;

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

    protected:
        /**
         * @brief Paint event handler to draw the pie chart.
         * @param event The paint event.
         */
        void paintEvent(QPaintEvent* event) override;

    private:
        /**
         * @brief Draws an empty donut ring when no values exist.
         * @param painter The QPainter to draw with.
         * @param center The center point of the chart.
         * @param outer_radius The outer radius of the chart.
         * @param inner_radius The inner radius of the chart.
         */
        auto draw_empty_ring(QPainter& painter, const QPointF& center, double outer_radius,
                             double inner_radius) -> void;

        /**
         * @brief Draws all segments in fixed order with gaps and inner hole.
         * @param painter The QPainter to draw with.
         * @param center The center point of the chart.
         * @param outer_radius The outer radius of the chart.
         * @param inner_radius The inner radius of the chart.
         * @param total_value The total sum of all segment values.
         */
        auto draw_segments(QPainter& painter, const QPointF& center, double outer_radius,
                           double inner_radius, int total_value) -> void;

        /**
         * @brief Draws a single slice polygon.
         * @param painter The QPainter to draw with.
         * @param center The center point of the chart.
         * @param outer_radius The outer radius of the chart.
         * @param inner_radius The inner radius of the chart.
         * @param start_angle_deg The starting angle (in degrees) of the slice.
         * @param sweep_angle_deg The sweep angle (in degrees) of the slice.
         * @param segment The segment name for color selection.
         */
        auto draw_slice(QPainter& painter, const QPointF& center, double outer_radius,
                        double inner_radius, double start_angle_deg, double sweep_angle_deg,
                        const QString& segment) -> void;

        /**
         * @brief Draws straight gaps between slices.
         * @param painter The QPainter to draw with.
         * @param center The center point of the chart.
         * @param outer_radius The outer radius of the chart.
         * @param inner_radius The inner radius of the chart.
         * @param boundary_angles_deg The angles (in degrees) where gaps should be drawn.
         */
        auto draw_gaps(QPainter& painter, const QPointF& center, double outer_radius,
                       double inner_radius, const QVector<double>& boundary_angles_deg) -> void;

        /**
         * @brief Draws the inner hole of the donut.
         * @param painter The QPainter to draw with.
         * @param center The center point of the chart.
         * @param inner_radius The inner radius of the hole.
         */
        auto draw_inner_hole(QPainter& painter, const QPointF& center, double inner_radius) -> void;

        /**
         * @brief Finds the value for a given segment (case-insensitive).
         * @param segment The segment name to find.
         * @return The count value for the segment, or 0 if not found.
         */
        [[nodiscard]] auto find_segment_value(const QString& segment) const -> int;

    private:
        QMap<QString, int> m_values;
        QMap<QString, QColor> m_segment_colors;
        int m_inner_radius_percent;
        int m_segment_gap_angle;
};

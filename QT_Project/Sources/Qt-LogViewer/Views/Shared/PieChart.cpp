/**
 * @file PieChart.cpp
 * @brief Implementation of PieChart widget using QPainter.
 */

#include "Qt-LogViewer/Views/Shared/PieChart.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

/**
 * @brief Constructs the PieChart widget.
 * @param parent The parent widget.
 */
PieChart::PieChart(QWidget* parent)
    : QWidget(parent),
      m_inner_radius_percent(k_default_inner_radius_percent),
      m_segment_gap_angle(k_default_segment_gap_angle),
      m_empty_ring_color(QColor(k_default_bg_color))
{
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("PieChart");
}

/**
 * @brief Sets the values for each segment and updates the chart.
 * @param values Map of segment name to value.
 */
auto PieChart::set_values(const QMap<QString, int>& values) -> void
{
    m_values = values;
    update();
}

/**
 * @brief Sets the color for a segment.
 * @param segment The segment name.
 * @param color The color to use.
 */
auto PieChart::set_segment_color(const QString& segment, const QColor& color) -> void
{
    m_segment_colors[segment.trimmed().toLower()] = color;
    update();
}

/**
 * @brief Gets the color for a segment.
 * @param segment The segment name.
 * @return The color for the segment.
 */
auto PieChart::get_segment_color(const QString& segment) const -> QColor
{
    QString key = segment.trimmed().toLower();
    QColor result;

    if (m_segment_colors.contains(key))
    {
        result = m_segment_colors.value(key);
    }
    else
    {
        result = QColor(k_default_bg_color);
    }

    return result;
}

/**
 * @brief Gets the inner radius as percent (0-100) of the outer radius.
 * @return The inner radius percent.
 */
auto PieChart::get_inner_radius_percent() const -> int
{
    return m_inner_radius_percent;
}

/**
 * @brief Sets the inner radius as percent (0-100) of the outer radius.
 * @param percent The inner radius percent.
 */
auto PieChart::set_inner_radius_percent(int percent) -> void
{
    m_inner_radius_percent = qBound(0, percent, 100);
    update();
}

/**
 * @brief Gets the gap angle (in degrees) between segments.
 * @return The gap angle in degrees.
 */
auto PieChart::get_segment_gap_angle() const -> int
{
    return m_segment_gap_angle;
}

/**
 * @brief Sets the gap angle (in degrees) between segments.
 * @param degrees The gap angle in degrees.
 */
auto PieChart::set_segment_gap_angle(int degrees) -> void
{
    m_segment_gap_angle = qMax(0, degrees);
    update();
}

/**
 * @brief Gets the current empty (background) ring color.
 * @return The background ring color.
 */
auto PieChart::get_empty_ring_color() const -> QColor
{
    QColor result = m_empty_ring_color;
    return result;
}

/**
 * @brief Sets the empty (background) ring color and repaints.
 * @param color New background ring color.
 */
auto PieChart::set_empty_ring_color(const QColor& color) -> void
{
    if (m_empty_ring_color != color)
    {
        m_empty_ring_color = color;
        update();
    }
}

/**
 * @brief Paint event handler to draw the pie chart.
 * @param event The paint event.
 */
void PieChart::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    int side_length = qMin(width(), height());
    QPointF center_point(width() * 0.5, height() * 0.5);
    double outer_radius = side_length * 0.5;
    double inner_radius = outer_radius * m_inner_radius_percent / 100.0;

    int total_value = 0;

    for (auto v: m_values)
    {
        total_value += v;
    }

    if (total_value == 0)
    {
        draw_empty_ring(painter, center_point, outer_radius, inner_radius);
    }
    else
    {
        draw_segments(painter, center_point, outer_radius, inner_radius, total_value);
    }
}

/**
 * @brief Draws an empty donut ring when no values exist.
 * @param painter The QPainter to draw with.
 * @param center The center point of the chart.
 * @param outer_radius The outer radius of the chart.
 * @param inner_radius The inner radius of the chart.
 */
auto PieChart::draw_empty_ring(QPainter& painter, const QPointF& center, double outer_radius,
                               double inner_radius) -> void
{
    QPainterPath ring;
    ring.addEllipse(center, outer_radius, outer_radius);
    if (inner_radius > k_inner_radius_threshold)
    {
        ring.addEllipse(center, inner_radius, inner_radius);
    }

    painter.setBrush(m_empty_ring_color);
    painter.drawPath(ring);
}

/**
 * @brief Draws all segments in fixed order with gaps and inner hole.
 * @param painter The QPainter to draw with.
 * @param center The center point of the chart.
 * @param outer_radius The outer radius of the chart.
 * @param inner_radius The inner radius of the chart.
 * @param total_value The total sum of all segment values.
 */
auto PieChart::draw_segments(QPainter& painter, const QPointF& center, double outer_radius,
                             double inner_radius, int total_value) -> void
{
    QStringList ordered_segments = m_values.keys();
    QVector<double> values_ordered;
    values_ordered.reserve(ordered_segments.size());

    for (const QString& segment: ordered_segments)
    {
        values_ordered.append(find_segment_value(segment));
    }

    double smallest_slice_angle = 360.0;
    int non_zero_slices = 0;
    for (double value: values_ordered)
    {
        if (value > 0.0)
        {
            smallest_slice_angle = qMin(smallest_slice_angle, 360.0 * value / total_value);
            non_zero_slices += 1;
        }
    }

    // Scale gaps with the chart and preserve at least 75% of the smallest slice.
    const double gap_angle = non_zero_slices > 1 ? qMin(static_cast<double>(m_segment_gap_angle),
                                                        smallest_slice_angle * 0.25)
                                                 : 0.0;
    const double reference_radius = inner_radius > 0.0 ? inner_radius : outer_radius;
    const double gap_width = 2.0 * reference_radius * std::sin(qDegreesToRadians(gap_angle * 0.5));
    double current_angle = 0.0;

    for (int i = 0; i < values_ordered.size(); ++i)
    {
        double value = values_ordered.at(i);
        double percent = (total_value > 0) ? value / static_cast<double>(total_value) : 0.0;
        double slice_angle = 360.0 * percent;

        if (slice_angle > std::numeric_limits<double>::epsilon())
        {
            draw_slice(painter, center, outer_radius, inner_radius, current_angle, slice_angle,
                       gap_width, ordered_segments.at(i));
        }

        current_angle += slice_angle;
    }
}

/**
 * @brief Draws a single slice using outer and inner arcs.
 * @param painter The QPainter to draw
 * with.
 * @param center The center point of the chart.
 * @param outer_radius The outer radius of the chart.
 * @param inner_radius The inner radius of the chart.
 * @param start_angle_deg The starting angle (in degrees) of the slice.
 * @param sweep_angle_deg The sweep angle (in degrees) of the slice.
 * @param gap_width The uniform
 * distance between adjacent slice edges.
 * @param segment The segment name for color selection.
 */
auto PieChart::draw_slice(QPainter& painter, const QPointF& center, double outer_radius,
                          double inner_radius, double start_angle_deg, double sweep_angle_deg,
                          double gap_width, const QString& segment) -> void
{
    QPainterPath path;
    const QRectF outer_rect(center.x() - outer_radius, center.y() - outer_radius,
                            outer_radius * 2.0, outer_radius * 2.0);
    const QRectF inner_rect(center.x() - inner_radius, center.y() - inner_radius,
                            inner_radius * 2.0, inner_radius * 2.0);

    // Intersect parallel, offset slice edges with the original concentric circles.
    const double half_gap = gap_width * 0.5;
    const double outer_inset =
        outer_radius > 0.0 ? qRadiansToDegrees(std::asin(qBound(0.0, half_gap / outer_radius, 1.0)))
                           : 0.0;
    path.arcMoveTo(outer_rect, start_angle_deg + outer_inset);
    path.arcTo(outer_rect, start_angle_deg + outer_inset, sweep_angle_deg - 2.0 * outer_inset);

    // Inner arc (reverse); leave the hole and gaps unpainted.
    if (inner_radius > 0.0)
    {
        const double inner_inset =
            qRadiansToDegrees(std::asin(qBound(0.0, half_gap / inner_radius, 1.0)));
        path.arcTo(inner_rect, start_angle_deg + sweep_angle_deg - inner_inset,
                   -sweep_angle_deg + 2.0 * inner_inset);
    }
    else
    {
        // For a full pie, the offset edges meet on the slice bisector.
        const double half_sweep = qDegreesToRadians(sweep_angle_deg * 0.5);
        const double distance = half_gap > 0.0 ? half_gap / std::sin(half_sweep) : 0.0;
        const double bisector = qDegreesToRadians(start_angle_deg) + half_sweep;
        path.lineTo(center +
                    QPointF(distance * std::cos(bisector), -distance * std::sin(bisector)));
    }
    path.closeSubpath();

    painter.setBrush(get_segment_color(segment));
    painter.drawPath(path);
}

/**
 * @brief Finds the value for a given segment (case-insensitive).
 * @param segment The segment name to find.
 * @return The count value for the segment, or 0 if not found.
 */
auto PieChart::find_segment_value(const QString& segment) const -> int
{
    QString want = segment.trimmed().toLower();

    for (const QString& key: m_values.keys())
    {
        if (key.trimmed().toLower() == want)
        {
            return m_values.value(key);
        }
    }

    return 0;
}

#include "Qt-LogViewer/Views/Shared/PieChartTest.h"

#include <QColor>
#include <QImage>
#include <QMap>
#include <QPixmap>
#include <QString>

/**
 * @brief Sets up the test fixture for each test.
 */
void PieChartTest::SetUp()
{
    m_chart = new PieChart();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void PieChartTest::TearDown()
{
    delete m_chart;
    m_chart = nullptr;
}

/**
 * @brief Tests setting and retrieving segment colors.
 */
TEST_F(PieChartTest, SetAndGetSegmentColor)
{
    m_chart->set_segment_color("A", QColor("#ff0000"));
    m_chart->set_segment_color("B", QColor("#00ff00"));

    EXPECT_EQ(m_chart->get_segment_color("A"), QColor("#ff0000"));
    EXPECT_EQ(m_chart->get_segment_color("B"), QColor("#00ff00"));
}

/**
 * @brief Tests inner radius percent getter and setter.
 */
TEST_F(PieChartTest, InnerRadiusPercent)
{
    m_chart->set_inner_radius_percent(80);
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 80);

    m_chart->set_inner_radius_percent(-10);
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 0);

    m_chart->set_inner_radius_percent(150);
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 100);
}

/**
 * @brief Tests segment gap angle getter and setter.
 */
TEST_F(PieChartTest, SegmentGapAngle)
{
    m_chart->set_segment_gap_angle(7);
    EXPECT_EQ(m_chart->get_segment_gap_angle(), 7);

    m_chart->set_segment_gap_angle(-5);
    EXPECT_EQ(m_chart->get_segment_gap_angle(), 0);
}

/**
 * @brief Tests default color for unknown segment.
 */
TEST_F(PieChartTest, DefaultSegmentColor)
{
    EXPECT_EQ(m_chart->get_segment_color("unknown"), QColor("#e0e0e0"));
}

/**
 * @brief Tests that segment color lookup is case-insensitive.
 */
TEST_F(PieChartTest, SegmentColorCaseInsensitive)
{
    m_chart->set_segment_color("Error", QColor("#ff0000"));
    EXPECT_EQ(m_chart->get_segment_color("error"), QColor("#ff0000"));
    EXPECT_EQ(m_chart->get_segment_color("ERROR"), QColor("#ff0000"));
}

/**
 * @brief Tests that setting a segment color twice overwrites the previous value.
 */
TEST_F(PieChartTest, SegmentColorOverwrite)
{
    m_chart->set_segment_color("A", QColor("#ff0000"));
    m_chart->set_segment_color("A", QColor("#00ff00"));
    EXPECT_EQ(m_chart->get_segment_color("A"), QColor("#00ff00"));
}

/**
 * @brief Tests that setting empty values and colors does not crash and returns defaults.
 */
TEST_F(PieChartTest, EmptyValuesAndColors)
{
    QMap<QString, int> empty_values;
    m_chart->set_values(empty_values);
    EXPECT_EQ(m_chart->get_segment_color("any"), QColor("#e0e0e0"));
}

/**
 * @brief Tests that initial values are set correctly after construction.
 */
TEST_F(PieChartTest, InitialValues)
{
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 60);
    EXPECT_EQ(m_chart->get_segment_gap_angle(), 4);
}

/**
 * @brief Tests that segment names are trimmed before color assignment and lookup.
 */
TEST_F(PieChartTest, SegmentNameTrimming)
{
    m_chart->set_segment_color("  A  ", QColor("#ff0000"));
    EXPECT_EQ(m_chart->get_segment_color("A"), QColor("#ff0000"));
    EXPECT_EQ(m_chart->get_segment_color("  A  "), QColor("#ff0000"));
}

/**
 * @brief Tests that negative and large values for radius and gap are clamped.
 */
TEST_F(PieChartTest, ClampRadiusAndGap)
{
    m_chart->set_inner_radius_percent(-100);
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 0);

    m_chart->set_inner_radius_percent(999);
    EXPECT_EQ(m_chart->get_inner_radius_percent(), 100);

    m_chart->set_segment_gap_angle(-100);
    EXPECT_EQ(m_chart->get_segment_gap_angle(), 0);
}

/**
 * @brief Tests that PieChart renders a non-empty image when values are set.
 */
TEST_F(PieChartTest, PaintEventRendersChart)
{
    QMap<QString, int> values;
    values["A"] = 10;
    values["B"] = 20;
    m_chart->set_values(values);
    m_chart->set_segment_color("A", QColor("#ff0000"));
    m_chart->set_segment_color("B", QColor("#00ff00"));
    m_chart->resize(100, 100);
    m_chart->show();

    QPixmap pixmap(m_chart->size());
    m_chart->render(&pixmap);

    QImage image = pixmap.toImage();
    int non_bg_pixel_count = 0;
    QColor bg_color = QColor("#e0e0e0");

    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (image.pixelColor(x, y) != bg_color)
            {
                ++non_bg_pixel_count;
            }
        }
    }
    EXPECT_GT(non_bg_pixel_count, 0);  // Chart should render something other than background
}

/**
 * @brief Tests that PieChart renders an empty ring when no values are set.
 */
TEST_F(PieChartTest, PaintEventEmptyRing)
{
    m_chart->set_values(QMap<QString, int>());
    m_chart->resize(100, 100);
    m_chart->show();

    QPixmap pixmap(m_chart->size());
    m_chart->render(&pixmap);

    QImage image = pixmap.toImage();
    QColor bg_color = QColor("#e0e0e0");
    int bg_pixel_count = 0;

    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (image.pixelColor(x, y) == bg_color)
            {
                ++bg_pixel_count;
            }
        }
    }
    EXPECT_GT(bg_pixel_count, 0);  // Should have background pixels
}

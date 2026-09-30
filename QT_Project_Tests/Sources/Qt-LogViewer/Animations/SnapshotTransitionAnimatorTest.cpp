/**
 * @file SnapshotTransitionAnimatorTest.cpp
 * @brief Implements tests for generic snapshot-based widget transitions.
 */

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimatorTest.h"

#include <QApplication>
#include <QLabel>
#include <QSignalSpy>
#include <QTest>
#include <QVBoxLayout>
#include <QWidget>

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimator.h"

/** @brief Creates a visible widget surface and its animator. */
void SnapshotTransitionAnimatorTest::SetUp()
{
    m_surface = new QWidget();
    auto* layout = new QVBoxLayout(m_surface);
    m_label = new QLabel(QStringLiteral("Before"), m_surface);
    layout->addWidget(m_label);
    m_surface->resize(320, 180);
    m_animator = new SnapshotTransitionAnimator();
}

/** @brief Destroys the animator and widget surface. */
void SnapshotTransitionAnimatorTest::TearDown()
{
    delete m_animator;
    m_animator = nullptr;
    delete m_surface;
    m_surface = nullptr;
    m_label = nullptr;
}

/** @test Verifies hidden surfaces apply their target state synchronously. */
TEST_F(SnapshotTransitionAnimatorTest, AppliesHiddenSurfaceStateImmediately)
{
    QSignalSpy finished_spy(m_animator, &SnapshotTransitionAnimator::finished);

    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });

    EXPECT_EQ(m_label->text(), QStringLiteral("After"));
    EXPECT_FALSE(m_animator->is_running());
    EXPECT_EQ(finished_spy.count(), 1);
}

/** @test Verifies a crossfade animates an arbitrary visible widget surface. */
TEST_F(SnapshotTransitionAnimatorTest, CrossFadesVisibleSurface)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::CrossFade;
    configuration.duration_ms = 60;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();
    QSignalSpy finished_spy(m_animator, &SnapshotTransitionAnimator::finished);

    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });

    EXPECT_TRUE(m_animator->is_running());
    EXPECT_EQ(m_label->text(), QStringLiteral("After"));
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 1000);
    EXPECT_FALSE(m_animator->is_running());
}

/** @test Verifies a full slide animates an arbitrary visible widget surface. */
TEST_F(SnapshotTransitionAnimatorTest, SlidesVisibleSurface)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::Slide;
    configuration.direction = WidgetTransitionDirection::Left;
    configuration.duration_ms = 60;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();
    QSignalSpy finished_spy(m_animator, &SnapshotTransitionAnimator::finished);

    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });

    EXPECT_TRUE(m_animator->is_running());
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 1000);
    EXPECT_EQ(m_label->text(), QStringLiteral("After"));
}

/** @test Verifies faded and shifted snapshots conceal the unchanged real widget beneath them. */
TEST_F(SnapshotTransitionAnimatorTest, FadeAndSlideUsesOpaqueBackdrop)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::FadeAndSlide;
    configuration.direction = WidgetTransitionDirection::Left;
    configuration.duration_ms = 500;
    configuration.motion_distance = 50;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();

    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });

    QLabel* backdrop = nullptr;
    const QList<QLabel*> direct_labels =
        m_surface->findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly);
    for (QLabel* label: direct_labels)
    {
        if (label->testAttribute(Qt::WA_OpaquePaintEvent))
        {
            backdrop = label;
        }
    }

    ASSERT_NE(backdrop, nullptr);
    ASSERT_FALSE(backdrop->pixmap().isNull());
    const QImage backdrop_image = backdrop->pixmap().toImage();
    EXPECT_EQ(backdrop_image.pixelColor(backdrop_image.rect().center()).alpha(), 255);
    m_animator->finish();
}

/** @test Verifies both snapshots move and the incoming snapshot remains visually on top. */
TEST_F(SnapshotTransitionAnimatorTest, FadeAndSlideMovesBothSnapshots)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::FadeAndSlide;
    configuration.direction = WidgetTransitionDirection::Left;
    configuration.duration_ms = 500;
    configuration.motion_distance = 32;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();

    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });
    m_animator->setProperty("transition_progress", 0.5);

    QList<QLabel*> animated_overlays;
    const QList<QLabel*> direct_labels =
        m_surface->findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly);
    for (QLabel* label: direct_labels)
    {
        if (label->graphicsEffect() != nullptr)
        {
            animated_overlays.append(label);
        }
    }

    ASSERT_EQ(animated_overlays.size(), 2);
    EXPECT_TRUE(animated_overlays.first()->x() == -16 || animated_overlays.last()->x() == -16);
    EXPECT_TRUE(animated_overlays.first()->x() == 16 || animated_overlays.last()->x() == 16);
    EXPECT_EQ(animated_overlays.last()->x(), 16);
    m_animator->finish();
}
/** @test Verifies hiding the surface completes an animation instead of replaying it later. */
TEST_F(SnapshotTransitionAnimatorTest, CompletesTransitionWhenSurfaceIsHidden)
{
    WidgetTransitionConfiguration configuration;
    configuration.duration_ms = 500;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();
    QSignalSpy finished_spy(m_animator, &SnapshotTransitionAnimator::finished);
    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });
    ASSERT_TRUE(m_animator->is_running());

    m_surface->hide();
    QApplication::processEvents();

    EXPECT_FALSE(m_animator->is_running());
    EXPECT_EQ(finished_spy.count(), 1);
}
/** @test Verifies minimizing the surface completes the active transition. */
TEST_F(SnapshotTransitionAnimatorTest, CompletesTransitionWhenSurfaceIsMinimized)
{
    WidgetTransitionConfiguration configuration;
    configuration.duration_ms = 500;
    m_animator->set_configuration(configuration);
    m_surface->show();
    QApplication::processEvents();
    QSignalSpy finished_spy(m_animator, &SnapshotTransitionAnimator::finished);
    m_animator->transition(m_surface, [this] { m_label->setText(QStringLiteral("After")); });
    ASSERT_TRUE(m_animator->is_running());

    m_surface->showMinimized();
    QApplication::processEvents();

    EXPECT_FALSE(m_animator->is_running());
    EXPECT_EQ(finished_spy.count(), 1);
}
/** @test Verifies grouped configuration clamps invalid numeric settings. */
TEST_F(SnapshotTransitionAnimatorTest, AppliesAndClampsConfiguration)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::FadeAndSlide;
    configuration.direction = WidgetTransitionDirection::Down;
    configuration.easing_curve = QEasingCurve::InOutQuad;
    configuration.duration_ms = -1;
    configuration.motion_distance = -5;

    m_animator->set_configuration(configuration);

    const WidgetTransitionConfiguration effective_configuration = m_animator->get_configuration();
    EXPECT_EQ(effective_configuration.effect, WidgetTransitionEffect::FadeAndSlide);
    EXPECT_EQ(effective_configuration.direction, WidgetTransitionDirection::Down);
    EXPECT_EQ(effective_configuration.easing_curve.type(), QEasingCurve::InOutQuad);
    EXPECT_EQ(effective_configuration.duration_ms, 0);
    EXPECT_EQ(effective_configuration.motion_distance, 0);
}

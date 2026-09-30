/**
 * @file StackedWidgetTransitionTest.cpp
 * @brief Implements tests for reusable stacked-widget page transitions.
 */

#include "Qt-LogViewer/Animations/StackedWidgetTransitionTest.h"

#include <QApplication>
#include <QLabel>
#include <QSignalSpy>
#include <QStackedWidget>
#include <QTest>
#include <QVBoxLayout>

#include "Qt-LogViewer/Animations/StackedWidgetTransition.h"

/** @brief Creates a two-page stack and its transition helper. */
void StackedWidgetTransitionTest::SetUp()
{
    m_stack = new QStackedWidget();
    m_stack->resize(320, 180);
    m_stack->addWidget(new QLabel(QStringLiteral("Start page")));
    m_stack->addWidget(new QLabel(QStringLiteral("Workspace")));
    m_transition = new StackedWidgetTransition(m_stack);
}

/** @brief Destroys the transition and its stack. */
void StackedWidgetTransitionTest::TearDown()
{
    delete m_transition;
    m_transition = nullptr;
    delete m_stack;
    m_stack = nullptr;
}

/** @test Verifies hidden stacks switch synchronously without starting animations. */
TEST_F(StackedWidgetTransitionTest, SwitchesHiddenStackImmediately)
{
    QSignalSpy finished_spy(m_transition, &StackedWidgetTransition::finished);
    QSignalSpy activated_spy(m_transition, &StackedWidgetTransition::target_page_activated);

    m_transition->transition_to(1);

    EXPECT_EQ(m_stack->currentIndex(), 1);
    EXPECT_FALSE(m_transition->is_running());
    EXPECT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(finished_spy.count(), 1);
}

/** @test Verifies selecting the current page does not animate or emit transition signals. */
TEST_F(StackedWidgetTransitionTest, IgnoresCurrentPage)
{
    m_stack->show();
    QApplication::processEvents();
    QSignalSpy finished_spy(m_transition, &StackedWidgetTransition::finished);
    QSignalSpy activated_spy(m_transition, &StackedWidgetTransition::target_page_activated);

    m_transition->transition_to(m_stack->currentIndex());

    EXPECT_FALSE(m_transition->is_running());
    EXPECT_TRUE(finished_spy.isEmpty());
    EXPECT_TRUE(activated_spy.isEmpty());
}
/** @test Verifies visible stacks complete fade-through transitions and restore the real page. */
TEST_F(StackedWidgetTransitionTest, AnimatesVisibleStackToTargetPage)
{
    WidgetTransitionConfiguration configuration;
    configuration.duration_ms = 60;
    configuration.motion_distance = 8;
    m_transition->set_configuration(configuration);
    m_stack->show();
    QApplication::processEvents();

    QSignalSpy finished_spy(m_transition, &StackedWidgetTransition::finished);
    QSignalSpy activated_spy(m_transition, &StackedWidgetTransition::target_page_activated);
    m_transition->transition_to(1);

    EXPECT_TRUE(m_transition->is_running());
    m_transition->transition_to(1);
    EXPECT_TRUE(m_transition->is_running());
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 1000);
    EXPECT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(m_stack->currentIndex(), 1);
    EXPECT_TRUE(m_stack->currentWidget()->isVisible());
    EXPECT_FALSE(m_transition->is_running());
}

/** @test Verifies the movement-free crossfade effect completes a visible page change. */
TEST_F(StackedWidgetTransitionTest, AnimatesVisibleStackWithCrossFade)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::CrossFade;
    configuration.easing_curve = QEasingCurve::InOutQuad;
    configuration.duration_ms = 60;
    m_transition->set_configuration(configuration);
    m_stack->show();
    QApplication::processEvents();

    QSignalSpy finished_spy(m_transition, &StackedWidgetTransition::finished);

    m_transition->transition_to(1);

    EXPECT_TRUE(m_transition->is_running());
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 1000);
    EXPECT_EQ(m_stack->currentIndex(), 1);
    EXPECT_FALSE(m_transition->is_running());
}

/** @test Verifies surrounding widgets are updated beneath one shared transition surface. */
TEST_F(StackedWidgetTransitionTest, AnimatesSharedPresentationSurface)
{
    QWidget surface;
    auto* layout = new QVBoxLayout(&surface);
    auto* surrounding_label = new QLabel(QStringLiteral("Initial state"), &surface);
    auto* stack = new QStackedWidget(&surface);
    stack->addWidget(new QLabel(QStringLiteral("Start page")));
    stack->addWidget(new QLabel(QStringLiteral("Workspace")));
    layout->addWidget(surrounding_label);
    layout->addWidget(stack);

    StackedWidgetTransition transition(stack);
    transition.set_transition_surface(&surface);
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::Slide;
    configuration.direction = WidgetTransitionDirection::Left;
    configuration.duration_ms = 60;
    transition.set_configuration(configuration);
    QObject::connect(&transition, &StackedWidgetTransition::target_page_activated, &surface,
                     [surrounding_label](int) {
                         surrounding_label->setText(QStringLiteral("Workspace state"));
                     });

    surface.resize(320, 180);
    surface.show();
    QApplication::processEvents();
    QSignalSpy finished_spy(&transition, &StackedWidgetTransition::finished);

    transition.transition_to(1);

    EXPECT_TRUE(transition.is_running());
    EXPECT_EQ(surrounding_label->text(), QStringLiteral("Workspace state"));
    EXPECT_GE(surface.findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly).size(), 4);
    QTRY_COMPARE_WITH_TIMEOUT(finished_spy.count(), 1, 1000);
    EXPECT_EQ(stack->currentIndex(), 1);
}

/** @test Verifies configuration is applied together and invalid numeric values are clamped. */
TEST_F(StackedWidgetTransitionTest, AppliesAndClampsTransitionConfiguration)
{
    WidgetTransitionConfiguration configuration;
    configuration.effect = WidgetTransitionEffect::CrossFade;
    configuration.direction = WidgetTransitionDirection::Right;
    configuration.easing_curve = QEasingCurve::InOutQuad;
    configuration.duration_ms = -1;
    configuration.motion_distance = -5;

    m_transition->set_configuration(configuration);

    const WidgetTransitionConfiguration effective_configuration = m_transition->get_configuration();

    EXPECT_EQ(effective_configuration.effect, WidgetTransitionEffect::CrossFade);
    EXPECT_EQ(effective_configuration.direction, WidgetTransitionDirection::Right);
    EXPECT_EQ(effective_configuration.easing_curve.type(), QEasingCurve::InOutQuad);
    EXPECT_EQ(effective_configuration.duration_ms, 0);
    EXPECT_EQ(effective_configuration.motion_distance, 0);
}

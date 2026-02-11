/**
 * @file DialogHeaderWidgetTest.cpp
 * @brief Implements tests for the DialogHeaderWidget.
 */

#include "Qt-LogViewer/Views/Shared/DialogHeaderWidgetTest.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>

void DialogHeaderWidgetTest::SetUp()
{
    m_header = new DialogHeaderWidget(QStringLiteral("Initial Title"));
    m_header->resize(300, 40);
    m_header->show();
    QApplication::processEvents();
}

void DialogHeaderWidgetTest::TearDown()
{
    delete m_header;
    m_header = nullptr;
}

/**
 * @test Constructor sets up object names, layout children, and close button properties.
 */
TEST_F(DialogHeaderWidgetTest, ConstructorWiringAndDefaults)
{
    ASSERT_NE(m_header, nullptr);

    EXPECT_EQ(m_header->objectName(), QStringLiteral("DialogHeaderWidget"));

    // Find title label and close button via object names
    auto* title_label = m_header->findChild<QLabel*>();
    ASSERT_NE(title_label, nullptr);
    EXPECT_EQ(title_label->text(), QStringLiteral("Initial Title"));

    auto* close_btn = m_header->findChild<QPushButton*>(QStringLiteral("DialogHeaderCloseButton"));
    ASSERT_NE(close_btn, nullptr);
    EXPECT_TRUE(close_btn->isFlat());
    EXPECT_EQ(close_btn->size(), QSize(24, 24));
    EXPECT_FALSE(close_btn->icon().isNull());
}

/**
 * @test set_title updates the label text.
 */
TEST_F(DialogHeaderWidgetTest, SetTitleUpdatesLabel)
{
    auto* title_label = m_header->findChild<QLabel*>();
    ASSERT_NE(title_label, nullptr);

    m_header->set_title(QStringLiteral("New Title"));
    QApplication::processEvents();

    EXPECT_EQ(title_label->text(), QStringLiteral("New Title"));
}

/**
 * @test Clicking the close button emits close_requested.
 */
TEST_F(DialogHeaderWidgetTest, CloseButtonEmitsCloseRequested)
{
    QSignalSpy spy(m_header, &DialogHeaderWidget::close_requested);
    ASSERT_TRUE(spy.isValid());

    auto* close_btn = m_header->findChild<QPushButton*>(QStringLiteral("DialogHeaderCloseButton"));
    ASSERT_NE(close_btn, nullptr);

    close_btn->click();
    QApplication::processEvents();

    EXPECT_GE(spy.count(), 1);
}

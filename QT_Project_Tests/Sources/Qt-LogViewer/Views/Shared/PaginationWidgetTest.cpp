#include "Qt-LogViewer/Views/Shared/PaginationWidgetTest.h"

#include <QComboBox>
#include <QLineEdit>
#include <QTest>
#include <QToolButton>

/**
 * @brief Sets up the test fixture for each test.
 */
void PaginationWidgetTest::SetUp()
{
    m_widget = new PaginationWidget();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void PaginationWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Tests that set_pagination sets current and total pages correctly.
 */
TEST_F(PaginationWidgetTest, SetPaginationSetsValues)
{
    m_widget->set_pagination(2, 5);
    EXPECT_EQ(m_widget->get_current_page(), 2);
    EXPECT_EQ(m_widget->get_total_pages(), 5);

    m_widget->set_pagination(0, 0);  // Test for lower bounds
    EXPECT_EQ(m_widget->get_current_page(), 1);
    EXPECT_EQ(m_widget->get_total_pages(), 1);

    m_widget->set_pagination(10, 3);  // Test for upper bounds
    EXPECT_EQ(m_widget->get_current_page(), 3);
    EXPECT_EQ(m_widget->get_total_pages(), 3);
}

/**
 * @brief Tests that set_max_page_buttons enforces minimum and updates state.
 */
TEST_F(PaginationWidgetTest, SetMaxPageButtons)
{
    m_widget->set_max_page_buttons(2);
    EXPECT_GE(m_widget->get_total_pages(), 1);  // Should not crash, min is enforced internally

    m_widget->set_max_page_buttons(10);
    m_widget->set_pagination(1, 10);
    EXPECT_EQ(m_widget->get_current_page(), 1);
}

/**
 * @brief Tests that page_changed signal is emitted when navigating pages via UI buttons.
 */
TEST_F(PaginationWidgetTest, PageChangedSignalEmittedByButton)
{
    QSignalSpy spy(m_widget, SIGNAL(page_changed(int)));
    m_widget->set_pagination(1, 3);

    QToolButton* next_button = m_widget->findChild<QToolButton*>("buttonNext");
    ASSERT_NE(next_button, nullptr);
    QTest::mouseClick(next_button, Qt::LeftButton);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 2);

    QToolButton* prev_button = m_widget->findChild<QToolButton*>("buttonPrev");
    ASSERT_NE(prev_button, nullptr);
    QTest::mouseClick(prev_button, Qt::LeftButton);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 1);
}

/**
 * @brief Tests that page_changed signal is emitted when jumping to a page via QLineEdit.
 */
TEST_F(PaginationWidgetTest, PageChangedSignalEmittedByJumpTo)
{
    QSignalSpy spy(m_widget, SIGNAL(page_changed(int)));
    m_widget->set_pagination(1, 5);

    QLineEdit* jump_edit = m_widget->findChild<QLineEdit*>("lineEditJumpTo");
    ASSERT_NE(jump_edit, nullptr);
    jump_edit->setText("3");
    QTest::keyClick(jump_edit, Qt::Key_Enter);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 3);
}

/**
 * @brief Tests that items_per_page_changed signal is emitted when changing items per page via
 * ComboBox.
 */
TEST_F(PaginationWidgetTest, ItemsPerPageChangedSignalEmittedByComboBox)
{
    QSignalSpy spy(m_widget, SIGNAL(items_per_page_changed(int)));
    QComboBox* combo = m_widget->findChild<QComboBox*>("comboBoxItemsPerPage");
    ASSERT_NE(combo, nullptr);
    combo->setCurrentIndex(1);  // Index 1 corresponds to 50
    QTest::qWait(10);           // Allow signal to propagate
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 50);
}

/**
 * @brief Tests get_items_per_page returns correct value after ComboBox change.
 */
TEST_F(PaginationWidgetTest, GetItemsPerPageReturnsCorrectValue)
{
    QComboBox* combo = m_widget->findChild<QComboBox*>("comboBoxItemsPerPage");
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(m_widget->get_items_per_page(), 25);
    combo->setCurrentIndex(2);  // Index 2 is 100
    QTest::qWait(10);
    EXPECT_EQ(m_widget->get_items_per_page(), 100);
}

/**
 * @brief Tests page button arrangement for max_page_buttons == 3.
 */
TEST_F(PaginationWidgetTest, PageButtonsWithThreeMaxButtons)
{
    m_widget->set_max_page_buttons(3);
    m_widget->set_pagination(1, 10);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 3 buttons, no ellipsis
    EXPECT_EQ(buttons.size(), 3);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "2");
    EXPECT_EQ(buttons[2]->text(), "10");
    EXPECT_TRUE(buttons[0]->isChecked());
}

/**
 * @brief Tests page button arrangement for max_page_buttons == 4.
 */
TEST_F(PaginationWidgetTest, PageButtonsWithFourMaxButtons)
{
    m_widget->set_max_page_buttons(4);
    m_widget->set_pagination(1, 10);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 4 buttons, one ellipsis
    EXPECT_EQ(buttons.size(), 4);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "2");
    EXPECT_EQ(buttons[2]->text(), "...");
    EXPECT_EQ(buttons[3]->text(), "10");
    EXPECT_TRUE(buttons[0]->isChecked());
}

/**
 * @brief Tests page button arrangement for max_page_buttons == 5.
 */
TEST_F(PaginationWidgetTest, PageButtonsWithFiveMaxButtons)
{
    m_widget->set_max_page_buttons(5);
    m_widget->set_pagination(1, 10);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 5 buttons, one ellipsis
    EXPECT_EQ(buttons.size(), 5);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "2");
    EXPECT_EQ(buttons[2]->text(), "3");
    EXPECT_EQ(buttons[3]->text(), "...");
    EXPECT_EQ(buttons[4]->text(), "10");
    EXPECT_TRUE(buttons[0]->isChecked());
}

/**
 * @brief Tests page button arrangement for max_page_buttons == 6.
 */
TEST_F(PaginationWidgetTest, PageButtonsWithSixMaxButtons)
{
    m_widget->set_max_page_buttons(6);
    m_widget->set_pagination(1, 10);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 6 buttons, one ellipsis
    EXPECT_EQ(buttons.size(), 6);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "2");
    EXPECT_EQ(buttons[2]->text(), "3");
    EXPECT_EQ(buttons[3]->text(), "4");
    EXPECT_EQ(buttons[4]->text(), "...");
    EXPECT_EQ(buttons[5]->text(), "10");
    EXPECT_TRUE(buttons[0]->isChecked());
}

/**
 * @brief Tests page button arrangement for max_page_buttons == 7.
 */
TEST_F(PaginationWidgetTest, PageButtonsWithSevenMaxButtons)
{
    m_widget->set_max_page_buttons(7);
    m_widget->set_pagination(1, 20);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 7 buttons, one ellipsis
    EXPECT_EQ(buttons.size(), 7);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "2");
    EXPECT_EQ(buttons[2]->text(), "3");
    EXPECT_EQ(buttons[3]->text(), "4");
    EXPECT_EQ(buttons[4]->text(), "5");
    EXPECT_EQ(buttons[5]->text(), "...");
    EXPECT_EQ(buttons[6]->text(), "20");
    EXPECT_TRUE(buttons[0]->isChecked());
}

/**
 * @brief Tests ellipsis at both ends when current page is in the middle.
 */
TEST_F(PaginationWidgetTest, PageButtonsEllipsisBothEnds)
{
    m_widget->set_max_page_buttons(7);
    m_widget->set_pagination(10, 20);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 7 buttons, two ellipsis
    EXPECT_EQ(buttons.size(), 7);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "...");
    EXPECT_EQ(buttons[5]->text(), "...");
    EXPECT_EQ(buttons[6]->text(), "20");
    EXPECT_TRUE(buttons[3]->isChecked());  // current page button
    EXPECT_EQ(buttons[3]->text(), "10");
}

/**
 * @brief Tests ellipsis at start when current page is last.
 */
TEST_F(PaginationWidgetTest, PageButtonsEllipsisAtStart)
{
    m_widget->set_max_page_buttons(7);
    m_widget->set_pagination(20, 20);

    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);
    QList<QToolButton*> buttons = page_buttons_widget->findChildren<QToolButton*>();

    // Should have exactly 7 buttons, one ellipsis at start
    EXPECT_EQ(buttons.size(), 7);
    EXPECT_EQ(buttons[0]->text(), "1");
    EXPECT_EQ(buttons[1]->text(), "...");
    EXPECT_EQ(buttons[6]->text(), "20");
    EXPECT_TRUE(buttons[6]->isChecked());
}

/**
 * @brief Tests that navigation buttons are enabled/disabled correctly at boundaries.
 */
TEST_F(PaginationWidgetTest, NavigationButtonsEnabledDisabled)
{
    m_widget->set_pagination(1, 3);

    QToolButton* prev_button = m_widget->findChild<QToolButton*>("buttonPrev");
    QToolButton* next_button = m_widget->findChild<QToolButton*>("buttonNext");
    QToolButton* jump_prev_button = m_widget->findChild<QToolButton*>("buttonJumpPrev");
    QToolButton* jump_next_button = m_widget->findChild<QToolButton*>("buttonJumpNext");
    ASSERT_NE(prev_button, nullptr);
    ASSERT_NE(next_button, nullptr);
    ASSERT_NE(jump_prev_button, nullptr);
    ASSERT_NE(jump_next_button, nullptr);

    // On first page
    EXPECT_FALSE(prev_button->isEnabled());
    EXPECT_TRUE(next_button->isEnabled());
    EXPECT_FALSE(jump_prev_button->isEnabled());
    EXPECT_TRUE(jump_next_button->isEnabled());

    // On last page
    m_widget->set_pagination(3, 3);
    EXPECT_TRUE(prev_button->isEnabled());
    EXPECT_FALSE(next_button->isEnabled());
    EXPECT_TRUE(jump_prev_button->isEnabled());
    EXPECT_FALSE(jump_next_button->isEnabled());

    // Only one page
    m_widget->set_pagination(1, 1);
    EXPECT_FALSE(prev_button->isEnabled());
    EXPECT_FALSE(next_button->isEnabled());
    EXPECT_FALSE(jump_prev_button->isEnabled());
    EXPECT_FALSE(jump_next_button->isEnabled());
}

/**
 * @brief Tests that invalid jump-to input is corrected to valid page.
 */
TEST_F(PaginationWidgetTest, JumpToInputValidation)
{
    QSignalSpy spy(m_widget, SIGNAL(page_changed(int)));
    m_widget->set_pagination(2, 5);

    QLineEdit* jump_edit = m_widget->findChild<QLineEdit*>("lineEditJumpTo");
    ASSERT_NE(jump_edit, nullptr);

    // Negative input
    jump_edit->setText("-1");
    QTest::keyClick(jump_edit, Qt::Key_Enter);
    EXPECT_EQ(spy.count(), 0);

    // Too large input
    jump_edit->setText("999");
    QTest::keyClick(jump_edit, Qt::Key_Enter);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 5);

    // Non-numeric input (should stay on current page, no signal)
    jump_edit->setText("abc");
    QTest::keyClick(jump_edit, Qt::Key_Enter);
    EXPECT_EQ(spy.count(), 0);

    // Input equals current page (no signal)
    jump_edit->setText("5");
    m_widget->set_pagination(5, 5);
    QTest::keyClick(jump_edit, Qt::Key_Enter);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Tests that page_changed signal is only emitted on actual page change.
 */
TEST_F(PaginationWidgetTest, PageChangedSignalEmittedOnlyOnChange)
{
    QSignalSpy spy(m_widget, SIGNAL(page_changed(int)));
    m_widget->set_pagination(2, 5);

    // Click next (should emit)
    QToolButton* next_button = m_widget->findChild<QToolButton*>("buttonNext");
    ASSERT_NE(next_button, nullptr);
    QTest::mouseClick(next_button, Qt::LeftButton);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 3);

    // Click next again (should emit)
    QTest::mouseClick(next_button, Qt::LeftButton);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 4);

    // Click next on last page (should not emit)
    m_widget->set_pagination(5, 5);
    QTest::mouseClick(next_button, Qt::LeftButton);
    EXPECT_EQ(spy.count(), 0);
}

/**
 * @brief Tests that ComboBox contains correct items and data.
 */
TEST_F(PaginationWidgetTest, ComboBoxItemsAndData)
{
    QComboBox* combo = m_widget->findChild<QComboBox*>("comboBoxItemsPerPage");
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), 4);
    EXPECT_EQ(combo->itemText(0), "25");
    EXPECT_EQ(combo->itemData(0).toInt(), 25);
    EXPECT_EQ(combo->itemText(1), "50");
    EXPECT_EQ(combo->itemData(1).toInt(), 50);
    EXPECT_EQ(combo->itemText(2), "100");
    EXPECT_EQ(combo->itemData(2).toInt(), 100);
    EXPECT_EQ(combo->itemText(3), "200");
    EXPECT_EQ(combo->itemData(3).toInt(), 200);
}

/**
 * @brief Tests that page buttons are not duplicated after multiple updates.
 */
TEST_F(PaginationWidgetTest, NoDuplicatePageButtonsAfterUpdates)
{
    QWidget* page_buttons_widget = m_widget->findChild<QWidget*>("pageButtonsWidget");
    ASSERT_NE(page_buttons_widget, nullptr);

    m_widget->set_max_page_buttons(7);
    m_widget->set_pagination(1, 10);
    QList<QToolButton*> buttons1 = page_buttons_widget->findChildren<QToolButton*>();
    int count1 = buttons1.size();

    m_widget->set_pagination(5, 10);
    QList<QToolButton*> buttons2 = page_buttons_widget->findChildren<QToolButton*>();
    int count2 = buttons2.size();

    EXPECT_EQ(count1, count2);  // Button count should remain consistent
}

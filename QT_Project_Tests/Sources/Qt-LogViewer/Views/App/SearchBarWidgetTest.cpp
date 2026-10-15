/**
 * @file SearchBarWidgetTest.cpp
 * @brief Tests inline regular-expression validation in the search bar.
 */

#include "Qt-LogViewer/Views/App/SearchBarWidgetTest.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

/** @test Verifies that an invalid regex is explained and cannot start a search. */
TEST_F(SearchBarWidgetTest, RejectsInvalidRegularExpression)
{
    SearchBarWidget widget;
    auto* search_edit = widget.findChild<QLineEdit*>(QStringLiteral("lineEditSearch"));
    auto* regex_check_box = widget.findChild<QCheckBox*>(QStringLiteral("checkBoxRegEx"));
    auto* search_button = widget.findChild<QPushButton*>(QStringLiteral("pushButtonSearch"));
    auto* error_label = widget.findChild<QLabel*>(QStringLiteral("labelRegexError"));
    QSignalSpy search_spy(&widget, &SearchBarWidget::search_requested);

    ASSERT_NE(search_edit, nullptr);
    ASSERT_NE(regex_check_box, nullptr);
    ASSERT_NE(search_button, nullptr);
    ASSERT_NE(error_label, nullptr);

    regex_check_box->setChecked(true);
    search_edit->setText(QStringLiteral("("));

    EXPECT_FALSE(widget.has_valid_search_pattern());
    EXPECT_EQ(search_edit->property("validationState").toString(), QStringLiteral("invalid"));
    EXPECT_FALSE(search_button->isEnabled());
    EXPECT_FALSE(error_label->isHidden());
    EXPECT_FALSE(error_label->text().isEmpty());
    EXPECT_EQ(search_spy.count(), 0);
}

/** @test Verifies that correcting a regex clears validation and resumes live search. */
TEST_F(SearchBarWidgetTest, AcceptsCorrectedRegularExpression)
{
    SearchBarWidget widget;
    auto* search_edit = widget.findChild<QLineEdit*>(QStringLiteral("lineEditSearch"));
    auto* regex_check_box = widget.findChild<QCheckBox*>(QStringLiteral("checkBoxRegEx"));
    auto* search_button = widget.findChild<QPushButton*>(QStringLiteral("pushButtonSearch"));
    auto* error_label = widget.findChild<QLabel*>(QStringLiteral("labelRegexError"));
    QSignalSpy search_spy(&widget, &SearchBarWidget::search_requested);

    ASSERT_NE(search_edit, nullptr);
    ASSERT_NE(regex_check_box, nullptr);
    ASSERT_NE(search_button, nullptr);
    ASSERT_NE(error_label, nullptr);

    regex_check_box->setChecked(true);
    search_edit->setText(QStringLiteral("("));
    search_edit->setText(QStringLiteral("error.*"));

    EXPECT_TRUE(widget.has_valid_search_pattern());
    EXPECT_EQ(search_edit->property("validationState").toString(), QStringLiteral("valid"));
    EXPECT_TRUE(search_button->isEnabled());
    EXPECT_TRUE(error_label->isHidden());
    ASSERT_EQ(search_spy.count(), 1);
    EXPECT_EQ(search_spy.takeFirst().at(0).toString(), QStringLiteral("error.*"));
}

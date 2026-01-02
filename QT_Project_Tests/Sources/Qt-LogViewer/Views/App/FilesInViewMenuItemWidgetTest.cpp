#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidgetTest.h"

#include <QFileInfo>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QTextStream>
#include <QToolButton>

/**
 * @brief Constructs the test fixture.
 */
FilesInViewMenuItemWidgetTest::FilesInViewMenuItemWidgetTest(): m_widget(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
FilesInViewMenuItemWidgetTest::~FilesInViewMenuItemWidgetTest() = default;

/**
 * @brief Set up before each test: create widget.
 */
void FilesInViewMenuItemWidgetTest::SetUp()
{
    m_widget = new TestableFilesInViewMenuItemWidget();
    m_widget->resize(300, 24);
}

/**
 * @brief Tear down after each test.
 */
void FilesInViewMenuItemWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Accesses private child controls via QObject lookup.
 */
auto FilesInViewMenuItemWidgetTest::get_controls(FilesInViewMenuItemWidget* widget) const
    -> std::tuple<QLabel*, QToolButton*, QToolButton*, QToolButton*>
{
    QLabel* label = widget->findChild<QLabel*>();
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle_visibility = nullptr;
    QToolButton* btn_remove = nullptr;

    const QList<QToolButton*> buttons = widget->findChildren<QToolButton*>();
    for (const auto* b: buttons)
    {
        const QString tt = b->toolTip();
        if (tt.contains("Show only", Qt::CaseInsensitive))
        {
            btn_show_only = const_cast<QToolButton*>(b);
        }
        else
        {
            if (tt.contains("Hide", Qt::CaseInsensitive) || tt.contains("Show this file"))
            {
                btn_toggle_visibility = const_cast<QToolButton*>(b);
            }
            else
            {
                btn_remove = const_cast<QToolButton*>(b);
            }
        }
    }

    auto result = std::make_tuple(label, btn_show_only, btn_toggle_visibility, btn_remove);
    return result;
}

/**
 * @brief Initial state: defaults and no crash updating label.
 */
TEST_F(FilesInViewMenuItemWidgetTest, InitialStateDefaults)
{
    ASSERT_NE(m_widget, nullptr);

    EXPECT_EQ(m_widget->get_icon_color(), QColor("#666666"));
    EXPECT_EQ(m_widget->get_icon_color_hover(), QColor("#42a5f5"));
    EXPECT_FALSE(m_widget->get_icon_color_eye().isValid());
    EXPECT_FALSE(m_widget->get_icon_color_eye_off().isValid());
    EXPECT_FALSE(m_widget->get_icon_color_trash().isValid());
    EXPECT_EQ(m_widget->get_icon_px(), 14);
    EXPECT_TRUE(m_widget->get_file_path().isEmpty());
    EXPECT_FALSE(m_widget->get_icon_eye_path().isEmpty());
    EXPECT_FALSE(m_widget->get_icon_eye_off_path().isEmpty());
    EXPECT_FALSE(m_widget->get_icon_trash_path().isEmpty());
    EXPECT_FALSE(m_widget->get_icon_show_only_path().isEmpty());
    EXPECT_EQ(m_widget->get_label_reserved_px(), 60);

    QLabel* label = nullptr;
    QToolButton* b1 = nullptr;
    QToolButton* b2 = nullptr;
    QToolButton* b3 = nullptr;
    std::tie(label, b1, b2, b3) = get_controls(m_widget);
    ASSERT_NE(label, nullptr);
    ASSERT_NE(b1, nullptr);
    ASSERT_NE(b2, nullptr);
    ASSERT_NE(b3, nullptr);

    EXPECT_TRUE(label->text().isEmpty());
    EXPECT_TRUE(label->toolTip().isEmpty());
}

/**
 * @brief set_file_path updates label tooltip, and filename elision shows base name for existing
 * file.
 */
TEST_F(FilesInViewMenuItemWidgetTest, SetFilePathUpdatesLabelAndTooltip)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "dummy";
    out.flush();
    temp_file.close();

    const QString path = temp_file.fileName();
    const QString base = QFileInfo(path).fileName();

    m_widget->set_file_path(path);

    EXPECT_EQ(m_widget->get_file_path(), path);

    QLabel* label = nullptr;
    QToolButton* b1 = nullptr;
    QToolButton* b2 = nullptr;
    QToolButton* b3 = nullptr;
    std::tie(label, b1, b2, b3) = get_controls(m_widget);
    ASSERT_NE(label, nullptr);

    EXPECT_EQ(label->toolTip(), path);
    EXPECT_TRUE(label->text().contains(base));
}

/**
 * @brief Reserved label pixels clamps to >=0 and affects elision width.
 */
TEST_F(FilesInViewMenuItemWidgetTest, LabelReservedPixelsClampAndAffectElision)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    const QString path = temp_file.fileName();
    temp_file.close();

    m_widget->resize(200, 24);
    m_widget->set_file_path(path);

    QLabel* label = nullptr;
    QToolButton* b1 = nullptr;
    QToolButton* b2 = nullptr;
    QToolButton* b3 = nullptr;
    std::tie(label, b1, b2, b3) = get_controls(m_widget);
    ASSERT_NE(label, nullptr);

    const QString t1 = label->text();

    m_widget->set_label_reserved_px(120);
    const QString t2 = label->text();
    EXPECT_NE(t1, t2);

    m_widget->set_label_reserved_px(-5);
    EXPECT_EQ(m_widget->get_label_reserved_px(), 0);
}

/**
 * @brief Hidden state toggles tooltip text for visibility button and keeps buttons enabled.
 */
TEST_F(FilesInViewMenuItemWidgetTest, HiddenStateTogglesTooltipAndKeepsButtonsEnabled)
{
    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_controls(m_widget);

    ASSERT_NE(btn_show_only, nullptr);
    ASSERT_NE(btn_toggle, nullptr);
    ASSERT_NE(btn_remove, nullptr);

    m_widget->set_hidden_state(false);
    EXPECT_TRUE(btn_toggle->toolTip().contains("Hide", Qt::CaseInsensitive));
    EXPECT_TRUE(btn_toggle->isEnabled());
    EXPECT_TRUE(btn_show_only->isEnabled());

    m_widget->set_hidden_state(true);
    EXPECT_TRUE(btn_toggle->toolTip().contains("Show", Qt::CaseInsensitive));
    EXPECT_TRUE(btn_toggle->isEnabled());
    EXPECT_TRUE(btn_show_only->isEnabled());
}

/**
 * @brief Icon size setter clamps and updates button icon sizes.
 */
TEST_F(FilesInViewMenuItemWidgetTest, IconPxClampAndApplyToButtons)
{
    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_controls(m_widget);

    ASSERT_NE(btn_show_only, nullptr);
    ASSERT_NE(btn_toggle, nullptr);
    ASSERT_NE(btn_remove, nullptr);

    m_widget->set_icon_px(20);
    EXPECT_EQ(m_widget->get_icon_px(), 20);
    EXPECT_EQ(btn_show_only->iconSize(), QSize(20, 20));
    EXPECT_EQ(btn_toggle->iconSize(), QSize(20, 20));
    EXPECT_EQ(btn_remove->iconSize(), QSize(20, 20));

    m_widget->set_icon_px(0);
    EXPECT_EQ(m_widget->get_icon_px(), 1);
    EXPECT_EQ(btn_show_only->iconSize(), QSize(1, 1));
    EXPECT_EQ(btn_toggle->iconSize(), QSize(1, 1));
    EXPECT_EQ(btn_remove->iconSize(), QSize(1, 1));
}

/**
 * @brief Icon paths setters update and reapply icons without crashing.
 */
TEST_F(FilesInViewMenuItemWidgetTest, IconPathsSettersUpdateValues)
{
    const QString eye = QStringLiteral(":/Resources/Icons/eye.svg");
    const QString eye_off = QStringLiteral(":/Resources/Icons/eye-off.svg");
    const QString trash = QStringLiteral(":/Resources/Icons/trash.svg");
    const QString show_only = QStringLiteral(":/Resources/Icons/show-only.svg");

    m_widget->set_icon_paths(eye, eye_off, trash);
    EXPECT_EQ(m_widget->get_icon_eye_path(), eye);
    EXPECT_EQ(m_widget->get_icon_eye_off_path(), eye_off);
    EXPECT_EQ(m_widget->get_icon_trash_path(), trash);

    m_widget->set_icon_eye_path(eye);
    EXPECT_EQ(m_widget->get_icon_eye_path(), eye);

    m_widget->set_icon_eye_off_path(eye_off);
    EXPECT_EQ(m_widget->get_icon_eye_off_path(), eye_off);

    m_widget->set_icon_trash_path(trash);
    EXPECT_EQ(m_widget->get_icon_trash_path(), trash);

    m_widget->set_icon_show_only_path(show_only);
    EXPECT_EQ(m_widget->get_icon_show_only_path(), show_only);
}

/**
 * @brief Icon color setters update values and trigger reapply without crashing.
 */
TEST_F(FilesInViewMenuItemWidgetTest, IconColorSettersUpdateValues)
{
    const QColor base("#123456");
    const QColor hover("#abcdef");
    const QColor eye("#111111");
    const QColor eye_off("#222222");
    const QColor trash("#333333");

    m_widget->set_icon_color(base);
    m_widget->set_icon_color_hover(hover);
    m_widget->set_icon_color_eye(eye);
    m_widget->set_icon_color_eye_off(eye_off);
    m_widget->set_icon_color_trash(trash);

    EXPECT_EQ(m_widget->get_icon_color(), base);
    EXPECT_EQ(m_widget->get_icon_color_hover(), hover);
    EXPECT_EQ(m_widget->get_icon_color_eye(), eye);
    EXPECT_EQ(m_widget->get_icon_color_eye_off(), eye_off);
    EXPECT_EQ(m_widget->get_icon_color_trash(), trash);
}

/**
 * @brief eventFilter returns false on hover/leave/press/release without
 * consuming.
 */
TEST_F(FilesInViewMenuItemWidgetTest, EventFilterHoverPressReleaseReturnsFalse)
{
    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_controls(m_widget);

    ASSERT_NE(btn_show_only, nullptr);
    ASSERT_NE(btn_toggle, nullptr);
    ASSERT_NE(btn_remove, nullptr);

    QEvent enter_event(QEvent::Enter);
    const bool enter_handled = m_widget->call_event_filter(btn_show_only, &enter_event);
    EXPECT_FALSE(enter_handled);

    QEvent leave_event(QEvent::Leave);
    const bool leave_handled = m_widget->call_event_filter(btn_toggle, &leave_event);
    EXPECT_FALSE(leave_handled);

    QEvent press_event(QEvent::MouseButtonPress);
    const bool press_handled = m_widget->call_event_filter(btn_remove, &press_event);
    EXPECT_FALSE(press_handled);

    QEvent release_event(QEvent::MouseButtonRelease);
    const bool release_handled = m_widget->call_event_filter(btn_toggle, &release_event);
    EXPECT_FALSE(release_handled);
}

/**
 * @brief Clicking buttons emits signals with the current file path.
 */
TEST_F(FilesInViewMenuItemWidgetTest, ButtonClicksEmitSignalsWithFilePath)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    temp_file.close();

    const QString path = temp_file.fileName();
    m_widget->set_file_path(path);

    QSignalSpy spy_show_only(m_widget, &FilesInViewMenuItemWidget::show_only_requested);
    QSignalSpy spy_toggle(m_widget, &FilesInViewMenuItemWidget::toggle_visibility_requested);
    QSignalSpy spy_remove(m_widget, &FilesInViewMenuItemWidget::remove_requested);

    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_controls(m_widget);

    ASSERT_NE(btn_show_only, nullptr);
    ASSERT_NE(btn_toggle, nullptr);
    ASSERT_NE(btn_remove, nullptr);

    btn_show_only->click();
    btn_toggle->click();
    btn_remove->click();

    EXPECT_EQ(spy_show_only.count(), 1);
    EXPECT_EQ(spy_toggle.count(), 1);
    EXPECT_EQ(spy_remove.count(), 1);

    EXPECT_EQ(spy_show_only.takeFirst().at(0).toString(), path);
    EXPECT_EQ(spy_toggle.takeFirst().at(0).toString(), path);
    EXPECT_EQ(spy_remove.takeFirst().at(0).toString(), path);
}

#include "Qt-LogViewer/Views/App/FilesInViewWidgetActionTest.h"

#include <QApplication>
#include <QColor>
#include <QMenu>
#include <QSignalSpy>
#include <QWidgetAction>

struct HostedRow {
        QMenu* menu = nullptr;
        FilesInViewMenuItemWidget* row = nullptr;
};

/**
 * @brief Helper: create hosted row and keep menu alive until caller deletes it.
 */
static auto create_hosted_row_with_menu(FilesInViewWidgetAction* action) -> HostedRow
{
    HostedRow result;

    auto* menu = new QMenu();
    menu->addAction(action);
    menu->show();
    QApplication::processEvents();

    const auto rows = menu->findChildren<FilesInViewMenuItemWidget*>();
    if (!rows.isEmpty())
    {
        result.menu = menu;
        result.row = rows.first();
    }
    else
    {
        delete menu;
    }

    return result;
}

/**
 * @brief Constructs the test fixture.
 */
FilesInViewWidgetActionTest::FilesInViewWidgetActionTest(): m_action(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
FilesInViewWidgetActionTest::~FilesInViewWidgetActionTest() = default;

/**
 * @brief Set up before each test: create the action under test.
 */
void FilesInViewWidgetActionTest::SetUp()
{
    m_action = new FilesInViewWidgetAction();
}

/**
 * @brief Tear down after each test.
 */
void FilesInViewWidgetActionTest::TearDown()
{
    delete m_action;
    m_action = nullptr;
}

/**
 * @brief Creates a hosted row widget via QMenu+QWidgetAction and returns the created widget.
 */
auto FilesInViewWidgetActionTest::create_hosted_row(FilesInViewWidgetAction* action)
    -> FilesInViewMenuItemWidget*
{
    FilesInViewMenuItemWidget* created_row = nullptr;

    QMenu menu;
    menu.addAction(action);

    menu.show();
    QApplication::processEvents();

    const auto rows = menu.findChildren<FilesInViewMenuItemWidget*>();
    if (!rows.isEmpty())
    {
        created_row = rows.first();
    }

    auto result = created_row;
    return result;
}

/**
 * @brief Finds inline buttons from the embedded row widget.
 */
auto FilesInViewWidgetActionTest::get_row_controls(FilesInViewMenuItemWidget* row)
    -> std::tuple<QLabel*, QToolButton*, QToolButton*, QToolButton*>
{
    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle_visibility = nullptr;
    QToolButton* btn_remove = nullptr;

    if (row != nullptr)
    {
        label = row->findChild<QLabel*>();
        const QList<QToolButton*> buttons = row->findChildren<QToolButton*>();

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
    }

    auto result = std::make_tuple(label, btn_show_only, btn_toggle_visibility, btn_remove);
    return result;
}

/**
 * @brief Default construction: verify defaults.
 */
TEST_F(FilesInViewWidgetActionTest, DefaultsOnConstruction)
{
    ASSERT_NE(m_action, nullptr);

    EXPECT_EQ(m_action->get_file_path(), QString());
    EXPECT_EQ(m_action->get_icon_color(), QColor("#666666"));
    EXPECT_EQ(m_action->get_icon_color_hover(), QColor("#42a5f5"));
    EXPECT_EQ(m_action->get_icon_px(), 14);
    EXPECT_FALSE(m_action->get_hidden_effective());
}

/**
 * @brief Set/get basic properties: file path, colors, size, hidden flag.
 */
TEST_F(FilesInViewWidgetActionTest, SettersAndGettersReflectValues)
{
    ASSERT_NE(m_action, nullptr);

    const QString path = QStringLiteral("C:/tmp/sample.log");
    const QColor base("#111111");
    const QColor hover("#222222");

    m_action->set_file_path(path);
    m_action->set_icon_color(base);
    m_action->set_icon_color_hover(hover);

    m_action->set_icon_px(20);
    EXPECT_EQ(m_action->get_icon_px(), 20);

    m_action->set_icon_px(0);  // clamp
    EXPECT_EQ(m_action->get_icon_px(), 1);

    m_action->set_hidden_effective(true);
    EXPECT_TRUE(m_action->get_hidden_effective());

    EXPECT_EQ(m_action->get_file_path(), path);
    EXPECT_EQ(m_action->get_icon_color(), base);
    EXPECT_EQ(m_action->get_icon_color_hover(), hover);
}

/**
 * @brief Widget creation applies configured properties and wires signals.
 */
TEST_F(FilesInViewWidgetActionTest, CreateWidgetAppliesPropertiesAndWiresSignals)
{
    ASSERT_NE(m_action, nullptr);

    const QString path = QStringLiteral("C:/logs/a.log");
    const QString eye = QStringLiteral(":/Resources/Icons/eye.svg");
    const QString eye_off = QStringLiteral(":/Resources/Icons/eye-off.svg");
    const QString trash = QStringLiteral(":/Resources/Icons/trash.svg");
    const QColor base("#010101");
    const QColor hover("#020202");
    const int px = 19;

    m_action->set_file_path(path);
    m_action->set_icon_paths(eye, eye_off, trash);
    m_action->set_icon_color(base);
    m_action->set_icon_color_hover(hover);
    m_action->set_icon_px(px);
    m_action->set_hidden_effective(false);

    auto hosted = create_hosted_row_with_menu(m_action);
    ASSERT_NE(hosted.menu, nullptr);
    ASSERT_NE(hosted.row, nullptr);

    EXPECT_EQ(hosted.row->get_file_path(), path);
    EXPECT_EQ(hosted.row->get_icon_eye_path(), eye);
    EXPECT_EQ(hosted.row->get_icon_eye_off_path(), eye_off);
    EXPECT_EQ(hosted.row->get_icon_trash_path(), trash);
    EXPECT_EQ(hosted.row->get_icon_color(), base);
    EXPECT_EQ(hosted.row->get_icon_color_hover(), hover);
    EXPECT_EQ(hosted.row->get_icon_px(), px);

    QSignalSpy spy_show_only(m_action, &FilesInViewWidgetAction::show_only_requested);
    QSignalSpy spy_toggle(m_action, &FilesInViewWidgetAction::toggle_visibility_requested);
    QSignalSpy spy_remove(m_action, &FilesInViewWidgetAction::remove_requested);

    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_row_controls(hosted.row);

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

    delete hosted.menu;
    QApplication::processEvents();
}

/**
 * @brief Hidden effective toggles the embedded row visibility presentation after creation.
 */
TEST_F(FilesInViewWidgetActionTest, HiddenEffectivePropagatesToRow)
{
    ASSERT_NE(m_action, nullptr);

    m_action->set_file_path(QStringLiteral("C:/logs/b.log"));
    m_action->set_hidden_effective(false);

    auto hosted = create_hosted_row_with_menu(m_action);
    ASSERT_NE(hosted.menu, nullptr);
    ASSERT_NE(hosted.row, nullptr);

    QLabel* label = nullptr;
    QToolButton* btn_show_only = nullptr;
    QToolButton* btn_toggle = nullptr;
    QToolButton* btn_remove = nullptr;
    std::tie(label, btn_show_only, btn_toggle, btn_remove) = get_row_controls(hosted.row);

    ASSERT_NE(btn_toggle, nullptr);
    EXPECT_TRUE(btn_toggle->toolTip().contains("Hide", Qt::CaseInsensitive));

    m_action->set_hidden_effective(true);
    QApplication::processEvents();
    EXPECT_TRUE(btn_toggle->toolTip().contains("Show", Qt::CaseInsensitive));

    m_action->set_hidden_effective(false);
    QApplication::processEvents();
    EXPECT_TRUE(btn_toggle->toolTip().contains("Hide", Qt::CaseInsensitive));

    delete hosted.menu;
    QApplication::processEvents();
}

/**
 * @brief Safe deletion of the embedded row without using protected API.
 */
TEST_F(FilesInViewWidgetActionTest, SafeRowDeletionWithoutProtectedApi)
{
    ASSERT_NE(m_action, nullptr);

    auto hosted = create_hosted_row_with_menu(m_action);
    ASSERT_NE(hosted.menu, nullptr);
    ASSERT_NE(hosted.row, nullptr);

    delete hosted.menu;
    QApplication::processEvents();
}

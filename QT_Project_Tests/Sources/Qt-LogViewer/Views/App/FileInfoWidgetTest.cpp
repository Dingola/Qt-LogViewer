#include "Qt-LogViewer/Views/App/FileInfoWidgetTest.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QLineEdit>
#include <QSignalSpy>
#include <QToolButton>

#include "Qt-LogViewer/Views/App/FileInfoWidget.h"

/**
 * @file FileInfoWidgetTest.cpp
 * @brief Implements presentation tests for the file inspector widget.
 */

/** @brief Creates a fresh file inspector widget. */
void FileInfoWidgetTest::SetUp()
{
    m_widget = new FileInfoWidget();
}

/** @brief Destroys the file inspector widget. */
void FileInfoWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/** @test Verifies the inspector starts with guidance and without a source selector. */
TEST_F(FileInfoWidgetTest, StartsWithEmptyState)
{
    const QComboBox* selector =
        m_widget->findChild<QComboBox*>(QStringLiteral("fileInfoSourceSelector"));
    ASSERT_NE(selector, nullptr);
    EXPECT_TRUE(selector->isHidden());
    EXPECT_EQ(selector->count(), 0);
}

/** @test Verifies aggregate and individual source snapshots remain selectable. */
TEST_F(FileInfoWidgetTest, PresentsAggregateAndSelectedSource)
{
    FileInfoSnapshot aggregate;
    aggregate.display_name = QStringLiteral("All files (2)");
    aggregate.path = QStringLiteral("Multiple sources");
    aggregate.record_count = QStringLiteral("42");
    FileInfoSnapshot source;
    source.display_name = QStringLiteral("application.log");
    source.path = QStringLiteral("C:/logs/application.log");
    source.record_count = QStringLiteral("21");
    source.local_path_available = true;

    m_widget->set_snapshots({aggregate, source});
    auto* selector = m_widget->findChild<QComboBox*>(QStringLiteral("fileInfoSourceSelector"));
    auto* name = m_widget->findChild<QLineEdit*>(QStringLiteral("fileInfoValue0"));
    auto* path = m_widget->findChild<QLineEdit*>(QStringLiteral("fileInfoValue1"));
    ASSERT_NE(selector, nullptr);
    ASSERT_NE(name, nullptr);
    ASSERT_NE(path, nullptr);
    EXPECT_EQ(selector->count(), 2);
    EXPECT_FALSE(selector->isHidden());
    EXPECT_EQ(name->text(), aggregate.display_name);

    QSignalSpy selection_spy(m_widget, &FileInfoWidget::snapshot_selected);
    selector->setCurrentIndex(1);
    EXPECT_EQ(selection_spy.count(), 1);
    EXPECT_EQ(name->text(), source.display_name);
    EXPECT_EQ(path->text(), source.path);
    EXPECT_EQ(path->toolTip(), source.path);
}

/** @test Verifies copy and file-system actions follow the selected snapshot capabilities. */
TEST_F(FileInfoWidgetTest, ProvidesContextualInspectorActions)
{
    FileInfoSnapshot aggregate;
    aggregate.display_name = QStringLiteral("All files (2)");
    aggregate.path = QStringLiteral("Multiple sources");
    FileInfoSnapshot source;
    source.display_name = QStringLiteral("application.log");
    source.path = QStringLiteral("C:/logs/application.log");
    source.local_path_available = true;

    m_widget->set_snapshots({aggregate, source});
    auto* selector = m_widget->findChild<QComboBox*>(QStringLiteral("fileInfoSourceSelector"));
    auto* copy_all = m_widget->findChild<QToolButton*>(QStringLiteral("fileInfoCopyAllButton"));
    auto* copy_path = m_widget->findChild<QToolButton*>(QStringLiteral("fileInfoCopyPathButton"));
    auto* open_folder =
        m_widget->findChild<QToolButton*>(QStringLiteral("fileInfoOpenFolderButton"));
    ASSERT_NE(selector, nullptr);
    ASSERT_NE(copy_all, nullptr);
    ASSERT_NE(copy_path, nullptr);
    ASSERT_NE(open_folder, nullptr);
    EXPECT_TRUE(copy_all->isEnabled());
    EXPECT_FALSE(copy_path->isEnabled());
    EXPECT_FALSE(open_folder->isEnabled());

    selector->setCurrentIndex(1);
    EXPECT_TRUE(copy_path->isEnabled());
    EXPECT_TRUE(open_folder->isEnabled());
    copy_path->click();
    EXPECT_EQ(QApplication::clipboard()->text(), source.path);
    EXPECT_TRUE(copy_path->property("copyConfirmed").toBool());
}

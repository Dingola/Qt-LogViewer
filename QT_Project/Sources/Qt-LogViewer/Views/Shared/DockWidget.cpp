#include "Qt-LogViewer/Views/Shared/DockWidget.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>

/**
 * @brief Creates a custom title bar widget for a QDockWidget.
 * @param dock_widget The dock widget to create the title bar for.
 * @param object_name The object name for QSS styling.
 * @return Pointer to the created QWidget.
 */
auto DockWidget::create_dock_title_bar(QDockWidget* dock_widget,
                                       const QString& object_name) -> QWidget*
{
    auto* title_bar = new QWidget(dock_widget);
    title_bar->setObjectName("dock_title_bar");

    auto* layout = new QHBoxLayout(title_bar);
    // Set layout margins to match or exceed the QSS top margin for consistent vertical alignment.
    // The top margin must be at least as high as specified in the QSS for #dock_title_bar.
    layout->setContentsMargins(10, 9, 10, 0);
    layout->setSpacing(0);

    // Add a label for the dock title
    auto* label = new QLabel(dock_widget->windowTitle(), title_bar);
    label->setObjectName("dock_title_label");
    layout->addWidget(label);

    layout->addStretch(1);

    // Add a close button with standard icon
    auto* close_button = new QPushButton(title_bar);
    close_button->setObjectName("dock_title_bar_close_button");
    close_button->setIcon(
        title_bar->style()->standardIcon(QStyle::SP_TitleBarCloseButton, nullptr, title_bar));
    close_button->setFlat(true);
    layout->addWidget(close_button);

    connect(close_button, &QPushButton::clicked, dock_widget, &QDockWidget::close);

    return title_bar;
}

/**
 * @brief Handles the close event.
 * @param event The close event.
 */
void DockWidget::closeEvent(QCloseEvent* event)
{
    emit closed();
    QDockWidget::closeEvent(event);
}

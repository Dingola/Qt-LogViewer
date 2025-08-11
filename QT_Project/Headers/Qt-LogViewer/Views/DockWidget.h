#pragma once

#include <QDockWidget>

/**
 * @class DockWidget
 * @brief Dock widget base class for application dockable panels.
 *
 * Provides a base implementation for dockable widgets in the application.
 */
class DockWidget: public QDockWidget
{
        Q_OBJECT

    public:
        using QDockWidget::QDockWidget;

        /**
         * @brief Destroys the DockWidget object.
         */
        ~DockWidget() override = default;

        /**
         * @brief Creates a custom title bar widget for a QDockWidget.
         * @param dock_widget The dock widget to create the title bar for.
         * @param object_name The object name for QSS styling.
         * @return Pointer to the created QWidget.
         */
        [[nodiscard]] static auto create_dock_title_bar(QDockWidget* dock_widget,
                                                        const QString& object_name) -> QWidget*;

    protected:
        /**
         * @brief Handles the close event.
         * @param event The close event.
         */
        void closeEvent(QCloseEvent* event) override;

    signals:
        /**
         * @brief Emitted when the dock widget is closed.
         */
        void closed();
};

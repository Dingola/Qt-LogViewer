#pragma once

#include <QSize>
#include <QString>
#include <memory>

class LogViewerSettings;

/**
 * @struct LogViewerApplicationOptions
 * @brief Defines application-level window settings that callers may override at startup.
 *
 * The defaults represent the preferred Qt-LogViewer presentation and may be replaced with values
 * derived from the startup environment.
 */
struct LogViewerApplicationOptions {
        QSize initial_window_size{1120, 800};  ///< Initial size of the outer window.
        QString window_title{QStringLiteral("Qt-LogViewer")};  ///< Title-bar text.
        QString window_icon_path{
            QStringLiteral(":/Resources/Icons/App/AppIcon.svg")};  ///< Resource or file path.
        bool adopt_menubar{true};  ///< Whether the title bar adopts the main menu.
};

/**
 * @file LogViewerApplication.h
 * @brief Declares the application-specific composition root for Qt-LogViewer.
 */

/**
 * @class LogViewerApplication
 * @brief Owns and connects the components of the Qt-LogViewer desktop application.
 *
 * This class defines component ownership, construction order, and application-level window
 * configuration. It does not expose internal domain services; reusable behavior remains in
 * focused components.
 */
class LogViewerApplication final
{
    public:
        /**
         * @brief Constructs the complete Qt-LogViewer desktop application.
         * @param settings Application settings that outlive this composition root.
         * @param options Application-level window settings selected at startup.
         */
        explicit LogViewerApplication(
            LogViewerSettings& settings,
            const LogViewerApplicationOptions& options = LogViewerApplicationOptions());

        /**
         * @brief Destroys the window before its injected application components.
         */
        ~LogViewerApplication();

        LogViewerApplication(const LogViewerApplication&) = delete;
        auto operator=(const LogViewerApplication&) -> LogViewerApplication& = delete;
        LogViewerApplication(LogViewerApplication&&) = delete;
        auto operator=(LogViewerApplication&&) -> LogViewerApplication& = delete;

        /**
         * @brief Shows the composed application window.
         */
        auto show() -> void;

    private:
        class Implementation;
        std::unique_ptr<Implementation> m_implementation;
};

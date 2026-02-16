#pragma once

#include <QByteArray>

/**
 * @class IMainWindowStatePersistence
 * @brief Interface for persisting main window state.
 *
 * This interface defines methods to save and retrieve the main window's geometry and state.
 */
class IMainWindowStatePersistence
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~IMainWindowStatePersistence() = default;

        /**
         * @brief Returns the last saved main window geometry (position and size).
         * @param geometry The geometry data to save.
         */
        [[nodiscard]] virtual auto get_mainwindow_geometry() -> QByteArray = 0;

        /**
         * @brief Sets the main window geometry (position and size).
         * @param geometry The geometry data to save.
         */
        virtual auto set_mainwindow_geometry(const QByteArray& geometry) -> void = 0;

        /**
         * @brief Returns the last saved main window state.
         * @return The state data as a QByteArray.
         */
        [[nodiscard]] virtual auto get_mainwindow_state() -> QByteArray = 0;

        /**
         * @brief Sets the main window state.
         * @param state The state data to save as a QByteArray.
         */
        virtual auto set_mainwindow_state(const QByteArray& state) -> void = 0;

        /**
         * @brief Returns the last saved main window window state.
         * @return The window state as an integer (e.g. Qt::WindowNoState, Qt::WindowMinimized,
         * etc.).
         */
        [[nodiscard]] virtual auto get_mainwindow_windowstate() -> int = 0;

        /**
         * @brief Sets the main window window state.
         * @param state The window state to save as an integer (e.g. Qt::WindowNoState,
         * Qt::WindowMinimized, etc.).
         */
        virtual auto set_mainwindow_windowstate(int state) -> void = 0;
};

Q_DECLARE_INTERFACE(IMainWindowStatePersistence, "de.adrianhelbig.IMainWindowStatePersistence")

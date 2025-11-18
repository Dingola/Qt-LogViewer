#pragma once

/**
 * @file AppWindow.h
 * @brief Windows-only custom top-level window with a fully customizable title bar.
 *
 * This class implements a Qt top-level QWidget that:
 *  - removes the native title bar and provides a client-side title bar area
 *  - preserves native window behaviors on Windows: aero snap, border resize,
 *    minimize/maximize/close, multi-monitor maximize, DPI-aware sizing
 *  - maps clicks on caption buttons to HTCLIENT so Qt can handle them
 *
 * Usage:
 *  - Create your WindowTitleBar widget and pass it as the title bar child
 *  - Put your main content widget into this AppWindow (set_central_widget)
 *
 * Note: Windows-only implementation: all WinAPI code is #ifdef Q_OS_WIN.
 */

#include <QIcon>
#include <QPointer>
#include <QWidget>

#include "Qt-LogViewer/Views/Shared/WindowTitleBar.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class QMenu;

class AppWindow: public QWidget
{
        Q_OBJECT

        Q_DISABLE_COPY_MOVE(AppWindow)

    public:
        /**
         * @brief Constructs an AppWindow.
         *
         * The AppWindow is a top-level native QWidget with a client-side title bar. The provided
         * content_widget (if non-null) is placed below the title bar. If title_bar is nullptr,
         * a default WindowTitleBar is created.
         *
         * @param parent Optional logical parent.
         * @param content_widget Optional central content widget placed under the title bar.
         * @param title_bar Optional custom title bar widget.
         */
        explicit AppWindow(QWidget* parent = nullptr, QWidget* content_widget = nullptr,
                           WindowTitleBar* title_bar = nullptr);

        /**
         * @brief Destroys the AppWindow.
         *
         * Cleans up any native resources allocated by this window.
         */
        ~AppWindow() override;

        /**
         * @brief Replace or set the central content widget.
         *
         * The previous content widget (if any) is removed from the layout and scheduled for
         * deletion.
         *
         * @param widget New central widget or nullptr to clear.
         */
        auto set_central_widget(QWidget* widget) -> void;

        /**
         * @brief Returns the current central widget (may be nullptr).
         *
         * @return QWidget* Pointer to the current central widget, or nullptr if none set.
         */
        [[nodiscard]] auto get_central_widget() const noexcept -> QWidget*
        {
            return m_content_widget;
        }

        /**
         * @brief Set the application / window icon.
         *
         * On Windows this will set both the Qt window icon and the native HWND icons
         * (small and big) so the taskbar shows the correct icon immediately.
         *
         * @param icon QIcon to use for the window.
         */
        auto set_app_icon(const QIcon& icon) -> void;

        /**
         * @brief Set the window title text.
         *
         * Updates both the native window title and the custom title bar text.
         *
         * @param title New window title.
         */
        auto set_app_title(const QString& title) -> void;

        /**
         * @brief Enable/disable Mica-like backdrop on Windows 11 at runtime.
         *
         * When enabled, attempts to use DWMWA_SYSTEM_BACKDROP_TYPE (preferred) or the legacy
         * DWMWA_MICA_EFFECT fallback when available. Does nothing on non-Windows platforms.
         *
         * @param enabled True to enable Mica backdrop, false to disable it.
         */
        auto set_use_mica(bool enabled) -> void;

        /**
         * @brief Query whether Mica-like backdrop is requested.
         *
         * @return bool True if Mica effect is requested, false otherwise.
         */
        [[nodiscard]] auto get_use_mica() const noexcept -> bool;

        /**
         * @brief Enable/disable rounded corners on Windows 11 at runtime.
         *
         * Uses DWMWA_WINDOW_CORNER_PREFERENCE with either rounded or do-not-round,
         * when supported by the OS.
         *
         * @param enabled True to prefer rounded corners; false to request squared corners.
         */
        auto set_use_rounded_corners(bool enabled) -> void;

        /**
         * @brief Query whether rounded corners are requested.
         *
         * @return bool True if rounded corners are requested, false otherwise.
         */
        [[nodiscard]] auto get_use_rounded_corners() const noexcept -> bool;

        /**
         * @brief Returns the WindowTitleBar instance.
         *
         * @return WindowTitleBar* Title bar pointer or nullptr.
         */
        [[nodiscard]] auto get_window_title_bar() const noexcept -> WindowTitleBar*;

        /**
         * @brief Enable or disable adopting the central widget's QMenuBar into the title bar.
         *
         * When enabled and a QMenuBar is found as a descendant of the central widget, it will be
         * reparented to the WindowTitleBar and placed on the specified row.
         *
         * @param enabled True to adopt the menubar, false to leave it in the central widget.
         * @param row Row where the menubar should be placed.
         */
        auto set_adopt_menubar(bool enabled, WindowTitleBar::RowPosition row) -> void;

        /**
         * @brief Query whether adopting the central menubar is enabled.
         *
         * @return bool True if adoption is enabled, false otherwise.
         */
        [[nodiscard]] auto get_adopt_menubar() const noexcept -> bool
        {
            bool result = m_adopt_menubar;
            return result;
        }

        /**
         * @brief Query the target row for an adopted menubar.
         *
         * @return RowPosition Target row.
         */
        [[nodiscard]] auto get_adopt_menubar_row() const noexcept -> WindowTitleBar::RowPosition
        {
            WindowTitleBar::RowPosition result = m_menubar_row;
            return result;
        }

    protected:
        /**
         * @brief Native event filter for Windows messages.
         *
         * Processes Windows messages required for the client-side frame (WM_NCCALCSIZE,
         * WM_NCHITTEST, WM_GETMINMAXINFO, WM_DPICHANGED, etc.).
         *
         * @param eventType Platform event type (unused on Windows).
         * @param message Pointer to the native MSG structure to process (MSG*).
         * @param result Output pointer where a platform-specific result (LRESULT) may be stored.
         * @return true if the event was handled (result contains a valid LRESULT), false to defer
         *         to the base class implementation.
         */
        auto nativeEvent(const QByteArray& eventType, void* message,
                         qintptr* result) -> bool override;

    private:  // private methods (cross-platform)
        /**
         * @brief Adopt the central widget's QMenuBar into the title bar according to current flags.
         */
        auto adopt_menubar_from_content_widget() -> void;

    private:  // private methods (Windows-only)
#ifdef Q_OS_WIN
        /**
         * @brief Return the native HWND for this widget.
         *
         * Uses winId() and casts to HWND. Callers should ensure the native window exists
         * (createWinId()) before using the returned handle.
         *
         * @return HWND native window handle, or nullptr if not created.
         */
        [[nodiscard]] HWND nativeWindowHandle() const noexcept
        {
            return reinterpret_cast<HWND>(winId());
        }

        /**
         * @brief Returns the system resize border thickness in pixels (DPI-aware).
         *
         * Uses SM_CXSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
         *
         * @return int Total border width in pixels for the current DPI.
         */
        [[nodiscard]] auto system_resize_border_width() const noexcept -> int;

        /**
         * @brief Returns the system caption (titlebar) height in pixels (DPI-aware).
         *
         * Uses SM_CYCAPTION scaled for the window's DPI.
         *
         * @return int Caption height in pixels for the current DPI.
         */
        [[nodiscard]] auto system_caption_height() const noexcept -> int;

        /**
         * @brief Perform custom hit-testing for frameless window.
         *
         * Returns HT* values for caption dragging, client controls and resize edges.
         *
         * Notes:
         *  - To allow resizing near the edges while using a custom title bar, points that fall
         *    inside the system resize border are not treated as caption.
         *  - When the window is maximized, resize zones are ignored so that clicks at the very
         *    outermost pixels of the titlebar behave like a restore+drag (native behavior).
         *
         * @param msg Native MSG pointer containing screen coordinates in lParam.
         * @return LRESULT Hit-test code (HT*), e.g. HTCAPTION, HTCLIENT, HTLEFT, etc.
         */
        auto handle_nc_hit_test(MSG* msg) -> LRESULT;

        /**
         * @brief Handle WM_NCCALCSIZE - remove the standard non-client frame.
         *
         * Returning 0 indicates we've handled the sizing and the whole window is client area.
         *
         * @param msg Native MSG pointer.
         * @return LRESULT Result value for WM_NCCALCSIZE handling (0 = handled).
         */
        auto handle_nc_calc_size(MSG* msg) -> LRESULT;

        /**
         * @brief Handle WM_GETMINMAXINFO so maximize fits monitor work area (taskbar-aware).
         *
         * Sets ptMaxPosition and ptMaxSize based on the monitor work area. Also sets minimum track
         * size based on widget minimumSize().
         *
         * @param lParam LPARAM passed to WM_GETMINMAXINFO (pointer to MINMAXINFO).
         */
        auto handle_get_min_max_info(LPARAM lParam) -> void;

        /**
         * @brief Enable required native window styles for snap and resize behavior.
         *
         * Ensures thick frame, minimize/maximize boxes and appropriate extended class styles.
         */
        auto enable_native_window_styles() -> void;

        /**
         * @brief Extend DWM frame into client area to avoid white borders.
         *
         * When DWM composition is available, extend a 1px margin to avoid white border artifacts.
         * Otherwise apply a drop-shadow via class style as fallback.
         */
        auto extend_frame_into_client_area() -> void;

        /**
         * @brief Enable Windows 11 specific DWM features if available.
         *
         * Applies rounded corners and optional Mica backdrop, gated by runtime availability and
         * the user preferences set via set_use_mica() and set_use_rounded_corners().
         * Also attempts to enable immersive dark mode for borders/caption where supported.
         */
        auto enable_win11_features() -> void;

        /**
         * @brief Map a global (screen) point to local coordinates relative to titlebar.
         *
         * @param global_x Global X coordinate in screen space.
         * @param global_y Global Y coordinate in screen space.
         * @return QPoint Local coordinates relative to the title bar (or sentinel negative point).
         */
        [[nodiscard]] auto map_global_to_titlebar_local(int global_x, int global_y) const -> QPoint;
#endif  // Q_OS_WIN

    private:  // private members (all platforms)
        /**
         * @brief Pointer to the custom title bar widget.
         *
         * This is a weak pointer (QPointer) so it becomes null if the widget is deleted elsewhere.
         */
        QPointer<WindowTitleBar> m_title_bar;

        /**
         * @brief The central content widget placed below the title bar.
         *
         * Owned by this AppWindow; when replaced the previous widget is scheduled for deletion.
         */
        QWidget* m_content_widget = nullptr;

        /**
         * @brief User preference: enable Mica-like backdrop when supported (Windows 11+).
         */
        bool m_use_mica = false;

        /**
         * @brief User preference: enable rounded corners when supported (Windows 11+).
         */
        bool m_use_rounded_corners = true;

        /**
         * @brief Whether the central QMenuBar is adopted into the title bar.
         */
        bool m_adopt_menubar = false;

        /**
         * @brief Target row for an adopted menubar.
         */
        WindowTitleBar::RowPosition m_menubar_row = WindowTitleBar::RowPosition::Top;

#ifdef Q_OS_WIN
        /**
         * @brief Native small icon handle (ICON_SMALL).
         *
         * Created when set_app_icon is called. Destroyed in the destructor.
         */
        HICON m_hicon_small = nullptr;

        /**
         * @brief Native big icon handle (ICON_BIG).
         *
         * Created when set_app_icon is called. Destroyed in the destructor.
         */
        HICON m_hicon_big = nullptr;
#endif
};

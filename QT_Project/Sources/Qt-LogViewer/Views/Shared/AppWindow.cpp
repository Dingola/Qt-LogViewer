#include "Qt-LogViewer/Views/Shared/AppWindow.h"

#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include "Qt-LogViewer/Views/Shared/WindowTitleBar.h"

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <windows.h>
#include <windowsx.h>
#pragma comment(lib, "dwmapi.lib")

namespace
{
// ---- Cached WinAPI entry points and constants (runtime-gated) ----------------

/**
 * @brief Structure holding function pointers to DWM and related WinAPI functions.
 */
struct dwm_functions {
        using p_dwm_is_composition_enabled = HRESULT(WINAPI*)(BOOL*);
        using p_dwm_extend_frame_into_client_area = HRESULT(WINAPI*)(HWND, const MARGINS*);
        using p_dwm_set_window_attribute = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
        using p_get_dpi_for_window = UINT(WINAPI*)(HWND);

        p_dwm_is_composition_enabled dwm_is_composition_enabled = nullptr;
        p_dwm_extend_frame_into_client_area dwm_extend_frame_into_client_area = nullptr;
        p_dwm_set_window_attribute dwm_set_window_attribute = nullptr;
        p_get_dpi_for_window get_dpi_for_window = nullptr;
        bool initialized = false;
};

/**
 * @brief Get cached DWM function pointers, resolving them on first call.
 *
 * @return dwm_functions& Reference to the static structure with function pointers.
 */
[[nodiscard]] static auto get_dwm_functions() -> dwm_functions&
{
    static dwm_functions f;

    if (!f.initialized)
    {
        HMODULE h_dwm = GetModuleHandleW(L"dwmapi");
        HMODULE h_user32 = GetModuleHandleW(L"user32");

        if (h_dwm)
        {
            f.dwm_is_composition_enabled =
                reinterpret_cast<dwm_functions::p_dwm_is_composition_enabled>(
                    GetProcAddress(h_dwm, "DwmIsCompositionEnabled"));
            f.dwm_extend_frame_into_client_area =
                reinterpret_cast<dwm_functions::p_dwm_extend_frame_into_client_area>(
                    GetProcAddress(h_dwm, "DwmExtendFrameIntoClientArea"));
            f.dwm_set_window_attribute =
                reinterpret_cast<dwm_functions::p_dwm_set_window_attribute>(
                    GetProcAddress(h_dwm, "DwmSetWindowAttribute"));
        }

        if (h_user32)
        {
            f.get_dpi_for_window = reinterpret_cast<dwm_functions::p_get_dpi_for_window>(
                GetProcAddress(h_user32, "GetDpiForWindow"));
        }

        f.initialized = true;
    }

    return f;
}

// Attribute IDs: use unique names to avoid collisions with SDK's enum identifiers.
constexpr DWORD kDWMWA_WINDOW_CORNER_PREFERENCE = 33u;
constexpr DWORD kDWMWA_BORDER_COLOR = 34u;
constexpr DWORD kDWMWA_CAPTION_COLOR = 35u;
constexpr DWORD kDWMWA_SYSTEMBACKDROP_TYPE = 38u;
// Documented on Win10+ but not always present in headers
constexpr DWORD kDWMWA_USE_IMMERSIVE_DARK_MODE = 20u;
// Legacy Mica attribute used on early Windows 11 builds
constexpr DWORD kDWMWA_MICA_EFFECT = 1029u;

// Corner preference values
enum class dwm_window_corner_pref : DWORD
{
    default_pref = 0,
    do_not_round = 1,
    round = 2,
    round_small = 3
};

// System backdrop types (Windows 11)
enum class dwm_system_backdrop_type : DWORD
{
    auto_type = 0,
    none = 1,
    main_window = 2,
    transient_window = 3,
    tabbed_window = 4
};

/**
 * @brief Resolve GetDpiForWindow at runtime and return DPI for the given HWND.
 *
 * Keeps runtime resolution of GetDpiForWindow in one place; falls back to 96 DPI.
 *
 * @param hwnd Window handle to query DPI for.
 * @return UINT DPI value in dots per inch.
 */
[[nodiscard]] static auto get_window_dpi(HWND hwnd) -> UINT
{
    const auto& f = get_dwm_functions();
    UINT dpi_value = 96u;

    if (f.get_dpi_for_window)
    {
        dpi_value = f.get_dpi_for_window(hwnd);
    }

    return dpi_value;
}

/**
 * @brief Convert QPixmap to HICON (fallback if QtWinExtras not available).
 *
 * Creates a 32bpp ARGB HICON from a QPixmap by creating a DIB section,
 * copying pixels (BGRA) and constructing an ICON via CreateIconIndirect.
 *
 * This function follows the project rule: single return at the end, and all control
 * structures use braces.
 *
 * @param pixmap Source pixmap.
 * @return HICON HICON handle (or nullptr on failure).
 */
[[nodiscard]] static HICON create_hicon_from_pixmap(const QPixmap& pixmap)
{
    HICON h_icon = nullptr;

    if (!pixmap.isNull())
    {
        QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        const int width = image.width();
        const int height = image.height();

        // Prepare BITMAPV5HEADER for 32bpp RGBA
        BITMAPV5HEADER bi{};
        bi.bV5Size = sizeof(BITMAPV5HEADER);
        bi.bV5Width = width;
        bi.bV5Height = -height;  // top-down
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        bi.bV5RedMask = 0x00FF0000;
        bi.bV5GreenMask = 0x0000FF00;
        bi.bV5BlueMask = 0x000000FF;
        bi.bV5AlphaMask = 0xFF000000;

        void* pv_bits = nullptr;
        HBITMAP h_bitmap = nullptr;
        HBITMAP h_mask = nullptr;

        HDC screen_dc = GetDC(nullptr);

        h_bitmap = CreateDIBSection(screen_dc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS,
                                    &pv_bits, nullptr, 0);

        ReleaseDC(nullptr, screen_dc);

        if (h_bitmap && pv_bits)
        {
            // copy pixels (BGRA)
            const uint8_t* src_pixels = image.constBits();
            const int src_stride = image.bytesPerLine();
            const int dst_stride = width * 4;

            for (int y = 0; y < height; ++y)
            {
                uint8_t* dest_row = reinterpret_cast<uint8_t*>(pv_bits) + y * dst_stride;
                const uint8_t* src_row = src_pixels + y * src_stride;

                for (int x = 0; x < width; ++x)
                {
                    const QRgb px = reinterpret_cast<const QRgb*>(src_row)[x];
                    dest_row[x * 4 + 0] = qBlue(px);
                    dest_row[x * 4 + 1] = qGreen(px);
                    dest_row[x * 4 + 2] = qRed(px);
                    dest_row[x * 4 + 3] = qAlpha(px);
                }
            }

            // Create an empty monochrome mask bitmap
            h_mask = CreateBitmap(width, height, 1, 1, nullptr);

            if (h_mask)
            {
                ICONINFO ii{};
                ii.fIcon = TRUE;
                ii.hbmMask = h_mask;
                ii.hbmColor = h_bitmap;

                h_icon = CreateIconIndirect(&ii);
            }

            // system makes copies; safe to delete bitmaps now
            if (h_bitmap)
            {
                DeleteObject(h_bitmap);
            }

            if (h_mask)
            {
                DeleteObject(h_mask);
            }
        }
    }

    return h_icon;
}
}  // namespace

#endif

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
AppWindow::AppWindow(QWidget* parent, QWidget* content_widget, WindowTitleBar* title_bar)
    : QWidget(parent), m_title_bar(title_bar), m_content_widget(content_widget)
{
    setAttribute(Qt::WA_NativeWindow, true);
    setAttribute(Qt::WA_DontCreateNativeAncestors, true);

    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Window);

    auto* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);

    if (m_title_bar.isNull())
    {
        m_title_bar = new WindowTitleBar(this);
    }

    main_layout->addWidget(m_title_bar);

    if (m_content_widget != nullptr)
    {
        main_layout->addWidget(m_content_widget);
        m_content_widget->setParent(this);
    }

    setLayout(main_layout);

#ifdef Q_OS_WIN
    createWinId();

    enable_native_window_styles();
    extend_frame_into_client_area();  // remove white borders from DWM
    enable_win11_features();          // apply initial user prefs (mica/corners)
#endif

    // Connect title bar signals (cross-platform)
    if (!m_title_bar.isNull())
    {
        connect(m_title_bar, &WindowTitleBar::minimize_requested, this, &AppWindow::showMinimized);
        connect(m_title_bar, &WindowTitleBar::maximize_requested, this, &AppWindow::showMaximized);
        connect(m_title_bar, &WindowTitleBar::restore_requested, this, &AppWindow::showNormal);
        connect(m_title_bar, &WindowTitleBar::close_requested, this, &AppWindow::close);
    }
}

/**
 * @brief Destructor.
 *
 * Cleans up any native resources allocated by the AppWindow (HICONs).
 */
AppWindow::~AppWindow()
{
#ifdef Q_OS_WIN
    if (m_hicon_small)
    {
        DestroyIcon(m_hicon_small);
        m_hicon_small = nullptr;
    }

    if (m_hicon_big)
    {
        DestroyIcon(m_hicon_big);
        m_hicon_big = nullptr;
    }

    // nothing special here; Qt will destroy native window
    Q_UNUSED(nativeWindowHandle());
#endif
}

/**
 * @brief Replace or set the central content widget.
 *
 * Removes the previous central widget from the layout and schedules it for deletion.
 *
 * @param widget Pointer to the new central widget. Pass nullptr to clear the central widget.
 *
 * @note The function takes ownership of the widget by reparenting it to this AppWindow.
 */
auto AppWindow::set_central_widget(QWidget* widget) -> void
{
    if (m_content_widget != widget)
    {
        if (m_content_widget != nullptr)
        {
            layout()->removeWidget(m_content_widget);
            m_content_widget->deleteLater();
            m_content_widget = nullptr;
        }

        m_content_widget = widget;

        if (m_content_widget != nullptr)
        {
            layout()->addWidget(m_content_widget);
            m_content_widget->setParent(this);
        }
    }
}

#ifdef Q_OS_WIN
/**
 * @brief Set the application / window icon.
 *
 * On Windows this will set both the Qt window icon and the native HWND icons
 * (small and big) so the taskbar shows the correct icon immediately.
 *
 * @param icon QIcon to use for the window.
 */
void AppWindow::set_app_icon(const QIcon& icon)
{
    // set Qt-level icons (application and widget)
    if (!icon.isNull())
    {
        QGuiApplication::setWindowIcon(icon);
        setWindowIcon(icon);

        // Forward to custom title bar icon
        if (!m_title_bar.isNull())
        {
            m_title_bar->set_icon(icon);
        }
    }

    // Ensure native window exists
    createWinId();
    HWND hwnd = nativeWindowHandle();

    if (IsWindow(hwnd))
    {
        // create HICONs for small and big sizes
        const QPixmap small_pm = icon.pixmap(16, 16);
        const QPixmap big_pm = icon.pixmap(32, 32);

        // destroy previous icons if present
        if (m_hicon_small)
        {
            DestroyIcon(m_hicon_small);
            m_hicon_small = nullptr;
        }

        if (m_hicon_big)
        {
            DestroyIcon(m_hicon_big);
            m_hicon_big = nullptr;
        }

        // Try QtWinExtras first (if available). Otherwise fallback to our converter.
#if defined(QT_WINEXTRAS_LIB)
        m_hicon_small = QtWin::toHICON(small_pm);
        m_hicon_big = QtWin::toHICON(big_pm);
        if (!m_hicon_small)
        {
            m_hicon_small = create_hicon_from_pixmap(small_pm);
        }
        if (!m_hicon_big)
        {
            m_hicon_big = create_hicon_from_pixmap(big_pm);
        }
#else
        m_hicon_small = create_hicon_from_pixmap(small_pm);
        m_hicon_big = create_hicon_from_pixmap(big_pm);
#endif

        if (m_hicon_small)
        {
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(m_hicon_small));
        }

        if (m_hicon_big)
        {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(m_hicon_big));
        }

        // also set the class icons so the taskbar picks it up immediately
        if (m_hicon_big)
        {
            SetClassLongPtrW(hwnd, GCLP_HICON, reinterpret_cast<LONG_PTR>(m_hicon_big));
        }

        if (m_hicon_small)
        {
            SetClassLongPtrW(hwnd, GCLP_HICONSM, reinterpret_cast<LONG_PTR>(m_hicon_small));
        }
    }
}
#else
void AppWindow::set_app_icon(const QIcon& icon)
{
    if (!icon.isNull())
    {
        QGuiApplication::setWindowIcon(icon);
        setWindowIcon(icon);
    }
}
#endif  // Q_OS_WIN

/**
 * @brief Enable/disable Mica-like backdrop on Windows 11 at runtime.
 *
 * When enabled, attempts to use DWMWA_SYSTEM_BACKDROP_TYPE (preferred) or the legacy
 * DWMWA_MICA_EFFECT fallback when available. Does nothing on non-Windows platforms.
 *
 * @param enabled True to enable Mica backdrop, false to disable it.
 */
auto AppWindow::set_use_mica(bool enabled) -> void
{
#ifdef Q_OS_WIN
    m_use_mica = enabled;
    enable_win11_features();
#else
    Q_UNUSED(enabled);
#endif
}

/**
 * @brief Query whether Mica-like backdrop is requested.
 *
 * @return bool True if Mica effect is requested, false otherwise.
 */
[[nodiscard]] auto AppWindow::get_use_mica() const noexcept -> bool
{
#ifdef Q_OS_WIN
    return m_use_mica;
#else
    return false;
#endif
}

/**
 * @brief Enable/disable rounded corners on Windows 11 at runtime.
 *
 * Uses DWMWA_WINDOW_CORNER_PREFERENCE with either rounded or do-not-round,
 * when supported by the OS.
 *
 * @param enabled True to prefer rounded corners; false to request squared corners.
 */
auto AppWindow::set_use_rounded_corners(bool enabled) -> void
{
#ifdef Q_OS_WIN
    m_use_rounded_corners = enabled;
    enable_win11_features();
#else
    Q_UNUSED(enabled);
#endif
}

/**
 * @brief Query whether rounded corners are requested.
 *
 * @return bool True if rounded corners are requested, false otherwise.
 */
[[nodiscard]] auto AppWindow::get_use_rounded_corners() const noexcept -> bool
{
#ifdef Q_OS_WIN
    return m_use_rounded_corners;
#else
    return false;
#endif
}

/**
 * @brief Returns the WindowTitleBar instance.
 *
 * @return WindowTitleBar* Title bar pointer or nullptr.
 */
auto AppWindow::get_window_title_bar() const noexcept -> WindowTitleBar*
{
    WindowTitleBar* result = m_title_bar;
    return result;
}

/**
 * @brief Enable or disable adopting the central widget's QMenuBar into the title bar.
 *
 * When enabled and a QMenuBar is found as a descendant of the central widget, it will be
 * reparented to the WindowTitleBar and placed on the specified row.
 *
 * @param enabled True to adopt the menubar, false to leave it in the central widget.
 * @param row Row where the menubar should be placed.
 */
auto AppWindow::set_adopt_menubar(bool enabled, WindowTitleBar::RowPosition row) -> void
{
    if (m_adopt_menubar != enabled || m_menubar_row != row)
    {
        m_adopt_menubar = enabled;
        m_menubar_row = row;

        adopt_menubar_from_content_widget();
    }
}

#ifdef Q_OS_WIN

/**
 * @brief Returns the system resize border thickness in pixels (DPI-aware).
 *
 * Uses SM_CXSIZEFRAME + SM_CXPADDEDBORDER scaled for the window's DPI.
 *
 * @return int Total border width in pixels for the current DPI.
 */
auto AppWindow::system_resize_border_width() const noexcept -> int
{
    HWND hwnd = nativeWindowHandle();
    const UINT dpi = get_window_dpi(hwnd);

    const int cx_size_frame = static_cast<int>(GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi));
    const int cx_padded_border = static_cast<int>(GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi));
    const int result = cx_size_frame + cx_padded_border;

    return result;
}

/**
 * @brief Returns the system caption (titlebar) height in pixels (DPI-aware).
 *
 * Uses SM_CYCAPTION scaled for the window's DPI.
 *
 * @return int Caption height in pixels for the current DPI.
 */
auto AppWindow::system_caption_height() const noexcept -> int
{
    HWND hwnd = nativeWindowHandle();
    const UINT dpi = get_window_dpi(hwnd);

    const int result = static_cast<int>(GetSystemMetricsForDpi(SM_CYCAPTION, dpi));

    return result;
}

/**
 * @brief Enable required native window styles for snap and resize behavior.
 *
 * Ensures thick frame, minimize/maximize boxes and appropriate extended class styles.
 */
auto AppWindow::enable_native_window_styles() -> void
{
    HWND hwnd = nativeWindowHandle();

    if (IsWindow(hwnd))
    {
        // Keep existing style but ensure we have thick frame and minimize/maximize boxes so aero
        // snap works.
        LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
        style |= WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

        // remove WS_POPUP if present (we want overlapped behavior)
        style &= ~WS_POPUP;
        SetWindowLongPtrW(hwnd, GWL_STYLE, style);

        // Ensure extended styles include a standard edge look (consistent shadowing on fallback).
        LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        ex |= WS_EX_WINDOWEDGE;
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex);

        // Force a style refresh
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                     SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

        extend_frame_into_client_area();
        enable_win11_features();
    }
}

/**
 * @brief Extend DWM frame into client area to avoid white borders.
 *
 * When DWM composition is available, extend a 1px margin to avoid white border artifacts.
 * Otherwise apply a drop-shadow via class style as fallback.
 */
auto AppWindow::extend_frame_into_client_area() -> void
{
    HWND hwnd = nativeWindowHandle();

    if (IsWindow(hwnd))
    {
        const auto& f = get_dwm_functions();

        // Check whether DWM (Desktop Window Manager) composition is enabled.
        BOOL composition_enabled = FALSE;

        if (f.dwm_is_composition_enabled)
        {
            f.dwm_is_composition_enabled(&composition_enabled);
        }

        if (composition_enabled && f.dwm_extend_frame_into_client_area)
        {
            // Extend a tiny bit into the client area (1px on all sides) to remove white border
            // artifacts.
            MARGINS margins = {1, 1, 1, 1};
            f.dwm_extend_frame_into_client_area(hwnd, &margins);
        }
        else
        {
            // Fallback for systems without DWM (classic theme, remote desktop, etc.)
            // Enable a basic drop shadow to mimic Aero appearance.
            const LONG_PTR class_style = GetClassLongPtrW(hwnd, GCL_STYLE);
            SetClassLongPtrW(hwnd, GCL_STYLE, class_style | CS_DROPSHADOW);

            // Force a style refresh so the shadow is applied immediately.
            SetWindowPos(
                hwnd, nullptr, 0, 0, 0, 0,
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    }
}

/**
 * @brief Enable Windows 11 specific DWM features if available.
 *
 * Applies rounded corners and optional Mica backdrop, gated by runtime availability and
 * the user preferences set via set_use_mica() and set_use_rounded_corners().
 * Also attempts to enable immersive dark mode for borders/caption where supported.
 */
auto AppWindow::enable_win11_features() -> void
{
    HWND hwnd = nativeWindowHandle();

    if (IsWindow(hwnd))
    {
        const auto& f = get_dwm_functions();

        // Corner preference (Windows 11)
        if (f.dwm_set_window_attribute)
        {
            const dwm_window_corner_pref pref = m_use_rounded_corners
                                                    ? dwm_window_corner_pref::round
                                                    : dwm_window_corner_pref::do_not_round;

            const DWORD value = static_cast<DWORD>(pref);
            f.dwm_set_window_attribute(hwnd, kDWMWA_WINDOW_CORNER_PREFERENCE, &value,
                                       sizeof(value));
        }

        // Try immersive dark mode (Win10+ attribute, not always available)
        if (f.dwm_set_window_attribute)
        {
            BOOL enable_dark = TRUE;

            if (SUCCEEDED(f.dwm_set_window_attribute(hwnd, kDWMWA_USE_IMMERSIVE_DARK_MODE,
                                                     &enable_dark, sizeof(enable_dark))) == FALSE)
            {
                // If immersive dark mode is not available, prefer darker border color as fallback
                COLORREF border_color = RGB(30, 30, 30);
                f.dwm_set_window_attribute(hwnd, kDWMWA_BORDER_COLOR, &border_color,
                                           sizeof(border_color));
            }
        }

        // Mica/system backdrop (Windows 11)
        if (f.dwm_set_window_attribute)
        {
            if (m_use_mica)
            {
                // Prefer the modern backdrop attribute
                DWORD backdrop = static_cast<DWORD>(dwm_system_backdrop_type::main_window);
                HRESULT hr = f.dwm_set_window_attribute(hwnd, kDWMWA_SYSTEMBACKDROP_TYPE, &backdrop,
                                                        sizeof(backdrop));

                if (FAILED(hr))
                {
                    // Fallback to legacy Mica attribute if present
                    BOOL mica_enable = TRUE;
                    f.dwm_set_window_attribute(hwnd, kDWMWA_MICA_EFFECT, &mica_enable,
                                               sizeof(mica_enable));
                }
            }
            else
            {
                // Disable Mica/backdrop when user preference is off
                DWORD backdrop_none = static_cast<DWORD>(dwm_system_backdrop_type::none);
                HRESULT hr = f.dwm_set_window_attribute(hwnd, kDWMWA_SYSTEMBACKDROP_TYPE,
                                                        &backdrop_none, sizeof(backdrop_none));

                if (FAILED(hr))
                {
                    BOOL mica_disable = FALSE;
                    f.dwm_set_window_attribute(hwnd, kDWMWA_MICA_EFFECT, &mica_disable,
                                               sizeof(mica_disable));
                }
            }
        }
    }
}

/**
 * @brief Map a global (screen) point to local titlebar coordinates.
 *
 * Returns a large negative point when title bar is not available to avoid accidental matches.
 *
 * @param global_x Global X coordinate in screen space.
 * @param global_y Global Y coordinate in screen space.
 * @return QPoint Local coordinates relative to the title bar (or a sentinel negative point).
 */
auto AppWindow::map_global_to_titlebar_local(int global_x, int global_y) const -> QPoint
{
    QPoint global(global_x, global_y);
    QPoint local = m_title_bar ? m_title_bar->mapFromGlobal(global) : QPoint(-10000, -10000);
    return local;
}

/**
 * @brief Handle WM_NCCALCSIZE - remove the standard non-client frame.
 *
 * Returning 0 indicates we've handled the sizing and the whole window is client area.
 *
 * @param msg Native MSG pointer.
 * @return LRESULT Result value for WM_NCCALCSIZE handling (0 = handled).
 */
auto AppWindow::handle_nc_calc_size(MSG* msg) -> LRESULT
{
    (void)msg;
    LRESULT result = 0;
    return result;
}

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
auto AppWindow::handle_nc_hit_test(MSG* msg) -> LRESULT
{
    const int x = GET_X_LPARAM(msg->lParam);
    const int y = GET_Y_LPARAM(msg->lParam);

    RECT wr{};
    GetWindowRect(nativeWindowHandle(), &wr);

    const int left = wr.left;
    const int top = wr.top;
    const int right = wr.right;
    const int bottom = wr.bottom;

    const int cx = x - left;
    const int cy = y - top;
    const int w = right - left;
    const int h = bottom - top;

    // Border thickness (DPI aware)
    const int border = system_resize_border_width();
    const int caption = m_title_bar ? m_title_bar->height() : system_caption_height();

    // Compute edge booleans early so titlebar logic can respect resize zones.
    bool on_left = cx < border;
    bool on_right = cx >= w - border;
    bool on_top = cy < border;
    bool on_bottom = cy >= h - border;

    // If the window is maximized, ignore the resize zones so clicks at the outermost
    // pixels of the titlebar behave like titlebar drags (restore + move) instead of resize.
    if (::IsZoomed(nativeWindowHandle()))
    {
        on_left = on_right = on_top = on_bottom = false;
    }

    LRESULT hit = HTCLIENT;
    bool over_button = false;

    // First check titlebar buttons: if the global point is over a button, treat as client
    if (m_title_bar)
    {
        QPoint local = map_global_to_titlebar_local(x, y);

        if (m_title_bar->get_minimize_button() &&
            m_title_bar->get_minimize_button()->geometry().contains(local))
        {
            over_button = true;
            hit = HTCLIENT;
        }
        else if (m_title_bar->get_maximize_button() &&
                 m_title_bar->get_maximize_button()->geometry().contains(local))
        {
            over_button = true;
            hit = HTCLIENT;
        }
        else if (m_title_bar->get_close_button() &&
                 m_title_bar->get_close_button()->geometry().contains(local))
        {
            over_button = true;
            hit = HTCLIENT;
        }
        else
        {
            // Treat menubar and custom widget as interactive client area
            QMenuBar* tb_menubar = m_title_bar->get_menubar();
            QWidget* tb_custom = m_title_bar->get_custom_widget();

            if (tb_menubar != nullptr && tb_menubar->isVisible() &&
                tb_menubar->geometry().contains(local))
            {
                // Only treat as client when actually over a menu action; whitespace on the
                // top-row menubar should remain draggable (HTCAPTION).
                QPoint menubar_local = tb_menubar->mapFrom(m_title_bar, local);
                QAction* under_action = tb_menubar->actionAt(menubar_local);
                const int mid_y = m_title_bar->height() / 2;
                const bool menubar_on_top_row = tb_menubar->geometry().center().y() < mid_y;

                if (under_action != nullptr || !menubar_on_top_row)
                {
                    over_button = true;
                    hit = HTCLIENT;
                }
                // else: whitespace on top row menubar -> keep as caption-eligible
            }
            else if (tb_custom != nullptr && tb_custom->isVisible() &&
                     tb_custom->geometry().contains(local))
            {
                over_button = true;
                hit = HTCLIENT;
            }
        }
    }

    // If inside the titlebar area (but not buttons) -> allow dragging
    // Only set HTCAPTION when not over a button.
    if (m_title_bar && !over_button)
    {
        QPoint local = map_global_to_titlebar_local(x, y);

        if (m_title_bar->rect().contains(local))
        {
            // If the point lies inside the system resize border, allow the resize handling
            // below to run by not returning HTCAPTION here.
            if (!(on_top || on_left || on_right || on_bottom))
            {
                hit = HTCAPTION;
            }
            // else: fall through so edge/corner checks below can set resize values
        }
    }
    else if (!m_title_bar && !over_button)
    {
        // Fallback: allow top area as caption when no custom titlebar is present
        if (cy >= 0 && cy < caption)
        {
            hit = HTCAPTION;
        }
    }

    // Now handle resize zones (corners + edges) if caption wasn't set
    if (hit != HTCAPTION)
    {
        if (on_left && on_top)
        {
            hit = HTTOPLEFT;
        }
        else if (on_right && on_top)
        {
            hit = HTTOPRIGHT;
        }
        else if (on_left && on_bottom)
        {
            hit = HTBOTTOMLEFT;
        }
        else if (on_right && on_bottom)
        {
            hit = HTBOTTOMRIGHT;
        }
        else if (on_top)
        {
            hit = HTTOP;
        }
        else if (on_bottom)
        {
            hit = HTBOTTOM;
        }
        else if (on_left)
        {
            hit = HTLEFT;
        }
        else if (on_right)
        {
            hit = HTRIGHT;
        }
        else
        {
            hit = HTCLIENT;
        }
    }

    return hit;
}

/**
 * @brief Handle WM_GETMINMAXINFO so maximize fits monitor work area (taskbar-aware).
 *
 * Sets ptMaxPosition and ptMaxSize based on the monitor work area. Also sets minimum track
 * size based on widget minimumSize().
 *
 * @param lParam LPARAM passed to WM_GETMINMAXINFO (pointer to MINMAXINFO).
 */
auto AppWindow::handle_get_min_max_info(LPARAM lParam) -> void
{
    auto mmi = reinterpret_cast<MINMAXINFO*>(lParam);

    HMONITOR hMon = MonitorFromWindow(nativeWindowHandle(), MONITOR_DEFAULTTONEAREST);

    if (hMon)
    {
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);

        if (GetMonitorInfoW(hMon, &mi))
        {
            // Work area is the screen area excluding taskbar, etc.
            RECT work = mi.rcWork;
            RECT monitor = mi.rcMonitor;

            // Set the max position and size to the work area (so maximize doesn't cover taskbar)
            mmi->ptMaxPosition.x = work.left - monitor.left;
            mmi->ptMaxPosition.y = work.top - monitor.top;
            mmi->ptMaxSize.x = work.right - work.left;
            mmi->ptMaxSize.y = work.bottom - work.top;

            // Minimum size (you can customize)
            const QSize min_size = minimumSize();
            mmi->ptMinTrackSize.x = min_size.width();
            mmi->ptMinTrackSize.y = min_size.height();
        }
    }
}

/**
 * @brief Native event handler for Windows messages used by the custom frame implementation.
 *
 * Handles WM_NCCALCSIZE, WM_NCHITTEST, WM_GETMINMAXINFO, WM_DPICHANGED and a few others.
 *
 * Uses a single return at the end and avoids early returns to conform to project style.
 *
 * @param eventType Platform event type (unused on Windows).
 * @param message Pointer to a native MSG structure (MSG*).
 * @param result Output pointer where a platform-specific result (LRESULT) may be stored.
 * @return bool true when the message was handled by this function, false when the message should
 *              be forwarded to the base class implementation.
 */
auto AppWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) -> bool
{
    Q_UNUSED(eventType);
    MSG* msg = static_cast<MSG*>(message);

    bool handled = false;
    LRESULT out_result = 0;

    if (msg->message == WM_NCCALCSIZE)
    {
        out_result = handle_nc_calc_size(msg);
        handled = true;
    }
    else if (msg->message == WM_NCHITTEST)
    {
        out_result = handle_nc_hit_test(msg);
        handled = true;
    }
    else if (msg->message == WM_GETMINMAXINFO)
    {
        handle_get_min_max_info(msg->lParam);
        handled = false;  // allow DefWindowProc to process it as well
    }
    else if (msg->message == WM_SIZE || msg->message == WM_MOVE)
    {
        handled = false;
    }
    else if (msg->message == WM_DWMCOMPOSITIONCHANGED)
    {
        extend_frame_into_client_area();
        enable_win11_features();
        handled = false;
    }
    else if (msg->message == WM_DPICHANGED)
    {
        RECT* const prc_new_window = reinterpret_cast<RECT*>(msg->lParam);

        SetWindowPos(nativeWindowHandle(), nullptr, prc_new_window->left, prc_new_window->top,
                     prc_new_window->right - prc_new_window->left,
                     prc_new_window->bottom - prc_new_window->top, SWP_NOZORDER | SWP_NOACTIVATE);

        // Also reapply frame margins and Win11 attributes
        extend_frame_into_client_area();
        enable_win11_features();

        handled = false;
    }
    else if (msg->message == WM_NCACTIVATE)
    {
        // Prevent Windows from drawing the standard non-client area when activation changes.
        out_result = 1;
        handled = true;
    }
    else if (msg->message == WM_ERASEBKGND)
    {
        // Let Qt handle painting; claim the message handled to avoid flicker.
        out_result = 1;
        handled = true;
    }
    else
    {
        handled = false;
    }

    bool final_result = false;

    if (handled)
    {
        if (result)
        {
            *result = out_result;
        }

        final_result = true;
    }
    else
    {
        final_result = QWidget::nativeEvent(eventType, message, result);
    }

    return final_result;
}

#endif  // Q_OS_WIN

/**
 * @brief Adopt the central widget's QMenuBar into the title bar according to current flags.
 */
auto AppWindow::adopt_menubar_from_content_widget() -> void
{
    if (m_title_bar.isNull())
    {
        return;
    }

    QMenuBar* menubar = nullptr;

    if (m_content_widget != nullptr)
    {
        menubar = m_content_widget->findChild<QMenuBar*>();
    }

    if (m_adopt_menubar && menubar != nullptr)
    {
        m_title_bar->set_menubar(menubar);
        WindowTitleBar::RowPosition row = (m_menubar_row == WindowTitleBar::RowPosition::Top)
                                              ? WindowTitleBar::RowPosition::Top
                                              : WindowTitleBar::RowPosition::Bottom;
        m_title_bar->set_menubar_row(row);
    }
    else
    {
        m_title_bar->set_menubar(nullptr);
    }
}

#include "application.h"
#include "win32_config.h"
#include "logger.h"
#include <cassert>

namespace juce
{

LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    application* app = nullptr;

    if (msg == WM_NCCREATE)
    {
        // On window creation, store the 'this' pointer passed from CreateWindowEx.
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lp);
        app = reinterpret_cast<application*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);
    }
    else
    {
        // For other messages, retrieve the stored 'this' pointer.
        app = reinterpret_cast<application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (app)
    {
        switch (msg)
        {
        case WM_SIZE:
        {
            uint32_t width = LOWORD(lp);
            uint32_t height = HIWORD(lp);
            app->on_window_resized(width, height);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
        {
            return ::DefWindowProc(hwnd, msg, wp, lp);
        }
        }
        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wp, lp);
}

application::application(int args, char* argv[], int cx, int cy)
    : m_hwnd(nullptr), m_context(nullptr)
{
    // 1. Register the window class
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = static_wnd_proc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wc.lpszClassName = "Juce Engine";
    wc.hIcon = 0;

    if (!::RegisterClassExA(&wc))
    {
        assert(0 && "failed to registered class");
        return;
    }

    // 2. Create the window
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    int x = (screen_width - cx) / 2;
    int y = (screen_height - cy) / 2;

    m_hwnd = CreateWindowExA(
        NULL,
        wc.lpszClassName,
        wc.lpszClassName,
        WS_OVERLAPPEDWINDOW,
        x, y, cx, cy,
        nullptr,
        nullptr,
        wc.hInstance,
        this // Pass 'this' to be captured in WM_NCCREATE
    );

    assert(m_hwnd && L"failed to create window");

    // Initialize backend Renderer
    m_context = new context();
    if (!m_context->initialize(m_hwnd, wc.hInstance, cx, cy))
    {
        log_error("Failed to initialize backend");
        delete m_context;
        m_context = nullptr;
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOW);
    log_info("%s window created with Vulkan", wc.lpszClassName);
}

application::~application()
{
    if (m_context)
    {
        delete m_context;
    }
    // HWND is destroyed by the OS
}

int application::exec(void* scene)
{
    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // Main loop logic
        update();
        render();
    }
    return static_cast<int>(msg.wParam);
}

void application::update()
{
    // Game/application logic updates go here
}

void application::render()
{
    /**
    if (m_backend)
    {
        try
        {
            m_backend->draw_frame();
        }
        catch (const std::exception& e)
        {
            log_error("Error during rendering: %s", e.what());
            // Exit the loop on a critical rendering error
            PostQuitMessage(1);
        }
    }
         */
}

void application::on_window_resized(uint32_t width, uint32_t height)
{
    /**
    if (m_backend)
    {
        m_backend->on_window_resized(width, height);
    }
         */
}

HWND application::get_hwnd() const
{
    return m_hwnd;
}

} // namespace juce

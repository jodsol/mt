#include "application.h"
#include "win32_config.h"
#include "logger.h"
#include "vk_context.h"
#include "swapchain.h"
#include "backend.h"
#include <cassert>

namespace juce 
{


LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) 
{
    Application* app = nullptr;

    if (msg == WM_NCCREATE) 
    {
        // On window creation, store the 'this' pointer passed from CreateWindowEx.
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lp);
        app = reinterpret_cast<Application*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);
    }
    else 
    {
        // For other messages, retrieve the stored 'this' pointer.
        app = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
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

Application::Application(int args, char* argv[], int cx, int cy) 
    : m_hwnd(nullptr)
    , m_context(nullptr)
    , m_swapchain(nullptr)
    , m_backend(nullptr) // Initialize backend pointer
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

    // 3. Initialize Vulkan components in sequence
    // Initialize Vulkan Context
    m_context = new VKContext();
    if (!m_context->initialize(m_hwnd, wc.hInstance)) {
        log_error("Failed to initialize Vulkan context");
        delete m_context; m_context = nullptr;
        return;
    }

    // Initialize Swapchain
    m_swapchain = new Swapchain();
    if (!m_swapchain->initialize(m_context, cx, cy)) {
        log_error("Failed to initialize swapchain");
        delete m_swapchain; m_swapchain = nullptr;
        delete m_context; m_context = nullptr;
        return;
    }

    // Initialize Backend Renderer
    m_backend = new Backend(m_context, m_swapchain);
    if (!m_backend->initialize()) {
        log_error("Failed to initialize backend");
        delete m_backend; m_backend = nullptr;
        delete m_swapchain; m_swapchain = nullptr;
        delete m_context; m_context = nullptr;
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOW);
    log_info("%s window created with Vulkan", wc.lpszClassName);
}

Application::~Application() 
{
    // Clean up resources in reverse order of creation
    if (m_backend) {
        delete m_backend;
    }
    if (m_swapchain) {
        delete m_swapchain;
    }
    if (m_context) {
        delete m_context;
    }
    // HWND is destroyed by the OS
}

int Application::exec(void* scene) 
{
    MSG msg{};
    while (msg.message != WM_QUIT) 
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
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

void Application::update() 
{
    // Game/application logic updates go here
}

void Application::render() 
{
    if (m_backend) 
    {
        try {
            m_backend->draw_frame();
        } 
        catch (const std::exception& e) {
            log_error("Error during rendering: %s", e.what());
            // Exit the loop on a critical rendering error
            PostQuitMessage(1);
        }
    }
}

void Application::on_window_resized(uint32_t width, uint32_t height) 
{
    if (m_backend) {
        m_backend->on_window_resized(width, height);
    }
}

// --- Getters ---

VKContext* Application::get_context() const {
    return m_context;
}

Swapchain* Application::get_swapchain() const {
    return m_swapchain;
}

HWND Application::get_hwnd() const {
    return m_hwnd;
}

} // namespace juce

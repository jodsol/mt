#include "application.h"
#include "win32_config.h"
#include "logger.h"

namespace juce 
{

LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

Application::Application(int args, char* argv[], int cx, int cy) 
    : m_hwnd(nullptr)
    , m_context(nullptr)
    , m_swapchain(nullptr)
{
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpfnWndProc = static_wnd_proc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = 0;
    wc.lpszClassName = "Jure Engine";

    if (!::RegisterClassExA(&wc)) 
    {
        assert(0 && "failed to registered class");
    }

    int x = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;

    m_hwnd = CreateWindowExA(
        NULL,
        wc.lpszClassName,
        wc.lpszClassName,
        WS_OVERLAPPEDWINDOW,
        x, y, cx, cy, nullptr,
        0, wc.hInstance, nullptr);

    assert(m_hwnd && L"failed to create window");

    // Vulkan 초기화
    m_context = new VkContext();
    if (!m_context->initialize(m_hwnd, wc.hInstance)) {
        log_error("Failed to initialize Vulkan context");
        delete m_context;
        m_context = nullptr;
        return;
    }

    m_swapchain = new Swapchain();
    if (!m_swapchain->initialize(m_context, cx, cy)) {
        log_error("Failed to initialize swapchain");
        delete m_swapchain;
        m_swapchain = nullptr;
        delete m_context;
        m_context = nullptr;
        return;
    }

    ::ShowWindow(m_hwnd, SW_SHOW);
    log_info("%s window created with Vulkan", wc.lpszClassName);
}

Application::~Application() {
    if (m_swapchain) {
        delete m_swapchain;
        m_swapchain = nullptr;
    }
    
    if (m_context) {
        delete m_context;
        m_context = nullptr;
    }
}

LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE: {
        // 윈도우 크기 변경 시 스왑체인 재생성 필요
        // 실제 구현에서는 Application 인스턴스에 접근해서 스왑체인을 재생성해야 함
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    return ::DefWindowProc(hwnd, msg, wp, lp);
}

int Application::exec(void* scene) {
    MSG msg{};
    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            // FIXME : 임시 처리 추후 이벤트로 종료
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // update
        update();
        
        // render
        render();
    }
    return 0;
}

void Application::update() {
    // 게임 로직 업데이트
}

void Application::render() {
    if (!m_context || !m_swapchain) {
        return;
    }

    // 간단한 렌더링 루프
    // 실제로는 여기서 커맨드 버퍼를 기록하고 제출해야 함
    
    // 현재는 빈 구현
}

VkContext* Application::get_context() const {
    return m_context;
}

Swapchain* Application::get_swapchain() const {
    return m_swapchain;
}

HWND Application::get_hwnd() const {
    return m_hwnd;
}

} // namespace juce
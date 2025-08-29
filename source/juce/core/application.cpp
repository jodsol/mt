#include "application.h"
#include "win32_config.h"
#include "logger.h"

namespace juce
{

LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

Application::Application(int args, char* argv[], int cx, int cy)
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

    // m_context = new vk_context()
    ::ShowWindow(m_hwnd, SW_SHOW);

    log_info("%s window created", wc.lpszClassName);
}

LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_SIZE:

        break;

    default:
        break;
    }
    return ::DefWindowProc(hwnd, msg, wp, lp);
}

int Application::exec(void* scene)
{
    MSG msg{};
    while (msg.message != WM_QUIT)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            // FIXME : 임시 처리 추후 이벤트로 종료
            if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        // update

        // render
    }
    return 0;
}

} // namespace juce

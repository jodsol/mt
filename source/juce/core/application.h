#pragma once
#include "win32_config.h"

// Forward declarations
namespace juce {
    class VKContext;
    class Swapchain;
    class Backend; // Backend 클래스 전방 선언
}

namespace juce 
{

class Application {
public:
    Application(int args, char* argv[], int cx, int cy);
    ~Application();

    int exec(void* scene);
    void update();
    void render();
    
    // 창 크기 변경 이벤트를 처리할 함수
    void on_window_resized(uint32_t width, uint32_t height);

    // Getters
    VKContext* get_context() const;
    Swapchain* get_swapchain() const;
    HWND get_hwnd() const;

private:
    HWND m_hwnd;
    VKContext* m_context;
    Swapchain* m_swapchain;
    Backend* m_backend; // Backend 멤버 변수 추가
};

} // namespace juce
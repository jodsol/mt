#pragma once
#include "win32_config.h"

// Forward declarations
namespace juce
{
class vk_context;
class swapchain;
class backend; // backend 클래스 전방 선언
} // namespace juce

namespace juce
{

class application
{
public:
    application(int args, char* argv[], int cx, int cy);
    ~application();

    int exec(void* scene);
    void update();
    void render();

    // 창 크기 변경 이벤트를 처리할 함수
    void on_window_resized(uint32_t width, uint32_t height);

    // Getters
    vk_context* get_context() const;
    swapchain* get_swapchain() const;
    HWND get_hwnd() const;

private:
    HWND m_hwnd;
    vk_context* m_context;
    swapchain* m_swapchain;
    backend* m_backend; // backend 멤버 변수 추가
};

} // namespace juce
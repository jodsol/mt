#pragma once
#include "win32_config.h"
#include "typedef.h"
#include <juce/context/context.h>

// Forward declarations
namespace juce
{
class vk_context;
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
    HWND get_hwnd() const;

private:
    HWND m_hwnd;
    context* m_context;
};

} // namespace juce
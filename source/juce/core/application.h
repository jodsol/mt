#pragma once
#include "win32_config.h"
#include "vk_context.h"
#include "swapchain.h"

namespace juce {

class Application {
public:
    Application(int args, char* argv[], int cx, int cy);
    ~Application();

    int exec(void* scene);
    
    VkContext* get_context() const;
    Swapchain* get_swapchain() const;
    HWND get_hwnd() const;

private:
    void update();
    void render();

private:
    HWND m_hwnd;
    VkContext* m_context;
    Swapchain* m_swapchain;
};

} // namespace juce
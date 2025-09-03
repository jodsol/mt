#pragma once

#include <juce/core/typedef.h>

#include <juce/context/vulkan/vk_context.h>
#include <juce/context/vulkan/swapchain.h>

#include <vector>
#include <string>

namespace juce
{

struct native_window
{
#ifdef _WIN32
    HWND hwnd;
    HINSTANCE hinstance;
#else
    void* window; // Linux / macOS용
#endif
};

class context
{
public:
    context() = default;
    ~context() { cleanup(); }

    bool initialize(HWND hwnd, HINSTANCE hinstance, uint32_t width, uint32_t height);
    void cleanup();

private:
    vk_context m_context;
    swapchain m_swapchain;
};

} // namespace juce

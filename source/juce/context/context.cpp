#include "context.h"

namespace juce
{

bool context::initialize(HWND hwnd, HINSTANCE hinstance, uint32_t width, uint32_t height)
{
    m_context.initialize(hwnd, hinstance);
    m_swapchain.initialize(&m_context, width, height); // swapchain이 context 포인터 받는 경우
    return true;
}

void context::cleanup()
{
    m_swapchain.cleanup();
    m_context.cleanup();
}

} // namespace juce

#pragma once

#include "typedef.h"

namespace juce
{
class Application
{
public:
    Application(int args, char* argv[], int cx, int cy);

    int exec(void* scene);

    /// context/swapchain

private:
    HWND m_hwnd;
};
} // namespace juce

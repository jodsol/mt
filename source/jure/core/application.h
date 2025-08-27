#pragma once

#include "typedef.h"

namespace jure
{
class Application
{
public:
    Application(int args, char* argv[], int cx, int cy);

    int exec(void* scene);

private:
    HWND m_hwnd;
};
} // namespace jure

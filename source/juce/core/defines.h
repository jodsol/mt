#pragma once

// macro defines

// juce engine
#define ENGINE_NAME    "Juce Engine"
#define MAX_SYNC_FRAME 2

// clang-format off
#define unused(x) (void)(x)
#define safe_delete(p)      do { if(p){ delete (p); (p)=nullptr; } } while(0)
#define safe_delete_array(p) do { if(p){ delete[](p); (p)=nullptr; } } while(0)


#if defined(_DEBUG) && defined(_MSC_VER) 
#include <crtdbg.h>
#define debug_new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define check_mem_leak() _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
#else
#define check_mem_leak() (void)0
#define debug_new new
#endif

// clang-format on

#include "logger.h"

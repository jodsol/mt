#pragma once

// newmeric
#include <stdint.h>
typedef uint8_t  byte, uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// Win32
struct HWND__;
typedef HWND__* HWND;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

// Juce Forward Declare
namespace juce
{
class graphics_context;
}

// macro defines

// clang-format off
#define unused(x) (void)(x)
#define safe_delete(p)      do { if(p){ delete (p); (p)=nullptr; } } while(0)
#define safe_delete_array(p) do { if(p){ delete[](p); (p)=nullptr; } } while(0)
// clang-format on

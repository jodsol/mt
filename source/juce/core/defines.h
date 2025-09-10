#pragma once

// macro defines

// juce engine
#define juce_engine_name "Juce Engine"

// clang-format off
#define unused(x) (void)(x)
#define safe_delete(p)      do { if(p){ delete (p); (p)=nullptr; } } while(0)
#define safe_delete_array(p) do { if(p){ delete[](p); (p)=nullptr; } } while(0)

#define juce_release(p) do { if(p){ p->release(); (p)=nullptr; } } while(0)
// clang-format on

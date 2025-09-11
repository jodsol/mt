#pragma once

// macro defines

// juce engine
#define JUCE_ENGINE_NAME "Juce Engine"

// clang-format off
#define unused(x) (void)(x)
#define safe_delete(p)      do { if(p){ delete (p); (p)=nullptr; } } while(0)
#define safe_delete_array(p) do { if(p){ delete[](p); (p)=nullptr; } } while(0)

#define juce_release(p) do { if(p){ p->release(); (p)=nullptr; } } while(0)

#define debug_new new
// clang-format on

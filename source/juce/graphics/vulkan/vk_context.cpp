#include "vk_context.h"
#include "vk_config.h"

namespace juce
{
vk_context::vk_context(uint32_t cx, uint32_t cy, void* platform_handle) :
    graphics_context(cx, cy, platform_handle)
{
}

void vk_context::on_resized(uint32 cx, uint32 cy)
{
	unused(cx);
	unused(cy);
}

}        // namespace juce

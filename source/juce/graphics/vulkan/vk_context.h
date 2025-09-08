#pragma once

#include <juce/graphics/context.h>

namespace juce
{
class vk_context : public graphics_context
{
public:
	vk_context(uint32_t cx, uint32_t cy, void* platform_handle);

	void on_resized(uint32 cx, uint32 cy) override;
};
}        // namespace juce

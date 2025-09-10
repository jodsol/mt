#pragma once

#include "vk_config.h"
#include <juce/graphics/context.h>
#include <memory>
#include <vector>

namespace juce
{
class vk_instance;
class vk_context : public graphics_context
{
public:
	vk_context(uint32_t cx, uint32_t cy, void* platform_handle);

	void on_resized(uint32 cx, uint32 cy) override;

	VkInstance get_instance() const;

	// vk_instance* m_instance{nullptr};
	std::unique_ptr<vk_instance> m_instance;
};
}        // namespace juce

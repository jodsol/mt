#pragma once

#include "vk_config.h"
#include <juce/graphics/context.h>
#include <memory>
#include <vector>

namespace juce
{
class vk_instance;
class vk_surface;
class vk_device;
class vk_context : public graphics_context
{
public:
	vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle);
	~vk_context();
	void on_resized(uint32 cx, uint32 cy) override;

	vk_instance* m_instance{nullptr};
	vk_surface*  m_surface{nullptr};
	vk_device*   m_device{};
};
}        // namespace juce

#pragma once

#include <juce/graphics/vulkan/vk_config.h>
#include <juce/core/typedef.h>

namespace juce
{
struct vk_debug
{
	static void print_instance(vk_instance* instance);
	static void print_queue_families(vk_logical_device* device);
	static void print_physical_device(const vk_logical_device* device);
	static void print_logical_device_info(vk_logical_device* device);
};

}        // namespace juce

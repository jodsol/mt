#pragma once

#include <juce/graphics/vulkan/vk_config.h>
#include <juce/graphics/vulkan/vk_handle.h>

namespace juce
{
// 리 소스 상태
enum class resource_state : uint32_t
{
	undefined,
	render_target,
	present
};

struct image_resource
{
	VkImageAspectFlags aspect      = VK_IMAGE_ASPECT_COLOR_BIT;
	uint32_t           base_mip    = 0;
	uint32_t           base_layer  = 0;
	uint32_t           level_count = 1;
	uint32_t           layer_count = 1;
};

struct vk_resouce_state
{
	VkPipelineStageFlags2 stage;
	VkAccessFlags2        acess;
	VkImageLayout         layout;
};

struct image_transition
{
	VkImage        image = VK_NULL_HANDLE;
	image_resource imaget_resource{};
	resource_state before = resource_state::undefined;
	resource_state after  = resource_state::undefined;
};

class vk_command_list : public vk_handle<VkCommandBuffer>
{
public:
	vk_command_list() = default;

	void init(VkCommandBuffer cmd);

	void begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	void end();

	static void convert_vk_resource_state(const resource_state& resouce_state, vk_resouce_state& states);

	// resource barriers
	void resouce_barrier(const image_transition& trans);
};
}        // namespace juce

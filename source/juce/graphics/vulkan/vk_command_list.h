#pragma once

#include <juce/core/typedef.h>
#include "vk_config.h"
#include "vk_handle.h"

namespace juce
{
// 리 소스 상태

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
	VkImage         image = VK_NULL_HANDLE;
	image_resource  imaget_resource{};
	resource_layout before = resource_layout::undefined;
	resource_layout after  = resource_layout::undefined;
};

#define MAX_COLOR_ATTACHMENT 12

class vk_command_list : public vk_handle<VkCommandBuffer>
{
public:
	vk_command_list(VkDevice device, VkCommandPool pool, VkCommandBufferLevel level);
	~vk_command_list();

	void reset();
	void close();

	void begin_render_target(uint32_t rtv_count, vk_render_target** rtvs, vk_render_target* dsv);

	void end_render_target();

	void clear_color_render_target(vk_render_target* rtv, const float* value);

	static void convert_vk_resource_layout(const resource_layout& resouce_state,
	                                       vk_resouce_state&      states);

	// resource barriers
	void resouce_barrier(const image_transition& trans);

	struct
	{
		VkRenderingAttachmentInfo color_info[MAX_COLOR_ATTACHMENT]{};
		VkRenderingAttachmentInfo depth_info{};
		uint32_t                  color_info_count;
	} m_render_attachments;

	VkDevice      m_device                 = VK_NULL_HANDLE;
	VkCommandPool m_pool                   = VK_NULL_HANDLE;
	bool          m_is_cmd_rendering_bound = false;
	bool          m_is_clear_color         = false;
};
}        // namespace juce

#include "vk_command_list.h"

namespace juce
{

void vk_command_list::init(VkCommandBuffer cmd)
{
	assert(cmd && "failed to init command list");
	m_handle = cmd;
}

void vk_command_list::begin(VkCommandBufferUsageFlags flags)
{
	VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	info.flags = flags;
	VK(vkBeginCommandBuffer(m_handle, &info));
}

void vk_command_list::end()
{
	VK(vkEndCommandBuffer(m_handle));
}

void vk_command_list::convert_vk_resource_state(const resource_state& resouce_state, vk_resouce_state& states)
{
	switch(resouce_state) {
		case resource_state::render_target:
		{
			states.stage  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			states.acess  = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
			states.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		} break;
		case resource_state::present:
		{
			states.stage  = VK_PIPELINE_STAGE_2_NONE;
			states.acess  = 0;
			states.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		} break;
		case resource_state::undefined:
		{
			states.stage  = VK_PIPELINE_STAGE_2_NONE;
			states.acess  = 0;
			states.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		} break;
		default:
		{
			states.stage  = VK_PIPELINE_STAGE_2_NONE;
			states.acess  = 0;
			states.layout = VK_IMAGE_LAYOUT_GENERAL;
		} break;
	}
}

void vk_command_list::resouce_barrier(const image_transition& trans)
{
	// image 만
	vk_resouce_state src{};        // stage , access , layout
	vk_resouce_state dst{};
	convert_vk_resource_state(trans.before, src);
	convert_vk_resource_state(trans.after, dst);

	VkImageSubresourceRange range{};
	range.aspectMask     = trans.imaget_resource.aspect;
	range.baseMipLevel   = trans.imaget_resource.base_mip;
	range.levelCount     = trans.imaget_resource.level_count;
	range.baseArrayLayer = trans.imaget_resource.base_layer;
	range.layerCount     = trans.imaget_resource.layer_count;

	VkImageMemoryBarrier2 barrier_info{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
	barrier_info.srcStageMask        = src.stage;
	barrier_info.srcAccessMask       = src.acess;
	barrier_info.dstStageMask        = dst.stage;
	barrier_info.dstAccessMask       = dst.acess;
	barrier_info.oldLayout           = src.layout;
	barrier_info.newLayout           = dst.layout;
	barrier_info.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier_info.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier_info.image               = trans.image;
	barrier_info.subresourceRange    = range;

	// temporary
	// 1.3 이후 표준 모델
	VkDependencyInfo dep_info{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
	dep_info.imageMemoryBarrierCount = 1;
	dep_info.pImageMemoryBarriers    = &barrier_info;

	vkCmdPipelineBarrier2(m_handle, &dep_info);
}

}        // namespace juce

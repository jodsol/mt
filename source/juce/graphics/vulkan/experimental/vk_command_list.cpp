#include "vk_command_list.h"
#include "vk_transform.h"

namespace juce
{

void vk_command_list::init(VkCommandBuffer cmd)
{
	assert(cmd && "failed to init command list");
	m_handle = cmd;
}

void vk_command_list::reset()
{
	vkResetCommandBuffer(m_handle, 0);
	VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK(vkBeginCommandBuffer(m_handle, &info));
	m_is_cmd_rendering_bound = false;
}

void vk_command_list::close()
{
	VK(vkEndCommandBuffer(m_handle));
}
// render pass
void vk_command_list::begin_render_target(uint32_t rtv_count, vk_render_target** rtvs,
                                          vk_render_target* dsv)
{
	assert(rtv_count < MAX_COLOR_ATTACHMENT && "out of max color attachments");
	// assert(m_is_cmd_rendering == false && "make sure close the end_render_target");
	if(m_is_cmd_rendering_bound) {
		// assert(false && "make sure close the end_render_target");
		vkCmdEndRendering(m_handle);
	}

	m_render_attachments                  = {};
	m_render_attachments.color_info_count = rtv_count;

	for(uint32_t i = 0; i < rtv_count; ++i) {
		VkRenderingAttachmentInfo& color_info = m_render_attachments.color_info[i];
		const vk_render_target*    rtv        = rtvs[i];
		color_info.sType                      = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		color_info.imageView                  = rtv->view;
		color_info.clearValue                 = rtv->clear_value;
		color_info.imageLayout                = rtv->layout;
		// loadop = 렌더 시작시 어떻게 처리 할지 정의
		// LOAD_OP_LOAD = 기존 이미지 내용 그대로 유지
		// LOAD_OP_CLEAR = 랜더 시작전 clear value 로 초기화
		// LOAD_OP_DONT_CARE = 기존 값 버리고 최적화 (fullscreen quad)
		// color_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_info.loadOp = vk_transform::cast(rtv->load_op);
		// storeop = 렌더가 끝난후 어떻게 처리 할지 정의
		// STORE_OP_STORE : 렌더 종료시 값을 GPU 메모리에 저장
		// swapchain backbuffer 를 화면으로 present 할때
		// G-Nuffer 등을 후속 패스로 사용 해야할때
		// 최종 결과를 텍스처로 샘플링 할때
		// STORE_OP_DONT_CARE
		// 렌더후 내용 Discard(폐기)
		// 깊이 버퍼 매프레임 새로 클리어
		// color_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_info.storeOp = vk_transform::cast(rtv->store_op);
	}

	if(dsv) {
	}

	VkRenderingInfo info{VK_STRUCTURE_TYPE_RENDERING_INFO};
	info.renderArea.extent    = rtvs[0]->extend;
	info.layerCount           = 1;
	info.colorAttachmentCount = m_render_attachments.color_info_count;
	info.pColorAttachments    = m_render_attachments.color_info;

	vkCmdBeginRendering(m_handle, &info);
	m_is_cmd_rendering_bound = true;
}

void vk_command_list::end_render_target()
{
	if(m_is_cmd_rendering_bound) {
		vkCmdEndRendering(m_handle);
		m_is_cmd_rendering_bound = false;
	}
}

void vk_command_list::clear_color_render_target(vk_render_target* rtv, const float* value)
{
	std::memcpy(rtv->clear_value.color.float32, value, sizeof(float) * 4);
}

void vk_command_list::convert_vk_resource_state(const resource_state& resouce_state,
                                                vk_resouce_state&     states)
{
	switch(resouce_state) {
		case resource_state::render_target:
		{
			states.stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
			states.acess =
			    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
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

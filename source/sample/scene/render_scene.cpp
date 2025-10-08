#include "render_scene.h"
#include <juce/graphics/vulkan/experimental/vk_context_ext.h>
#include <juce/graphics/vulkan/experimental/vk_resouce_cache.h>
#include <juce/graphics/vulkan/vk_buffer.h>

namespace juce
{
void render_scene::init()
{
	const vertex vertices[] = {{1.f, 1.f, 1.f}};

	buffer_create_info info{};
	info.p_data  = vertices;
	info.cb_size = std::size(vertices);
	info.type    = buffer_type::vertex;

	vk_resource_cache::create_buffer(&info, &m_vertex_buffer);

	vkDeviceWaitIdle(m_context->device());

	safe_delete(m_vertex_buffer);

	int a = 0;
}

void render_scene::update_frame(float dt) {}

void render_scene::render_frame()
{
	vk_context_ext*  ctx      = dynamic_cast<vk_context_ext*>(m_context);
	vk_command_list* cmd_list = ctx->get_current_command_list();
	// VkCommandBuffer  cmd          = reinterpret_cast<VkCommandBuffer>(command_list);
	auto back_buffer = ctx->get_current_swapchain_render_target();
	ctx->get_current_swapchain_render_target();

	const float clear[] = {0.5f, 0.5f, 0.2f, 1.f};

	cmd_list->clear_color_render_target(back_buffer, clear);

	cmd_list->begin_render_target(1, &back_buffer, nullptr);

	// draw
	cmd_list->end_render_target();
}

void render_scene::release()
{
	// safe_delete(m_vertex_buffer);
	vkDeviceWaitIdle(vk_resource_cache::m_device);

	// delete m_vertex_buffer;
}

}        // namespace juce

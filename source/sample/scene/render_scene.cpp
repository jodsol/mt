#include "render_scene.h"

#include <juce/graphics/vulkan/vk_buffer.h>
#include <juce/graphics/vulkan/vk_logical_device.h>
#include <juce/graphics/vulkan/vk_command_list.h>

namespace juce
{
void render_scene::init()
{
	const vertex vertices[] = {{1.f, 1.f, 1.f}};

	buffer_create_info info{};
	info.data = vertices;
	info.size = std::size(vertices);
	info.type = buffer_type::vertex;

	int a = 0;
}

void render_scene::update_frame(float dt) {}

void render_scene::render_frame()
{
	vk_command_list* cmd_list = m_context->get_current_command_list();

	vk_render_target* back_buffer = m_context->get_current_swapchain_render_target();

	const float clear[] = {0.5f, 0.5f, 0.2f, 1.f};

	cmd_list->clear_color_render_target(back_buffer, clear);

	cmd_list->begin_render_target(1, &back_buffer, nullptr);

	// draw
	cmd_list->end_render_target();
}

void render_scene::release()
{
	// safe_delete(m_vertex_buffer);

	// delete m_vertex_buffer;
}

}        // namespace juce

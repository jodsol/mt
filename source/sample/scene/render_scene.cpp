#include "render_scene.h"

#include <juce/graphics/vulkan/vk_buffer.h>
#include <juce/graphics/vulkan/vk_device.h>
#include <juce/graphics/vulkan/vk_shader.h>
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

	shader_create_info vs{};
	vs.stage    = shader_stage::vertex;
	vs.filename = "simple.vert";
	vs.entry    = "main";

	shader_create_info fs{};
	fs.stage    = shader_stage::pixel;
	fs.filename = "simple.frag";
	fs.entry    = "main";

	vk_shader* vert_shader = nullptr;
	vk_shader* frag_shader = nullptr;

	m_context->m_device->create_spv_from_file(&vs, &vert_shader);
	m_context->m_device->create_spv_from_file(&fs, &frag_shader);

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

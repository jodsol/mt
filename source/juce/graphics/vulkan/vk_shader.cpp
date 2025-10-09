#include "vk_shader.h"
#include "vk_config.h"
#include <stdexcept>

namespace juce
{

vk_shader::vk_shader(VkDevice device, const std::vector<uint32_t>& spirv, shader_stage vk_stage) : device(device)
{
	switch(vk_stage) {
		case shader_stage::vertex: stage = VK_SHADER_STAGE_VERTEX_BIT; break;
		case shader_stage::pixel: stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
		case shader_stage::geometry: stage = VK_SHADER_STAGE_GEOMETRY_BIT; break;
		case shader_stage::hull: stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; break;
		default: stage = VK_SHADER_STAGE_VERTEX_BIT; break;
	}

	VkShaderModuleCreateInfo create_info{};
	create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = spirv.size() * sizeof(uint32_t);
	create_info.pCode    = spirv.data();

	if(vkCreateShaderModule(device, &create_info, nullptr, &module) != VK_SUCCESS) {
		log_error("Failed to create shader module");
	}
}

vk_shader::~vk_shader()
{
	if(module != VK_NULL_HANDLE)
		vkDestroyShaderModule(device, module, nullptr);
}

VkPipelineShaderStageCreateInfo vk_shader::get_stage_info(const char* entry) const
{
	VkPipelineShaderStageCreateInfo stage_info{};
	stage_info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage_info.stage  = stage;
	stage_info.module = module;
	stage_info.pName  = entry;        // GLSL의 entry point (ex. "main")

	return stage_info;
}

}        // namespace juce

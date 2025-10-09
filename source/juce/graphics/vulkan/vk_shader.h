#pragma once

#include <juce/core/typedef.h>
#include "vk_config.h"
#include <string>
#include <vector>

namespace juce
{

struct vk_shader
{

    vk_shader(VkDevice device, const std::vector<uint32_t>& spirv, shader_stage vk_stage);
    ~vk_shader();

    VkPipelineShaderStageCreateInfo get_stage_info(const char* entry = "main") const;

    VkDevice              device = VK_NULL_HANDLE;
    VkShaderModule        module = VK_NULL_HANDLE;
    VkShaderStageFlagBits stage{};
};

} // namespace juce
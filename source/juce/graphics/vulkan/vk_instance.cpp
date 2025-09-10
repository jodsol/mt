#include "vk_instnace.h"
#include <juce/core/logger.h>
#include <vulkan/vulkan.hpp>

namespace juce
{
vk_instance::vk_instance(const char* name, const std::vector<const char*>& req_extentions, bool enable_validation)
{
	create_instnace(name, req_extentions, enable_validation);
}

vk_instance::~vk_instance()
{
	if (!m_instance)
		vkDestroyInstance(m_instance, nullptr);
	m_instance = VK_NULL_HANDLE;
}

void vk_instance::create_instnace(
    const char*                     name,
    const std::vector<const char*>& req_extensions,
    bool                            enable_validation)
{
	VkApplicationInfo app_info{};
	app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName   = name;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName        = "juce-engine";
	app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion         = VK_VERSION_1_3;

	std::vector<const char*> extensions = req_extensions;

	get_supported_extensions();

	for (auto& e : m_instance_extension)
	{
		log_info("instance extension : %s", e.extensionName);
	}

	int a = 0;
}

void vk_instance::get_supported_extensions()
{
	uint32_t ext_count{};

	vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
	m_instance_extension.resize(ext_count);
	if (ext_count)
	{
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, m_instance_extension.data());
	}

	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	if (layer_count)
	{
		vkEnumerateInstanceLayerProperties(&layer_count, m_instance_layers.data());
	}
}

}        // namespace juce

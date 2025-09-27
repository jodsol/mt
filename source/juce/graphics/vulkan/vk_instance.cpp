#include "vk_instance.h"
#include <juce/core/logger.h>

namespace juce
{
vk_instance::vk_instance(const char*                     name,
                         const std::vector<const char*>& req_extentions,
                         const std::vector<const char*>& req_layers)
{
	create_instnace(name, req_extentions, req_layers);
}

vk_instance::~vk_instance()
{
	if(!m_handle)
		vkDestroyInstance(m_handle, nullptr);
	m_handle = VK_NULL_HANDLE;
}

void vk_instance::create_instnace(
    const char*                     name,
    const std::vector<const char*>& req_extensions,
    const std::vector<const char*>& req_layers)
{
	get_supported_exts_and_layers();

	std::vector<const char*> availble_extension;
	std::vector<const char*> availble_layer;

	for(auto const& req_ext : req_extensions) {
		if(is_supported_extension(req_ext)) {
			log_debug("[juce] instance extension : %s", req_ext);
			availble_extension.emplace_back(req_ext);
		}
	}

	for(auto const& req_layer : req_layers) {
		if(is_supported_layer(req_layer)) {
			log_debug("[juce] instance layer : %s", req_layer);
			availble_layer.emplace_back(req_layer);
		}
	}

	VkApplicationInfo app_info{};
	app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName   = name;
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName        = "juce-engine";
	app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion         = VK_API_VERSION_1_3;

	VkInstanceCreateInfo instance_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_info.pApplicationInfo        = &app_info;
	instance_info.enabledExtensionCount   = availble_extension.size();
	instance_info.ppEnabledExtensionNames = availble_extension.data();
	instance_info.enabledLayerCount       = availble_layer.size();
	instance_info.ppEnabledLayerNames     = availble_layer.data();

	VK(vkCreateInstance(&instance_info, nullptr, &m_handle));
}

bool vk_instance::is_supported_extension(const char* name) const
{
	return m_instance_extensions.contains(name);
}

bool vk_instance::is_supported_layer(const char* name) const
{
	return m_instance_layers.contains(name);
}

void vk_instance::get_supported_exts_and_layers()
{
	// ----- Instance Extensions -----
	uint32_t ext_count{};
	vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);

	if(ext_count) {
		std::vector<VkExtensionProperties> props(ext_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, props.data());

		m_instance_extensions.clear();
		for(const auto& e : props) {
			m_instance_extensions.insert(e.extensionName);
		}
	}

	// ----- Instance Layers -----
	uint32_t layer_count{};
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	if(layer_count) {
		std::vector<VkLayerProperties> layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

		m_instance_layers.clear();
		for(const auto& l : layers) {
			m_instance_layers.insert(l.layerName);
		}
	}
}

}        // namespace juce

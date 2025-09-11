#pragma once

#include "vk_config.h"
#include <string>
#include <vector>
#include <unordered_set>
#include "vk_handle.h"

namespace juce
{
class vk_instance : public vk_handle<VkInstance>
{
public:
	friend vk_handle<VkInstance>;
	vk_instance(const char*                     name,
	            const std::vector<const char*>& req_extentions,
	            const std::vector<const char*>& req_layers);
	~vk_instance();

	void create_instnace(
	    const char*                     name,
	    const std::vector<const char*>& req_extensions,
	    const std::vector<const char*>& req_layers);

	bool is_supported_extension(const char* name) const;
	bool is_supported_layer(const char* name) const;

	void get_supported_exts_and_layers();

private:
	// VkInstance                      m_instance           = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT        m_debug_messger      = VK_NULL_HANDLE;
	bool                            m_validation_enabled = false;
	std::unordered_set<std::string> m_instance_extensions;
	std::unordered_set<std::string> m_instance_layers;
};

}        // namespace juce

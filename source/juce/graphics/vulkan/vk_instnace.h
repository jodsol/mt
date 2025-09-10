#pragma once

#include "vk_config.h"
#include <string>
#include <vector>

namespace juce
{
class vk_instance
{
public:
	vk_instance(const char* name, const std::vector<const char*>& req_extentions = {}, bool enable_validation = true);
	~vk_instance();

	void create_instnace(
	    const char*                     name,
	    const std::vector<const char*>& req_extensions,
	    bool                            enable_validation);

	VkInstance get_handle() const
	{
		return m_instance;
	}

	void get_supported_extensions();

private:
	VkInstance                         m_instance{VK_NULL_HANDLE};
	std::vector<VkExtensionProperties> m_instance_extension;
	std::vector<VkLayerProperties>     m_instance_layers;
};

}        // namespace juce

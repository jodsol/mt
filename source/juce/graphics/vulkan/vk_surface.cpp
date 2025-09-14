#include "vk_surface.h"

namespace juce
{
vk_surface::vk_surface(VkInstance instance, platform_handle plaform_handle) :
    m_instance(instance)
{
#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
	info.hinstance = GetModuleHandle(nullptr);
	info.hwnd      = plaform_handle;
	VK(vkCreateWin32SurfaceKHR(m_instance, &info, nullptr, &m_handle));
#else
	// TODO linux macos other platform
	unused(plaform_handle);
	m_surface = VK_NULL_HANDLE;
#endif
}

vk_surface::~vk_surface()
{
	if (m_handle && m_instance) {
		vkDestroySurfaceKHR(m_instance, m_handle, nullptr);
		m_handle   = VK_NULL_HANDLE;
		m_instance = VK_NULL_HANDLE;
	}
}

bool vk_surface::valid() const
{
	return m_handle != VK_NULL_HANDLE;
}

bool vk_surface::query(VkPhysicalDevice device)
{
	if (!this->valid())
		return false;

	VkPhysicalDeviceSurfaceInfo2KHR info{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR};
	info.surface = m_handle;

	caps = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR};

	if (vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &info, &caps) != VK_SUCCESS) {
		return false;
	}

	return false;
}

}        // namespace juce

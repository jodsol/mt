#include "vk_debug.h"
#include "../vk_instance.h"
#include "../vk_logical_device.h"

namespace juce
{
void vk_debug::print_instance(vk_instance* instance)
{
}

void vk_debug::print_logical_device_info(vk_logical_device* device)
{
	auto gpu            = device->get_gpu()->handle;
	auto graphics_queue = device->graphics_queue();
	auto present_queue  = device->present_queue();
	auto compute_queue  = device->compute_queue();
	auto transfer_queue = device->transfer_queue();

	auto yn = [](VkBool32 b) { return b ? "YES" : "NO"; };

	log_debug("+--------------------------- LOGICAL DEVICE INFO ----------------------------+");
	log_debug("| %-22s | %-49p |", "VkDevice handle", (void*)device->handle());
	log_debug("+-------------------------+--------------------------------------------------+");

	log_debug("| %-22s | %-49p |", "Graphics Queue", (void*)graphics_queue);
	log_debug("| %-22s | %-49p |", "Present Queue ", present_queue ? (void*)present_queue : "(none)");
	log_debug("| %-22s | %-49p |", "Compute Queue ", compute_queue ? (void*)compute_queue : "(none)");
	log_debug("| %-22s | %-49p |", "Transfer Queue", transfer_queue ? (void*)transfer_queue : "(none)");
	log_debug("+-------------------------+--------------------------------------------------+");

	// Vulkan features
	VkPhysicalDeviceVulkan13Features feat13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	VkPhysicalDeviceVulkan12Features feat12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
	VkPhysicalDeviceFeatures2        feat2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
	feat2.pNext  = &feat12;
	feat12.pNext = &feat13;
	vkGetPhysicalDeviceFeatures2(gpu, &feat2);

	log_debug("| %-22s | %-49s |", "Vulkan 1.2 Features", "");
	log_debug("|   %-20s | %-49s |", "timelineSemaphore", yn(feat12.timelineSemaphore));
	log_debug("|   %-20s | %-49s |", "bufferDeviceAddress", yn(feat12.bufferDeviceAddress));
	log_debug("|   %-20s | %-49s |", "descriptorIndexing", yn(feat12.descriptorIndexing));

	log_debug("| %-22s | %-49s |", "Vulkan 1.3 Features", "");
	log_debug("|   %-20s | %-49s |", "synchronization2", yn(feat13.synchronization2));
	log_debug("|   %-20s | %-49s |", "dynamicRendering", yn(feat13.dynamicRendering));
	log_debug("|   %-20s | %-49s |", "maintenance4", yn(feat13.maintenance4));
	log_debug("|   %-20s | %-49s |", "inlineUniformBlock", yn(feat13.inlineUniformBlock));

	log_debug("+----------------------------------------------------------------------------+");
}

void vk_debug::print_queue_families(vk_logical_device* device)
{
	auto gpu     = device->get_gpu()->handle;
	auto surface = device->surface();

	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
	std::vector<VkQueueFamilyProperties> props(count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, props.data());

	log_debug("----------------------------------- QUEUE FAMILY INFO ----------------------------------------");
	log_debug("+---------+----------------------------------------------------+------------+----------------+");
	log_debug("| Family# | Flags (queueFlags)                                 | queueCount | Present(surf)  |");
	log_debug("+---------+----------------------------------------------------+------------+----------------+");

	for(uint32_t i = 0; i < count; ++i) {
		auto f = props[i].queueFlags;
		bool g = (f & VK_QUEUE_GRAPHICS_BIT) != 0;
		bool c = (f & VK_QUEUE_COMPUTE_BIT) != 0;
		bool t = (f & VK_QUEUE_TRANSFER_BIT) != 0;
		bool s = (f & VK_QUEUE_SPARSE_BINDING_BIT) != 0;
#ifdef VK_ENABLE_BETA_EXTENSIONS
		bool v = (f & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0 ||
		    (f & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0;
#endif

		std::string fs;
		if(g)
			fs += "GRAPHICS | ";
		if(c)
			fs += "COMPUTE  | ";
		if(t)
			fs += "TRANSFER | ";
		if(s)
			fs += "SPARSE   | ";
#ifdef VK_ENABLE_BETA_EXTENSIONS
		if(v)
			fs += "VIDEO | ";
#endif
		if(!fs.empty())
			fs.resize(fs.size() - 3);        // 마지막 " | " 제거

		VkBool32 present = VK_FALSE;
		if(surface != VK_NULL_HANDLE) {
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &present);
		}

		log_debug("| %7u | %-50s | %10u | %14s |",
		          i,
		          fs.empty() ? "-" : fs.c_str(),
		          props[i].queueCount,
		          present ? "YES" : "NO");
	}

	log_debug("+---------+----------------------------------------------------+------------+----------------+");
}

static inline const char* device_type_str(VkPhysicalDeviceType type)
{
	switch(type) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";
		default:
			return "Other/Unknown";
	}
}

void vk_debug::print_physical_device(const vk_logical_device* device)
{
	auto               gpu = *device->get_gpu();
	unsigned long long vramMB =
	    static_cast<unsigned long long>(vk_logical_device::get_gpu_available_vram_byte(gpu.memory_props) / (1024ull * 1024ull));

	char apiVerStr[32];
	snprintf(apiVerStr, sizeof(apiVerStr), "%u.%u.%u",
	         VK_VERSION_MAJOR(gpu.device_props.apiVersion),
	         VK_VERSION_MINOR(gpu.device_props.apiVersion),
	         VK_VERSION_PATCH(gpu.device_props.apiVersion));

	log_debug("+------------- PHYSICAL DEVICE INFO -----------------+");
	log_debug("+----------------+-----------------------------------+");
	log_debug("| %-14s | %-33s |", "Name", gpu.device_props.deviceName);
	log_debug("| %-14s | %-33s |", "Type", device_type_str(gpu.device_props.deviceType));
	log_debug("| %-14s | %-33u |", "Driver Ver.", gpu.device_props.driverVersion);
	log_debug("| %-14s | %-33s |", "API Ver.", apiVerStr);
	log_debug("| %-14s | %-33llu |", "VRAM (MB)", vramMB);
	log_debug("+----------------+-----------------------------------+");
}

}        // namespace juce

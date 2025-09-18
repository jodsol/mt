#include "vk_device.h"

namespace juce
{
vk_device::vk_device(VkInstance instance, VkSurfaceKHR surface) :
    m_surface(surface)
{
	create_device(instance, surface);
}

vk_device::~vk_device()
{
	if (m_handle)
		vkDestroyDevice(m_handle, nullptr);
}

void vk_device::create_device(VkInstance instance, VkSurfaceKHR surface)
{
	if (!choose_physical_device(instance, surface, &m_gpu)) {
		assert(0 && "failed create device");
	}

	create_logical_device(surface);
}

queue_indices find_queue_indices(
    VkPhysicalDevice                            gpu,
    VkSurfaceKHR                                surface,
    const std::vector<VkQueueFamilyProperties>& queue_props)
{
	queue_indices queue_indices{};

	for (uint32_t i = 0; i < (uint32_t) queue_props.size(); ++i) {
		VkQueueFlags flag = queue_props[i].queueFlags;

		if ((flag & VK_QUEUE_GRAPHICS_BIT) && !queue_indices.graphics) {
			queue_indices.graphics = i;
		}
		if (!queue_indices.present) {
			VkBool32 supported_present = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supported_present);
			if (supported_present)
				queue_indices.present = i;
		}
		if ((flag & VK_QUEUE_COMPUTE_BIT) && !(flag & VK_QUEUE_GRAPHICS_BIT) && !queue_indices.compute)
			queue_indices.compute = i;

		if ((flag & VK_QUEUE_TRANSFER_BIT) && !(flag & VK_QUEUE_GRAPHICS_BIT) && !queue_indices.transfer)
			queue_indices.transfer = i;
	}
	return queue_indices;
}

uint32_t get_gpu_type_priority(VkPhysicalDeviceType type)
{
	switch (type) {
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return 5;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return 4;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return 3;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return 2;
		default:
			return 1;
	}
}

uint64_t vk_device::get_gpu_available_vram_byte(const VkPhysicalDeviceMemoryProperties& mem)
{
	uint64_t mem_size = 0;
	for (uint32_t i = 0; i < mem.memoryHeapCount; ++i) {
		if (mem.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			mem_size += mem.memoryHeaps[i].size;
		}
	}
	return mem_size;
}

bool vk_device::is_gpu_supported_surface(VkPhysicalDevice gpu, const VkSurfaceKHR surface)
{
	if (!surface)
		return true;

	uint32_t surface_count = 0;
	uint32_t present_count = 0;

	bool has_format = false;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surface_count, nullptr) == VK_SUCCESS) {
		if (surface_count > 0)
			has_format = true;
	}

	bool has_present = false;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_count, nullptr) == VK_SUCCESS) {
		if (present_count > 0)
			has_present = true;
	}

	return (has_format && has_present);
}

bool vk_device::choose_physical_device(VkInstance instance, VkSurfaceKHR surface, vk_physical_device* pp_gpu)
{
	uint32_t gpu_count = 0;
	VK(vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr));

	std::vector<VkPhysicalDevice> gpus(gpu_count);
	VK(vkEnumeratePhysicalDevices(instance, &gpu_count, gpus.data()));

	bool found = false;

	vk_physical_device selected_gpu{};
	uint32_t           priority_level      = 0;
	uint64_t           priority_level_vram = 0;

	for (const VkPhysicalDevice& gpu : gpus) {
		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, nullptr);
		if (queue_count == 0)
			continue;
		std::vector<VkQueueFamilyProperties> queue_props(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props.data());

		queue_indices queue_indices = find_queue_indices(gpu, surface, queue_props);

		if (!queue_indices.graphics)
			continue;
		if (!queue_indices.present && surface)
			continue;
		if (!is_gpu_supported_surface(gpu, surface))
			continue;

		// get gpu properties
		VkPhysicalDeviceProperties       gpu_props{};
		VkPhysicalDeviceMemoryProperties gpu_memory{};
		vkGetPhysicalDeviceProperties(gpu, &gpu_props);
		vkGetPhysicalDeviceMemoryProperties(gpu, &gpu_memory);

		const uint32_t current_level      = get_gpu_type_priority(gpu_props.deviceType);
		const uint64_t current_level_vram = get_gpu_available_vram_byte(gpu_memory);

		bool is_higher = false;
		if (!found)        // first gpu
			is_higher = true;
		else if (current_level > priority_level)        // compute type priority
			is_higher = true;
		else if (current_level == priority_level &&        // compute vram size if type same
		         current_level_vram > priority_level_vram) {
			is_higher = true;
		}

		if (is_higher) {
			found               = true;
			priority_level      = current_level;
			priority_level_vram = current_level_vram;

			selected_gpu.handle        = gpu;
			selected_gpu.queue_indices = queue_indices;
			selected_gpu.device_props  = gpu_props;
			selected_gpu.memory_props  = gpu_memory;
			selected_gpu.flags         = queue_props[queue_indices.graphics.value()].queueFlags;
		}
	}
	if (!found)
		return false;

	*pp_gpu = selected_gpu;

	return true;
}

uint32_t vk_device::graphics_queue_family_index() const
{
	assert(m_gpu.queue_indices.graphics.has_value() && "Graphics queue family not available!");
	return *m_gpu.queue_indices.graphics;
}

uint32_t vk_device::present_queue_family_index() const
{
	assert(m_gpu.queue_indices.present.has_value() && "Present queue family not available!");
	return *m_gpu.queue_indices.present;
}

uint32_t vk_device::compute_queue_family_index() const
{
	if (m_gpu.queue_indices.compute.has_value())
		return *m_gpu.queue_indices.compute;
	return graphics_queue_family_index();
}

uint32_t vk_device::transfer_queue_family_index() const
{
	if (m_gpu.queue_indices.transfer.has_value())
		return *m_gpu.queue_indices.transfer;
	return graphics_queue_family_index();
}

const VkQueue vk_device::graphics_queue() const
{
	return m_graphics_queue;
}

const VkQueue vk_device::present_queue() const
{
	return m_graphics_queue;
}

const VkQueue vk_device::compute_queue() const
{
	return m_compute_queue;
}

const VkQueue vk_device::transfer_queue() const
{
	return m_compute_queue;
}

const VkSurfaceKHR vk_device::surface() const
{
	return m_surface;
}

const vk_physical_device* vk_device::get_gpu() const
{
	return &m_gpu;
}

void vk_device::create_logical_device(VkSurfaceKHR surface)
{
	assert(m_gpu.handle != VK_NULL_HANDLE);

	std::vector<uint32_t> unique_families;
	unique_families.reserve(4);

	auto push_unique = [&](std::optional<uint32_t> idx) {
		if (!idx)
			return;
		if (std::find(unique_families.begin(), unique_families.end(), *idx) == unique_families.end())
			unique_families.push_back(*idx);
	};

	push_unique(m_gpu.queue_indices.graphics);
	if (surface)
		push_unique(m_gpu.queue_indices.present);
	push_unique(m_gpu.queue_indices.compute);
	push_unique(m_gpu.queue_indices.transfer);

	std::vector<VkDeviceQueueCreateInfo> queue_infos;
	queue_infos.reserve(unique_families.size());
	const float qprio = 1.0f;
	for (uint32_t family : unique_families) {
		VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
		queue_info.queueFamilyIndex = family;
		queue_info.queueCount       = 1;
		queue_info.pQueuePriorities = &qprio;
		queue_infos.push_back(queue_info);
	}

	std::vector<const char*> device_exts;
	if (surface) {
		device_exts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	// VkPhysicalDeviceFeatures2 + VkPhysicalDeviceVulkan12Features + VkPhysicalDeviceVulkan13Features
	VkPhysicalDeviceVulkan13Features feat13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
	VkPhysicalDeviceVulkan12Features feat12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
	VkPhysicalDeviceFeatures2        feat2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};

	// feat2 -> feat12 -> feat13
	feat2.pNext  = &feat12;
	feat12.pNext = &feat13;
	feat13.pNext = nullptr;

	vkGetPhysicalDeviceFeatures2(m_gpu.handle, &feat2);

	// Vulkan 1.2
	if (feat12.timelineSemaphore)
		feat12.timelineSemaphore = VK_TRUE;
	if (feat12.bufferDeviceAddress)
		feat12.bufferDeviceAddress = VK_TRUE;
	if (feat12.descriptorIndexing)
		feat12.descriptorIndexing = VK_TRUE;
	if (feat12.shaderSampledImageArrayNonUniformIndexing)
		feat12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	if (feat12.runtimeDescriptorArray)
		feat12.runtimeDescriptorArray = VK_TRUE;
	if (feat12.scalarBlockLayout)
		feat12.scalarBlockLayout = VK_TRUE;
	if (feat12.hostQueryReset)
		feat12.hostQueryReset = VK_TRUE;

	// Vulkan 1.3
	if (feat13.synchronization2)
		feat13.synchronization2 = VK_TRUE;        // vkCmdPipelineBarrier2, etc.
	if (feat13.dynamicRendering)
		feat13.dynamicRendering = VK_TRUE;        // vkCmdBeginRendering
	if (feat13.maintenance4)
		feat13.maintenance4 = VK_TRUE;
	if (feat13.inlineUniformBlock)
		feat13.inlineUniformBlock = VK_TRUE;
	if (feat13.shaderDemoteToHelperInvocation)
		feat13.shaderDemoteToHelperInvocation = VK_TRUE;

	feat2.features.samplerAnisotropy = VK_TRUE;
	feat2.features.sampleRateShading = VK_TRUE;

	VkDeviceCreateInfo device_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	device_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_infos.size());
	device_info.pQueueCreateInfos       = queue_infos.data();
	device_info.enabledExtensionCount   = static_cast<uint32_t>(device_exts.size());
	device_info.ppEnabledExtensionNames = device_exts.empty() ? nullptr : device_exts.data();
	device_info.pNext                   = &feat2;        // 1.3
	device_info.pEnabledFeatures        = nullptr;

	VK(vkCreateDevice(m_gpu.handle, &device_info, nullptr, &m_handle));

	if (m_gpu.queue_indices.graphics) {
		vkGetDeviceQueue(m_handle, *m_gpu.queue_indices.graphics, 0, &m_graphics_queue);
	}
	if (surface && m_gpu.queue_indices.present) {
		vkGetDeviceQueue(m_handle, *m_gpu.queue_indices.present, 0, &m_present_queue);
	}
	else {
		m_present_queue = VK_NULL_HANDLE;
	}
	if (m_gpu.queue_indices.compute) {
		vkGetDeviceQueue(m_handle, *m_gpu.queue_indices.compute, 0, &m_compute_queue);
	}
	else {
		m_compute_queue = VK_NULL_HANDLE;
	}
	if (m_gpu.queue_indices.transfer) {
		vkGetDeviceQueue(m_handle, *m_gpu.queue_indices.transfer, 0, &m_transfer_queue);
	}
	else {
		m_transfer_queue = VK_NULL_HANDLE;
	}
}

static inline const char* device_type_str(VkPhysicalDeviceType type)
{
	switch (type) {
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

}        // namespace juce

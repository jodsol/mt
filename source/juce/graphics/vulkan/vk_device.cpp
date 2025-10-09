#include "vk_device.h"
#include "vk_shader.h"
#include "vk_context.h"
#include <filesystem>
#include <fstream>
#define VMA_IMPLEMENTATION
// #define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// #define VMA_STATIC_VULKAN_FUNCTIONS  0
#include "util/vk_mem_alloc.h"

namespace juce
{
vk_device::vk_device(vk_context* context) :
    m_context(context)
{
	create();
}

vk_device::~vk_device()
{
	release();
}

void vk_device::create_buffer(const buffer_create_info* info, vk_buffer_ext** pp_buffer)
{
}

void vk_device::create()
{
	init_vulkan_memory_allocator();

	m_device = m_context->get_logical_device_handle();

	VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	pool_info.queueFamilyIndex = m_context->graphics_queue_index();
	pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_pool));

	VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	info.commandBufferCount = 1;
	info.commandPool        = m_pool;

	vkAllocateCommandBuffers(m_device, &info, &m_cmd);

	VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK(vkCreateFence(m_device, &fence_info, nullptr, &m_fence));

	// TODO Descriptor Pool ..etc
}

void vk_device::init_vulkan_memory_allocator()
{
	VmaAllocatorCreateInfo alloc_info{};
	alloc_info.device         = m_context->get_logical_device_handle();
	alloc_info.instance       = m_context->instance();
	alloc_info.physicalDevice = m_context->get_physical_device();
	// alloc_info.pVulkanFunctions = &funcs;
	alloc_info.vulkanApiVersion = VK_API_VERSION_1_3;
	VK(vmaCreateAllocator(&alloc_info, &m_allocator));
}

void vk_device::release()
{
	VK(vkDeviceWaitIdle(m_device));
	vmaDestroyAllocator(m_allocator);
	vkFreeCommandBuffers(m_device, m_pool, 1, &m_cmd);
	vk_safe_destroy(m_device, m_pool);
	vk_safe_destroy(m_device, m_fence);
}

void vk_device::create_spv_from_file(const shader_create_info* info, vk_shader** pp_shader)
{
	if(!info || !info->filename) {
		std::cerr << "[vk_device] ❌ Invalid shader_create_info (nullptr)" << std::endl;
		return;
	}

	// 데이터 경로 설정
	std::filesystem::path shader_dir  = std::filesystem::path(DATA_DIR) / "shaders";
	std::filesystem::path shader_path = shader_dir / info->filename;

	if(!std::filesystem::exists(shader_path)) {
		std::cerr << "[vk_device] ❌ Shader file not found: " << shader_path << std::endl;
		return;
	}

	// shader stage 확장자
	std::string stage_ext;
	switch(info->stage) {
		case shader_stage::vertex: stage_ext = "vert"; break;
		case shader_stage::pixel: stage_ext = "frag"; break;
		case shader_stage::geometry: stage_ext = "geom"; break;
		case shader_stage::hull: stage_ext = "tesc"; break;
		default: stage_ext = "spv"; break;
	}

	// 출력 파일 (.spv)
	std::filesystem::path spv_path = shader_path;
	spv_path.replace_extension(stage_ext + ".spv");

#ifdef _WIN32
	const char* validator = "glslangValidator.exe";
#else
	const char* validator = "glslangValidator";
#endif

	// glslangValidator 실행 명령
	std::string cmd = std::string(validator) + " -V \"" + shader_path.string() + "\" -o \"" + spv_path.string() + "\"";

	if(info->entry && std::strlen(info->entry) > 0)
		cmd += " -e " + std::string(info->entry);

	std::cout << "[vk_device] 🔧 Compiling shader: " << cmd << std::endl;

	// 실제 실행
	int result = std::system(cmd.c_str());
	if(result != 0) {
		std::cerr << "[vk_device] ❌ glslangValidator failed (" << result << ")" << std::endl;
		return;
	}

	// SPIR-V 파일 존재 확인
	if(!std::filesystem::exists(spv_path)) {
		std::cerr << "[vk_device] ❌ SPIR-V file not found after compilation: " << spv_path << std::endl;
		return;
	}

	// SPIR-V 파일을 읽어서 vector<uint32_t>로 로드
	std::ifstream file(spv_path, std::ios::binary | std::ios::ate);
	if(!file) {
		std::cerr << "[vk_device] ❌ Failed to open SPIR-V: " << spv_path << std::endl;
		return;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint32_t> spirv(size / sizeof(uint32_t));
	if(!file.read(reinterpret_cast<char*>(spirv.data()), size)) {
		std::cerr << "[vk_device] ❌ Failed to read SPIR-V file data" << std::endl;
		return;
	}

	file.close();

	std::cout << "[vk_device] ✅ Loaded SPIR-V (" << size << " bytes): " << spv_path << std::endl;

	// vk_shader 객체 생성
	*pp_shader = new vk_shader(m_device, spirv, info->stage);
	std::cout << "[vk_device] ✅ Shader module created successfully." << std::endl;
}

}        // namespace juce

#pragma once



#include <vulkan/vulkan.h>

#include <vector>

#include <string>



// Forward declarations
namespace juce {
    class VKContext;
    class Swapchain;
}

namespace juce 
{
class Backend 
{
public:
    Backend(VKContext* context, Swapchain* swapchain);
    ~Backend();
    // Backend 초기화 (RenderPass, Pipeline, CommandBuffer 등 생성)
    bool initialize();
    // 렌더링 루프에서 매 프레임 호출될 함수
    void draw_frame();
    // 창 크기 변경 시 호출될 함수
    void on_window_resized(uint32_t width, uint32_t height);

private:
    // 초기화 헬퍼 함수들
    void create_render_pass();
    void create_graphics_pipeline();
    void create_command_buffers();
    void create_sync_objects();

    // 셰이더 파일을 읽고 모듈을 생성하는 헬퍼 함수
    static std::vector<char> read_file(const std::string& filename);
    VkShaderModule create_shader_module(const std::vector<char>& code);

    // Command Buffer에 렌더링 명령을 기록하는 함수
    void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);

    // 리소스 정리 함수
    void cleanup();
    void cleanup_swapchain_dependents();

    // 창 크기 변경에 따른 리소스 재생성
    void recreate_swapchain_dependents();


    VKContext* m_context;       // 소유하지 않음
    Swapchain* m_swapchain;     // 소유하지 않음
    VkRenderPass m_render_pass;
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkCommandBuffer> m_command_buffers;

    // 동기화 객체
    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;
    uint32_t m_current_frame = 0;
    const int MAX_FRAMES_IN_FLIGHT = 2;

    // 창 크기 변경 여부를 추적하는 플래그
    bool m_framebuffer_resized = false;

};
} // namespace juce 
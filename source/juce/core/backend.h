// backend.h
#pragma once
#include "vk_context.h"
#include "swapchain.h"

namespace juce {

class Backend {
public:
    Backend();
    ~Backend();

    void init();
    void draw_frame();
    void cleanup();

private:
    std::unique_ptr<VkContext> m_context;
    std::unique_ptr<Swapchain> m_swapchain;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> m_commandBuffers;

    void create_render_pass();       // RenderPass
    void create_graphics_pipeline(); // Pipeline
    void create_command_buffers();   // Record draw calls
    void draw_frame();               // Main loop
};

} // namespace juce

// backend.cpp
#include "backend.h"

namespace juce {

Backend::Backend() {}
Backend::~Backend() { cleanup(); }

void Backend::init() {
    m_context = std::make_unique<VkContext>();
    m_context->init();

    m_swapchain = std::make_unique<Swapchain>(m_context.get());
    m_swapchain->init();

    create_render_pass();
    create_graphics_pipeline();
    create_command_buffers();
}

void Backend::draw_frame() {
    // Acquire swapchain image
    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_context->get_device(), m_swapchain->get_swapchain(),
                          UINT64_MAX, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex);

    // Submit recorded command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkCommandBuffer cmdBuffer = m_commandBuffers[imageIndex];
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(m_context->get_graphics_queue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_context->get_graphics_queue());

    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    VkSwapchainKHR swapChains[] = {m_swapchain->get_swapchain()};
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_context->get_present_queue(), &presentInfo);
}

void Swapchain::create_render_pass() {
    VkDevice device = m_context->get_device();

    // 1. Color attachment (스왑체인 이미지)
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_image_format;  // 스왑체인 이미지 포맷
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;      // 렌더링 시작 시 클리어
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;    // 렌더링 끝난 후 저장
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 화면 출력용

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // 2. Depth attachment (깊이 버퍼)
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_depth_format;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // 깊이는 보통 저장 필요 없음
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // 3. Subpass (하나의 그래픽 파이프라인)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // 4. Subpass dependency (레이아웃 전환 동기화)
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // 5. Render pass 생성
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Swapchain::cleanup_render_pass() {
    if (m_render_pass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_context->get_device(), m_render_pass, nullptr);
        m_render_pass = VK_NULL_HANDLE;
    }
}


void Backend::cleanup() {
    vkDestroyPipeline(m_context->get_device(), m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_context->get_device(), m_pipelineLayout, nullptr);
    cleanup_render_pass();

    if (m_swapchain) m_swapchain->cleanup();
    if (m_context) m_context->cleanup();
}

} // namespace juce

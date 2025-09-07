#include "sync.h"
#include <stdexcept>

namespace juce
{

Sync::Sync() {}
Sync::~Sync()
{
    destroy();
}

void Sync::init(VkDevice device, uint32_t maxFramesInFlight)
{
    this->device = device;

    imageAvailableSemaphores.resize(maxFramesInFlight);
    renderFinishedSemaphores.resize(maxFramesInFlight);
    inFlightFences.resize(maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 첫 프레임에서 대기 안 걸리도록

    for (uint32_t i = 0; i < maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    initialized = true;
}

void Sync::waitForFence(uint32_t currentFrame)
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

void Sync::resetFence(uint32_t currentFrame)
{
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
}

VkSemaphore Sync::getImageAvailableSemaphore(uint32_t frameIndex) const
{
    return imageAvailableSemaphores[frameIndex];
}

VkSemaphore Sync::getRenderFinishedSemaphore(uint32_t frameIndex) const
{
    return renderFinishedSemaphores[frameIndex];
}

VkFence Sync::getInFlightFence(uint32_t frameIndex) const
{
    return inFlightFences[frameIndex];
}

void Sync::destroy()
{
    if (!initialized)
        return;

    for (size_t i = 0; i < inFlightFences.size(); i++)
    {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    initialized = false;
}

} // namespace juce

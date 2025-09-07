#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace juce
{

class Sync
{
public:
    Sync();
    ~Sync();

    // 초기화 (device, maxFramesInFlight 필요)
    void init(VkDevice device, uint32_t maxFramesInFlight);

    // 현재 프레임 Fence 기다리기
    void waitForFence(uint32_t currentFrame);

    // Fence reset
    void resetFence(uint32_t currentFrame);

    // Getter
    VkSemaphore getImageAvailableSemaphore(uint32_t frameIndex) const;
    VkSemaphore getRenderFinishedSemaphore(uint32_t frameIndex) const;
    VkFence getInFlightFence(uint32_t frameIndex) const;

    // Cleanup
    void destroy();

private:
    VkDevice device{VK_NULL_HANDLE};

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    bool initialized{false};
};

} // namespace juce

#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>

// test code
struct 
 {
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // 생성자: device와 semaphore/fence 생성
    vk_sync_object(VkDevice device) {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // 처음에는 signaled

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects!");
        }
    }

    // 소멸자: 자동으로 리소스 해제
    ~vk_sync_object() {
        // vkDestroyXXX는 device 필요 → 외부에서 명시적으로 destroy 필요
        // 혹은 RAII를 더 완벽히 구현하려면 device 포인터를 멤버로 저장
    }

    // 편의 함수: fence reset
    void resetFence(VkDevice device) {
        vkResetFences(device, 1, &inFlightFence);
    }
};

#pragma once

#include "vk_config.h"
#include <juce/core/typedef.h>
#include <juce/graphics/context.h>
#include <memory>
#include <vector>

namespace juce
{
class vk_instance;
class vk_surface;
class vk_device;
class vk_swapchain;
class vk_sync_objects;

class vk_context : public graphics_context
{
public:
    vk_context(uint32_t cx, uint32_t cy, platform_handle platform_handle);
    ~vk_context();

    void on_resized(uint32 cx, uint32 cy) override;

    void create_command_objects();
    void destroy_command_objects();

    vk_instance*     m_instance{nullptr};
    vk_surface*      m_surface{nullptr};
    vk_device*       m_device{nullptr};
    vk_swapchain*    m_swapchain{nullptr};
    vk_sync_objects* m_sync{nullptr};

    struct frame_object
    {
        VkCommandBuffer m_cmd{VK_NULL_HANDLE};
        VkCommandPool   m_cmd_pool{VK_NULL_HANDLE};
    };

    frame_object frames[MAX_SYNC_FRAME];
};
} // namespace juce

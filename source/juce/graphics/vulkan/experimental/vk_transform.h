#pragma once

#include <juce/core/typedef.h>
#include "../vk_config.h"

namespace juce::vk_transform
{
VkAttachmentLoadOp  cast(load_operator op);
VkAttachmentStoreOp cast(store_operator op);
}        // namespace juce::vk_transform

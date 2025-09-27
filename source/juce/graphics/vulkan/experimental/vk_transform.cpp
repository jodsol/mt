#include "vk_transform.h"

namespace juce::vk_transform
{
VkAttachmentLoadOp cast(load_operator load_op)
{
	switch(load_op) {
		case load_operator::load: return VK_ATTACHMENT_LOAD_OP_LOAD;
		case load_operator::clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case load_operator::discard: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp cast(store_operator op)
{
	switch(op) {
		case store_operator::store: return VK_ATTACHMENT_STORE_OP_STORE;
		case store_operator::discard: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case store_operator::no_access: return VK_ATTACHMENT_STORE_OP_NONE_KHR;
	}
	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

}        // namespace juce::vk_transform

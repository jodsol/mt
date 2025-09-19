#include "vk_context_ext.h"

namespace juce
{
vk_context_ext::vk_context_ext(uint32_t cx, uint32_t cy, platform_handle handle) :
    vk_context(cx, cy, handle)
{
	log_info("use extension");
	init_command_list();
}

void vk_context_ext::init_command_list()
{
	for(uint32_t i = 0; i < MAX_SYNC_FRAME; ++i) {
		VkCommandBuffer cmd = frames[i].m_cmd;
		m_command_lists[i].init(cmd);
	}
}

}        // namespace juce

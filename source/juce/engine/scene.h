#pragma once

#include <juce/core/typedef.h>
#include <juce/graphics/vulkan/vk_context.h>

namespace juce
{
class scene
{
public:
	scene(vk_context* context) :
	    m_context(context) {
	    };
	virtual ~scene()                 = default;
	virtual void init()              = 0;
	virtual void update_frame(float) = 0;
	virtual void render_frame()      = 0;
	virtual void release()           = 0;

private:
	vk_context* m_context;
};
}        // namespace juce

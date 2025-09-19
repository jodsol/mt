#pragma once

#include <juce/core/typedef.h>

namespace juce
{
class context
{
public:
	context(uint32 cx, uint32 cy, platform_handle platform_handle) :
	    m_cx(cx),
	    m_cy(cy),
	    m_handle(platform_handle) {};

	virtual ~context() {};

	virtual void on_resized(uint32 cx, uint32 cy) = 0;

	virtual void draw_frame(float dt) = 0;

	virtual void begin_frame() = 0;
	virtual void end_frame()   = 0;

	virtual uint32_t current_frame() const   = 0;
	virtual uint32_t swapchain_frame() const = 0;

	platform_handle get_window_handle() const
	{
		return m_handle;
	}
	uint32_t width() const
	{
		return m_cx;
	}
	uint32_t height() const
	{
		return m_cy;
	}

private:
#ifdef _WIN32
	platform_handle m_handle{nullptr};
#endif
	uint32 m_cx;
	uint32 m_cy;
};
}        // namespace juce

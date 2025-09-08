#pragma once

#include <juce/core/typedef.h>

namespace juce
{
class graphics_context
{
public:
	graphics_context(uint32 cx, uint32 cy, void* platform_handle) :
	    m_cx(cx),
	    m_cy(cy),
	    m_handle(static_cast<HWND>(platform_handle)) {};

	virtual ~graphics_context() {};

	virtual void on_resized(uint32 cx, uint32 cy) = 0;

	void* get_window_handle() const
	{
		return m_handle;
	}

private:
#ifdef _WIN32
	HWND m_handle{nullptr};
#endif
	uint32 m_cx;
	uint32 m_cy;
};
}        // namespace juce

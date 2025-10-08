#include "timer.h"

namespace juce
{

frame_timer::frame_timer() :
    m_last(mono_clock::now()), m_start(m_last)
{
}

void frame_timer::begin_frame()
{
	m_start = mono_clock::now();
}

void frame_timer::end_frame()
{
	time_point now = mono_clock::now();
	m_delta        = second_f(now - m_start).count();
	m_elapsed      = second_f(now - m_last).count();

	++m_frame;

	if(m_elapsed >= 1.0f) {
		m_fsp   = (float)m_frame / m_elapsed;
		m_last  = now;
		m_frame = 0;
	}
}

float frame_timer::delta() const
{
	return m_delta;
}

float frame_timer::fps() const
{
	return m_fsp;
}

uint32_t frame_timer::frame() const
{
	return m_frame;
}

}        // namespace juce

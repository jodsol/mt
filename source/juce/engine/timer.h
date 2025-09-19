#pragma once

#include <chrono>

namespace juce
{

typedef std::chrono::high_resolution_clock std_clock;
typedef std::chrono::steady_clock          mono_clock;

typedef mono_clock::time_point        time_point;
typedef std::chrono::duration<float>  second_f;
typedef std::chrono::duration<double> second_d;

class frame_timer
{
public:
	frame_timer();

	void begin_frame();

	void end_frame();

	float    delta() const;
	float    fps() const;
	uint32_t frame() const;

private:
	time_point m_start;
	time_point m_last;
	float      m_delta   = 0.0f;
	float      m_elapsed = 0.0f;
	float      m_fsp     = 0.0f;
	uint32_t   m_frame   = 0;
};
}        // namespace juce

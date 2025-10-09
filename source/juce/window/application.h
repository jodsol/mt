#pragma once

#include <juce/core/typedef.h>

namespace juce
{

class application
{
public:
	application(int args, char* argv[], int cx, int cy);
	~application();

	int execute_scene(scene* p_scene);
	// 창 크기 변경 이벤트를 처리할 함수
	void on_resized(uint32_t cx, uint32_t cy);

	// Getters
	HWND           get_hwnd() const;
	void           set_hwnd(HWND hwnd);
	const context* get_context() const;
	bool           is_runtime_loop();

	LRESULT local_wnd_proc(UINT msg, WPARAM wp, LPARAM lp);

private:
	static LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	HWND     m_hwnd;
	context* m_context;
	bool     m_runtime_loop    = true;
	bool     m_sizing_and_move = false;
	bool     m_minimized       = false;
	uint32_t m_cx;
	uint32_t m_cy;
};

}        // namespace juce
#pragma once

#include <juce/core/typedef.h>

namespace juce
{

class application
{
public:
	application(int args, char* argv[], int cx, int cy);
	~application();

	int  exec(void* scene);
	void update();
	void render();

	// 창 크기 변경 이벤트를 처리할 함수
	void on_window_resized(uint32_t width, uint32_t height);

	// Getters
	HWND get_hwnd() const;
	void set_hwnd(HWND hwnd);

	LRESULT               local_wnd_proc(UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT WINAPI static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	HWND              m_hwnd;
	graphics_context* m_context;
};

}        // namespace juce
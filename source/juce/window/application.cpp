#include "application.h"
#include <cassert>
#include <juce/core/config.h>
#include <juce/core/logger.h>
#include <juce/graphics/vulkan/vk_context.h>
#include <juce/engine/scene.h>
#include <juce/graphics/vulkan/experimental/vk_context_ext.h>
#include <juce/engine/timer.h>

namespace juce
{

application::application(int args, char* argv[], int cx, int cy) :
    m_hwnd(nullptr), m_context(nullptr)
{
	unused(args);
	unused(argv);

	// 1. Register the window class
	WNDCLASSEXA wc{};
	wc.cbSize      = sizeof(WNDCLASSEXA);
	wc.style       = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = application::static_wnd_proc;
	// wc.lpfnWndProc   = troll_wnd;
	wc.hInstance     = GetModuleHandle(nullptr);
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(DKGRAY_BRUSH);
	wc.lpszClassName = "Juce Engine";
	wc.hIcon         = 0;

	if(!::RegisterClassExA(&wc)) {
		assert(0 && "failed to registered class");
		return;
	}

	// 2. Create the window
	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int x             = (screen_width - cx) / 2;
	int y             = (screen_height - cy) / 2;

	m_hwnd = CreateWindowExA(
	    NULL,
	    wc.lpszClassName,
	    wc.lpszClassName,
	    WS_OVERLAPPEDWINDOW,
	    x, y, cx, cy,
	    nullptr,
	    nullptr,
	    wc.hInstance,
	    this        // Pass 'this' to be captured in WM_NCCREATE
	);

	assert(m_hwnd && L"failed to create window");

	// client area
	RECT rc{};
	::GetClientRect(get_hwnd(), &rc);

	uint32 width  = rc.right - rc.left;
	uint32 height = rc.top - rc.bottom;

#if defined(USE_EXPERIMENTAL)
	m_context = new vk_context_ext(width, height, m_hwnd);
#else
	m_context = new vk_context(width, height, get_hwnd());
#endif

	::ShowWindow(m_hwnd, SW_SHOW);
	log_info("%s window created with Vulkan", wc.lpszClassName);
}

application::~application()
{
	safe_delete(m_context);
	::DestroyWindow(m_hwnd);
}

int application::exec(scene* p_scene)
{
	scene* current_scene = p_scene;
	if(current_scene) {
		current_scene->init();
	}

	frame_timer timer;

	MSG msg{};
	while(msg.message != WM_QUIT) {
		while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
				PostQuitMessage(0);
			}
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		timer.begin_frame();

		if(m_context) {
			// m_context->begin_frame();

			if(current_scene) {
				current_scene->update_frame(0.f);
				current_scene->render_frame();
			}
			m_context->draw_frame(timer.delta());
			// m_context->end_frame();
		}

		timer.end_frame();

		if(timer.frame() == 0) {
			log_debug("fps : %2f", timer.fps());
		}
	}
	// remove reource
	if(current_scene) {
		current_scene->release();
	}
	safe_delete(m_context);

	return static_cast<int>(msg.wParam);
}

void application::on_window_resized(uint32_t width, uint32_t height)
{
}

HWND application::get_hwnd() const
{
	return m_hwnd;
}

void application::set_hwnd(HWND hwnd)
{
	m_hwnd = hwnd;
}

LRESULT application::local_wnd_proc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg) {
		case WM_SIZE:
		{
			uint32_t width  = LOWORD(lp);
			uint32_t height = HIWORD(lp);
			on_window_resized(width, height);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		default:
			break;
	}
	return ::DefWindowProc(m_hwnd, msg, wp, lp);
}

const context* application::get_context() const
{
	return m_context;
}

LRESULT WINAPI application::static_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	application* app = reinterpret_cast<application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if(msg == WM_NCCREATE) {
		// On window creation, store the 'this' pointer passed from CreateWindowEx.
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lp);
		app                   = reinterpret_cast<application*>(pCreate->lpCreateParams);
		app->set_hwnd(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) app);
	}

	if(app) {
		return app->local_wnd_proc(msg, wp, lp);
	}

	return ::DefWindowProc(hwnd, msg, wp, lp);
}

}        // namespace juce

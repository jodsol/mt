#include <juce/juce.h>
#include "scene/render_scene.h"

int main(int args, char* argv[])
{
	check_mem_leak();
	juce::application app(args, argv, 1024, 760);

	juce::vk_context* context = (juce::vk_context*) app.get_context();

	juce::render_scene scene_0(context);

	auto code = app.exec(nullptr);

	return code;
}
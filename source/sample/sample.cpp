// #include <juce/window/application.h>

#include <juce/juce.h>

int main(int args, char* argv[])
{
	juce::application app(args, argv, 1024, 760);

	auto code = app.exec(nullptr);

	return code;
}
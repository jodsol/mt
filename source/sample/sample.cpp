#include <juce/core/application.h>

int main(int args, char* argv[])
{
    juce::Application app(args, argv, 1024, 760);

    auto code = app.exec(nullptr);

    return code;
}
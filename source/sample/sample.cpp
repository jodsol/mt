#include <core/application.h>

int main(int args, char* argv[])
{
    jure::Application app(args, argv, 1024, 760);

    auto code = app.exec(nullptr);

    return code;
}
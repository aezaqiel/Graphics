#include "Log.hpp"
#include "Application.hpp"

int main()
{
    Graphics::Log::Init();

    Graphics::Application* app = new Graphics::Application();
    app->Run();
    delete app;

    Graphics::Log::Shutdown();
}

#include "Application.hpp"

int main()
{
    Graphics::Application* app = new Graphics::Application();
    app->Run();
    delete app;
}

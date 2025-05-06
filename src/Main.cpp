// TODO: Temporary
#include "Window.hpp"

int main()
{
    Graphics::Window window(Graphics::Window::Config(1280, 720, "Graphics"));

    while (!window.CloseRequested())
        window.PollEvents();
}

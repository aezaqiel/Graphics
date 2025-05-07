// TODO: Temporary
#include <iostream>
#include "Window.hpp"
#include "Events/Event.hpp"

int main()
{
    Graphics::Window window(Graphics::Window::Config(1280, 720, "Graphics"));
    window.SetEventCallback([](Graphics::Event& event) -> void {
        std::cout << event << std::endl;
    });

    while (!window.CloseRequested())
        window.PollEvents();
}

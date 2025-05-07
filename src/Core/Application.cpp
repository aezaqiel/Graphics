#include "Application.hpp"

#include "Log.hpp"
#include "Window.hpp"
#include "Events/ApplicationEvent.hpp"

namespace Graphics {

    Application::Application()
    {
        m_Window = std::make_unique<Window>(Window::Config(1280, 720, "Graphics"));
        m_Window->SetEventCallback(std::bind(&Application::EventHandler, this, std::placeholders::_1));
    }

    void Application::Run()
    {
        while (m_Running) {
            // TODO: Timer

            m_Window->PollEvents();

            if (!m_Minimized) {
                // Render
            }
        }
    }

    void Application::EventHandler(Event& event)
    {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<WindowCloseEvent>([&](WindowCloseEvent&) -> bool {
            m_Running = false;
            return false;
        });

        dispatcher.Dispatch<WindowResizeEvent>([&](WindowResizeEvent& e) -> bool {
            (void)e;
            // const u32 width = e.GetWidth();
            // const u32 height = e.GetHeight();

            // TODO: renderer resize
            return false;
        });

        dispatcher.Dispatch<WindowMinimizeEvent>([&](WindowMinimizeEvent& e) -> bool {
            m_Minimized = e.IsMinimized();
            return false;
        });

        LOG_TRACE("{}", event.ToString());
    }

}

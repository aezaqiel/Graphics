#include "Application.hpp"

#include "Log.hpp"
#include "Window.hpp"
#include "Events/ApplicationEvent.hpp"

namespace Graphics {

    Application::Application()
    {
        if (s_Instance != nullptr) {
            LOG_ERROR("An instance of application exists");
            return;
        } else { s_Instance = this; }

        m_Window = std::make_shared<Window>(Window::Config(1280, 720, "Graphics"));
        m_Window->SetEventCallback(std::bind(&Application::EventHandler, this, std::placeholders::_1));

        m_Renderer = std::make_unique<Renderer>(m_Window);
    }

    void Application::Run()
    {
        while (m_Running) {
            // TODO: Timer

            m_Window->PollEvents();

            if (!m_Minimized) {
                m_Renderer->Render(0.0f);
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
            m_Renderer->Resize();

            return false;
        });

        dispatcher.Dispatch<WindowMinimizeEvent>([&](WindowMinimizeEvent& e) -> bool {
            m_Minimized = e.IsMinimized();
            return false;
        });

        //LOG_TRACE("{}", event.ToString());
    }

}

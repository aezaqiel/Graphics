#pragma once

#include <memory>

#include "Window.hpp"
#include "Events/Event.hpp"
#include "Renderer/Renderer.hpp"

namespace Graphics {

    class Application
    {
    public:
        Application();
        ~Application() = default;

        void Run();

        inline Window& GetWindow() { return *m_Window; }
        inline static Application& GetInstance() { return *s_Instance; }
    
    private:
        void EventHandler(Event& event);

    private:
        bool m_Running { true };
        bool m_Minimized { false };

        std::shared_ptr<Window> m_Window;
        std::unique_ptr<Renderer> m_Renderer;

    private:
        inline static Application* s_Instance { nullptr };
    };

}

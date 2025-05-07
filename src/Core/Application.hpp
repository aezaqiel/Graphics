#pragma once

#include <memory>

#include "Window.hpp"
#include "Events/Event.hpp"

namespace Graphics {

    class Application
    {
    public:
        Application();
        ~Application() = default;

        void Run();
    
    private:
        void EventHandler(Event& event);

    private:
        bool m_Running { true };
        bool m_Minimized { false };

        std::unique_ptr<Window> m_Window;
    };

}

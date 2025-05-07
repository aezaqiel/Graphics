#include "Window.hpp"

#include <iostream>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"

namespace Graphics {

    Window::Window(const Config& config)
    {
        m_Data.Width = config.Width;
        m_Data.Height = config.Height;

        glfwSetErrorCallback([](i32 code, const char* desc) -> void {
            std::cerr << "GLFW Error " << code << ": " << desc << std::endl;
        });

        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, config.Title.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(m_Window, &m_Data);

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, i32 width, i32 height) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
            data.EventCallback(event);
            data.Width = width;
            data.Height = height;
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowCloseEvent event;
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                } break;
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                } break;
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                } break;
                default:
                    std::cerr << "Unknown key action " << action << std::endl;
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, u32 codepoint) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            KeyTypedEvent event(static_cast<i32>(codepoint));
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, i32 button, i32 action, i32 mods) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                } break;
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                } break;
                default:
                    std::cerr << "Unknown mouse button action " << action << std::endl;
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, f64 x, f64 y) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseScrolledEvent event(static_cast<f32>(x), static_cast<f32>(y));
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, f64 x, f64 y) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            MouseMovedEvent event(static_cast<f32>(x), static_cast<f32>(y));
            data.EventCallback(event);
        });

        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* window, i32 iconified) -> void {
            WindowData& data = *reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));

            WindowMinimizeEvent event(static_cast<bool>(iconified));
            data.EventCallback(event);
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    bool Window::CloseRequested()
    {
        return glfwWindowShouldClose(m_Window);
    }

}

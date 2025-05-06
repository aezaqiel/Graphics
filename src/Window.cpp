#include "Window.hpp"

#include <iostream>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
            data.Width = width;
            data.Height = height;

            // TODO: tie window resize to call renderer resize (event system?)
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

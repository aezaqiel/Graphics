#pragma once

#include <optional>

#include <volk.h>

#include "Types.hpp"
#include "Core/Log.hpp"

namespace Graphics {

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void Render(f32 dt);
        void Resize(u32 width, u32 height);
    
    private:
        void CreateInstance();

        void PickPhysicalDevice();
        void CreateDevice();

    private:

        struct Queue
        {
            std::optional<u32> Index;
            VkQueue Queue;
        };

    private:
        inline static VkInstance s_Instance { VK_NULL_HANDLE };

        VkSurfaceKHR m_Surface;

        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;

        Queue m_GraphicQueue;
        Queue m_PresentQueue;

#ifndef NDEBUG
        VkDebugUtilsMessengerEXT m_Messenger;
#endif
    };

}

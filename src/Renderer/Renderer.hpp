#pragma once

#include <set>

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
        struct QueueFamilyIndex
        {
            i32 Graphics { -1 };
            i32 Present { -1 };

            inline bool Complete() const
            {
                return Graphics >= 0 && Present >= 0;
            }

            inline i32 operator[](usize index) const
            {
                switch (index) {
                    case 0: return Graphics;
                    case 1: return Present;
                    default: {
                        LOG_ERROR("Queue family index {} out of bound", index);
                        return -1;
                    }
                }
            }

            inline usize Count() const
            {
                std::set<i32> uniqueIndices = { Graphics, Present };
                return uniqueIndices.size();
            }
        };
    
    private:
        void CreateInstance();

        QueueFamilyIndex FindQueueFamilies(VkPhysicalDevice device);
        bool PhysicalDeviceSuitable(VkPhysicalDevice device);
        void PickPhysicalDevice();
        void CreateDevice();

    private:
        inline static VkInstance s_Instance { VK_NULL_HANDLE };

        VkSurfaceKHR m_Surface;

        QueueFamilyIndex m_QueueFamilyIndex;
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_Device;

        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;

#ifndef NDEBUG
        VkDebugUtilsMessengerEXT m_Messenger;
#endif
    };

}

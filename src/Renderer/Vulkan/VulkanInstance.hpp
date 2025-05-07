#pragma once

#include "VulkanBase.hpp"

namespace Graphics {

    class VulkanInstance
    {
    public:
        VulkanInstance();
        ~VulkanInstance();

        inline static VkInstance& GetInstance() { return s_Instance; }

    private:
        inline static VkInstance s_Instance { VK_NULL_HANDLE };

#ifndef NDEBUG
        VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif
    };

}

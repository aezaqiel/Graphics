#include "VulkanInstance.hpp"

#include <vector>
#include <sstream>

#include "Core/Application.hpp"

namespace Graphics {

    static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        (void)pUserData;

        std::stringstream ss;

        switch (messageTypes) {
            case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
                ss << "[GENERAL]";
            } break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: {
                ss << "[VALIDATION]";
            } break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: {
                ss << "[PERFORMANCE]";
            } break;
            case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: {
                ss << "[ADDR BINDING]";
            } break;
            default:
                ss << "[UNKNOWN]";
        }

        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
                LOG_DEBUG("{}: {}", ss.str(), pCallbackData->pMessage);
            } break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
                LOG_INFO("{}: {}", ss.str(), pCallbackData->pMessage);
            } break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
                LOG_WARN("{}: {}", ss.str(), pCallbackData->pMessage);
            } break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
                LOG_ERROR("{}: {}", ss.str(), pCallbackData->pMessage);
            } break;
            default:
                LOG_CRITICAL("{}: {}", ss.str(), pCallbackData->pMessage);
        }

        return VK_FALSE;
    }

    VulkanInstance::VulkanInstance()
    {
        VK_CHECK(volkInitialize());

        VkApplicationInfo info;
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pNext = nullptr;
        info.pApplicationName = Application::GetInstance().GetWindow().Title().c_str();
        info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        info.pEngineName = "Graphics";
        info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        info.apiVersion = VK_API_VERSION_1_4;

        std::vector<const char*> extensions;
        std::vector<const char*> layers;

#ifndef NDEBUG
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back("VK_EXT_debug_utils");
#endif

        extensions.push_back("VK_KHR_surface");
#ifdef VK_USE_PLATFORM_WIN32_KHR
        extensions.push_back("VK_KHR_win32_surface");
#endif

        VkInstanceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.pApplicationInfo = &info;
        createInfo.enabledLayerCount = layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

#ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
        debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessengerInfo.pNext = nullptr;
        debugMessengerInfo.flags = 0;
        debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessengerInfo.pfnUserCallback = &DebugMessengerCallback;
        debugMessengerInfo.pUserData = nullptr;

        createInfo.pNext = &debugMessengerInfo;
#endif

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &s_Instance));
        volkLoadInstance(s_Instance);

#ifndef NDEBUG
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(s_Instance, &debugMessengerInfo, nullptr, &m_DebugMessenger));
#endif
    }

    VulkanInstance::~VulkanInstance()
    {
#ifndef NDEBUG
        vkDestroyDebugUtilsMessengerEXT(s_Instance, m_DebugMessenger, nullptr);
#endif
        vkDestroyInstance(s_Instance, nullptr);
    }

}

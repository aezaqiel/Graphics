#include "Renderer.hpp"

#include <sstream>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Core/Application.hpp"

#define VK_CHECK(fn) do { VkResult res_ = fn; if (res_ != VK_SUCCESS) { LOG_ERROR("VK_CHECK Failed: {}", #fn); } } while (false)

namespace Graphics {

    static VkBool32 DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData
    );

    Renderer::Renderer()
    {
        VK_CHECK(volkInitialize());
        CreateInstance();

        VK_CHECK(glfwCreateWindowSurface(s_Instance, static_cast<GLFWwindow*>(Application::GetInstance().GetWindow().GetNative()), nullptr, &m_Surface));
        PickPhysicalDevice();
        CreateDevice();
    }

    Renderer::~Renderer()
    {
        vkDestroyDevice(m_Device, nullptr);
        vkDestroySurfaceKHR(s_Instance, m_Surface, nullptr);
#ifndef NDEBUG
        vkDestroyDebugUtilsMessengerEXT(s_Instance, m_Messenger, nullptr);
#endif
        vkDestroyInstance(s_Instance, nullptr);
    }

    void Renderer::Render(f32 dt)
    {
        (void)dt;
    }

    void Renderer::Resize(u32 width, u32 height)
    {
        (void)width;
        (void)height;
    }

    void Renderer::CreateInstance()
    {
        VkApplicationInfo info;
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pNext = nullptr;
        info.pApplicationName = Application::GetInstance().GetWindow().Title().c_str();
        info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        info.pEngineName = "Graphics";
        info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        info.apiVersion = VK_API_VERSION_1_4;

        std::vector<const char*> layers;
        std::vector<const char*> extensions;

        extensions.push_back("VK_KHR_surface");
#ifdef VK_USE_PLATFORM_WIN32_KHR
        extensions.push_back("VK_KHR_win32_surface");
#endif

#ifndef NDEBUG
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back("VK_EXT_debug_utils");
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
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
        messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.pNext = nullptr;
        messengerInfo.flags = 0;
        // messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.pfnUserCallback = &DebugMessengerCallback;
        messengerInfo.pUserData = nullptr;

        createInfo.pNext = &messengerInfo;
#endif

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &s_Instance));
        volkLoadInstance(s_Instance);

#ifndef NDEBUG
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(s_Instance, &messengerInfo, nullptr, &m_Messenger));
#endif
    }

    Renderer::QueueFamilyIndex Renderer::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndex indices;

        u32 count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

        i32 i = 0;
        for (auto& family : queueFamilies)
        {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.Graphics = i;

            VkBool32 supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &supported);

            if (supported == VK_TRUE)
                indices.Present = i;

            i++;
        }

        return indices;
    }

    bool Renderer::PhysicalDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            return false;
        
        if (!FindQueueFamilies(device).Complete())
            return false;

        return true;
    }

    void Renderer::PickPhysicalDevice()
    {
        u32 count = 0;
        vkEnumeratePhysicalDevices(s_Instance, &count, nullptr);
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(s_Instance, &count, devices.data());

        m_PhysicalDevice = VK_NULL_HANDLE;

        for (auto& device : devices) {
            if (PhysicalDeviceSuitable(device)) {
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE) {
            LOG_ERROR("No suitable physical device found");
        } else {
            m_QueueFamilyIndex = FindQueueFamilies(m_PhysicalDevice);
        }
    }

    void Renderer::CreateDevice()
    {
        f32 priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(m_QueueFamilyIndex.Count());
        for (u32 i = 0; i < queueCreateInfos.size(); i++) {
            if (m_QueueFamilyIndex[i] < 0)
                i++;

            queueCreateInfos[i] = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = static_cast<u32>(m_QueueFamilyIndex[i]),
                .queueCount = 1,
                .pQueuePriorities = &priority
            };
        }

        VkPhysicalDeviceFeatures features;
        memset(&features, 0, sizeof(VkPhysicalDeviceFeatures));

        std::vector<const char*> extensions;

        VkDeviceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.pEnabledFeatures = &features;

        VK_CHECK(vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device));
        volkLoadDevice(m_Device);

        vkGetDeviceQueue(m_Device, m_QueueFamilyIndex.Graphics, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_QueueFamilyIndex.Present, 0, &m_PresentQueue);
    }

    VkBool32 DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData
    )
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
                ss << "[ADDR BIND]";
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

}

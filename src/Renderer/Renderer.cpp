#include "Renderer.hpp"

#include <sstream>
#include <fstream>
#include <set>
#include <algorithm>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define VK_CHECK(fn) do { VkResult res_ = fn; if (res_ != VK_SUCCESS) { LOG_ERROR("VK_CHECK Failed: {}", #fn); } } while (false)

namespace Graphics {

    static VkBool32 DebugMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
        void*                                            pUserData
    );

    Renderer::Renderer(const std::shared_ptr<Window> window)
        : m_Window(window)
    {
        VK_CHECK(volkInitialize());
        CreateInstance();

        VK_CHECK(glfwCreateWindowSurface(s_Instance, static_cast<GLFWwindow*>(m_Window->GetNative()), nullptr, &m_Surface));

        PickPhysicalDevice();
        CreateDevice();

        QuerySwapchainCapabilities();
        CreateSwapchain();

        CreateRenderPass();

        CreateFramebuffers();

        CreateGraphicsPipeline();

        CreateCommandPool();
        AllocateCommandBuffers();

        CreateSyncObjects();
    }

    Renderer::~Renderer()
    {
        vkDeviceWaitIdle(m_Device);

        for (usize i = 0; i < s_FrameInFlight; ++i) {
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
			vkDestroySemaphore(m_Device, m_RenderFinishedSemphores[i], nullptr);
			vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        }

        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

        vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_GraphicsPipelineLayout, nullptr);

        for (auto& framebuffer : m_Swapchain.Framebuffers)
            vkDestroyFramebuffer(m_Device, framebuffer, nullptr);

        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

        for (auto& imageView : m_Swapchain.ImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);

        vkDestroySwapchainKHR(m_Device, m_Swapchain.Swapchain, nullptr);

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

		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_FrameIndex], VK_TRUE, std::numeric_limits<u64>::max());

        u32 imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain.Swapchain, std::numeric_limits<u64>::max(), m_ImageAvailableSemaphores[m_FrameIndex], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Resize();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            LOG_ERROR("Failed to acquire swapchain image");
            return;
        }

        vkResetFences(m_Device, 1, &m_InFlightFences[m_FrameIndex]);

        vkResetCommandBuffer(m_CommandBuffers[m_FrameIndex], 0);
        RecordCommandBuffer(m_CommandBuffers[m_FrameIndex], imageIndex);

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_ImageAvailableSemaphores[m_FrameIndex];
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[m_FrameIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderFinishedSemphores[m_FrameIndex];

        VK_CHECK(vkQueueSubmit(m_GraphicQueue.Queue, 1, &submitInfo, m_InFlightFences[m_FrameIndex]));

        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_RenderFinishedSemphores[m_FrameIndex];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain.Swapchain;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = NULL;

        result = vkQueuePresentKHR(m_PresentQueue.Queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            Resize();
        } else if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to present swapchain images");
            return;
        }

        m_FrameIndex = (m_FrameIndex + 1) % s_FrameInFlight;
    }

    void Renderer::Resize()
    {
        vkDeviceWaitIdle(m_Device);

        for (auto& framebuffer : m_Swapchain.Framebuffers)
            vkDestroyFramebuffer(m_Device, framebuffer, nullptr);

        for (auto& imageView : m_Swapchain.ImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);

        vkDestroySwapchainKHR(m_Device, m_Swapchain.Swapchain, nullptr);

        CreateSwapchain();
        CreateFramebuffers();
    }

    void Renderer::CreateInstance()
    {
        VkApplicationInfo info;
        info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        info.pNext = nullptr;
        info.pApplicationName = m_Window->Title().c_str();
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

    void Renderer::PickPhysicalDevice()
    {
        u32 count = 0;
        vkEnumeratePhysicalDevices(s_Instance, &count, nullptr);
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(s_Instance, &count, devices.data());

        if (m_PhysicalDevice != VK_NULL_HANDLE) {
            LOG_WARN("Physical device already set");
            return;
        }

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                continue;

            m_GraphicQueue.Index.reset();
            m_PresentQueue.Index.reset();

            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            u32 idx = 0;
            for (const auto& family : queueFamilies) {
                if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    m_GraphicQueue.Index = idx;

                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, idx, m_Surface, &presentSupport);

                if (presentSupport == VK_TRUE)
                    m_PresentQueue.Index = idx;

                idx++;
            }

            if (!m_GraphicQueue.Index.has_value()) continue;
            if (!m_PresentQueue.Index.has_value()) continue;

            u32 formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
            if (formatCount <= 0) continue;

            u32 presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
            if (presentModeCount <= 0) continue;

            m_PhysicalDevice = device;
            LOG_INFO("Physical device: {}", props.deviceName);

            break;
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
            LOG_ERROR("No suitable physical device found");
    }

    void Renderer::CreateDevice()
    {
        f32 priority = 1.0f;

        std::set<u32> indices = { m_GraphicQueue.Index.value(), m_PresentQueue.Index.value() };
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (u32 index : indices) {
            queueCreateInfos.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = index,
				.queueCount = 1,
				.pQueuePriorities = &priority
            });
        }

        VkPhysicalDeviceFeatures features {};

        std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

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

        vkGetDeviceQueue(m_Device, m_GraphicQueue.Index.value(), 0, &m_GraphicQueue.Queue);
        vkGetDeviceQueue(m_Device, m_PresentQueue.Index.value(), 0, &m_PresentQueue.Queue);
    }

    void Renderer::QuerySwapchainCapabilities()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_Swapchain.Capabilities);

		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &formatCount, surfaceFormats.data());

        m_Swapchain.SurfaceFormat = surfaceFormats[0];
        for (const auto& format : surfaceFormats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                m_Swapchain.SurfaceFormat = format;
        }

		u32 presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, presentModes.data());

        m_Swapchain.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& mode : presentModes) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                m_Swapchain.PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    VkExtent2D Renderer::GetSwapchainExtent()
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &m_Swapchain.Capabilities);

        if (m_Swapchain.Capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
            return m_Swapchain.Capabilities.currentExtent;
        } else {
            VkExtent2D extent = {
                std::clamp(static_cast<u32>(m_Window->Width()), m_Swapchain.Capabilities.minImageExtent.width, m_Swapchain.Capabilities.maxImageExtent.width),
                std::clamp(static_cast<u32>(m_Window->Height()), m_Swapchain.Capabilities.minImageExtent.height, m_Swapchain.Capabilities.maxImageExtent.height)
            };

            return extent;
        }
    }

    void Renderer::CreateSwapchain()
    {
        m_Swapchain.Extent = GetSwapchainExtent();

        m_Swapchain.ImageCount = m_Swapchain.Capabilities.minImageCount + 1;
        if (m_Swapchain.Capabilities.maxImageCount > 0 && m_Swapchain.ImageCount > m_Swapchain.Capabilities.maxImageCount)
            m_Swapchain.ImageCount = m_Swapchain.Capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.surface = m_Surface;
        createInfo.minImageCount = m_Swapchain.ImageCount;
        createInfo.imageFormat = m_Swapchain.SurfaceFormat.format;
        createInfo.imageColorSpace = m_Swapchain.SurfaceFormat.colorSpace;
        createInfo.imageExtent = m_Swapchain.Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (m_GraphicQueue.Index.value() != m_PresentQueue.Index.value()) {
            std::array<u32, 2> indices = {
                m_GraphicQueue.Index.value(),
                m_PresentQueue.Index.value()
            };

            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = indices.size();
            createInfo.pQueueFamilyIndices = indices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = m_Swapchain.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_Swapchain.PresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain.Swapchain));

        vkGetSwapchainImagesKHR(m_Device, m_Swapchain.Swapchain, &m_Swapchain.ImageCount, nullptr);
        m_Swapchain.Images.resize(m_Swapchain.ImageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain.Swapchain, &m_Swapchain.ImageCount, m_Swapchain.Images.data());

        m_Swapchain.ImageViews.resize(m_Swapchain.ImageCount);
        for (usize i = 0; i < m_Swapchain.ImageCount; ++i) {
            VkImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.image = m_Swapchain.Images[i];
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = m_Swapchain.SurfaceFormat.format;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(m_Device, &imageViewCreateInfo, nullptr, &m_Swapchain.ImageViews[i]));
        }
    }

    void Renderer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment;
		colorAttachment.flags = 0;
		colorAttachment.format = m_Swapchain.SurfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass;
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		VK_CHECK(vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_RenderPass));
	}

    void Renderer::CreateFramebuffers()
    {
        m_Swapchain.Framebuffers.resize(m_Swapchain.ImageCount);

        for (usize i = 0; i < m_Swapchain.ImageCount; ++i) {
            VkImageView attachment[] = { m_Swapchain.ImageViews[i] };

            VkFramebufferCreateInfo createInfo;
			createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.renderPass = m_RenderPass;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = attachment;
			createInfo.width = m_Swapchain.Extent.width;
			createInfo.height = m_Swapchain.Extent.height;
			createInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_Device, &createInfo, nullptr, &m_Swapchain.Framebuffers[i]));
        }
    }

    VkShaderModule Renderer::LoadShader(const std::string& filepath)
    {
        LOG_DEBUG("Loading shader {}", filepath);

        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file {}", filepath);
            return VK_NULL_HANDLE;
        }

        usize fileSize = static_cast<usize>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<const u32*>(buffer.data());

        VkShaderModule shaderModule;
        VK_CHECK(vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

	void Renderer::CreateGraphicsPipeline()
	{
		VkShaderModule vertShader = LoadShader("shaders/Triangle.vert.spv");
		VkShaderModule fragShader = LoadShader("shaders/Triangle.frag.spv");

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
			VkPipelineShaderStageCreateInfo {
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertShader,
				.pName = "main",
				.pSpecializationInfo = nullptr
			},
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragShader,
				.pName = "main",
				.pSpecializationInfo = nullptr
			}
		};

		std::array<VkDynamicState, 2> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.flags = 0;
		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputState.pNext = nullptr;
		vertexInputState.flags = 0;
		vertexInputState.vertexBindingDescriptionCount = 0;
		vertexInputState.pVertexBindingDescriptions = nullptr;
		vertexInputState.vertexAttributeDescriptionCount = 0;
		vertexInputState.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.pNext = nullptr;
		inputAssemblyState.flags = 0;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = m_Swapchain.Extent.width;
		viewport.height = m_Swapchain.Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = VkOffset2D{ 0, 0 };
		scissor.extent = m_Swapchain.Extent;

		VkPipelineViewportStateCreateInfo viewportState;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationState;
		rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationState.pNext = nullptr;
		rasterizationState.flags = 0;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.depthBiasConstantFactor = 0.0f;
		rasterizationState.depthBiasClamp = 0.0f;
		rasterizationState.depthBiasSlopeFactor = 0.0f;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.pNext = nullptr;
		multisampleState.flags = 0;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.minSampleShading = 1.0f;
		multisampleState.pSampleMask = nullptr;
		multisampleState.alphaToCoverageEnable = VK_FALSE;
		multisampleState.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.pNext = nullptr;
		colorBlendState.flags = 0;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;
		colorBlendState.blendConstants[0] = 0.0f;
		colorBlendState.blendConstants[1] = 0.0f;
		colorBlendState.blendConstants[2] = 0.0f;
		colorBlendState.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pNext = nullptr;
		layoutCreateInfo.flags = 0;
		layoutCreateInfo.setLayoutCount = 0;
		layoutCreateInfo.pSetLayouts = nullptr;
		layoutCreateInfo.pushConstantRangeCount = 0;
		layoutCreateInfo.pPushConstantRanges = nullptr;

		VK_CHECK(vkCreatePipelineLayout(m_Device, &layoutCreateInfo, nullptr, &m_GraphicsPipelineLayout));

		VkGraphicsPipelineCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stageCount = shaderStages.size();
		createInfo.pStages = shaderStages.data();
		createInfo.pVertexInputState = &vertexInputState;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pTessellationState = nullptr;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDepthStencilState = nullptr;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.layout = m_GraphicsPipelineLayout;
        createInfo.renderPass = m_RenderPass;
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;

		VK_CHECK(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_GraphicsPipeline));

		vkDestroyShaderModule(m_Device, vertShader, nullptr);
		vkDestroyShaderModule(m_Device, fragShader, nullptr);
	}

    void Renderer::CreateCommandPool()
    {
		VkCommandPoolCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		createInfo.queueFamilyIndex = m_GraphicQueue.Index.value();

		VK_CHECK(vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool));
    }

    void Renderer::AllocateCommandBuffers()
    {
		VkCommandBufferAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (usize i = 0; i < s_FrameInFlight; ++i)
			VK_CHECK(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_CommandBuffers[i]));
    }

    void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex)
    {
		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

		VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};

		VkRenderPassBeginInfo renderPassInfo;
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = m_Swapchain.Framebuffers[imageIndex];
		renderPassInfo.renderArea.offset = VkOffset2D{ 0, 0 };
		renderPassInfo.renderArea.extent = m_Swapchain.Extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = m_Swapchain.Extent.width;
		viewport.height = m_Swapchain.Extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor;
		scissor.offset = VkOffset2D{ 0, 0 };
		scissor.extent = m_Swapchain.Extent;

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffer);
		VK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void Renderer::CreateSyncObjects()
    {
		VkSemaphoreCreateInfo semaphoreInfo;
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		VkFenceCreateInfo fenceInfo;
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (usize i = 0; i < s_FrameInFlight; ++i) {
			VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemphores[i]));
			VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]));
		}
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

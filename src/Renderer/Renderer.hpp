#pragma once

#include <optional>
#include <vector>
#include <string>
#include <array>

#include <volk.h>
#include <glm/glm.hpp>

#include "Types.hpp"
#include "Core/Window.hpp"

namespace Graphics {

    class Renderer
    {
    public:
        Renderer(const std::shared_ptr<Window>& window);
        ~Renderer();

        void Render(f32 dt);
        void Resize();
    
    private:
        void CreateInstance();

        void PickPhysicalDevice();
        void CreateDevice();

        void QuerySwapchainCapabilities();
        VkExtent2D GetSwapchainExtent();
        void CreateSwapchain();

        void CreateRenderPass();

        void CreateFramebuffers();

        VkShaderModule LoadShader(const std::string& filepath);
        void CreateGraphicsPipeline();

        void CreateCommandPool();

        u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
        void CreateVertexBuffer();

        void AllocateCommandBuffers();
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);

        void CreateSyncObjects();

    private:

        struct Queue
        {
            std::optional<u32> Index;
            VkQueue Queue;
        };

        struct Swapchain
        {
            VkSwapchainKHR Swapchain;

            VkSurfaceCapabilitiesKHR Capabilities;
            VkSurfaceFormatKHR SurfaceFormat;
            VkPresentModeKHR PresentMode;

            VkExtent2D Extent;

            u32 ImageCount;
            std::vector<VkImage> Images;
            std::vector<VkImageView> ImageViews;
            std::vector<VkFramebuffer> Framebuffers;
        };

        struct Vertex
        {
            glm::vec2 Pos;
            glm::vec3 Color;

            inline static VkVertexInputBindingDescription BindingDescription()
            {
                VkVertexInputBindingDescription description;
                description.binding = 0;
                description.stride = sizeof(Vertex);
                description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return description;
            };

            inline static std::array<VkVertexInputAttributeDescription, 2> AttributeDescription()
            {
                std::array<VkVertexInputAttributeDescription, 2> descriptions;

                descriptions[0].location = 0;
                descriptions[0].binding = 0;
                descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
                descriptions[0].offset = offsetof(Vertex, Pos);

                descriptions[1].location = 1;
                descriptions[1].binding = 0;
                descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                descriptions[1].offset = offsetof(Vertex, Color);

                return descriptions;
            }
        };

    private:
        std::shared_ptr<Window> m_Window;

        inline static VkInstance s_Instance { VK_NULL_HANDLE };
        inline static constexpr usize s_FrameInFlight { 2 };

        usize m_FrameIndex { 0 };

        VkSurfaceKHR m_Surface;

        VkPhysicalDevice m_PhysicalDevice { VK_NULL_HANDLE };
        VkDevice m_Device;

        Queue m_GraphicQueue;
        Queue m_PresentQueue;

        Swapchain m_Swapchain;

        VkRenderPass m_RenderPass;

        VkPipelineLayout m_GraphicsPipelineLayout;
        VkPipeline m_GraphicsPipeline;

        std::vector<Vertex> m_Vertices;
        VkBuffer m_VertexBuffer;
        VkDeviceMemory m_VertexBufferMemory;

        VkCommandPool m_CommandPool;
        std::array<VkCommandBuffer, s_FrameInFlight> m_CommandBuffers;

        std::array<VkSemaphore, s_FrameInFlight> m_ImageAvailableSemaphores;
        std::array<VkSemaphore, s_FrameInFlight> m_RenderFinishedSemphores;
        std::array<VkFence, s_FrameInFlight> m_InFlightFences;

#ifndef NDEBUG
        VkDebugUtilsMessengerEXT m_Messenger;
#endif
    };

}

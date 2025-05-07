#pragma once

#include "Types.hpp"

#include "Vulkan/VulkanInstance.hpp"

namespace Graphics {

    class Renderer
    {
    public:
        Renderer();
        ~Renderer() = default;

        void Render(f32 dt);
        void Resize(u32 width, u32 height);

    private:
        VulkanInstance m_Instance;
    };

}

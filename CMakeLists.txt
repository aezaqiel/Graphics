cmake_minimum_required(VERSION 3.30)

project(Graphics)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif()

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/volk)

add_executable(${PROJECT_NAME}
    src/Main.cpp
    src/Types.hpp

    src/Core/Log.hpp
    src/Core/Log.cpp
    src/Core/Application.hpp
    src/Core/Application.cpp
    src/Core/Window.hpp
    src/Core/Window.cpp
    src/Core/KeyCodes.hpp
    src/Core/Events/Event.hpp
    src/Core/Events/ApplicationEvent.hpp
    src/Core/Events/KeyEvent.hpp
    src/Core/Events/MouseEvent.hpp

    src/Renderer/Renderer.hpp
    src/Renderer/Renderer.cpp
    src/Renderer/Vulkan/VulkanBase.hpp
    src/Renderer/Vulkan/VulkanInstance.hpp
    src/Renderer/Vulkan/VulkanInstance.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
    spdlog
    volk
)

target_compile_definitions(${PROJECT_NAME}
PRIVATE
    GLFW_INCLUDE_NONE
)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
    )
endif()

target_compile_options(${PROJECT_NAME}
PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
)

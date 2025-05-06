cmake_minimum_required(VERSION 3.30)

project(Graphics)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(vendor/glfw)

add_executable(${PROJECT_NAME}
    src/Main.cpp
    src/Core.hpp
    src/Window.hpp
    src/Window.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
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

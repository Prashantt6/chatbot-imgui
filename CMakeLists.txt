cmake_minimum_required(VERSION 3.10)
project(CHATBOT_IMGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_WIN32_EXECUTABLE OFF)  # Ensure it expects main(), not WinMain()

# Disable building extra GLFW components
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Add GLFW subdirectory
add_subdirectory(libs/glfw)

# Add GLAD
add_library(glad STATIC libs/glad/src/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

# Add ImGui source files
set(IMGUI_SRC
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

# Use imgui_lib instead of imgui to avoid conflicts
add_library(imgui_lib STATIC ${IMGUI_SRC})
target_include_directories(imgui_lib PUBLIC
    libs/imgui
    libs/imgui/backends
    libs/glfw/include
)

# Main executable
add_executable(CHATBOT_IMGUI src/main.cpp src/response.cpp)

# Include directories for the main target
target_include_directories(CHATBOT_IMGUI PRIVATE
    libs/glad/include
    libs/imgui
    libs/imgui/backends
    libs/glfw/include
)

# Link all libraries to the main executable
target_link_libraries(CHATBOT_IMGUI PRIVATE glfw glad imgui_lib)

# Link against OpenGL on Windows
if (WIN32)
    target_link_libraries(CHATBOT_IMGUI PRIVATE opengl32)
endif()

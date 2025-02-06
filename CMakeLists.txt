cmake_minimum_required(VERSION 3.5)

project(FLUID)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# dependencies
find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# targets
set(FLUID_SOURCES "src/main.cpp" "src/graphics.cpp" "src/fluid.cpp")

add_executable(test ${FLUID_SOURCES})

# links
target_link_libraries(test Vulkan::Vulkan glfw)

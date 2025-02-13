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

# compile shaders
set(SHADER_SOURCE_FILES "shaders/shader.frag" "shaders/shader.vert")

set(SHADER_OUTPUT_FILES)
add_custom_target(shaders)

foreach(SHADER_SOURCE_FILE ${SHADER_SOURCE_FILES})
    get_filename_component(SHADER_NAME ${SHADER_SOURCE_FILE} NAME)

    set(SHADER_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")

    add_custom_command(
        TARGET shaders
        COMMAND Vulkan::glslc -o ${SHADER_OUTPUT_FILE} ${SHADER_SOURCE_FILE}
        DEPENDS ${SHADER_OUTPUT_FILE}
        COMMENT "Compiling shader ${SHADER_SOURCE_FILE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    list(APPEND SHADER_OUTPUT_FILES ${SHADER_OUTPUT_FILE})
endforeach()

# compile project
set(PROJECT_SOURCE_FILES "src/main.cpp" "src/graphics.cpp" "src/fluid.cpp")

add_executable(main ${PROJECT_SOURCE_FILES})
add_dependencies(main shaders)
target_link_libraries(main Vulkan::Vulkan glfw)
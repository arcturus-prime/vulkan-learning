cmake_minimum_required(VERSION 3.5)

project(FLUID)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# dependencies
find_package(Vulkan REQUIRED)

function(COMPILE_SHADERS TARGET_NAME)
    set(SHADER_FILES ${ARGN})

    set(COMPILE_COMMANDS)
    set(COMPILED_SHADERS)
    foreach(SHADER IN LISTS SHADER_FILES)
        cmake_path(GET SHADER FILENAME SHADER_NAME)

        set(OUTPUT_SHADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")

        list(APPEND COMPILE_COMMANDS Vulkan::glslc)
        list(APPEND COMPILE_COMMANDS ${SHADER})
        list(APPEND COMPILE_COMMANDS "-o ${OUTPUT_SHADER_FILE}")
        list(APPEND COMPILED_SHADER OUTPUT_SHADER_FILE)

    endforeach()

    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${COMPILE_COMMANDS}
        SOURCES ${COMPILED_SHADERS}
    )

endfunction(COMPILE_SHADERS)


include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# targets
add_executable(test "src/main.cpp" "src/graphics.cpp" "src/fluid.cpp")
compile_shaders(shaders "shader/shader.frag")

# links
target_link_libraries(test Vulkan::Vulkan glfw)

#include <iostream>
#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct QueueFamiliesIndices {
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    
    QueueFamiliesIndices() = default;
    QueueFamiliesIndices(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool IsComplete();
};

struct GraphicsContext {
    QueueFamiliesIndices queueIndices;


    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    GLFWwindow* window = nullptr;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    void Init();
    bool WantsToTerminate();
    void Step();
    
    ~GraphicsContext();
private:
    void SetWindow();
    void SetQueues();
    void SetSurface();
    void SetInstance();
    void SetPhysicalDevice();
    void SetLogicalDevice();
};
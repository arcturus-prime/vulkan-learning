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

struct SwapChainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;
};

class GraphicsContext {
    std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    QueueFamiliesIndices queueIndices;
    SwapChainDetails swapDetails;

    VkInstance instance = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    GLFWwindow* window = nullptr;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    void SetWindow();
    void SetQueues();
    void SetSurface();
    void SetSwapchain();
    void SetInstance();
    void SetPhysicalDevice();
    void SetLogicalDevice();

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

public:
    bool WantsToTerminate();
    void Step();

    GraphicsContext();    
    ~GraphicsContext();
};
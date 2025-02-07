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
    const std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkInstance instance = VK_NULL_HANDLE;

    QueueFamiliesIndices queueIndices;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    SwapChainDetails swapDetails;
    std::vector<VkImage> swapImages;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR swapChainFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapImageViews;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    GLFWwindow* window = nullptr;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    void SetWindow();
    void SetQueues();
    void SetSurface();
    void SetSwapchain();
    void SetInstance();
    void SetPhysicalDevice();
    void SetLogicalDevice();
    void SetImageViews();
    void SetGraphicPipeline();

    bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

public:
    bool WantsToTerminate();
    void Step();

    GraphicsContext();    
    ~GraphicsContext();
};
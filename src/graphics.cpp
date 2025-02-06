#include <set>
#include <algorithm>
#include <iostream>

#include "graphics.hpp"

QueueFamiliesIndices::QueueFamiliesIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    for (uint32_t i = 0; i < count; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) this->graphics = i;
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) this->present = i;
    }
}

bool QueueFamiliesIndices::IsComplete() {
    return this->graphics.has_value() && this->present.has_value();
}

void GraphicsContext::SetInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;  

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if (glfwExtensions == nullptr) {
        throw std::runtime_error("Machine does not support Vulkan according to GLFW!");
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

bool GraphicsContext::IsPhysicalDeviceSuitable(const VkPhysicalDevice device) {

    bool isSuitable = false;

    //check features and properteis

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    isSuitable = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader;

    //check extensions

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> required(this->extensions.begin(), this->extensions.end());

    for (const auto& extension : availableExtensions) {
        required.erase(extension.extensionName);
    }

    isSuitable = isSuitable && required.empty();

    //check swapchain

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->surface, &this->swapDetails.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        this->swapDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, this->swapDetails.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        this->swapDetails.modes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, this->swapDetails.modes.data());
    }

    isSuitable &= !this->swapDetails.modes.empty() && !this->swapDetails.formats.empty();

    return isSuitable;
}

void GraphicsContext::SetSwapchain() {
    //select format
    VkSurfaceFormatKHR selectedFormat = this->swapDetails.formats[0];

    for (const auto& format : this->swapDetails.formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) selectedFormat = format;
    }

    //select presentation mode
    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    
    for (const auto& mode : this->swapDetails.modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) selectedMode = mode;
    }

    //select buffer extents
    VkExtent2D selectedExtent;
    VkSurfaceCapabilitiesKHR& capabilities = this->swapDetails.capabilities;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        selectedExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(this->window, &width, &height);

        selectedExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        selectedExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    // create swapchain
    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = selectedFormat.format;
    createInfo.imageColorSpace = selectedFormat.colorSpace;
    createInfo.imageExtent = selectedExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t indicies[] = { this->queueIndices.graphics.value(), this->queueIndices.present.value() };

    if (indicies[0] != indicies[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indicies;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = selectedMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }
}

void GraphicsContext::SetPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(this->instance, &count, nullptr);

    if (count == 0) {
        throw std::runtime_error("Failed to find any physical devices!");
    }

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(this->instance, &count, devices.data());

    for (const auto device : devices) {
        this->queueIndices = QueueFamiliesIndices(device, this->surface);

        if (IsPhysicalDeviceSuitable(device) && this->queueIndices.IsComplete()) this->physicalDevice = device; 
    }

    if (this->physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Couldn't select a suitable physical device!");
    }
}

void GraphicsContext::SetLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueFamilies = {this->queueIndices.graphics.value(), this->queueIndices.present.value()};

    float queuePriority = 1.0f;
    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(this->extensions.size());
    createInfo.ppEnabledExtensionNames = this->extensions.data();
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }
}

void GraphicsContext::SetWindow() {
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Error initalizing GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->window = glfwCreateWindow(800, 600, "Fluid", nullptr, nullptr);
}

void GraphicsContext::SetQueues() {
    vkGetDeviceQueue(this->logicalDevice, this->queueIndices.graphics.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->logicalDevice, this->queueIndices.present.value(), 0, &this->presentQueue);
}

void GraphicsContext::SetSurface() {
    if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

bool GraphicsContext::WantsToTerminate()
{
    return glfwWindowShouldClose(this->window);
}

void GraphicsContext::Step()
{
    glfwPollEvents();
}


GraphicsContext::GraphicsContext() {
    SetWindow();
    SetInstance();
    SetSurface();

    SetPhysicalDevice();
    SetLogicalDevice();

    SetSwapchain();
    SetQueues();
}

GraphicsContext::~GraphicsContext() {
    vkDestroySwapchainKHR(this->logicalDevice, this->swapchain, nullptr);
    vkDestroyDevice(this->logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);

    glfwDestroyWindow(this->window);
    glfwTerminate();
}
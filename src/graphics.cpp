#include <set>
#include <algorithm>
#include <iostream>
#include <fstream>

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
    this->swapChainFormat = this->swapDetails.formats[0];

    for (const auto& format : this->swapDetails.formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) this->swapChainFormat = format;
    }

    //select presentation mode
    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    
    for (const auto& mode : this->swapDetails.modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) selectedMode = mode;
    }

    //select buffer extents
    VkSurfaceCapabilitiesKHR& capabilities = this->swapDetails.capabilities;

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        this->swapChainExtent = capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(this->window, &width, &height);

        this->swapChainExtent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        this->swapChainExtent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    // create swapchain
    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = this->swapChainFormat.format;
    createInfo.imageColorSpace = this->swapChainFormat.colorSpace;
    createInfo.imageExtent = this->swapChainExtent;
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

    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, nullptr);
    this->swapImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapchain, &imageCount, this->swapImages.data());
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

void GraphicsContext::SetImageViews() {
    this->swapImageViews.resize(this->swapImages.size());

    for (size_t i = 0; i < this->swapImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->swapImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = this->swapChainFormat.format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(this->logicalDevice, &createInfo, nullptr, &this->swapImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create an image view!");
        }
    }
}

std::vector<char> ReadShaderFile(const std::string& path) {
    std::fstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw new std::runtime_error("Couldn't open shader file!");
    }

    size_t size = file.tellg();
    file.seekg(0);

    std::vector<char> code(size);
    file.read(code.data(), size);
    file.close();

    return code;
}

VkShaderModule GraphicsContext::CreateShaderModule(std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(this->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Could not create shader module!");
    }

    return shaderModule;
}

void GraphicsContext::SetGraphicPipeline() {
    auto vertex = ReadShaderFile("../shader.vert.spv");
    auto frag = ReadShaderFile("../shader.frag.spv");

    auto vertModule = CreateShaderModule(vertex);
    auto fragModule = CreateShaderModule(frag);

    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertModule;
    vertStageInfo.pName = "vert";

    VkPipelineShaderStageCreateInfo fragStageInfo;
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragModule;
    fragStageInfo.pName = "frag";

    VkPipelineShaderStageCreateInfo stages[] = { fragStageInfo, vertStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    vkDestroyShaderModule(this->logicalDevice, fragModule, nullptr);
    vkDestroyShaderModule(this->logicalDevice, vertModule, nullptr);
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
    // NOTE: THIS ORDERING OF METHOD CALLS MATTERS A LOT
    // TODO: MAKE THIS LESS OBTUSE
    // {
    SetWindow();
    SetInstance();
    SetSurface();

    SetPhysicalDevice();
    SetLogicalDevice();

    SetSwapchain();
    SetQueues();
    SetGraphicPipeline();
    // }
}

GraphicsContext::~GraphicsContext() {
    // NOTE: THIS ORDERING OF METHOD CALLS MATTERS A LOT
    // TODO: MAKE THIS LESS OBTUSE
    // {
    for (auto imageView : this->swapImageViews) {
        vkDestroyImageView(this->logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(this->logicalDevice, this->swapchain, nullptr);
    vkDestroyDevice(this->logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);

    glfwDestroyWindow(this->window);
    glfwTerminate();
    // }
}
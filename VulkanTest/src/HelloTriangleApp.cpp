#include "HelloTriangleApp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <set>
#include <string>

#include "Log.h"
#include "VulkanExts.h"
#include "VulkanUtils.h"
#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"

HelloTriangleApp::~HelloTriangleApp()
{
    Cleanup();
}

void HelloTriangleApp::InitWindow()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to init GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(WindowWidth, WindowHeight, "Vulkan Test", nullptr, nullptr);
    if (!mWindow)
        throw std::runtime_error("Failed to create GLFW window");
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
}

void HelloTriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup()
{
    if constexpr (enableValidationLayers)
        vk::ext::DestroyDebugUtilsMessenger(mInstance, mDebugMessenger);

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

    vkDestroyDevice(mDevice, nullptr);

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

    vkDestroyInstance(mInstance, nullptr);

    glfwDestroyWindow(mWindow);
    mWindow = nullptr;

    glfwTerminate();
}

void HelloTriangleApp::CreateInstance()
{
    if constexpr (enableValidationLayers)
    {
        if (!CheckValidationLayerSupport())
            throw std::runtime_error("Validation layers requested but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const auto extensions = GetRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = std::data(extensions);
    createInfo.enabledExtensionCount = (uint32_t)std::size(extensions);

    auto debugCreateInfo = CreateDebugMessengerCreateInfo();
    if constexpr (enableValidationLayers)
    {
        createInfo.ppEnabledLayerNames = std::data(validationLayers);
        createInfo.enabledLayerCount = (uint32_t)std::size(validationLayers);
        createInfo.pNext = &debugCreateInfo;
    }

    if (const auto result = vkCreateInstance(&createInfo, nullptr, &mInstance); result != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance");

    vk::ext::Init(mInstance);

#ifdef LOGGING_ENABLED
    auto extensionProps = vk::utils::GetInstanceExtProps(mInstance);

    LOG_INFO("Available Extensions:");
    for (const auto& e : extensionProps)
        LOG_INFO("    {0}", e.extensionName);
#endif
}

void HelloTriangleApp::CreateSurface()
{
    if (const auto result = glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface); result != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

void HelloTriangleApp::PickPhysicalDevice()
{
    auto devices = vk::utils::GetPhysicalDevices(mInstance);
    if (std::empty(devices))
        throw std::runtime_error("Failed to find GPUs with Vulkan support");

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU");
}

void HelloTriangleApp::CreateLogicalDevice()
{
    constexpr float queuePriority = 1.0f;

    auto indices = QueueFamilyIndices::Find(mPhysicalDevice, mSurface);
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = std::data(queueCreateInfos);
    createInfo.queueCreateInfoCount = (uint32_t)std::size(queueCreateInfos);
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = std::data(deviceExtensions);
    createInfo.enabledExtensionCount = (uint32_t)std::size(deviceExtensions);

    if constexpr (enableValidationLayers)
    {
        createInfo.ppEnabledLayerNames = std::data(validationLayers);
        createInfo.enabledLayerCount = (uint32_t)std::size(validationLayers);
    }

    if (const auto result = vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice); result != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

void HelloTriangleApp::CreateSwapChain()
{
    auto swapChainSupport = SwapChainSupportDetails::Query(mPhysicalDevice, mSurface);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    mSwapChainExtent = ChooseSwapExtent(swapChainSupport.capabilities);

    mSwapChainImageFormat = surfaceFormat.format;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;
    createInfo.minImageCount = swapChainSupport.GetImageCount();
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = mSwapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    auto indices = QueueFamilyIndices::Find(mPhysicalDevice, mSurface);
    const auto familyIndices = indices.GetIndices();

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.pQueueFamilyIndices = std::data(familyIndices);
        createInfo.queueFamilyIndexCount = (uint32_t)std::size(familyIndices);
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (const auto result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain); result != VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain");

    mSwapChainImages = vk::utils::GetSwapChainImages(mDevice, mSwapChain);
    if (std::empty(mSwapChainImages))
        throw std::runtime_error("Failed to get swap chain images");
}

bool HelloTriangleApp::CheckValidationLayerSupport() const
{
    auto availableLayers = vk::utils::GetInstanceLayerProps();

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProps : availableLayers)
        {
            if (strcmp(layerName, layerProps.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

std::vector<const char*> HelloTriangleApp::GetRequiredExtensions() const
{
    uint32_t glfwExtCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtCount);

    if constexpr (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if constexpr (!enableValidationLayers)
        return;

    auto createInfo = CreateDebugMessengerCreateInfo();

    if (const auto result = vk::ext::CreateDebugUtilsMessenger(mInstance, &createInfo, &mDebugMessenger); result != VK_SUCCESS)
        throw std::runtime_error("Failed to set up debug messenger");
}

bool HelloTriangleApp::IsDeviceSuitable(VkPhysicalDevice device) const
{
    /*VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(device, &props);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(device, &features);

    return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader;*/

    QueueFamilyIndices indices = QueueFamilyIndices::Find(device, mSurface);

    const bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        auto swapChainSupport = SwapChainSupportDetails::Query(device, mSurface);
        swapChainAdequate = !std::empty(swapChainSupport.formats) && !std::empty(swapChainSupport.presentModes);
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}

bool HelloTriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
{
    auto availableExtensions = vk::utils::GetPhysicalDeviceExtProps(device);
    std::set<std::string> requiredExts(std::cbegin(deviceExtensions), std::cend(deviceExtensions));
    for (const auto& extension : availableExtensions)
        requiredExts.erase(extension.extensionName);

    return std::empty(requiredExts);
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return availableFormats.front();
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
{
    for (const auto& presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return presentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps) const
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return caps.currentExtent;

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);

    return {
        std::clamp((uint32_t)width, caps.minImageExtent.width, caps.maxImageExtent.width),
        std::clamp((uint32_t)height, caps.minImageExtent.height, caps.maxImageExtent.height)
    };
}

VkDebugUtilsMessengerCreateInfoEXT HelloTriangleApp::CreateDebugMessengerCreateInfo() const
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vk::ext::DebugCallback;
    return createInfo;
}

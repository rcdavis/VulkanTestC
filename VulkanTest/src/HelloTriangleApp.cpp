#include "HelloTriangleApp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <set>

#include "Log.h"
#include "VulkanExts.h"
#include "QueueFamilyIndices.h"

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
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extensionProps(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, std::data(extensionProps));

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
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, std::data(devices));

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

bool HelloTriangleApp::CheckValidationLayerSupport() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(availableLayers));

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

    return indices.IsComplete();
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

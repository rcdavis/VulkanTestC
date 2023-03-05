#include "HelloTriangleApp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "Log.h"

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
    vkDestroyInstance(mInstance, nullptr);

    glfwDestroyWindow(mWindow);
    mWindow = nullptr;

    glfwTerminate();
}

void HelloTriangleApp::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = glfwExts;
    createInfo.enabledExtensionCount = glfwExtCount;

    if (const auto result = vkCreateInstance(&createInfo, nullptr, &mInstance); result != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance");

#ifdef LOGGING_ENABLED
    uint32_t extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, std::data(extensions));

    LOG_INFO("Available Extensions:");
    for (const auto& e : extensions)
        LOG_INFO("    {0}", e.extensionName);
#endif
}

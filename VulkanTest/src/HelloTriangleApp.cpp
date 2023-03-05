#include "HelloTriangleApp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

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
    glfwDestroyWindow(mWindow);
    mWindow = nullptr;

    glfwTerminate();
}

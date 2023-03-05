#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

struct GLFWwindow;

class HelloTriangleApp
{
public:
    HelloTriangleApp() = default;
    ~HelloTriangleApp();

    void Run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        //Cleanup();
    }

private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    void CreateInstance();

private:
    GLFWwindow* mWindow = nullptr;
    VkInstance mInstance {};

    static constexpr uint32_t WindowWidth = 800;
    static constexpr uint32_t WindowHeight = 600;
};

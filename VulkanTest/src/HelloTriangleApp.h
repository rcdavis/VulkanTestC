#pragma once

#include <cstdint>

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
        Cleanup();
    }

private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

private:
    GLFWwindow* mWindow = nullptr;

    static constexpr uint32_t WindowWidth = 800;
    static constexpr uint32_t WindowHeight = 600;
};

#pragma once

class HelloTriangleApp
{
public:
    void Run()
    {
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    void InitVulkan();
    void MainLoop();
    void Cleanup();
};

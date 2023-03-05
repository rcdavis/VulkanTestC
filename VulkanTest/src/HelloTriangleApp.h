#pragma once

#include <cstdint>
#include <vector>
#include <array>

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
    void PickPhysicalDevice();

    bool CheckValidationLayerSupport() const;
    std::vector<const char*> GetRequiredExtensions() const;
    void SetupDebugMessenger();
    bool IsDeviceSuitable(VkPhysicalDevice device) const;

    VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo() const;

private:
    GLFWwindow* mWindow = nullptr;
    VkInstance mInstance{};

    VkPhysicalDevice mPhysicalDevice{};

    VkDebugUtilsMessengerEXT mDebugMessenger{};

    static constexpr uint32_t WindowWidth = 800;
    static constexpr uint32_t WindowHeight = 600;

    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef DEBUG
    static constexpr bool enableValidationLayers = true;
#else
    static constexpr bool enableValidationLayers = false;
#endif
};

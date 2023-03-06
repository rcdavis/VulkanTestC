#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include "Vertex.h"

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

    void DrawFrame();

    void CreateInstance();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void RecreateSwapChain();
    void CleanupSwapChain();

    bool CheckValidationLayerSupport() const;
    std::vector<const char*> GetRequiredExtensions() const;
    void SetupDebugMessenger();
    bool IsDeviceSuitable(VkPhysicalDevice device) const;
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps) const;
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo() const;
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
        VkBuffer& buffer, VkDeviceMemory& bufferMem);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
    static constexpr uint32_t MaxFramesInFlight = 2;

    GLFWwindow* mWindow = nullptr;
    VkInstance mInstance{};

    VkPhysicalDevice mPhysicalDevice{};
    VkDevice mDevice{};

    VkSurfaceKHR mSurface{};

    VkQueue mGraphicsQueue{};
    VkQueue mPresentQueue{};

    VkSwapchainKHR mSwapChain{};
    std::vector<VkImage> mSwapChainImages;
    std::vector<VkImageView> mSwapChainImageViews;
    std::vector<VkFramebuffer> mSwapChainFramebuffers;
    VkFormat mSwapChainImageFormat{};
    VkExtent2D mSwapChainExtent{};

    VkRenderPass mRenderPass{};
    VkPipelineLayout mPipelineLayout{};
    VkPipeline mGraphicsPipeline{};

    VkCommandPool mCommandPool{};
    std::vector<VkCommandBuffer> mCommandBuffers;

    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;

    VkBuffer mVertexBuffer{};
    VkDeviceMemory mVertexBufferMem{};

    VkBuffer mIndexBuffer{};
    VkDeviceMemory mIndexBufferMem{};

    VkDebugUtilsMessengerEXT mDebugMessenger{};

    uint32_t mCurrentFrame = 0;

    bool mFramebufferResized = false;

    static constexpr uint32_t WindowWidth = 800;
    static constexpr uint32_t WindowHeight = 600;

    static constexpr std::array<Vertex, 4> vertices = {
        Vertex {{ -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }},
        Vertex {{ 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }},
        Vertex {{ 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }},
        Vertex {{ -0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }}
    };

    static constexpr std::array<uint16_t, 6> indices = {
        0, 1, 2,
        2, 3, 0
    };

    static constexpr std::array<const char*, 1> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef DEBUG
    static constexpr bool enableValidationLayers = true;
#else
    static constexpr bool enableValidationLayers = false;
#endif
};

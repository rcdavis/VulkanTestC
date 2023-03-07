#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include "Vertex.h"

#include "Vulkan/VulkanImage.h"

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
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateDepthResources();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void LoadModel();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
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
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
        VkBuffer& buffer, VkDeviceMemory& bufferMem);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void UpdateUniformBuffer(uint32_t curImage);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags props, VkImage& image, VkDeviceMemory& imageMem);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkFormat FindDepthFormat() const;
    bool HasStencilComponent(VkFormat format) const;

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
    VkDescriptorSetLayout mDescriptorSetLayout{};
    VkPipelineLayout mPipelineLayout{};
    VkPipeline mGraphicsPipeline{};

    VkCommandPool mCommandPool{};
    std::vector<VkCommandBuffer> mCommandBuffers;

    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    VkBuffer mVertexBuffer{};
    VkDeviceMemory mVertexBufferMem{};

    VkBuffer mIndexBuffer{};
    VkDeviceMemory mIndexBufferMem{};

    std::vector<VkBuffer> mUniformBuffers;
    std::vector<VkDeviceMemory> mUniformBuffersMem;
    std::vector<void*> mUniformBuffersMapped;

    VkDescriptorPool mDescriptorPool{};
    std::vector<VkDescriptorSet> mDescriptorSets;

    VulkanImage mTexImage;
    VkSampler mTexSampler{};

    VulkanImage mDepthImage;

    VkDebugUtilsMessengerEXT mDebugMessenger{};

    uint32_t mCurrentFrame = 0;

    bool mFramebufferResized = false;

    static constexpr uint32_t WindowWidth = 800;
    static constexpr uint32_t WindowHeight = 600;

    static constexpr const char* MeshName = "assets/meshes/VikingRoom.obj";
    static constexpr const char* TextureName = "assets/textures/VikingRoom.png";

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

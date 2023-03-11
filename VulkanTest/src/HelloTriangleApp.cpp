#include "HelloTriangleApp.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include <chrono>

#include <stdexcept>
#include <set>
#include <string>
#include <unordered_map>

#include "Log.h"
#include "Vulkan/VulkanExts.h"
#include "Vulkan/VulkanUtils.h"
#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"
#include "FileUtils.h"
#include "UniformBufferObject.h"
#include "Mesh.h"

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

    mWindow = glfwCreateWindow(WindowWidth, WindowHeight, "Vulkan Test", nullptr, nullptr);
    if (!mWindow)
        throw std::runtime_error("Failed to create GLFW window");
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    LoadModel();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void HelloTriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(mDevice);
}

void HelloTriangleApp::Cleanup()
{
    CleanupSwapChain();

    vkDestroySampler(mDevice, mTexSampler, nullptr);
    mTexSampler = VK_NULL_HANDLE;
    mTexImage.Destroy();

    for (int i = 0; i < std::size(mUniformBuffers); ++i)
    {
        vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
        vkFreeMemory(mDevice, mUniformBuffersMem[i], nullptr);
    }
    mUniformBuffers.clear();
    mUniformBuffersMem.clear();
    mUniformBuffersMapped.clear();

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
    mDescriptorPool = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
    mDescriptorSetLayout = VK_NULL_HANDLE;

    vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
    mIndexBuffer = VK_NULL_HANDLE;
    vkFreeMemory(mDevice, mIndexBufferMem, nullptr);
    mIndexBufferMem = VK_NULL_HANDLE;

    vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
    mVertexBuffer = VK_NULL_HANDLE;
    vkFreeMemory(mDevice, mVertexBufferMem, nullptr);
    mVertexBufferMem = VK_NULL_HANDLE;

    vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
    mVertexBuffer = VK_NULL_HANDLE;

    vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
    mGraphicsPipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    mPipelineLayout = VK_NULL_HANDLE;

    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
    mRenderPass = VK_NULL_HANDLE;

    for (int i = 0; i < std::size(mImageAvailableSemaphores); ++i)
    {
        vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
    }
    mImageAvailableSemaphores.clear();
    mRenderFinishedSemaphores.clear();
    mInFlightFences.clear();

    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    mCommandPool = VK_NULL_HANDLE;
    mCommandBuffers.clear();

    vkDestroyDevice(mDevice, nullptr);
    mDevice = VK_NULL_HANDLE;

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    mSurface = VK_NULL_HANDLE;

    if constexpr (enableValidationLayers)
    {
        vk::ext::DestroyDebugUtilsMessenger(mInstance, mDebugMessenger);
        mDebugMessenger = VK_NULL_HANDLE;
    }

    vkDestroyInstance(mInstance, nullptr);
    mInstance = VK_NULL_HANDLE;

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
    appInfo.apiVersion = VK_API_VERSION_1_1;

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
            mMsaaSamples = vk::utils::GetMaxUsableSampleCount(mPhysicalDevice);
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU");
}

void HelloTriangleApp::CreateLogicalDevice()
{
    constexpr float queuePriority = 1.0f;

    auto familyIndices = QueueFamilyIndices::Find(mPhysicalDevice, mSurface);
    std::set<uint32_t> uniqueQueueFamilies = { familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value() };

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
    deviceFeatures.samplerAnisotropy = VK_TRUE;

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

    vkGetDeviceQueue(mDevice, familyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, familyIndices.presentFamily.value(), 0, &mPresentQueue);
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

    auto queueFamilyIndices = QueueFamilyIndices::Find(mPhysicalDevice, mSurface);
    const auto familyIndices = queueFamilyIndices.GetIndices();

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
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

void HelloTriangleApp::CreateImageViews()
{
    mSwapChainImageViews.resize(std::size(mSwapChainImages));

    for (int i = 0; i < std::size(mSwapChainImages); ++i)
    {
        mSwapChainImageViews[i] = CreateImageView(mSwapChainImages[i], mSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void HelloTriangleApp::CreateRenderPass()
{
    std::array<VkAttachmentDescription, 3> attachments{};
    attachments[0].format = mSwapChainImageFormat;
    attachments[0].samples = mMsaaSamples;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = FindDepthFormat();
    attachments[1].samples = mMsaaSamples;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments[2].format = mSwapChainImageFormat;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.colorAttachmentCount = 1;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pAttachments = std::data(attachments);
    renderPassInfo.attachmentCount = (uint32_t)std::size(attachments);
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pDependencies = &dependency;
    renderPassInfo.dependencyCount = 1;

    if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass");
}

void HelloTriangleApp::CreateDescriptorSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = std::data(bindings);
    layoutInfo.bindingCount = (uint32_t)std::size(bindings);

    if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor set layout");
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
    auto vertShaderModule = vk::utils::CreateShaderModule(mDevice, "assets/shaders/compiled/TriangleTest.vert.spv");
    auto fragShaderModule = vk::utils::CreateShaderModule(mDevice, "assets/shaders/compiled/TriangleTest.frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageInfos{};
    shaderStageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfos[0].module = vertShaderModule;
    shaderStageInfos[0].pName = "main";

    shaderStageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageInfos[1].module = fragShaderModule;
    shaderStageInfos[1].pName = "main";

    const auto bindingDescs = Vertex::GetBindingDescriptions();
    const auto attrDescs = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertInputInfo{};
    vertInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertInputInfo.pVertexBindingDescriptions = std::data(bindingDescs);
    vertInputInfo.vertexBindingDescriptionCount = (uint32_t)std::size(bindingDescs);
    vertInputInfo.pVertexAttributeDescriptions = std::data(attrDescs);
    vertInputInfo.vertexAttributeDescriptionCount = (uint32_t)std::size(attrDescs);

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Dynamic viewport and scissor
    constexpr std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = std::data(dynamicStates);
    dynamicState.dynamicStateCount = (uint32_t)std::size(dynamicStates);

    // Static viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mSwapChainExtent.width;
    viewport.height = (float)mSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pViewports = &viewport;
    viewportState.viewportCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = mMsaaSamples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.attachmentCount = 1;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
    pipelineLayoutInfo.setLayoutCount = 1;

    if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout");

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pStages = std::data(shaderStageInfos);
    pipelineInfo.stageCount = (uint32_t)std::size(shaderStageInfos);
    pipelineInfo.pVertexInputState = &vertInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.renderPass = mRenderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline");

    vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
}

void HelloTriangleApp::CreateFramebuffers()
{
    mSwapChainFramebuffers.resize(std::size(mSwapChainImageViews));
    for (int i = 0; i < std::size(mSwapChainImageViews); ++i)
    {
        const std::array<VkImageView, 3> attachments = {
            mColorImage.mImageView,
            mDepthImage.mImageView,
            mSwapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mRenderPass;
        framebufferInfo.pAttachments = std::data(attachments);
        framebufferInfo.attachmentCount = (uint32_t)std::size(attachments);
        framebufferInfo.width = mSwapChainExtent.width;
        framebufferInfo.height = mSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer");
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    auto queueFamilyIndices = QueueFamilyIndices::Find(mPhysicalDevice, mSurface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool");
}

void HelloTriangleApp::CreateColorResources()
{
    constexpr VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkFormat colorFormat = mSwapChainImageFormat;

    mColorImage.mDevice = mDevice;
    CreateImage(mSwapChainExtent.width, mSwapChainExtent.height, 1, mMsaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
        usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mColorImage.mImage, mColorImage.mImageMem);
    mColorImage.mImageView = CreateImageView(mColorImage.mImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void HelloTriangleApp::CreateDepthResources()
{
    mDepthImage.mDevice = mDevice;
    auto depthFormat = FindDepthFormat();
    CreateImage(mSwapChainExtent.width, mSwapChainExtent.height, 1, mMsaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage.mImage, mDepthImage.mImageMem);
    mDepthImage.mImageView = CreateImageView(mDepthImage.mImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void HelloTriangleApp::CreateTextureImage()
{
    constexpr VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    mTexImage.mDevice = mDevice;

    int texWidth = 0;
    int texHeight = 0;
    int texChannels = 0;
    const auto deleter = [&](stbi_uc* ptr) { stbi_image_free(ptr); };
    std::unique_ptr<stbi_uc, decltype(deleter)> pixels(stbi_load(TextureName, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha), deleter);

    if (!pixels)
        throw std::runtime_error("Failed to load texture image");

    const VkDeviceSize imageSize = VkDeviceSize(texWidth * texHeight * 4);

    mMipLevels = (uint32_t)std::floor(std::log2(std::max(texWidth, texHeight))) + 1;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMem{};
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, props, stagingBuffer, stagingBufferMem);

    void* data = nullptr;
    vkMapMemory(mDevice, stagingBufferMem, 0, imageSize, 0, &data);
    memcpy(data, pixels.get(), (size_t)imageSize);
    vkUnmapMemory(mDevice, stagingBufferMem);

    pixels = nullptr;

    constexpr VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateImage(texWidth, texHeight, mMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        imageUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mTexImage.mImage, mTexImage.mImageMem);

    TransitionImageLayout(mTexImage.mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mMipLevels);
    CopyBufferToImage(stagingBuffer, mTexImage.mImage, (uint32_t)texWidth, (uint32_t)texHeight);

    GenerateMipmaps(mTexImage.mImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mMipLevels);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMem, nullptr);
}

void HelloTriangleApp::CreateTextureImageView()
{
    mTexImage.mImageView = CreateImageView(mTexImage.mImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels);
}

void HelloTriangleApp::CreateTextureSampler()
{
    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = props.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (float)mMipLevels;
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &mTexSampler) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture sampler");
}

void HelloTriangleApp::LoadModel()
{
    auto model = Mesh::Load(MeshName);

    vertices = model->GetVertices();
    indices = model->GetIndices();
}

void HelloTriangleApp::CreateVertexBuffer()
{
    const VkDeviceSize bufferSize = VkDeviceSize(sizeof(Vertex) * std::size(vertices));
    constexpr VkMemoryPropertyFlags stagingProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMem{};
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, stagingBuffer, stagingBufferMem);

    void* data = nullptr;
    vkMapMemory(mDevice, stagingBufferMem, 0, bufferSize, 0, &data);
    memcpy(data, std::data(vertices), (size_t)bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMem);

    constexpr VkBufferUsageFlags vertUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    CreateBuffer(bufferSize, vertUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMem);

    CopyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMem, nullptr);
}

void HelloTriangleApp::CreateIndexBuffer()
{
    const VkDeviceSize bufferSize = VkDeviceSize(sizeof(uint16_t) * std::size(indices));
    constexpr VkMemoryPropertyFlags stagingProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMem{};
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingProps, stagingBuffer, stagingBufferMem);

    void* data = nullptr;
    vkMapMemory(mDevice, stagingBufferMem, 0, bufferSize, 0, &data);
    memcpy(data, std::data(indices), (size_t)bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMem);

    constexpr VkBufferUsageFlags indexUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    CreateBuffer(bufferSize, indexUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMem);

    CopyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMem, nullptr);
}

void HelloTriangleApp::CreateUniformBuffers()
{
    constexpr VkDeviceSize bufferSize = VkDeviceSize(sizeof(UniformBufferObject));
    constexpr VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    mUniformBuffers.resize(MaxFramesInFlight);
    mUniformBuffersMem.resize(MaxFramesInFlight);
    mUniformBuffersMapped.resize(MaxFramesInFlight);

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, props, mUniformBuffers[i], mUniformBuffersMem[i]);
        vkMapMemory(mDevice, mUniformBuffersMem[i], 0, bufferSize, 0, &mUniformBuffersMapped[i]);
    }
}

void HelloTriangleApp::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = MaxFramesInFlight;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MaxFramesInFlight;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = std::data(poolSizes);
    poolInfo.poolSizeCount = (uint32_t)std::size(poolSizes);
    poolInfo.maxSets = MaxFramesInFlight;

    if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool");
}

void HelloTriangleApp::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MaxFramesInFlight, mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = MaxFramesInFlight;
    allocInfo.pSetLayouts = std::data(layouts);

    mDescriptorSets.resize(MaxFramesInFlight);
    if (vkAllocateDescriptorSets(mDevice, &allocInfo, std::data(mDescriptorSets)) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets");

    for (int i = 0; i < std::size(mDescriptorSets); ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mUniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mTexImage.mImageView;
        imageInfo.sampler = mTexSampler;

        std::array<VkWriteDescriptorSet, 2> descWrites{};
        descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrites[0].dstSet = mDescriptorSets[i];
        descWrites[0].dstBinding = 0;
        descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descWrites[0].descriptorCount = 1;
        descWrites[0].pBufferInfo = &bufferInfo;

        descWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrites[1].dstSet = mDescriptorSets[i];
        descWrites[1].dstBinding = 1;
        descWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descWrites[1].descriptorCount = 1;
        descWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(mDevice, (uint32_t)std::size(descWrites), std::data(descWrites), 0, nullptr);
    }
}

void HelloTriangleApp::CreateCommandBuffers()
{
    mCommandBuffers.resize(MaxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.commandBufferCount = (uint32_t)std::size(mCommandBuffers);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(mDevice, &allocInfo, std::data(mCommandBuffers)) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers");
}

void HelloTriangleApp::CreateSyncObjects()
{
    mImageAvailableSemaphores.resize(MaxFramesInFlight);
    mRenderFinishedSemaphores.resize(MaxFramesInFlight);
    mInFlightFences.resize(MaxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MaxFramesInFlight; ++i)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphores");
        }
    }
}

void HelloTriangleApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer");

    constexpr VkClearColorValue clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr VkClearDepthStencilValue clearDepth{ 1.0f, 0 };
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = clearDepth;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mSwapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = mSwapChainExtent;
    renderPassInfo.pClearValues = std::data(clearValues);
    renderPassInfo.clearValueCount = (uint32_t)std::size(clearValues);

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

    std::array<VkBuffer, 1> vertBuffers = { mVertexBuffer };
    std::array<VkDeviceSize, 1> offsets = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, std::data(vertBuffers), std::data(offsets));
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, VkDeviceSize(0), VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout,
        0, 1, &mDescriptorSets[mCurrentFrame], 0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)mSwapChainExtent.width;
    viewport.height = (float)mSwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDrawIndexed(commandBuffer, (uint32_t)std::size(indices), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to record command buffer");
}

void HelloTriangleApp::DrawFrame()
{
    constexpr uint64_t timeout = UINT64_MAX;

    vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, timeout);

    uint32_t imageIndex = 0;
    auto result = vkAcquireNextImageKHR(mDevice, mSwapChain, timeout, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("Failed to acquire swap chain image");

    vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

    UpdateUniformBuffer(mCurrentFrame);

    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
    RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

    std::array<VkSemaphore, 1> waitSemaphores = { mImageAvailableSemaphores[mCurrentFrame] };
    std::array<VkSemaphore, 1> signalSemaphores = { mRenderFinishedSemaphores[mCurrentFrame] };
    constexpr std::array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitSemaphores = std::data(waitSemaphores);
    submitInfo.waitSemaphoreCount = (uint32_t)std::size(waitSemaphores);
    submitInfo.pWaitDstStageMask = std::data(waitStages);
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];
    submitInfo.commandBufferCount = 1;
    submitInfo.pSignalSemaphores = std::data(signalSemaphores);
    submitInfo.signalSemaphoreCount = (uint32_t)std::size(signalSemaphores);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit draw command buffer");

    std::array<VkSwapchainKHR, 1> swapChains = { mSwapChain };

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pWaitSemaphores = std::data(signalSemaphores);
    presentInfo.waitSemaphoreCount = (uint32_t)std::size(signalSemaphores);
    presentInfo.pSwapchains = std::data(swapChains);
    presentInfo.swapchainCount = (uint32_t)std::size(swapChains);
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
        mFramebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image");

    mCurrentFrame = (mCurrentFrame + 1) % MaxFramesInFlight;
}

void HelloTriangleApp::RecreateSwapChain()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
}

void HelloTriangleApp::CleanupSwapChain()
{
    mColorImage.Destroy();
    mDepthImage.Destroy();

    for (auto& fb : mSwapChainFramebuffers)
        vkDestroyFramebuffer(mDevice, fb, nullptr);
    mSwapChainFramebuffers.clear();

    for (auto& imageView : mSwapChainImageViews)
        vkDestroyImageView(mDevice, imageView, nullptr);
    mSwapChainImageViews.clear();
    mSwapChainImages.clear();

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
    mSwapChain = VK_NULL_HANDLE;
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
    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(device, &features);

    QueueFamilyIndices familyIndices = QueueFamilyIndices::Find(device, mSurface);

    const bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        auto swapChainSupport = SwapChainSupportDetails::Query(device, mSurface);
        swapChainAdequate = !std::empty(swapChainSupport.formats) && !std::empty(swapChainSupport.presentModes);
    }

    return familyIndices.IsComplete() && extensionsSupported && swapChainAdequate && features.samplerAnisotropy;
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

VkFormat HelloTriangleApp::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            return format;

        if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            return format;
    }

    throw std::runtime_error("Failed to find supported format");
}

void HelloTriangleApp::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
    VkBuffer& buffer, VkDeviceMemory& bufferMem)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer");

    VkMemoryRequirements memReqs{};
    vkGetBufferMemoryRequirements(mDevice, buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vk::utils::FindMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, props);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMem) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate buffer memory");

    vkBindBufferMemory(mDevice, buffer, bufferMem, 0);
}

void HelloTriangleApp::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

void HelloTriangleApp::UpdateUniformBuffer(uint32_t curImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto curTime = std::chrono::high_resolution_clock::now();
    const float time = std::chrono::duration<float, std::chrono::seconds::period>(curTime -startTime).count();

    UniformBufferObject ubo;
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
        mSwapChainExtent.width / (float)mSwapChainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1.0f;

    memcpy(mUniformBuffersMapped[curImage], &ubo, sizeof(ubo));
}

void HelloTriangleApp::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
    VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags props,
    VkImage& image, VkDeviceMemory& imageMem)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = numSamples;

    if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    VkMemoryRequirements memReqs{};
    vkGetImageMemoryRequirements(mDevice, image, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vk::utils::FindMemoryType(mPhysicalDevice, memReqs.memoryTypeBits, props);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMem) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory");

    vkBindImageMemory(mDevice, image, imageMem, 0);
}

void HelloTriangleApp::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage{};
    VkPipelineStageFlags dstStage{};

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
        throw std::invalid_argument("Unsupported layout transition");

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

void HelloTriangleApp::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommands(commandBuffer);
}

VkImageView HelloTriangleApp::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView{};
    if (vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create texture image view");

    return imageView;
}

void HelloTriangleApp::GenerateMipmaps(VkImage image, VkFormat imageFormat, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProps{};
    vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, imageFormat, &formatProps);

    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("Texture image format does not support linear blitting");

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;
    for (uint32_t i = 1; i < mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

VkCommandBuffer HelloTriangleApp::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void HelloTriangleApp::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);

    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

VkFormat HelloTriangleApp::FindDepthFormat() const
{
    return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool HelloTriangleApp::HasStencilComponent(VkFormat format) const
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void HelloTriangleApp::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = (HelloTriangleApp*)glfwGetWindowUserPointer(window);
    app->mFramebufferResized = true;
}

#include "VulkanUtils.h"

#include <stdexcept>

#include "FileUtils.h"

namespace vk::utils
{
    std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            return {};

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, std::data(devices));

        return devices;
    }

    std::vector<VkExtensionProperties> GetInstanceExtProps(VkInstance instance)
    {
        uint32_t extCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
        if (extCount == 0)
            return {};

        std::vector<VkExtensionProperties> extensionProps(extCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extCount, std::data(extensionProps));

        return extensionProps;
    }

    std::vector<VkExtensionProperties> GetPhysicalDeviceExtProps(VkPhysicalDevice physicalDevice)
    {
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
        if (extCount == 0)
            return {};

        std::vector<VkExtensionProperties> availableExtensions(extCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, std::data(availableExtensions));

        return availableExtensions;
    }

    std::vector<VkLayerProperties> GetInstanceLayerProps()
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        if (layerCount == 0)
            return {};

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, std::data(availableLayers));

        return availableLayers;
    }

    std::vector<VkImage> GetSwapChainImages(VkDevice device, VkSwapchainKHR swapChain)
    {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        if (imageCount == 0)
            return {};

        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, std::data(images));

        return images;
    }

    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags props)
    {
        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type");
    }

    VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
    {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physicalDevice, &props);

        const VkSampleCountFlags counts = (props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts);
        if (counts & VK_SAMPLE_COUNT_64_BIT)
            return VK_SAMPLE_COUNT_64_BIT;
        if (counts & VK_SAMPLE_COUNT_32_BIT)
            return VK_SAMPLE_COUNT_32_BIT;
        if (counts & VK_SAMPLE_COUNT_16_BIT)
            return VK_SAMPLE_COUNT_16_BIT;
        if (counts & VK_SAMPLE_COUNT_8_BIT)
            return VK_SAMPLE_COUNT_8_BIT;
        if (counts & VK_SAMPLE_COUNT_4_BIT)
            return VK_SAMPLE_COUNT_4_BIT;
        if (counts & VK_SAMPLE_COUNT_2_BIT)
            return VK_SAMPLE_COUNT_2_BIT;

        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkShaderModule CreateShaderModule(VkDevice device, const std::filesystem::path& filepath)
    {
        const auto code = FileUtils::ReadFile(filepath);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pCode = (const uint32_t*)std::data(code);
        createInfo.codeSize = std::size(code);

        VkShaderModule shaderModule{};
        if (const auto result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule); result != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module");

        return shaderModule;
    }
}

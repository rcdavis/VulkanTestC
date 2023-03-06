#include "VulkanUtils.h"

#include <stdexcept>

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

    std::vector<VkExtensionProperties> GetPhysicalDeviceExtProps(VkPhysicalDevice device)
    {
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
        if (extCount == 0)
            return {};

        std::vector<VkExtensionProperties> availableExtensions(extCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, std::data(availableExtensions));

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
}

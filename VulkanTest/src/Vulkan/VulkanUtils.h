#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace vk::utils
{
    std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance);

    std::vector<VkExtensionProperties> GetInstanceExtProps(VkInstance instance);
    std::vector<VkExtensionProperties> GetPhysicalDeviceExtProps(VkPhysicalDevice device);

    std::vector<VkLayerProperties> GetInstanceLayerProps();

    std::vector<VkImage> GetSwapChainImages(VkDevice device, VkSwapchainKHR swapChain);

    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags props);
}

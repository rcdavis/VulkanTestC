#pragma once

#include <vector>
#include <filesystem>

#include <vulkan/vulkan.h>

namespace vk::utils
{
    std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance);

    std::vector<VkExtensionProperties> GetInstanceExtProps(VkInstance instance);
    std::vector<VkExtensionProperties> GetPhysicalDeviceExtProps(VkPhysicalDevice physicalDevice);

    std::vector<VkLayerProperties> GetInstanceLayerProps();

    std::vector<VkImage> GetSwapChainImages(VkDevice device, VkSwapchainKHR swapChain);

    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags props);

    VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice physicalDevice);

    VkShaderModule CreateShaderModule(VkDevice device, const std::filesystem::path& filepath);
}

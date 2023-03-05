#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    uint32_t GetImageCount() const;

    static SwapChainSupportDetails Query(VkPhysicalDevice device, VkSurfaceKHR surface);
};

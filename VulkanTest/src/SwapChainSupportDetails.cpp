#include "SwapChainSupportDetails.h"

uint32_t SwapChainSupportDetails::GetImageCount() const
{
    if (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount)
        return capabilities.maxImageCount;

    return capabilities.minImageCount + 1;
}

SwapChainSupportDetails SwapChainSupportDetails::Query(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, std::data(details.formats));
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, std::data(details.presentModes));
    }

    return details;
}

#pragma once

#include <vulkan/vulkan.h>

struct VulkanImage
{
    VulkanImage() = default;
    ~VulkanImage();

    void Destroy();

    VkDevice mDevice = VK_NULL_HANDLE;
    VkImage mImage = VK_NULL_HANDLE;
    VkDeviceMemory mImageMem = VK_NULL_HANDLE;
    VkImageView mImageView = VK_NULL_HANDLE;
};

#include "VulkanImage.h"

#include "Vulkan/VulkanUtils.h"

VulkanImage::~VulkanImage()
{
    Destroy();
}

void VulkanImage::Destroy()
{
    if (mDevice == VK_NULL_HANDLE)
        return;

    if (mImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(mDevice, mImageView, nullptr);
        mImageView = VK_NULL_HANDLE;
    }

    if (mImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(mDevice, mImage, nullptr);
        mImage = VK_NULL_HANDLE;
    }

    if (mImageMem != VK_NULL_HANDLE)
    {
        vkFreeMemory(mDevice, mImageMem, nullptr);
        mImageMem = VK_NULL_HANDLE;
    }
}

#pragma once

#include <optional>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool IsComplete() const;

    static QueueFamilyIndices Find(VkPhysicalDevice device);
};

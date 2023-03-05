#pragma once

#include <optional>
#include <array>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const;

    std::array<uint32_t, 2> GetIndices() const;

    static QueueFamilyIndices Find(VkPhysicalDevice device, VkSurfaceKHR surface);
};

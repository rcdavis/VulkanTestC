#pragma once

#include <vulkan/vulkan.h>

namespace vk::ext
{
    void Init(VkInstance instance);

    VkResult CreateDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        VkDebugUtilsMessengerEXT* pDebugMessenger,
        const VkAllocationCallbacks* pAllocator = nullptr);

    VkResult DestroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator = nullptr);

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData);
}

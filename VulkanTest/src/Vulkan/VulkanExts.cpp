#include "VulkanExts.h"

#include "Log.h"

namespace vk::ext
{
    static PFN_vkCreateDebugUtilsMessengerEXT createDebugMessengerFunc = nullptr;
    static PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugMessengerFunc = nullptr;

    void Init(VkInstance instance)
    {
        createDebugMessengerFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        destroyDebugMessengerFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    }

    VkResult CreateDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        VkDebugUtilsMessengerEXT* pDebugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        if (createDebugMessengerFunc)
            return createDebugMessengerFunc(instance, pCreateInfo, pAllocator, pDebugMessenger);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator)
    {
        if (destroyDebugMessengerFunc)
            destroyDebugMessengerFunc(instance, debugMessenger, pAllocator);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LOG_TRACE(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LOG_INFO(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LOG_WARN(callbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LOG_ERROR(callbackData->pMessage);
            break;
        default:
            LOG_CRITICAL(callbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }
}

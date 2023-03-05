#include "QueueFamilyIndices.h"

#include <vector>

bool QueueFamilyIndices::IsComplete() const
{
    return graphicsFamily.has_value();
}

QueueFamilyIndices QueueFamilyIndices::Find(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, std::data(queueFamilies));

    for (int i = 0; i < std::size(queueFamilies); ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.IsComplete())
            break;
    }

    return indices;
}

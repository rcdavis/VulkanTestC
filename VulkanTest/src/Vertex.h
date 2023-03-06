#pragma once

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec3 pos{ 0.0f };
    glm::vec3 color{ 0.0f };
    glm::vec2 texCoord{ 0.0f };

    static std::array<VkVertexInputBindingDescription, 1> GetBindingDescriptions();

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
};

#pragma once

#include <array>

#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

struct Vertex
{
    glm::vec3 pos{ 0.0f };
    glm::vec3 color{ 1.0f };
    glm::vec2 texCoord{ 0.0f };

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }

    static std::array<VkVertexInputBindingDescription, 1> GetBindingDescriptions();

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();
};

namespace std
{
    template<>
    struct hash<Vertex>
    {
        size_t operator()(const Vertex& vert) const
        {
            return ((hash<glm::vec3>()(vert.pos) ^
                (hash<glm::vec3>()(vert.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vert.texCoord) << 1);
        }
    };
}

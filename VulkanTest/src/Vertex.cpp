#include "Vertex.h"

std::array<VkVertexInputBindingDescription, 1> Vertex::GetBindingDescriptions()
{
    std::array<VkVertexInputBindingDescription, 1> bindingDescs{};

    bindingDescs[0].binding = 0;
    bindingDescs[0].stride = sizeof(Vertex);
    bindingDescs[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescs;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::GetAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 2> attrDescs{};

    attrDescs[0].binding = 0;
    attrDescs[0].location = 0;
    attrDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDescs[0].offset = offsetof(Vertex, pos);

    attrDescs[1].binding = 0;
    attrDescs[1].location = 1;
    attrDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDescs[1].offset = offsetof(Vertex, color);

    return attrDescs;
}

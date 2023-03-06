#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject
{
    alignas(16) glm::mat4 model{ 1.0f };
    alignas(16) glm::mat4 view{ 1.0f };
    alignas(16) glm::mat4 proj{ 1.0f };
};

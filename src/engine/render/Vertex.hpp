#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/glm_include.hpp"

struct Vertex {
    uint16_t pos;
    glm::i16vec3 xyz;
    glm::i16vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};

#pragma once

#include <vulkan/vulkan.hpp>

#include "../core/glm_include.hpp"

struct Vertex {
    uint16_t pos;
    glm::i16vec3 xyz;
    glm::i16vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
    // bool operator==(const Vertex &other) const;
};

// namespace std {
// template <>
// struct hash<Vertex> {
//     size_t operator()(Vertex const &vertex) const {
//         size_t h1 = hash<uint16_t>()(vertex.pos);
//         size_t h2 = hash<glm::i16vec3>()(vertex.xyz);
//         size_t h3 = hash<glm::i16vec2>()(vertex.uv);
//         size_t h = h1 ^ (h2 << 1);
//         h = h ^ (h3 << 1);
//         return h;
//     }
// };
// }

#include "Vertex.hpp"

#include <array>

VkVertexInputBindingDescription Vertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R16_UINT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R16G16B16_SINT;
    attributeDescriptions[1].offset = offsetof(Vertex, xyz);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R16G16_SINT;
    attributeDescriptions[2].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
}

// bool Vertex::operator==(const Vertex &other) const {
//     return
//         pos == other.pos &&
//         xyz == other.xyz &&
//         uv == other.uv;
// }

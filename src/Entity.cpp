#include "Entity.hpp"
#include "World.hpp"

void Entity::setMaterial(int material) {
    _material = material;
    hide();
    show();
}

const int Entity::getMaterial() const {
    return _material;
}

const int& Entity::id() const {
    return _id;
}

void Entity::updateUBO(UniformBufferObject *ubo) {
    ubo->entityLocation[_id] = glm::vec4(location, 0.0);
    ubo->entityRotation[_id] = glm::vec4(rotation, 0.0);
}

Entity::Entity(World *worldIn, int material) : world(worldIn), _material(material) {

}

void Entity::init() {
    if (world->unusedEntityIDS.empty()) {
        std::cerr << "World is not initialized";
        return;
    }
    _id = world->unusedEntityIDS.back();
    world->unusedEntityIDS.pop_back();

    show();
}


void Entity::init(World *worldIn, int material) {
    world = worldIn;
    _material = material;

    init();
}

void Entity::destroy() {
    world->unusedEntityIDS.push_back(_id);
    hide();
}

void Entity::show() {
    for (int x = -3; x <= 3; x++) {
        if (x != 0) {
            world->createSide({_id, 32765, 0, 0, 0}, x, _material);
        }
    }

    std::cerr << world->vertices[world->verticesIndex-1].xyz.y  << std::endl << std::flush;

    std::cerr << _id << std::endl << std::flush;
    return;

    indexLocation = world->indicesIndex;
    vertexLocation = world->verticesIndex;

    auto xyz = glm::i16vec3(_id, 65535, 0);
    auto uv = glm::i16vec2(0, 0);

    uint16_t sp = (_material << 3);

    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 0) << 3) + 0b000), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 0) << 3) + 0b110), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 0) << 3) + 0b100), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 0) << 3) + 0b010), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 3;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 1;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;

    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 6) << 3) + 0b001), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 6) << 3) + 0b101), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 6) << 3) + 0b111), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 6) << 3) + 0b011), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 3;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;

    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 2) << 3) + 0b000), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 2) << 3) + 0b011), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 2) << 3) + 0b010), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 2) << 3) + 0b001), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 3;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 1;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;

    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 4) << 3) + 0b100), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 4) << 3) + 0b110), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 4) << 3) + 0b111), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 4) << 3) + 0b101), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 3;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;

    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 1) << 3) + 0b000), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 1) << 3) + 0b100), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 1) << 3) + 0b101), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 1) << 3) + 0b001), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 3;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;


    world->vertices[world->verticesIndex + 0] = {static_cast<uint16_t>(((sp + 5) << 3) + 0b010), xyz, uv};
    world->vertices[world->verticesIndex + 1] = {static_cast<uint16_t>(((sp + 5) << 3) + 0b111), xyz, uv};
    world->vertices[world->verticesIndex + 2] = {static_cast<uint16_t>(((sp + 5) << 3) + 0b110), xyz, uv};
    world->vertices[world->verticesIndex + 3] = {static_cast<uint16_t>(((sp + 5) << 3) + 0b011), xyz, uv};
    world->indices[world->indicesIndex + 0] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 1] = world->verticesIndex + 1;
    world->indices[world->indicesIndex + 2] = world->verticesIndex + 2;
    world->indices[world->indicesIndex + 3] = world->verticesIndex + 0;
    world->indices[world->indicesIndex + 4] = world->verticesIndex + 3;
    world->indices[world->indicesIndex + 5] = world->verticesIndex + 1;

    world->changedIndices.push_back(world->indicesIndex);
    world->changedVertices.push_back(world->verticesIndex);

    world->indicesIndex += 6;
    world->verticesIndex += 4;

    std::cerr << "added triangles" << std::endl << std::flush;
}

void Entity::hide() {
    for (int x = -3; x <= 3; x++) {
        if (x != 0) {
            world->removeSide({_id, 65565, 0, 0, 0}, x);
        }
    }
    std::cerr << "removed triangles" << std::endl << std::flush;


    return;
    for (size_t x = indexLocation; x < indexLocation + 6 * 6; x++) {
        world->indices[x] = 0;
    }

    for (size_t x = vertexLocation; x < vertexLocation + 4 * 6; x++) {
        world->vertices[x] = {};
    }

    std::cerr << "removed triangles" << std::endl << std::flush;
}

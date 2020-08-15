#include "World.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Entity.hpp"
#include "WorldClient.hpp"

/**
 * Divide one integer by another, rounding towards minus infinity.
 * @param x the dividend
 * @param y the divisor
 * @return the quoitant, rounded towards minus infinity
 */
int div_floor(int x, int y) {
    int q = x / y;
    int r = x % y;
    if ((r != 0) && ((r < 0) != (y < 0))) --q;
    return q;
}

/**
 * Calculate the remainder after dividing one integer by another,
 * rounding the quoitant towards minus infinity.
 * @param x the dividend
 * @param y the divisor
 * @return the remainder
 */
int mod_floor(int x, int y) {
    int r = x % y;
    if ((r != 0) && ((r < 0) != (y < 0))) {
        r += y;
    }
    return r;
}

void World::pollEvents() {
    if (client) {
        client->pollEvents();
        if (!client->requestedChunks.empty()) {
            auto locChunk = client->requestedChunks.front();
            client->requestedChunks.pop();
            auto loc = locChunk.first;
            auto chunk = locChunk.second;
            chunks[loc] = chunk;
            std::cout << "Adding chunk" << std::endl;
            loc.print();
            for (int x = 0; x < CHUNK_SIZE_XZUV; x++) {
                for (int y = 0; y < CHUNK_SIZE_Y; y++) {
                    for (int z = 0; z < CHUNK_SIZE_XZUV; z++) {
                        for (int u = 0; u < CHUNK_SIZE_XZUV; u++) {
                            for (int v = 0; v < CHUNK_SIZE_XZUV; v++) {
                                RelativeCellLoc rel = {x, y, z, u, v};
                                setCellInChunk(loc, rel, chunk[rel], false);
                            }
                        }
                    }
                }
            }
            sendVerticesAndIndicesToVulkan();
        }
    }
}

ChunkLoc World::chunkLocForCell(CellLoc loc) {
    ChunkLoc chunkLoc;
    int size = CHUNK_SIZE_XZUV;
    chunkLoc.x = div_floor(loc.x, size);
    chunkLoc.y = div_floor(loc.y, CHUNK_SIZE_Y);
    chunkLoc.z = div_floor(loc.z, size);
    chunkLoc.u = div_floor(loc.u, size);
    chunkLoc.v = div_floor(loc.v, size);
    return chunkLoc;
}

Cell World::getCell(CellLoc loc) {
    ChunkLoc chunkLoc = chunkLocForCell(loc);

    RelativeCellLoc relCell;
    relCell.x = mod_floor(loc.x, CHUNK_SIZE_XZUV);
    relCell.y = mod_floor(loc.y, CHUNK_SIZE_Y);
    relCell.z = mod_floor(loc.z, CHUNK_SIZE_XZUV);
    relCell.u = mod_floor(loc.u, CHUNK_SIZE_XZUV);
    relCell.v = mod_floor(loc.v, CHUNK_SIZE_XZUV);

    return getCellInChunk(chunkLoc, relCell);
}

Cell World::getCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc relLoc) {
    auto result = chunks.find(chunkLoc);

    if (result == chunks.end()) {
        return 0;
    }

    return (*result).second[relLoc];
}

void World::setCell(CellLoc loc, Cell cellData) {
    ChunkLoc chunkLoc = chunkLocForCell(loc);

    RelativeCellLoc relCell = loc;
    relCell.x = mod_floor(loc.x, CHUNK_SIZE_XZUV);
    relCell.y = mod_floor(loc.y, CHUNK_SIZE_Y);
    relCell.z = mod_floor(loc.z, CHUNK_SIZE_XZUV);
    relCell.u = mod_floor(loc.u, CHUNK_SIZE_XZUV);
    relCell.v = mod_floor(loc.v, CHUNK_SIZE_XZUV);

    setCellInChunk(chunkLoc, relCell, cellData, true);
}

void World::setCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc, Cell cellData, bool sendVertices) {
    auto result = chunks.find(chunkLoc);

    if (result == chunks.end()) {
        // throw ChunkNotLoadedException();
        std::cerr << "Chunk not loaded in setCellInChunk" << std::endl;
        return;
    }

    int x = chunkLoc.x * CHUNK_SIZE_XZUV + loc.x;
    int y = chunkLoc.y * CHUNK_SIZE_Y    + loc.y;
    int z = chunkLoc.z * CHUNK_SIZE_XZUV + loc.z;
    int u = chunkLoc.u * CHUNK_SIZE_XZUV + loc.u;
    int v = chunkLoc.v * CHUNK_SIZE_XZUV + loc.v;

    chunks[chunkLoc][loc] = cellData;

    // Trivial algorithm

    // if (cellData == 0) {
    //     for (int side = -3; side <= 3; side += 1) {
    //         if (side != 0) {
    //             removeSide({x, y, z, u, v}, side);
    //         }
    //     }
    // } else {
    //     for (int side = -3; side <= 3; side += 1) {
    //         if (side != 0) {
    //             createSide({x, y, z, u, v}, side);
    //         }
    //     }
    // }
    // return;

    // Smarter algorithm

    if (cellData == 0) {
        for (int side = -3; side <= 3; side += 1) {
            if (side != 0) {
                removeSide({x, y, z, u, v}, side);
            }
        }
        if (getCell({x - 1, y, z, u, v}) != 0) {
            createSide({x - 1, y, z, u, v}, 1);
        }
        if (getCell({x, y, z, u - 1, v}) != 0) {
            createSide({x, y, z, u - 1, v}, 1);
        }
        if (getCell({x + 1, y, z, u, v}) != 0) {
            createSide({x + 1, y, z, u, v}, -1);
        }
        if (getCell({x, y, z, u + 1, v}) != 0) {
            createSide({x, y, z, u + 1, v}, -1);
        }

        if (getCell({x, y - 1, z, u, v}) != 0) {
            createSide({x, y - 1, z, u, v}, 2);
        }
        if (getCell({x, y + 1, z, u, v}) != 0) {
            createSide({x, y + 1, z, u, v}, -2);
        }

        if (getCell({x, y, z - 1, u, v}) != 0) {
            createSide({x, y, z - 1, u, v}, 3);
        }
        if (getCell({x, y, z, u, v - 1}) != 0) {
            createSide({x, y, z, u, v - 1}, 3);
        }
        if (getCell({x, y, z + 1, u, v}) != 0) {
            createSide({x, y, z + 1, u, v}, -3);
        }
        if (getCell({x, y, z, u, v + 1}) != 0) {
            createSide({x, y, z, u, v + 1}, -3);
        }
    } else {
        if (getCell({x - 1, y, z, u, v}) == 0 || getCell({x, y, z, u - 1, v}) == 0) {
            createSide({x, y, z, u, v}, -1);
        }
        if (getCell({x - 1, y, z, u, v}) != 0 && getCell({x - 1, y, z, u + 1, v}) != 0) {
            removeSide({x - 1, y, z, u, v}, 1);
        }
        if (getCell({x, y, z, u - 1, v}) != 0 && getCell({x + 1, y, z, u - 1, v}) != 0) {
            removeSide({x, y, z, u - 1, v}, 1);
        }

        if (getCell({x + 1, y, z, u, v}) == 0 || getCell({x, y, z, u + 1, v}) == 0) {
            createSide({x, y, z, u, v}, 1);
        }
        if (getCell({x + 1, y, z, u, v}) != 0 && getCell({x + 1, y, z, u - 1, v}) != 0) {
            removeSide({x + 1, y, z, u, v}, -1);
        }
        if (getCell({x, y, z, u + 1, v}) != 0 && getCell({x - 1, y, z, u + 1, v}) != 0) {
            removeSide({x, y, z, u + 1, v}, -1);
        }

        if (getCell({x, y - 1, z, u, v}) == 0) {
            createSide({x, y, z, u, v}, -2);
        } else {
            removeSide({x, y - 1, z, u, v}, 2);
        }

        if (getCell({x, y + 1, z, u, v}) == 0) {
            createSide({x, y, z, u, v}, 2);
        } else {
            removeSide({x, y + 1, z, u, v}, -2);
        }

        if (getCell({x, y, z - 1, u, v}) == 0 || getCell({x, y, z, u, v - 1}) == 0) {
            createSide({x, y, z, u, v}, -3);
        }
        if (getCell({x, y, z - 1, u, v}) != 0 && getCell({x, y, z - 1, u, v + 1}) != 0) {
            removeSide({x, y, z - 1, u, v}, 3);
        }
        if (getCell({x, y, z, u, v - 1}) != 0 && getCell({x, y, z + 1, u, v - 1}) != 0) {
            removeSide({x, y, z, u, v - 1}, 3);
        }

        if (getCell({x, y, z + 1, u, v}) == 0 || getCell({x, y, z, u, v + 1}) == 0) {
            createSide({x, y, z, u, v}, 3);
        }
        if (getCell({x, y, z + 1, u, v}) != 0 && getCell({x, y, z + 1, u, v - 1}) != 0) {
            removeSide({x, y, z + 1, u, v}, -3);
        }
        if (getCell({x, y, z, u, v + 1}) != 0 && getCell({x, y, z - 1, u, v + 1}) != 0) {
            removeSide({x, y, z, u, v + 1}, -3);
        }
    }
};

void World::loadChunk(ChunkLoc loc) {
    if (client) {
        client->requestChunk(loc);
        return;
    }

    Chunk chunk;

    std::string filename = std::to_string(loc.x) + "_" + std::to_string(loc.y) + "_" + std::to_string(loc.z) + "_" + std::to_string(loc.u) + "_" + std::to_string(loc.v);
    std::ifstream file(dirname + "/" + filename, std::ios::out | std::ios::binary);
    file.read((char *) chunk.cells.data(), sizeof(int) * chunk.cells.size());
    chunks[loc] = chunk;
    if(!file.good()) {
        generateChunk(loc);
        return;
    }
    file.close();

    for (int x = 0; x < CHUNK_SIZE_XZUV; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_XZUV; z++) {
                for (int u = 0; u < CHUNK_SIZE_XZUV; u++) {
                    for (int v = 0; v < CHUNK_SIZE_XZUV; v++) {
                        RelativeCellLoc rel = {x, y, z, u, v};
                        setCellInChunk(loc, rel, chunk[rel], false);
                    }
                }
            }
        }
    }
}

void World::unloadChunk(ChunkLoc loc) {

    int size = CHUNK_SIZE_XZUV;

    for (int x = 0; x < size; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < size; z++) {
                for (int u = 0; u < size; u++) {
                    for (int v = 0; v < size; v++) {
                        RelativeCellLoc rel = {x, y, z, u, v};
                        setCellInChunk(loc, rel, 0, false);
                    }
                }
            }
        }
    }
}

void World::saveChunk(ChunkLoc loc) {

    auto chunk = chunks.find(loc);

    if (chunk == chunks.end()) {
        // throw ChunkNotLoadedException();
        std::cerr << "Chunk not loaded in saveChunk" << std::endl;
        return;
    }

    std::string filename = std::to_string(loc.x) + "_" + std::to_string(loc.y) + "_" + std::to_string(loc.z) + "_" + std::to_string(loc.u) + "_" + std::to_string(loc.v);
    std::ofstream file(dirname + "/" + filename, std::ios::out | std::ios::binary);

    file.write((char *) chunks[loc].cells.data(), sizeof(chunks[loc].cells[0]) * chunks[loc].cells.size());

    file.close();
}

void World::generateChunk(ChunkLoc loc) {
    for (int x = 0; x < CHUNK_SIZE_XZUV; x += 1) {
        for (int y = 0; y < CHUNK_SIZE_Y; y += 1) {
            for (int z = 0; z < CHUNK_SIZE_XZUV; z += 1) {
                for (int u = 0; u < CHUNK_SIZE_XZUV; u += 1) {
                    for (int v = 0; v < CHUNK_SIZE_XZUV; v += 1) {
                        // setCellInChunk(loc, {x, y, z, u, v}, rand() % 3, false);
                        // if (y < CHUNK_SIZE_Y / 2) {
                        int material = 0;
                        if (y == 3) {
                            material = 3;
                        }
                        if (y == 2) {
                            material = 2;
                        }
                        if (y == 1) {
                            material = 1;
                        }
                        if (material > 0) {
                            setCellInChunk(loc, {x, y, z, u, v}, material, false);
                        }
                    }
                }
            }
        }
    }
}

void World::printStats() {
    std::cout << "Number of chunks: " << chunks.size() << std::endl;
    std::cout << "Number of indices: " << indicesIndex << std::endl;
    std::cout << "Number of vertices: " << verticesIndex << std::endl;
    if (chunks.size() > 0) {
        std::cout << "Indices per chunk: " << indicesIndex / chunks.size() << std::endl;
        std::cout << "Vertices per chunk: " << verticesIndex / chunks.size() << std::endl;
    }
    std::cout << "Indices capacity: " << indices.size() << std::endl;
    std::cout << "Vertices capacity: " << vertices.size() << std::endl;
    std::cout << "Empty side indices slots: " << emptySideIndices.size() << std::endl;
    std::cout << "Empty side vertices slots: " << emptySideVertices.size() << std::endl;
}

World::World(VulkanUtil *_vulkan) {
    dirname = "world";
    vulkan = _vulkan;
}

World::World(VulkanUtil *_vulkan, WorldClient *_client) {
    vulkan = _vulkan;
    client = _client;
}

World::~World() {
    destroy();
}

// World::World(VulkanUtil *vulkan, std::string dirname) {
//     this->dirname = dirname;
//     this->vulkan = vulkan;
// }

void World::init() {
    vertices.resize(100 * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * 6 * 4, {0, {0, 0, 0}, {0, 0}});
    indices.resize(100 * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * 6 * 6, 0);
    std::cerr << sizeof(vertices[0]) << std::endl;

    for (int x = 0; x < MAX_ENTITYS; x++) {
        unusedEntityIDS.push_back(x);
    }

    verticesIndex++;
}

void World::destroy() {
}

void World::createSide(CellLoc loc, int side, Cell cellData) {
    SideIndex sideIndex = {loc.x, loc.y, loc.z, loc.u, loc.v, side};

    if (sideIndices.count(sideIndex)) {
        return;
    }

    bool usingEmptyIndicesSlot = false;
    size_t originalIndicesIndex = indicesIndex;
    if (emptySideIndices.size() > 0) {
        usingEmptyIndicesSlot = true;
        indicesIndex = emptySideIndices.back();
        emptySideIndices.pop_back();
    }

    bool usingEmptyVerticesSlot = false;
    size_t originalVerticesIndex = verticesIndex;
    if (emptySideVertices.size() > 0) {
        usingEmptyVerticesSlot = true;
        verticesIndex = emptySideVertices.back();
        emptySideVertices.pop_back();
    }

    changedIndices.push_back(indicesIndex);
    changedVertices.push_back(verticesIndex);

    sideIndices[sideIndex] = indicesIndex;
    sideVertices[sideIndex] = verticesIndex;

    Cell mat = getCell(loc);
    if (cellData != -1) {
        mat = cellData;
    };

    auto xyz = glm::i16vec3(loc.x, loc.y, loc.z);
    auto uv = glm::i16vec2(loc.u, loc.v);

    uint16_t sp = ((mat << 3) + side + 3) << 3;

    if (side == -3) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 3;
        indices[indicesIndex + 5] = verticesIndex + 1;
    } else if (side == 3) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 2;
        indices[indicesIndex + 5] = verticesIndex + 3;
    } else if (side == -1) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 3;
        indices[indicesIndex + 5] = verticesIndex + 1;
    } else if (side == 1) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 2;
        indices[indicesIndex + 5] = verticesIndex + 3;
    } else if (side == -2) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 2;
        indices[indicesIndex + 5] = verticesIndex + 3;
    } else if (side == 2) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
        indices[indicesIndex + 0] = verticesIndex + 0;
        indices[indicesIndex + 1] = verticesIndex + 1;
        indices[indicesIndex + 2] = verticesIndex + 2;
        indices[indicesIndex + 3] = verticesIndex + 0;
        indices[indicesIndex + 4] = verticesIndex + 3;
        indices[indicesIndex + 5] = verticesIndex + 1;
    }

    if (usingEmptyIndicesSlot) {
        indicesIndex = originalIndicesIndex;
    } else {
        indicesIndex += 6;
    }
    if (usingEmptyVerticesSlot) {
        verticesIndex = originalVerticesIndex;
    } else {
        verticesIndex += 4;
    }
}

void World::removeSide(CellLoc loc, int side) {
    SideIndex sideIndex = {loc.x, loc.y, loc.z, loc.u, loc.v, side};
    if (sideIndices.count(sideIndex) && sideVertices.count(sideIndex)) {
        size_t index = sideIndices[sideIndex];
        for (size_t i = index; i < index + 6; i += 1) {
            indices[i] = 0;
        }
        sideIndices.erase(sideIndex);
        emptySideIndices.push_back(index);
        changedIndices.push_back(index);

        index = sideVertices[sideIndex];
        for (size_t i = index; i < index + 4; i += 1) {
            vertices[i] = {};
        }
        sideVertices.erase(sideIndex);
        emptySideVertices.push_back(index);
        changedVertices.push_back(index);
    }
}

void World::sendVerticesAndIndicesToVulkan() {
    if (running) {
        if (changedIndices.size() < MAX_INDIVIDUAL_CHANGES && changedVertices.size() < MAX_INDIVIDUAL_CHANGES) {
            for (auto changedIndex : changedIndices) {
                vulkan->resetIndexRange(indices, changedIndex, 6);
            }
            for (auto changedVertex : changedVertices) {
                vulkan->resetVertexRange(vertices, changedVertex, 4);
            }
        } else {
            vulkan->resetVerticesAndIndices(vertices, indices);
        }
    } else {
        vulkan->setVerticesAndIndices(vertices, indices);
    }
    running = true;
    changedVertices.clear();
    changedIndices.clear();
}

void World::updateUBO(UniformBufferObject *ubo) {
    for (int i = 0; i < entities.size(); i++) {
        entities[i].updateUBO(ubo);
    }
}

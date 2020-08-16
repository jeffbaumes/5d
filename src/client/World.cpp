#include "World.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <chrono>

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
            chunk.generateGeometry();
            chunks[loc] = chunk;
            std::cout << "Adding chunk" << std::endl;
            loc.print();
            updateVulkan();
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

    chunks[chunkLoc].setCell(relCell, cellData);

    updateVulkan();
}

void World::loadChunk(ChunkLoc loc) {
    if (client) {
        client->requestChunk(loc);
        return;
    }

    // Chunk chunk;

    // std::string filename = std::to_string(loc.x) + "_" + std::to_string(loc.y) + "_" + std::to_string(loc.z) + "_" + std::to_string(loc.u) + "_" + std::to_string(loc.v);
    // std::ifstream file(dirname + "/" + filename, std::ios::out | std::ios::binary);
    // file.read((char *) chunk.cells.data(), sizeof(int) * chunk.cells.size());
    // chunks[loc] = chunk;
    // if(!file.good()) {
    //     generateChunk(loc);
    //     return;
    // }
    // file.close();

    // for (int x = 0; x < CHUNK_SIZE_XZUV; x++) {
    //     for (int y = 0; y < CHUNK_SIZE_Y; y++) {
    //         for (int z = 0; z < CHUNK_SIZE_XZUV; z++) {
    //             for (int u = 0; u < CHUNK_SIZE_XZUV; u++) {
    //                 for (int v = 0; v < CHUNK_SIZE_XZUV; v++) {
    //                     RelativeCellLoc rel = {x, y, z, u, v};
    //                     setCellInChunk(loc, rel, chunk[rel], false);
    //                 }
    //             }
    //         }
    //     }
    // }
}

// void World::unloadChunk(ChunkLoc loc) {

//     int size = CHUNK_SIZE_XZUV;

//     for (int x = 0; x < size; x++) {
//         for (int y = 0; y < CHUNK_SIZE_Y; y++) {
//             for (int z = 0; z < size; z++) {
//                 for (int u = 0; u < size; u++) {
//                     for (int v = 0; v < size; v++) {
//                         RelativeCellLoc rel = {x, y, z, u, v};
//                         setCellInChunk(loc, rel, 0, false);
//                     }
//                 }
//             }
//         }
//     }
// }

// void World::saveChunk(ChunkLoc loc) {

//     auto chunk = chunks.find(loc);

//     if (chunk == chunks.end()) {
//         // throw ChunkNotLoadedException();
//         std::cerr << "Chunk not loaded in saveChunk" << std::endl;
//         return;
//     }

//     std::string filename = std::to_string(loc.x) + "_" + std::to_string(loc.y) + "_" + std::to_string(loc.z) + "_" + std::to_string(loc.u) + "_" + std::to_string(loc.v);
//     std::ofstream file(dirname + "/" + filename, std::ios::out | std::ios::binary);

//     file.write((char *) chunks[loc].cells.data(), sizeof(chunks[loc].cells[0]) * chunks[loc].cells.size());

//     file.close();
// }

// void World::generateChunk(ChunkLoc loc) {
//     for (int x = 0; x < CHUNK_SIZE_XZUV; x += 1) {
//         for (int y = 0; y < CHUNK_SIZE_Y; y += 1) {
//             for (int z = 0; z < CHUNK_SIZE_XZUV; z += 1) {
//                 for (int u = 0; u < CHUNK_SIZE_XZUV; u += 1) {
//                     for (int v = 0; v < CHUNK_SIZE_XZUV; v += 1) {
//                         // setCellInChunk(loc, {x, y, z, u, v}, rand() % 3, false);
//                         // if (y < CHUNK_SIZE_Y / 2) {
//                         int material = 0;
//                         if (y == 3) {
//                             material = 3;
//                         }
//                         if (y == 2) {
//                             material = 2;
//                         }
//                         if (y == 1) {
//                             material = 1;
//                         }
//                         if (material > 0) {
//                             setCellInChunk(loc, {x, y, z, u, v}, material, false);
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

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
    std::cout << "Empty chunk indices slots: " << emptyChunkIndices.size() << std::endl;
    std::cout << "Empty chunk vertices slots: " << emptyChunkVertices.size() << std::endl;
}

void World::unloadChunk(ChunkLoc loc) {
    chunks.erase(loc);
    std::vector<uint> allocsIndices = chunkIndicesIndices[loc];
    for (int i = 0; i < allocsIndices.size(); i++) {
        uint allocIndex = allocsIndices[i];
        emptyChunkIndices.push_back(allocIndex);
        vulkan->resetIndexRange(EMPTY_INDICES_CHUNK_BLOCK, allocIndex * CHUNK_BLOCK_SIZE, CHUNK_BLOCK_SIZE, i * CHUNK_BLOCK_SIZE);
    }
    chunkIndicesIndices.erase(loc);


    std::vector<uint> allocsVertices = chunkVerticesIndices[loc];
    for (int i = 0; i < allocsVertices.size(); i++) {
        uint allocIndex = allocsVertices[i];
        emptyChunkVertices.push_back(allocIndex);
        vulkan->resetVertexRange(EMPTY_VERTICES_CHUNK_BLOCK, allocIndex * CHUNK_BLOCK_SIZE, CHUNK_BLOCK_SIZE, i * CHUNK_BLOCK_SIZE);
    }

    chunkVerticesIndices.erase(loc);
}

void World::updateVulkan() {
    for (auto const &[loc, chunk] : chunks) {
        std::vector<uint> allocsIndices = chunkIndicesIndices[loc];
        while (chunk.indices.size() > allocsIndices.size() * CHUNK_BLOCK_SIZE) {
            if (emptyChunkIndices.size() > 0) {
                allocsIndices.push_back(emptyChunkIndices.back());
                emptyChunkIndices.pop_back();
            } else {
                allocsIndices.push_back(indicesIndex);
                indicesIndex++;
            }
        }
        for (int i = 0; i < allocsIndices.size(); i++) {
            uint allocIndex = allocsIndices[i];
            vulkan->resetIndexRange(chunk.indices, allocIndex * CHUNK_BLOCK_SIZE, CHUNK_BLOCK_SIZE, i * CHUNK_BLOCK_SIZE);
        }

        std::vector<uint> allocsVertices = chunkVerticesIndices[loc];
        while (chunk.vertices.size() > allocsVertices.size() * CHUNK_BLOCK_SIZE) {
            if (emptyChunkVertices.size() > 0) {
                allocsVertices.push_back(emptyChunkVertices.back());
                emptyChunkVertices.pop_back();
            } else {
                allocsVertices.push_back(verticesIndex);
                verticesIndex++;
            }
        }
        for (int i = 0; i < allocsVertices.size(); i++) {
            uint allocIndex = allocsVertices[i];
            vulkan->resetVertexRange(chunk.vertices, allocIndex * CHUNK_BLOCK_SIZE, CHUNK_BLOCK_SIZE, i * CHUNK_BLOCK_SIZE);
        }
    }
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
    vulkan->setVerticesAndIndices(vertices, indices);
    verticesIndex++;
}

void World::destroy() {
}

// void World::sendVerticesAndIndicesToVulkan() {
//     if (running) {
//         if (changedIndices.size() < MAX_INDIVIDUAL_CHANGES && changedVertices.size() < MAX_INDIVIDUAL_CHANGES) {
//             for (auto changedIndex : changedIndices) {
//                 vulkan->resetIndexRange(indices, changedIndex, 6);
//             }
//             for (auto changedVertex : changedVertices) {
//                 vulkan->resetVertexRange(vertices, changedVertex, 4);
//             }
//         } else {
//             vulkan->resetIndexRange(indices, 0, indicesIndex);
//             vulkan->resetVertexRange(vertices, 0, verticesIndex);
//         }
//     } else {
//         vulkan->setVerticesAndIndices(vertices, indices);
//     }
//     running = true;
//     changedVertices.clear();
//     changedIndices.clear();
// }

void World::updateUBO(UniformBufferObject *ubo) {
    // for (int i = 0; i < entities.size(); i++) {
    //     entities[i].updateUBO(ubo);
    // }
}

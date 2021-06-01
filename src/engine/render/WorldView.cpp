#include "WorldView.hpp"

#include "../core/World.hpp"
#include "WorldViewTask.hpp"

#include <chrono>
#include <thread>

// size_t WorldView::chunkAllocationSize = 6 * 4 * Chunk::size.x * Chunk::size.y * Chunk::size.z * Chunk::size.u * Chunk::size.v;
size_t WorldView::chunkAllocationSize = 4 * (Chunk::size.x * Chunk::size.y * Chunk::size.z * Chunk::size.u * Chunk::size.v / 4);

WorldView::WorldView(std::vector<const char *> extensions) : renderer(extensions) {
    currentChunkSideCounts.resize(
        (Chunk::size.x) *
        (Chunk::size.y) *
        (Chunk::size.z) *
        (Chunk::size.u) *
        (Chunk::size.v) *
        3,
        0
    );

    emptyChunk.resize(chunkAllocationSize, {});
}

VkInstance WorldView::getInstance() {
    return renderer.instance;
}

void WorldView::initSurface(VkSurfaceKHR surface) {
    renderer.initSurface(surface);

    std::cout << "Chunk allocation size " << chunkAllocationSize << std::endl;

    std::vector<Vertex> vertices;
    vertices.resize(500 * chunkAllocationSize, {0, {0, 0, 0}, {0, 0}});

    std::vector<uint32_t> indices;
    indices.resize(500 * chunkAllocationSize / 4 * 6, 0);
    for (size_t i = 0; i < indices.size(); i += 6) {
        size_t v = i / 6 * 4;
        indices[i + 0] = v + 0;
        indices[i + 1] = v + 1;
        indices[i + 2] = v + 2;
        indices[i + 3] = v + 0;
        indices[i + 4] = v + 3;
        indices[i + 5] = v + 1;
    }

    renderer.setVerticesAndIndices(vertices, indices);

    for (int i = (vertices.size() / WorldView::chunkAllocationSize) - 1; i >= 0 ; i -= 1) {
        freeAllocations.push_back(i);
    }
}

void WorldView::setCell(World &world, CellLoc loc, Cell cell) {
    auto surroundingChunks = SurroundingChunks(world, loc);
    setCell(surroundingChunks, loc, cell);
}

void WorldView::setCell(SurroundingChunks &surroundingChunks, CellLoc loc, Cell cell) {
    SurroundingCells surroundingCells = SurroundingCells(surroundingChunks, loc);
    auto x = loc.x;
    auto y = loc.y;
    auto z = loc.z;
    auto u = loc.u;
    auto v = loc.v;

    if (cell == 0) {
        // deleteCellSidesAndAddOtherCellSides(loc);
        for (int side = -3; side <= 3; side += 1) {
            if (side != 0) {
                removeSide(loc, side);
            }
        }

        if (surroundingCells.negativeX.cell != AIR && surroundingCells.negativeX.cell != UNLOADED) {
        // if (surroundingCells.negativeX.cell != AIR) {
            createPosXUSide(surroundingCells.negativeX.loc, surroundingCells.negativeX.cell);
        }
        if (surroundingCells.negativeU.cell != AIR && surroundingCells.negativeU.cell != UNLOADED) {
        // if (surroundingCells.negativeU.cell != AIR) {
            createPosXUSide(surroundingCells.negativeU.loc, surroundingCells.negativeU.cell);
        }

        if (surroundingCells.positiveX.cell != AIR && surroundingCells.positiveX.cell != UNLOADED) {
        // if (surroundingCells.positiveX.cell != AIR) {
            createNegXUSide(surroundingCells.positiveX.loc, surroundingCells.positiveX.cell);
        }
        if (surroundingCells.positiveU.cell != AIR && surroundingCells.positiveU.cell != UNLOADED) {
        // if (surroundingCells.positiveU.cell != AIR) {
            createNegXUSide(surroundingCells.positiveU.loc, surroundingCells.positiveU.cell);
        }

        if (surroundingCells.negativeY.cell != AIR && surroundingCells.negativeY.cell != UNLOADED) {
        // if (surroundingCells.negativeY.cell != AIR) {
            createPosYSide(surroundingCells.negativeY.loc, surroundingCells.negativeY.cell);
        }
        if (surroundingCells.positiveY.cell != AIR && surroundingCells.positiveY.cell != UNLOADED) {
        // if (surroundingCells.positiveY.cell != AIR) {
            createNegYSide(surroundingCells.positiveY.loc, surroundingCells.positiveY.cell);
        }

        if (surroundingCells.negativeZ.cell != AIR && surroundingCells.negativeZ.cell != UNLOADED) {
        // if (surroundingCells.negativeZ.cell != AIR) {
            createPosZVSide(surroundingCells.negativeZ.loc, surroundingCells.negativeZ.cell);
        }
        if (surroundingCells.negativeV.cell != AIR && surroundingCells.negativeV.cell != UNLOADED) {
        // if (surroundingCells.negativeV.cell != AIR) {
            createPosZVSide(surroundingCells.negativeV.loc, surroundingCells.negativeV.cell);
        }

        if (surroundingCells.positiveZ.cell != AIR && surroundingCells.positiveZ.cell != UNLOADED) {
        // if (surroundingCells.positiveZ.cell != AIR) {
            createNegZVSide(surroundingCells.positiveZ.loc, surroundingCells.positiveZ.cell);
        }
        if (surroundingCells.positiveV.cell != AIR && surroundingCells.positiveV.cell != UNLOADED) {
        // if (surroundingCells.positiveV.cell != AIR) {
            createNegZVSide(surroundingCells.positiveV.loc, surroundingCells.positiveV.cell);
        }
    } else {
        if (surroundingCells.negativeX.cell == AIR || surroundingCells.negativeU.cell == AIR) {
            createNegXUSide(loc, cell);
        }
        if (surroundingCells.negativeX.cell != AIR && surroundingCells.negativeXpositiveU.cell != AIR) {
            removeSide({x - 1, y, z, u, v}, 1);
        }
        if (surroundingCells.negativeU.cell != AIR && surroundingCells.positiveXnegativeU.cell != AIR) {
            removeSide({x, y, z, u - 1, v}, 1);
        }

        if (surroundingCells.positiveX.cell == AIR || surroundingCells.positiveU.cell == AIR) {
            createPosXUSide(loc, cell);
        }
        if (surroundingCells.positiveX.cell != AIR && surroundingCells.positiveXnegativeU.cell != AIR) {
            removeSide({x + 1, y, z, u, v}, -1);
        }
        if (surroundingCells.positiveU.cell != AIR && surroundingCells.negativeXpositiveU.cell != AIR) {
            removeSide({x, y, z, u + 1, v}, -1);
        }

        if (surroundingCells.negativeY.cell == AIR) {
            createNegYSide(loc, cell);
        } else {
            removeSide({x, y - 1, z, u, v}, 2);
        }

        if (surroundingCells.positiveY.cell == AIR) {
            createPosYSide(loc, cell);
        } else {
            removeSide({x, y + 1, z, u, v}, -2);
        }

        if (surroundingCells.negativeZ.cell == AIR || surroundingCells.negativeV.cell == AIR) {
            createNegZVSide(loc, cell);
        }
        if (surroundingCells.negativeZ.cell != AIR && surroundingCells.negativeZpositiveV.cell != AIR) {
            removeSide({x, y, z - 1, u, v}, 3);
        }
        if (surroundingCells.negativeV.cell != AIR && surroundingCells.positiveZnegativeV.cell != AIR) {
            removeSide({x, y, z, u, v - 1}, 3);
        }

        if (surroundingCells.positiveZ.cell == AIR || surroundingCells.positiveV.cell == AIR) {
            createPosZVSide(loc, cell);
        }
        if (surroundingCells.positiveZ.cell != AIR && surroundingCells.positiveZnegativeV.cell != AIR) {
            removeSide({x, y, z + 1, u, v}, -3);
        }
        if (surroundingCells.positiveV.cell != AIR && surroundingCells.negativeZpositiveV.cell != AIR) {
            removeSide({x, y, z, u, v + 1}, -3);
        }
    }
}

void WorldView::updateEntity(World &world, Entity &entity, WorldPos pos) {

}

// void WorldView::addChunk(World &world, Chunk &chunk) {
//     std::cout << "addChunk " << chunk.index << std::endl;
//     auto t1 = std::chrono::high_resolution_clock::now();
//     auto geomChunk = std::make_unique<GeometryChunk>();
//     geomChunk->modified = true;
//     chunks[chunk.index] = std::move(geomChunk);
//     auto surroundingChunks = SurroundingChunks(world, Chunk::cellLocForRelativeCellLoc({}, chunk.index));
//     initializingChunkGeometry = true;
//     for (int x = 0; x < Chunk::size.x; x++) {
//         for (int y = 0; y < Chunk::size.y; y++) {
//             for (int z = 0; z < Chunk::size.z; z++) {
//                 for (int u = 0; u < Chunk::size.u; u++) {
//                     for (int v = 0; v < Chunk::size.v; v++) {
//                         RelativeCellLoc rel = {x, y, z, u, v};
//                         CellLoc loc = Chunk::cellLocForRelativeCellLoc(rel, chunk.index);
//                         setCell(surroundingChunks, loc, chunk.getCell(rel));
//                     }
//                 }
//             }
//         }
//     }
//     initializingChunkGeometry = false;
//     auto t2 = std::chrono::high_resolution_clock::now();
//     float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
//     std::cout << "WorldView::addChunk - " << dt << std::endl;
// }

// void WorldView::addChunk(World &world, Chunk &chunk) {
//     std::cout << "addChunk " << chunk.index << std::endl;
//     auto t1 = std::chrono::high_resolution_clock::now();
//     std::fill(currentChunkSides.begin(), currentChunkSides.end(), 0);
//     auto geomChunk = std::make_unique<GeometryChunk>();
//     geomChunk->modified = true;
//     chunks[chunk.index] = std::move(geomChunk);
//     auto surroundingChunks = SurroundingChunks(world, Chunk::cellLocForRelativeCellLoc({}, chunk.index));
//     initializingChunkGeometry = true;
//     int sx = Chunk::size.x;
//     int sy = Chunk::size.y;
//     int sz = Chunk::size.z;
//     int su = Chunk::size.u;
//     int sv = Chunk::size.v;
//     size_t sideBitLoc = 7;

//     for (int x = 1; x < sx + 1; x++) {
//         for (int y = 1; y < sy + 1; y++) {
//             for (int z = 1; z < sz + 1; z++) {
//                 for (int u = 1; u < su + 1; u++) {
//                     for (int v = 1; v < sv + 1; v++) {
//                         int locWithoutSide =
//                                     x * sy * sz * su * sv * 3 +
//                                     y * sz * su * sv * 3 +
//                                     z * su * sv * 3 +
//                                     u * sv * 3 +
//                                     v * 3;
//                         for (int side = 0; side < 5; side++) {
//                             Cell cellValue = chunk.getCell({x - 1, y - 1, z - 1, u - 1, v - 1});

//                             if (cellValue != AIR && cellValue != UNLOADED) {
//                                 int side3d = (side < 3 ? side : (side == 3 ? 0 : 2));
//                                 int8_t currentSideValue = currentChunkSides[locWithoutSide + side3d];
//                                 currentChunkSides[locWithoutSide + side3d] = (currentSideValue + 1) & ~(1UL << sideBitLoc);

//                                 int prevSideLoc = (x - (side == 0 ? 1 : 0)) * sy * sz * su * sv * 3 +
//                                                   (y - (side == 1 ? 1 : 0)) * sz * su * sv * 3 +
//                                                   (z - (side == 2 ? 1 : 0)) * su * sv * 3 +
//                                                   (u - (side == 3 ? 1 : 0)) * sv * 3 +
//                                                   //   (u) * sv * 3 +
//                                                   //   (v) * 3 +
//                                                   (v - (side == 4 ? 1 : 0)) * 3 +
//                                                   side3d;

//                                 int8_t prevSideValue = currentChunkSides[prevSideLoc];

//                                 currentChunkSides[prevSideLoc] = (prevSideValue + 1) | 1UL << sideBitLoc;
//                                 // currentChunkSides[prevSideLoc] = (prevSideValue + 1) & ~(1UL << sideBitLoc);
//                             }
//                         }
//                         //0 1 2 3 4 5
//                         // | | | | | |
//                         // 0 1 2 3 4 5
//                         // RelativeCellLoc rel = {x, y, z, u, v};
//                         // CellLoc loc = Chunk::cellLocForRelativeCellLoc(rel, chunk.index);
//                         // setCell(surroundingChunks, loc, chunk.getCell(rel));
//                     }
//                 }
//             }
//         }
//     }

//     for (int x = 0; x < sx + 1; x++) {
//         for (int y = 0; y < sy + 1; y++) {
//             for (int z = 0; z < sz + 1; z++) {
//                 for (int u = 0; u < su + 1; u++) {
//                     for (int v = 0; v < sv + 1; v++) {
//                         for (int sideBase = 0; sideBase < 5; sideBase++) {
//                             int sideBase3d = (sideBase < 3 ? sideBase : (sideBase == 3 ? 0 : 2));
//                             int8_t sidePacked = currentChunkSides[x * sy * sz * su * sv * 3 +
//                                                                   y * sz * su * sv * 3 +
//                                                                   z * su * sv * 3 +
//                                                                   u * sv * 3 +
//                                                                   v * 3 +
//                                                                   sideBase3d];
//                             bool backFront = (sidePacked >> sideBitLoc) & 1U;
//                             int side = backFront ? -(sideBase3d + 1) : (sideBase3d + 1);
//                             int8_t sides = sidePacked & ~(1UL << sideBitLoc);
                            
//                             if (sides == 1 || sides == 3) {
//                                 RelativeCellLoc loc = {
//                                     x - 1 + (backFront && sideBase == 0 ? 1 : 0),
//                                     y - 1 + (backFront && sideBase == 1 ? 1 : 0),
//                                     z - 1 + (backFront && sideBase == 2 ? 1 : 0),
//                                     // z - 1,
//                                     u - 1 + (backFront && sideBase == 3 ? 1 : 0),
//                                     // u,
//                                     v - 1 + (backFront && sideBase == 4 ? 1 : 0)};
//                                 // v};
//                                 Cell cellValue = chunk.getCell(loc);
//                                 switch (side) {
//                                     case POS_XU:
//                                         createPosXUSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                     case POS_Y:
//                                         createPosYSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                     case POS_ZV:
//                                         createPosZVSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                     case NEG_XU:
//                                         createNegXUSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                     case NEG_Y:
//                                         createNegYSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                     case NEG_ZV:
//                                         createNegZVSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
//                                         break;
//                                 }
//                             }
//                         }    
//                     }
//                 }
//             }
//         }
//     }
//     initializingChunkGeometry = false;
//     auto t2 = std::chrono::high_resolution_clock::now();
//     float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
//     std::cout << "WorldView::addChunk - " << dt << std::endl;
// }

void WorldView::addChunk(World &world, Chunk &chunk) {
    std::cout << "addChunk " << chunk.index << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    std::fill(currentChunkSideCounts.begin(), currentChunkSideCounts.end(), 0);
    auto geomChunk = std::make_unique<GeometryChunk>();
    geomChunk->modified = true;
    chunks[chunk.index] = std::move(geomChunk);
    int sx = Chunk::size.x;
    int sy = Chunk::size.y;
    int sz = Chunk::size.z;
    int su = Chunk::size.u;
    int sv = Chunk::size.v;
    int xStride = sy * sz * su * sv;
    int yStride = sz * su * sv;
    int zStride = su * sv;
    int uStride = sv;
    int vStride = 1;
    int allStrides = xStride + yStride + zStride + uStride + vStride;
    size_t bitLoc = 5;
    int zeroBit = ~(1UL << bitLoc);
    int oneBit = (1UL << bitLoc);

    for (int i = 0; i < chunk.cells.size(); i++) {
        Cell mat = chunk.cells[i];
        if (mat != AIR) {

            currentChunkSideCounts[i * 3 + 0] = (currentChunkSideCounts[i * 3 + 0] + 1) & zeroBit;
            currentChunkSideCounts[i * 3 + 1] = (currentChunkSideCounts[i * 3 + 1] + 1) & zeroBit;
            currentChunkSideCounts[i * 3 + 2] = (currentChunkSideCounts[i * 3 + 2] + 1) & zeroBit;

            currentChunkSideCounts[i * 3 + 0 + xStride] = (currentChunkSideCounts[i * 3 + 0 + xStride] + 1) | oneBit;
            currentChunkSideCounts[i * 3 + 1 + yStride] = (currentChunkSideCounts[i * 3 + 1 + yStride] + 1) | oneBit;
            currentChunkSideCounts[i * 3 + 2 + zStride] = (currentChunkSideCounts[i * 3 + 2 + zStride] + 1) | oneBit;
            currentChunkSideCounts[i * 3 + 0 + uStride] = (currentChunkSideCounts[i * 3 + 0 + uStride] + 1) | oneBit;
            currentChunkSideCounts[i * 3 + 2 + vStride] = (currentChunkSideCounts[i * 3 + 2 + vStride] + 1) | oneBit;
            // std::cout << "Got " << static_cast<int>(currentChunkSideCounts[i * 3 + 2 + vStride]) << std::endl;
        }
    }

    // int numOfSides = chunk.cells.size() * 3 - allStrides - 3;

    // for (int i = 0; i < numOfSides; i++) {
    //     int8_t packedSide = currentChunkSideCounts[i];
    //     int8_t sides = packedSide & zeroBit;

    //     if (sides == 1 || sides == 3) {
    //         bool isForward = (packedSide >> bitLoc) & 1U;
    //         int 
    //     }
    // }

    for (int x = 0; x < sx; x++) {
        for (int y = 0; y < sy; y++) {
            for (int z = 0; z < sz; z++) {
                for (int u = 0; u < su; u++) {
                    for (int v = 0; v < sv; v++) {
                        for (int sideBase = 0; sideBase < 5; sideBase++) {
                            int sideBase3d = (sideBase < 3 ? sideBase : (sideBase == 3 ? 0 : 2));
                            int8_t sidePacked = currentChunkSideCounts[x * xStride * 3 +
                                                                       y * yStride * 3 +
                                                                       z * zStride * 3 +
                                                                       u * uStride * 3 +
                                                                       v * vStride * 3 +
                                                                       sideBase3d];
                            bool isForward = (sidePacked >> bitLoc) & 1U;
                            int side = isForward ? -(sideBase3d + 1) : (sideBase3d + 1);
                            int8_t sides = sidePacked & ~(1UL << bitLoc);

                            std::cout << "Got " << sides << std::endl;

                            if (sides == 1 || sides == 3) {
                                RelativeCellLoc loc = {
                                    x - (isForward && sideBase == 0 ? 1 : 0),
                                    y - (isForward && sideBase == 1 ? 1 : 0),
                                    z - (isForward && sideBase == 2 ? 1 : 0),
                                    u - (isForward && sideBase == 3 ? 1 : 0),
                                    v - (isForward && sideBase == 4 ? 1 : 0)
                                };
                                Cell cellValue = chunk.getCell(loc);
                                switch (side) {
                                    case POS_XU:
                                        createPosXUSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                    case POS_Y:
                                        createPosYSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                    case POS_ZV:
                                        createPosZVSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                    case NEG_XU:
                                        createNegXUSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                    case NEG_Y:
                                        createNegYSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                    case NEG_ZV:
                                        createNegZVSide(Chunk::cellLocForRelativeCellLoc(loc, chunk.index), cellValue);
                                        break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    initializingChunkGeometry = false;
    auto t2 = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
    std::cout << "WorldView::addChunk - " << dt << std::endl;
}

void WorldView::removeChunk(World &world, ChunkIndex chunkInd) {
    std::cout << "removeChunk " << chunkInd << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    auto chunk = chunks[chunkInd].get();
    if (chunk) {
        for (auto alloc : chunk->allocations) {
            freeAllocations.push_back(alloc);
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        // Send zeroed out vertices to vulkan
        for (size_t i = 0; i < chunk->vertices.size(); i += 1) {
            chunk->vertices[i] = {};
        }
        // Make sure we have a full allocation
        while (chunk->vertices.size() % chunkAllocationSize != 0) {
            chunk->vertices.push_back({});
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
        std::cout << "WorldView::removeChunk fill - " << dt << std::endl;
        updateChunkInRenderer(chunk);
        chunks.erase(chunkInd);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
    std::cout << "WorldView::removeChunk - " << dt << std::endl;
}

void WorldView::executeTask(World &world, float timeDelta) {
    std::unordered_set<ChunkIndex> chunkIndices;
    visibleChunkIndices(chunkIndices);
    world.ensureChunks(chunkIndices);

    for (const auto &chunk : chunks) {
        if (chunk.second.get() && chunk.second->modified) {
            std::cout << "in: " << chunk.second.get()->vertices.size() << std::endl;
            ensureAllocationsForChunk(chunk.second.get());
            updateChunkInRenderer(chunk.second.get());
            chunk.second->modified = false;
        }
    }
}

void WorldView::addWorldViewTask(std::shared_ptr<WorldViewTask> task) {
    tasks.push_back(task);
}

void WorldView::setRenderDistanceXZ(float distance) {
    renderDistanceXZ = distance;
}

void WorldView::setRenderDistanceUV(float distance) {
    renderDistanceUV = distance;
}

void WorldView::setCameraPosition(WorldPos pos) {
    cameraPosition = pos;
}

void WorldView::setCameraLookAt(WorldPos pos) {
    cameraLookAt = pos;
}

void WorldView::setCameraViewAngle(float viewAngle) {
    cameraViewAngle = viewAngle;
}

void WorldView::setCameraUVView(float uvView) {
    cameraUVView = uvView;
}

float WorldView::getCameraUVView() {
    return cameraUVView;
}

void WorldView::run() {
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastTime = startTime;
    while (!stopped) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;
        timeDelta = std::min(timeDelta, 0.1f);
        for (auto task : tasks) {
            auto t1 = std::chrono::high_resolution_clock::now();
            task->executeTask(*this, timeDelta);
            auto t2 = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
            // std::cout << dt << std::endl;
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        rendererMutex.lock();
        updateUniforms(timeDelta);
        renderer.draw();
        rendererMutex.unlock();
        auto t2 = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
        // std::cout << dt << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void WorldView::stop() {
    stopped = true;
}

void WorldView::ensureAllocationsForChunk(GeometryChunk *geomChunk) {
    while (geomChunk->allocations.size() * WorldView::chunkAllocationSize < geomChunk->vertices.size()) {
        if (freeAllocations.size() == 0) {
            std::cerr << "Out of graphics memory. Too many chunks to render" << std::endl;
            break;
        }
        geomChunk->allocations.push_back(freeAllocations.back());
        freeAllocations.pop_back();
    }
}

void WorldView::updateChunkInRenderer(GeometryChunk *geomChunk) {
    for (size_t allocIndex = 0; allocIndex < geomChunk->allocations.size(); allocIndex += 1) {
        auto bufferOffset = geomChunk->allocations[allocIndex] * WorldView::chunkAllocationSize;
        auto arrayOffset = allocIndex * WorldView::chunkAllocationSize;
        rendererMutex.lock();
        // std::cout << "Sending in " << geomChunk->vertices.size() << " vertices" << std::endl;
        auto t1 = std::chrono::high_resolution_clock::now();
        renderer.resetVertexRange(geomChunk->vertices, bufferOffset, WorldView::chunkAllocationSize, arrayOffset);
        size_t stored = std::min(geomChunk->vertices.size() - arrayOffset, WorldView::chunkAllocationSize);
        size_t left = chunkAllocationSize - stored;
        if (left > 0) {
            renderer.resetVertexRange(emptyChunk, bufferOffset + stored, left, 0);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
        std::cout << "resetVertexRange - " << dt << std::endl;
        rendererMutex.unlock();
    }
}

UnfinishedVertex WorldView::getUnfinishedVertex(const CellLoc &loc, const Cell &cell) {
    auto xyz = glm::i16vec3(loc.x, loc.y, loc.z);
    auto uv = glm::i16vec2(loc.u, loc.v);

    uint16_t sp = (cell << 3);

    return {xyz, uv, sp};
}

void WorldView::createPosXUSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, POS_XU, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b100));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b101));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b110));
    chunk->modified = true;
}

void WorldView::createPosZVSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, POS_ZV, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b001));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b011));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b101));
    chunk->modified = true;
}

void WorldView::createPosYSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, POS_Y, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b010));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b110));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b011));
    chunk->modified = true;
}

void WorldView::createNegXUSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, NEG_XU, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b011));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b010));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b001));
    chunk->modified = true;
}

void WorldView::createNegZVSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, NEG_ZV, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b110));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b100));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b010));
    chunk->modified = true;
}

void WorldView::createNegYSide(const CellLoc &loc, const Cell &cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)].get();
    int sideSetupSuccessful = sideSetup(chunk, loc, NEG_Y, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b101));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b001));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b100));
    chunk->modified = true;
}

Vertex UnfinishedVertex::toVertex(int side, int corner) {
    return { static_cast<uint16_t>(((sp + side + 3) << 3) + corner), xyz, uv };
};

SideIndex WorldView::sideIndexFromVertex(const Vertex &vertex) {
    int side = ((vertex.pos / 8) % 8) - 3;
    RelativeCellLoc loc = {vertex.xyz.x, vertex.xyz.y, vertex.xyz.z, vertex.uv.x, vertex.uv.y};
    return {loc, side};
}

int WorldView::sideSetup(GeometryChunk *chunk, const CellLoc &loc, int side, const Cell &cell) {
    if (!chunk) {
        return 0;
    }

    SideIndex sideIndex = {loc, side};

    bool sideIsDuplicate = chunk->sideIndices.count(sideIndex);
    if (sideIsDuplicate) {
        return 0;
    }

    auto verticesStartIndex = chunk->vertices.size();
    chunk->sideIndices[sideIndex] = verticesStartIndex;
    return 1;
}

void WorldView::removeSide(CellLoc loc, int side) {
    // If we are initializing chunk geometry, we will never need to delete a side
    if (initializingChunkGeometry) {
        return;
    }
    ChunkIndex chunkIndex = Chunk::chunkIndexForCellLoc(loc);
    auto chunk = chunks[chunkIndex].get();
    if (!chunk) {
        return;
    }

    SideIndex sideIndex = {loc, side};
    if (chunk->sideIndices.count(sideIndex)) {
        auto verticesSize = chunk->vertices.size();
        auto index = chunk->sideIndices[sideIndex];

        // Take care of case where this is the last side
        if (index == verticesSize - 4) {
            chunk->vertices.pop_back();
            chunk->vertices.pop_back();
            chunk->vertices.pop_back();
            chunk->vertices.pop_back();
            return;
        }

        // Move last side to the removed slot
        SideIndex movedSideIndex = sideIndexFromVertex(chunk->vertices.back());
        for (int i = 3; i >= 0; i -= 1) {
            chunk->vertices[index + i] = chunk->vertices.back();
            chunk->vertices.pop_back();
        }
        chunk->sideIndices.erase(sideIndex);
        chunk->sideIndices[movedSideIndex] = index;
    }
}

void WorldView::visibleChunkIndices(std::unordered_set<ChunkIndex> &chunkIndices) {
    // auto chunkIndex = Chunk::chunkIndexForCellLoc(World::cellLocForWorldPos(cameraPosition));
    ChunkIndex chunkIndex = {0, 0, 0, 2, 2};
    int uvSightDistance = 0;
    int xzSightDistance = 1;
    for (int dx = -xzSightDistance; dx <= xzSightDistance; dx += 1) {
        for (int dz = -xzSightDistance; dz <= xzSightDistance; dz += 1) {
            for (int du = -uvSightDistance; du <= uvSightDistance; du += 1) {
                for (int dv = -uvSightDistance; dv <= uvSightDistance; dv += 1) {
                    auto curChunkIndex = chunkIndex;
                    curChunkIndex.x += dx;
                    curChunkIndex.z += dz;
                    curChunkIndex.u += du;
                    curChunkIndex.v += dv;
                    chunkIndices.insert(curChunkIndex);
                }
            }
        }
    }
}

void WorldView::updateUniforms(float timeDelta) {
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    glm::vec3 look = cameraLookAt.xyz();
    glm::vec3 eye = cameraPosition.xyz();
    if (cameraUVViewTween > 0.5f) {
        eye = glm::vec3(cameraPosition.u, cameraPosition.y, cameraPosition.v);
        look = glm::vec3(cameraLookAt.u, cameraLookAt.y, cameraLookAt.v);
    }
    ubo.view = glm::lookAt(eye, look, glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(
        glm::radians(cameraViewAngle),
        renderer.swapChainExtent.width / (float)renderer.swapChainExtent.height,
        0.02f, std::max(renderDistanceXZ, renderDistanceUV));
    ubo.proj[1][1] *= -1;
    ubo.xyz = cameraPosition.xyz();
    ubo.uv = cameraPosition.uv();

    if (glm::abs(cameraUVView - cameraUVViewTween) < 0.01f) {
        cameraUVViewTween = cameraUVView;
    } else {
        float alpha = glm::exp(-timeDelta * 7.0f / tweenTime);
        cameraUVViewTween = (1.0f - alpha) * cameraUVView + alpha * cameraUVViewTween;
    }
    ubo.uvView = cameraUVViewTween;

    renderer.ubo = ubo;
}

SurroundingChunks::SurroundingChunks(World &world, CellLoc loc) {
    auto ind = Chunk::chunkIndexForCellLoc(loc);
    chunk = world.getChunk(ind);
    negativeX = world.getChunk({ind.x - 1, ind.y, ind.z, ind.u, ind.v});
    negativeY = world.getChunk({ind.x, ind.y - 1, ind.z, ind.u, ind.v});
    negativeZ = world.getChunk({ind.x, ind.y, ind.z - 1, ind.u, ind.v});
    negativeU = world.getChunk({ind.x, ind.y, ind.z, ind.u - 1, ind.v});
    negativeV = world.getChunk({ind.x, ind.y, ind.z, ind.u, ind.v - 1});
    positiveX = world.getChunk({ind.x + 1, ind.y, ind.z, ind.u, ind.v});
    positiveY = world.getChunk({ind.x, ind.y + 1, ind.z, ind.u, ind.v});
    positiveZ = world.getChunk({ind.x, ind.y, ind.z + 1, ind.u, ind.v});
    positiveU = world.getChunk({ind.x, ind.y, ind.z, ind.u + 1, ind.v});
    positiveV = world.getChunk({ind.x, ind.y, ind.z, ind.u, ind.v + 1});
}

SurroundingCells::SurroundingCells(SurroundingChunks &surroundingChunks, CellLoc loc) {
    int x = loc.x;
    int y = loc.y;
    int z = loc.z;
    int u = loc.u;
    int v = loc.v;

    negativeX = CellWithLoc(surroundingChunks, {x - 1, y, z, u, v});
    negativeU = CellWithLoc(surroundingChunks, {x, y, z, u - 1, v});
    positiveX = CellWithLoc(surroundingChunks, {x + 1, y, z, u, v});
    positiveU = CellWithLoc(surroundingChunks, {x, y, z, u + 1, v});
    negativeZ = CellWithLoc(surroundingChunks, {x, y, z - 1, u, v});
    negativeV = CellWithLoc(surroundingChunks, {x, y, z, u, v - 1});
    positiveZ = CellWithLoc(surroundingChunks, {x, y, z + 1, u, v});
    positiveV = CellWithLoc(surroundingChunks, {x, y, z, u, v + 1});
    negativeY = CellWithLoc(surroundingChunks, {x, y - 1, z, u, v});
    positiveY = CellWithLoc(surroundingChunks, {x, y + 1, z, u, v});

    negativeXpositiveU = CellWithLoc(surroundingChunks, {x - 1, y, z, u + 1, v});
    positiveXnegativeU = CellWithLoc(surroundingChunks, {x + 1, y, z, u - 1, v});
    negativeZpositiveV = CellWithLoc(surroundingChunks, {x, y, z - 1, u, v + 1});
    positiveZnegativeV = CellWithLoc(surroundingChunks, {x, y, z + 1, u, v - 1});
}

CellWithLoc::CellWithLoc(Cell cell, CellLoc loc) {
    this->cell = cell;
    this->loc = loc;
}

CellWithLoc::CellWithLoc(SurroundingChunks &surroundingChunks, CellLoc loc) {
    this->loc = loc;
    auto relativeLoc = Chunk::relativeCellLocForCellLoc(loc);
    this->cell = surroundingChunks.chunk->getCell(relativeLoc);
}

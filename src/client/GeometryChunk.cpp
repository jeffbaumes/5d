#include "GeometryChunk.hpp"
#include "World.hpp"

GeometryChunk::GeometryChunk(World &w) : world(w) { }

void GeometryChunk::generateGeometry() {
    for (int x = 0; x < CHUNK_SIZE_XZUV; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_XZUV; z++) {
                for (int u = 0; u < CHUNK_SIZE_XZUV; u++) {
                    for (int v = 0; v < CHUNK_SIZE_XZUV; v++) {
                        RelativeCellLoc rel = {x, y, z, u, v};
                        setCell(rel, (*this)[rel]);
                    }
                }
            }
        }
    }
}

Cell GeometryChunk::getWorldCell(const RelativeCellLoc &loc) {
    CellLoc worldLoc = getCellLocFrom(loc, location);
    return world.getCell(worldLoc);
}

void GeometryChunk::setCell(const RelativeCellLoc &loc, Cell cellData) {
    int x = loc.x;
    int y = loc.y;
    int z = loc.z;
    int u = loc.u;
    int v = loc.v;

    (*this)[loc] = cellData;

    if (cellData == 0) {
        for (int side = -3; side <= 3; side += 1) {
            if (side != 0) {
                removeWorldSide({x, y, z, u, v}, side);
            }
        }
        if (getWorldCell({x - 1, y, z, u, v}) != 0) {
            createWorldSide({x - 1, y, z, u, v}, 1);
        }
        if (getWorldCell({x, y, z, u - 1, v}) != 0) {
            createWorldSide({x, y, z, u - 1, v}, 1);
        }
        if (getWorldCell({x + 1, y, z, u, v}) != 0) {
            createWorldSide({x + 1, y, z, u, v}, -1);
        }
        if (getWorldCell({x, y, z, u + 1, v}) != 0) {
            createWorldSide({x, y, z, u + 1, v}, -1);
        }

        if (getWorldCell({x, y - 1, z, u, v}) != 0) {
            createWorldSide({x, y - 1, z, u, v}, 2);
        }
        if (getWorldCell({x, y + 1, z, u, v}) != 0) {
            createWorldSide({x, y + 1, z, u, v}, -2);
        }

        if (getWorldCell({x, y, z - 1, u, v}) != 0) {
            createWorldSide({x, y, z - 1, u, v}, 3);
        }
        if (getWorldCell({x, y, z, u, v - 1}) != 0) {
            createWorldSide({x, y, z, u, v - 1}, 3);
        }
        if (getWorldCell({x, y, z + 1, u, v}) != 0) {
            createWorldSide({x, y, z + 1, u, v}, -3);
        }
        if (getWorldCell({x, y, z, u, v + 1}) != 0) {
            createWorldSide({x, y, z, u, v + 1}, -3);
        }
    } else {
        if (getWorldCell({x - 1, y, z, u, v}) == 0 || getWorldCell({x, y, z, u - 1, v}) == 0) {
            createWorldSide({x, y, z, u, v}, -1);
        }
        if (getWorldCell({x - 1, y, z, u, v}) != 0 && getWorldCell({x - 1, y, z, u + 1, v}) != 0) {
            removeWorldSide({x - 1, y, z, u, v}, 1);
        }
        if (getWorldCell({x, y, z, u - 1, v}) != 0 && getWorldCell({x + 1, y, z, u - 1, v}) != 0) {
            removeWorldSide({x, y, z, u - 1, v}, 1);
        }

        if (getWorldCell({x + 1, y, z, u, v}) == 0 || getWorldCell({x, y, z, u + 1, v}) == 0) {
            createWorldSide({x, y, z, u, v}, 1);
        }
        if (getWorldCell({x + 1, y, z, u, v}) != 0 && getWorldCell({x + 1, y, z, u - 1, v}) != 0) {
            removeWorldSide({x + 1, y, z, u, v}, -1);
        }
        if (getWorldCell({x, y, z, u + 1, v}) != 0 && getWorldCell({x - 1, y, z, u + 1, v}) != 0) {
            removeWorldSide({x, y, z, u + 1, v}, -1);
        }

        if (getWorldCell({x, y - 1, z, u, v}) == 0) {
            createWorldSide({x, y, z, u, v}, -2);
        } else {
            removeWorldSide({x, y - 1, z, u, v}, 2);
        }

        if (getWorldCell({x, y + 1, z, u, v}) == 0) {
            createWorldSide({x, y, z, u, v}, 2);
        } else {
            removeWorldSide({x, y + 1, z, u, v}, -2);
        }

        if (getWorldCell({x, y, z - 1, u, v}) == 0 || getWorldCell({x, y, z, u, v - 1}) == 0) {
            createWorldSide({x, y, z, u, v}, -3);
        }
        if (getWorldCell({x, y, z - 1, u, v}) != 0 && getWorldCell({x, y, z - 1, u, v + 1}) != 0) {
            removeWorldSide({x, y, z - 1, u, v}, 3);
        }
        if (getWorldCell({x, y, z, u, v - 1}) != 0 && getWorldCell({x, y, z + 1, u, v - 1}) != 0) {
            removeWorldSide({x, y, z, u, v - 1}, 3);
        }

        if (getWorldCell({x, y, z + 1, u, v}) == 0 || getWorldCell({x, y, z, u, v + 1}) == 0) {
            createWorldSide({x, y, z, u, v}, 3);
        }
        if (getWorldCell({x, y, z + 1, u, v}) != 0 && getWorldCell({x, y, z + 1, u, v - 1}) != 0) {
            removeWorldSide({x, y, z + 1, u, v}, -3);
        }
        if (getWorldCell({x, y, z, u, v + 1}) != 0 && getWorldCell({x, y, z - 1, u, v + 1}) != 0) {
            removeWorldSide({x, y, z, u, v + 1}, -3);
        }
    }
}

CellLoc GeometryChunk::getCellLocFrom(const RelativeCellLoc &relCellLoc, ChunkLoc chunkLoc) {
    return {
        chunkLoc.x *CHUNK_SIZE_XZUV + relCellLoc.x,
        chunkLoc.y *CHUNK_SIZE_Y + relCellLoc.y,
        chunkLoc.z *CHUNK_SIZE_XZUV + relCellLoc.z,
        chunkLoc.u *CHUNK_SIZE_XZUV + relCellLoc.u,
        chunkLoc.v *CHUNK_SIZE_XZUV + relCellLoc.v,
    };
}

void GeometryChunk::createWorldSide(const RelativeCellLoc &loc, int side, Cell cellData) {
    CellLoc worldLoc = getCellLocFrom(loc, location);
    world.createSide(worldLoc, side, cellData);
}

void GeometryChunk::removeWorldSide(const RelativeCellLoc &loc, int side) {
    CellLoc worldLoc = getCellLocFrom(loc, location);
    world.removeSide(worldLoc, side);
}

void GeometryChunk::createSide(const RelativeCellLoc &cellLoc, int side, Cell cellData) {
    SideIndex sideIndex = {cellLoc.x, cellLoc.y, cellLoc.z, cellLoc.u, cellLoc.v, side};

    if (sideVertices.count(sideIndex)) {
        return;
    }

    size_t verticesIndex = vertices.size();

    bool usingEmptyVerticesSlot = false;
    size_t originalVerticesIndex = verticesIndex;
    if (emptySideVertices.size() > 0) {
        usingEmptyVerticesSlot = true;
        verticesIndex = emptySideVertices.back();
        emptySideVertices.pop_back();
    } else {
        vertices.resize(verticesIndex + 4, {});
    }

    changedVertices.push_back(verticesIndex);

    sideVertices[sideIndex] = verticesIndex;

    Cell mat = getWorldCell(cellLoc);
    if (cellData != -1) {
        mat = cellData;
    };

    CellLoc globalLoc = getCellLocFrom(cellLoc, location);

    auto xyz = glm::i16vec3(globalLoc.x, globalLoc.y, globalLoc.z);
    auto uv = glm::i16vec2(globalLoc.u, globalLoc.v);

    uint16_t sp = ((mat << 3) + side + 3) << 3;

    if (side == -3) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
    } else if (side == 3) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
    } else if (side == -1) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
    } else if (side == 1) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
    } else if (side == -2) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b000), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b101), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b001), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b100), xyz, uv};
    } else if (side == 2) {
        vertices[verticesIndex + 0] = {static_cast<uint16_t>(sp + 0b010), xyz, uv};
        vertices[verticesIndex + 1] = {static_cast<uint16_t>(sp + 0b111), xyz, uv};
        vertices[verticesIndex + 2] = {static_cast<uint16_t>(sp + 0b110), xyz, uv};
        vertices[verticesIndex + 3] = {static_cast<uint16_t>(sp + 0b011), xyz, uv};
    }

    if (usingEmptyVerticesSlot) {
        verticesIndex = originalVerticesIndex;
    } else {
        verticesIndex += 4;
    }
}

void GeometryChunk::removeSide(const RelativeCellLoc &loc, int side) {
    SideIndex sideIndex = {loc.x, loc.y, loc.z, loc.u, loc.v, side};
    if (sideVertices.count(sideIndex)) {
        auto index = sideVertices[sideIndex];
        for (size_t i = index; i < index + 4; i += 1) {
            vertices[i] = {0, {0, 0, 0}, {0, 0}};
        }
        sideVertices.erase(sideIndex);
        emptySideVertices.push_back(index);
        changedVertices.push_back(index);
    }
}

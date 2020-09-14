#include "WorldView.hpp"

#include "../core/World.hpp"

WorldView::WorldView(std::vector<const char *> extensions) : renderer(extensions) {

}

VkInstance WorldView::getInstance() {
    return renderer.instance;
}

void WorldView::initSurface(VkSurfaceKHR surface) {
    renderer.initSurface(surface);

    std::vector<Vertex> vertices;
    vertices.resize(250 * Chunk::size.x * Chunk::size.y * Chunk::size.z * Chunk::size.u * Chunk::size.v * 6 * 4, {0, {0, 0, 0}, {0, 0}});
    std::vector<uint32_t> indices;
    indices.resize(250 * Chunk::size.x * Chunk::size.y * Chunk::size.z * Chunk::size.u * Chunk::size.v * 6 * 6, 0);
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
}

// TODO: Shrink
void WorldView::setCell(World &world, CellLoc loc, Cell cell) {
    SurroundingCells surroundingCells = SurroundingCells(world, loc);

    if (cell == 0) {
        // deleteCellSidesAndAddOtherCellSides(loc);
        for (int side = -3; side <= 3; side += 1) {
            if (side != 0) {
                removeSide(loc, side);
            }
        }

        if (surroundingCells.negativeX.cell != AIR) {
            createPosXUSide(surroundingCells.negativeX.loc, surroundingCells.negativeX.cell); // 1
        }
        if (surroundingCells.negativeU.cell != AIR) {
            createPosXUSide(surroundingCells.negativeU.loc, surroundingCells.negativeU.cell); // 1
        }

        if (surroundingCells.positiveX.cell != AIR) {
            createNegXUSide(surroundingCells.positiveX.loc, surroundingCells.positiveX.cell); // -1
        }
        if (surroundingCells.positiveU.cell != AIR) {
            createNegXUSide(surroundingCells.positiveU.loc, surroundingCells.positiveU.cell); // -1
        }

        if (surroundingCells.negativeY.cell != AIR) {
            createPosYSide(surroundingCells.negativeY.loc, surroundingCells.negativeY.cell);
        }
        if (surroundingCells.positiveY.cell != AIR) {
            createNegYSide(surroundingCells.positiveY.loc, surroundingCells.positiveY.cell);
        }

        if (surroundingCells.negativeZ.cell != AIR) {
            createPosZVSide(surroundingCells.negativeZ.loc, surroundingCells.negativeZ.cell);
        }
        if (surroundingCells.negativeV.cell != AIR) {
            createPosZVSide(surroundingCells.negativeV.loc, surroundingCells.negativeV.cell);
        }

        if (surroundingCells.positiveZ.cell != AIR) {
            createNegZVSide(surroundingCells.positiveZ.loc, surroundingCells.positiveZ.cell);
        }
        if (surroundingCells.positiveV.cell != AIR) {
            createNegZVSide(surroundingCells.positiveV.loc, surroundingCells.positiveV.cell);
        }
    } else {
        if (world.getCell({x - 1, y, z, u, v}) == 0 || world.getCell({x, y, z, u - 1, v}) == 0) {
            createSide({x, y, z, u, v}, -1);
        }
        if (world.getCell({x - 1, y, z, u, v}) != 0 && world.getCell({x - 1, y, z, u + 1, v}) != 0) {
            removeSide({x - 1, y, z, u, v}, 1);
        }
        if (world.getCell({x, y, z, u - 1, v}) != 0 && world.getCell({x + 1, y, z, u - 1, v}) != 0) {
            removeSide({x, y, z, u - 1, v}, 1);
        }

        if (world.getCell({x + 1, y, z, u, v}) == 0 || world.getCell({x, y, z, u + 1, v}) == 0) {
            createSide({x, y, z, u, v}, 1);
        }
        if (world.getCell({x + 1, y, z, u, v}) != 0 && world.getCell({x + 1, y, z, u - 1, v}) != 0) {
            removeSide({x + 1, y, z, u, v}, -1);
        }
        if (world.getCell({x, y, z, u + 1, v}) != 0 && world.getCell({x - 1, y, z, u + 1, v}) != 0) {
            removeSide({x, y, z, u + 1, v}, -1);
        }

        if (world.getCell({x, y - 1, z, u, v}) == 0) {
            createSide({x, y, z, u, v}, -2);
        } else {
            removeSide({x, y - 1, z, u, v}, 2);
        }

        if (world.getCell({x, y + 1, z, u, v}) == 0) {
            createSide({x, y, z, u, v}, 2);
        } else {
            removeSide({x, y + 1, z, u, v}, -2);
        }

        if (world.getCell({x, y, z - 1, u, v}) == 0 || world.getCell({x, y, z, u, v - 1}) == 0) {
            createSide({x, y, z, u, v}, -3);
        }
        if (world.getCell({x, y, z - 1, u, v}) != 0 && world.getCell({x, y, z - 1, u, v + 1}) != 0) {
            removeSide({x, y, z - 1, u, v}, 3);
        }
        if (world.getCell({x, y, z, u, v - 1}) != 0 && world.getCell({x, y, z + 1, u, v - 1}) != 0) {
            removeSide({x, y, z, u, v - 1}, 3);
        }

        if (world.getCell({x, y, z + 1, u, v}) == 0 || world.getCell({x, y, z, u, v + 1}) == 0) {
            createSide({x, y, z, u, v}, 3);
        }
        if (world.getCell({x, y, z + 1, u, v}) != 0 && world.getCell({x, y, z + 1, u, v - 1}) != 0) {
            removeSide({x, y, z + 1, u, v}, -3);
        }
        if (world.getCell({x, y, z, u, v + 1}) != 0 && world.getCell({x, y, z - 1, u, v + 1}) != 0) {
            removeSide({x, y, z, u, v + 1}, -3);
        }
    }
}

void WorldView::updateEntity(World &world, Entity &entity, WorldPos pos) {

}

void WorldView::addChunk(World &world, Chunk &chunk) {
    std::cout << "Chunk added with x index " << chunk.index.x << std::endl;
    for (int x = 0; x < Chunk::size.x; x++) {
        for (int y = 0; y < Chunk::size.y; y++) {
            for (int z = 0; z < Chunk::size.z; z++) {
                for (int u = 0; u < Chunk::size.u; u++) {
                    for (int v = 0; v < Chunk::size.v; v++) {
                        RelativeCellLoc rel = {x, y, z, u, v};
                        CellLoc loc = Chunk::cellLocForRelativeCellLoc(rel, chunk.index);
                        setCell(world, loc, chunk.getCell(rel));
                    }
                }
            }
        }
    }
}

void WorldView::removeChunk(World &world, ChunkIndex chunkInd) {

}

void WorldView::executeTask(World &world, float timeDelta) {
    std::set<ChunkIndex> chunkIndices;
    visibleChunkIndices(chunkIndices);
    world.ensureChunks(chunkIndices);

    updateUniforms(timeDelta);
    renderer.draw();
}

void WorldView::setRenderDistanceXZ(float distance) {

}

void WorldView::setRenderDistanceUV(float distance) {

}

void WorldView::setCameraPosition(WorldPos pos) {

}

void WorldView::setCameraLookAt(WorldPos pos) {

}

void WorldView::setCameraViewAngle(float viewAngle) {

}

UnfinishedVertex WorldView::getUnfinishedVertex(CellLoc &const loc, Cell &const cell) {
    auto xyz = glm::i16vec3(loc.xyz());
    auto uv = glm::i16vec2(loc.uv());

    uint16_t sp = (cell << 3);

    return {xyz, uv, sp};
}

void WorldView::createPosXUSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b100));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b101));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_XU, 0b110));
}

void WorldView::createPosZVSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b001));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b011));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_ZV, 0b101));
}

void WorldView::createPosYSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b010));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b111));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b110));
    chunk->vertices.push_back(unfinishedVertex.toVertex(POS_Y, 0b011));
}

void WorldView::createNegXUSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b011));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b010));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_XU, 0b001));
}

void WorldView::createNegZVSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b110));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b100));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_ZV, 0b010));
}

void WorldView::createNegYSide(CellLoc &const loc, Cell &const cell) {
    auto chunk = chunks[Chunk::chunkIndexForCellLoc(loc)];
    int sideSetupSuccessful = sideSetup(chunk, loc, cell);
    if (!sideSetupSuccessful) {
        return;
    }
    UnfinishedVertex unfinishedVertex = getUnfinishedVertex(loc, cell);

    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b000));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b101));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b001));
    chunk->vertices.push_back(unfinishedVertex.toVertex(NEG_Y, 0b100));
}

Vertex UnfinishedVertex::toVertex(int side, int corner) {
    return {static_cast<uint16_t>(sp + (side + 3) << 3) + corner, xyz, uv};
};

int WorldView::sideSetup(std::shared_ptr<GeometryChunk> &const chunk, CellLoc & const loc, Cell & const cell) {
    ChunkIndex chunkIndex = Chunk::chunkIndexForCellLoc(loc);
    std::shared_ptr<GeometryChunk> chunk = chunks[chunkIndex];
    RelativeCellLoc relCellLoc = Chunk::relativeCellLocForCellLoc(loc);

    SideIndex sideIndex = {loc, POS_XU};

    bool sideIsDuplicate = chunk->sideIndices.count(sideIndex);
    if (sideIsDuplicate) {
        return 0;
    }

    chunk->vertices.reserve(4);

    auto verticesStartIndex = chunk->vertices.size();

    chunk->changedVertices.push_back(verticesStartIndex);

    chunk->sideIndices[sideIndex] = verticesStartIndex;

    return 1;
}

void WorldView::removeSide(CellLoc loc, int side) {

}

void WorldView::removePosXUSide(CellLoc loc, Cell cellData) {

}
void WorldView::removePosZVSide(CellLoc loc, Cell cellData) {

}
void WorldView::removePosYSide(CellLoc loc, Cell cellData) {

}
void WorldView::removeNegXUSide(CellLoc loc, Cell cellData) {

}
void WorldView::removeNegZVSide(CellLoc loc, Cell cellData) {

}
void WorldView::removeNegYSide(CellLoc loc, Cell cellData) {

}

void WorldView::visibleChunkIndices(std::set<ChunkIndex> &chunkIndices) {
    auto chunkIndex = Chunk::chunkIndexForCellLoc(World::cellLocForWorldPos(cameraPosition));
    chunkIndices.insert(chunkIndex);
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
    ubo.view = glm::lookAt(eye, eye + look, glm::vec3(0.0f, 1.0f, 0.0f));
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

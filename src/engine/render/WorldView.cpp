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

void WorldView::setCell(World &world, CellLoc loc, Cell cell) {
    int x = loc.x;
    int y = loc.y;
    int z = loc.z;
    int u = loc.u;
    int v = loc.v;

    CellLoc negXLoc = {x - 1, y, z, u, v};
    CellLoc negULoc = {x, y, z, u - 1, v};
    CellLoc posXLoc = {x + 1, y, z, u, v};
    CellLoc posULoc = {x, y, z, u + 1, v};
    CellLoc negYLoc = {x, y - 1, z, u, v};
    CellLoc posYLoc = {x, y + 1, z, u, v};
    CellLoc negZLoc = {x, y, z - 1, u, v};
    CellLoc negVLoc = {x, y, z, u, v - 1};
    CellLoc posZLoc = {x, y, z + 1, u, v};
    CellLoc posVLoc = {x, y, z, u, v + 1};

    Cell negXCell = world.getCell(negXLoc);
    Cell negUCell = world.getCell(negULoc);
    Cell posXCell = world.getCell(posXLoc);
    Cell posUCell = world.getCell(posULoc);
    Cell negYCell = world.getCell(negYLoc);
    Cell posYCell = world.getCell(posYLoc);
    Cell negZCell = world.getCell(negZLoc);
    Cell negVCell = world.getCell(negVLoc);
    Cell posZCell = world.getCell(posZLoc);
    Cell posVCell = world.getCell(posVLoc);

    if (cell == 0) {
        for (int side = -3; side <= 3; side += 1) {
            if (side != 0) {
                removeSide({x, y, z, u, v}, side);
            }
        }
        if (negXCell != 0) {
            createPosXUSide(negXLoc, negXCell);
        }
        if (negUCell != 0) {
            createPosXUSide(negULoc, negUCell);
        }
        if (posXCell != 0) {
            createNegXUSide(posXLoc, posXCell);
        }
        if (posUCell != 0) {
            createNegXUSide(posULoc, posUCell);
        }

        if (negYCell != 0) {
            createPosYSide(negYLoc, negYCell);
        }
        if (posYCell != 0) {
            createNegYSide(posYLoc, posYCell);
        }

        if (negZCell != 0) {
            createPosZVSide(negZLoc, negZCell);
        }
        if (negVCell != 0) {
            createPosZVSide(negVLoc, negVCell);
        }
        if (posZCell != 0) {
            createNegZVSide(posZLoc, posZCell);
        }
        if (posVCell != 0) {
            createNegZVSide(posVLoc, posVCell);
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

void WorldView::createSide(CellLoc loc, int side, Cell cell) {

}

void WorldView::removeSide(CellLoc loc, int side) {

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

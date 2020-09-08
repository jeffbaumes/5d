#pragma once

#include "types.hpp"

#include "../core/WorldListener.hpp"
#include "../core/WorldTask.hpp"
#include "../core/ChunkRequestHandler.hpp"

class Chunk;

class WorldClient : public WorldListener, public WorldTask, public ChunkRequestHandler {
public:
    WorldClient(std::string serverAddress);

    // WorldListener
    void setCell(CellLoc loc, Cell cell) override;
    void updateEntity(Entity &entity, WorldPos pos) override;
    void addChunk(Chunk &chunk) override;
    void removeChunk(ChunkIndex chunkInd) override;

    // WorldTask
    void executeTask(World &world, float dt) override;

    // ChunkRequestHandler
    void requestChunk(ChunkIndex chunkInd) override;
    bool hasChunk() override;
    std::unique_ptr<Chunk> retrieveChunk() override;
};

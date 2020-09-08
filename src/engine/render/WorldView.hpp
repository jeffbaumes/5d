#pragma once

#include <vector>

#include "../core/types.hpp"
#include "../core/WorldListener.hpp"
#include "../core/WorldTask.hpp"

class Entity;

class WorldView : public WorldListener, public WorldTask {
public:
    WorldView();

    // WorldListener
    void setCell(CellLoc loc, Cell cell) override;
    void updateEntity(Entity &entity, WorldPos pos) override;
    void addChunk(Chunk &chunk) override;
    void removeChunk(ChunkIndex chunkInd) override;

    // WorldTask
    void executeTask(World &world, float dt) override;

    void setXZRenderDistance(float distance);
    void setUVRenderDistance(float distance);
    void setCameraPosition(WorldPos pos);
    void setCameraLookAt(WorldPos pos);
    void setCameraViewAngle(float viewAngle);

    void render();

private:
    void createSide(CellLoc loc, int side, Cell cell);
    void removeSide(CellLoc loc, int side);
    void visibleChunkIndices(std::vector<ChunkIndex> &chunkIndices);
};

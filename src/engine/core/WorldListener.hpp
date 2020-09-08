#pragma once

#include "types.hpp"

class Chunk;
class Entity;
class World;

class WorldListener {
public:
    virtual ~WorldListener() = default;
    virtual void setCell(World &world, CellLoc loc, Cell cell) = 0;
    virtual void updateEntity(Entity &entity, WorldPos pos) = 0;
    virtual void addChunk(Chunk &) = 0;
    virtual void removeChunk(ChunkIndex chunkInd) = 0;
};

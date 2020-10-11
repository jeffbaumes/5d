#pragma once

#include "types.hpp"

class Chunk;
class Entity;
class World;

class WorldListener {
public:
    virtual ~WorldListener() = default;
    virtual void setCell(World &world, CellLoc loc, Cell cell) = 0;
    virtual void updateEntity(World &world, Entity &entity, WorldPos pos) = 0;
    virtual void addChunk(World &world, Chunk &) = 0;
    virtual void removeChunk(World &world, ChunkIndex chunkInd) = 0;
};

#pragma once

#include <unordered_map>
#include <memory>
#include <set>
#include <vector>

#include "types.hpp"

class Chunk;
class ChunkRequestHandler;
class Entity;
class WorldListener;
class WorldTask;

class World {
public:
    void setCell(CellLoc loc, Cell cell);
    Cell getCell(CellLoc loc);

    void addEntity(std::unique_ptr<Entity> entity);

    Chunk &getChunk(ChunkIndex chunkInd);
    void requestChunk(ChunkIndex chunkInd);
    void addChunk(std::unique_ptr<Chunk> chunk);
    void removeChunk(ChunkIndex chunkInd);
    void ensureChunks(const std::set<ChunkIndex> &chunkInds);

    void addWorldListener(std::shared_ptr<WorldListener> listener);
    void addWorldTask(std::shared_ptr<WorldTask> task);
    void setChunkRequestHandler(std::shared_ptr<ChunkRequestHandler> handler);

    virtual void run();

private:
    std::unordered_map<ChunkIndex, std::unique_ptr<Chunk> > chunks = {};
    std::vector<std::shared_ptr<Entity> > entities = {};
    std::vector<std::shared_ptr<WorldListener> > listeners = {};
    std::vector<std::shared_ptr<WorldTask> > tasks = {};
    std::shared_ptr<ChunkRequestHandler> chunkRequestHandler = {};
    bool stop = false;
};

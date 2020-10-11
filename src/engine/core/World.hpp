#pragma once

#include <memory>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "types.hpp"

#include "Chunk.hpp"

class ChunkRequestHandler;
class Entity;
class WorldListener;
class WorldTask;

class World {
public:
    void setCell(CellLoc loc, Cell cell);
    Cell getCell(CellLoc loc);

    void addEntity(std::unique_ptr<Entity> entity);

    Chunk *getChunk(ChunkIndex chunkInd);
    void requestChunk(ChunkIndex chunkInd);
    void addChunk(std::unique_ptr<Chunk> chunk);
    void removeChunk(ChunkIndex chunkInd);
    void ensureChunks(const std::unordered_set<ChunkIndex> &chunkInds);

    void addWorldListener(std::shared_ptr<WorldListener> listener);
    void addWorldTask(std::shared_ptr<WorldTask> task);
    void setChunkRequestHandler(std::shared_ptr<ChunkRequestHandler> handler);

    std::thread &run();
    void stop();

    static CellLoc cellLocForWorldPos(WorldPos pos);

private:
    std::unordered_map<ChunkIndex, std::unique_ptr<Chunk> > chunks = {};
    std::vector<std::shared_ptr<Entity> > entities = {};
    std::vector<std::shared_ptr<WorldListener> > listeners = {};
    std::vector<std::shared_ptr<WorldTask> > tasks = {};
    std::shared_ptr<ChunkRequestHandler> chunkRequestHandler = {};
    bool stopped = false;
    std::thread runThread;
};

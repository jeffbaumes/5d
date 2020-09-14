#pragma once

#include <memory>
#include <queue>

#include "WorldListener.hpp"
#include "ChunkRequestHandler.hpp"

#include "types.hpp"

class Chunk;
class ChunkGenerator;

class LocalChunkHandler: public WorldListener, public ChunkRequestHandler {
public:
    LocalChunkHandler(std::string dir);
    LocalChunkHandler(std::string dir, std::unique_ptr<ChunkGenerator> generator);

    // WorldListener
    void setCell(World &world, CellLoc loc, Cell cell) override;
    void updateEntity(World &world, Entity &entity, WorldPos pos) override;
    void addChunk(World &world, Chunk &chunk) override;
    void removeChunk(World &world, ChunkIndex chunkInd) override;

    // ChunkRequestHandler
    void requestChunk(ChunkIndex chunkInd) override;
    bool hasChunk() override;
    std::unique_ptr<Chunk> retrieveChunk() override;

private:
    void saveChunk(const Chunk &chunk);

    std::string worldDir;
    std::unique_ptr<ChunkGenerator> chunkGenerator;
    std::queue<std::unique_ptr<Chunk> > handledChunks;
};

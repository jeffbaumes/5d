#include "LocalChunkHandler.hpp"

#include "FlatChunkGenerator.hpp"
#include "World.hpp"

#include <thread>

LocalChunkHandler::LocalChunkHandler(std::string dir) :
    worldDir(dir),
    chunkGenerator(std::move(std::make_unique<FlatChunkGenerator>())) { }

LocalChunkHandler::LocalChunkHandler(std::string dir, std::unique_ptr<ChunkGenerator> generator) :
    worldDir(dir),
    chunkGenerator(std::move(generator)) { }

void LocalChunkHandler::setCell(World &world, CellLoc loc, Cell cell) {
    ChunkIndex chunkInd = Chunk::chunkIndexForCellLoc(loc);
    saveChunk(world.getChunk(chunkInd));
}

void LocalChunkHandler::updateEntity(World &world, Entity &entity, WorldPos pos) {
    // Figure out how to save entities
    // ... and load entities
}

void LocalChunkHandler::addChunk(World &world, Chunk &chunk) {
    saveChunk(&chunk);
}

void LocalChunkHandler::removeChunk(World &world, ChunkIndex chunkInd) {
    // Nothing to do
}

void LocalChunkHandler::requestChunk(ChunkIndex chunkInd) {
    std::thread thread([chunkInd](LocalChunkHandler *_this) {
        auto chunk = std::make_unique<Chunk>();
        chunk->index = chunkInd;
        _this->chunkGenerator->fillChunk(*chunk);
        _this->handledChunksMutex.lock();
        _this->handledChunks.push(std::move(chunk));
        _this->handledChunksMutex.unlock();
    }, this);
    thread.detach();
}

bool LocalChunkHandler::hasChunk() {
    handledChunksMutex.lock();
    bool empty = handledChunks.empty();
    handledChunksMutex.unlock();
    return !empty;
}

std::unique_ptr<Chunk> LocalChunkHandler::retrieveChunk() {
    handledChunksMutex.lock();
    auto chunk = std::move(handledChunks.front());
    handledChunks.pop();
    handledChunksMutex.unlock();
    return chunk;
}

void LocalChunkHandler::saveChunk(Chunk *chunk) {
    // TODO: Save chunk to file
}

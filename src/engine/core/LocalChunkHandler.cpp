#include "LocalChunkHandler.hpp"

#include "FlatChunkGenerator.hpp"
#include "World.hpp"

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
    saveChunk(chunk);
}

void LocalChunkHandler::removeChunk(World &world, ChunkIndex chunkInd) {
    // Nothing to do
}

void LocalChunkHandler::requestChunk(ChunkIndex chunkInd) {
    auto chunk = std::make_unique<Chunk>();
    chunk->index = chunkInd;
    chunkGenerator->fillChunk(*chunk);
    handledChunks.push(std::move(chunk));
}

bool LocalChunkHandler::hasChunk() {
    return !handledChunks.empty();
}

std::unique_ptr<Chunk> LocalChunkHandler::retrieveChunk() {
    auto chunk = std::move(handledChunks.front());
    handledChunks.pop();
    return chunk;
}

void LocalChunkHandler::saveChunk(const Chunk &chunk) {
    // TODO: Save chunk to file
}

#include "World.hpp"

#include "Chunk.hpp"
#include "ChunkRequestHandler.hpp"
#include "Entity.hpp"
#include "WorldListener.hpp"
#include "WorldTask.hpp"

void World::setCell(CellLoc loc, Cell cell) {
    ChunkIndex ind = Chunk::chunkIndexForCellLoc(loc);
    auto chunkIter = chunks.find(ind);
    if (chunkIter != chunks.end()) {
        chunkIter->second->setCell(loc, cell);
        for (auto listener: listeners) {
            listener->setCell(*this, loc, cell);
        }
    }
}

Cell World::getCell(CellLoc loc) {
    ChunkIndex ind = Chunk::chunkIndexForCellLoc(loc);
    auto chunkIter = chunks.find(ind);
    if (chunkIter != chunks.end()) {
        return chunkIter->second->getCell(loc);
    }
    return -1;
}

void World::addEntity(std::unique_ptr<Entity> entity) {
    entities.push_back(std::move(entity));
}

Chunk &World::getChunk(ChunkIndex chunkInd) {
    return *(chunks.find(chunkInd)->second);
}

void World::requestChunk(ChunkIndex chunkInd) {
    chunkRequestHandler->requestChunk(chunkInd);
}

void World::addChunk(std::unique_ptr<Chunk> chunk) {
    chunks[chunk->index] = std::move(chunk);
    for (auto listener: listeners) {
        listener->addChunk(*chunk);
    }
}

void World::removeChunk(ChunkIndex chunkInd) {
    for (auto listener: listeners) {
        listener->removeChunk(chunkInd);
    }
    chunks.erase(chunkInd);
}

void World::ensureChunks(const std::set<ChunkIndex> &chunkInds) {
    for (auto chunkInd: chunkInds) {
        if (chunks.find(chunkInd) == chunks.end()) {
            requestChunk(chunkInd);
        }
    }
    for (const auto &chunkIter: chunks) {
        if (chunkInds.find(chunkIter.first) == chunkInds.end()) {
            removeChunk(chunkIter.first);
        }
    }
}

void World::addWorldListener(std::shared_ptr<WorldListener> listener) {
    listeners.push_back(listener);
}

void World::addWorldTask(std::shared_ptr<WorldTask> task) {
    tasks.push_back(task);
}

void World::setChunkRequestHandler(std::shared_ptr<ChunkRequestHandler> handler) {
    chunkRequestHandler = handler;
}

void World::run() {
    while (!stop) {
        while (chunkRequestHandler->hasChunk()) {
            auto chunk = chunkRequestHandler->retrieveChunk();
            addChunk(std::move(chunk));
        }
        for (auto task: tasks) {
            task->executeTask(*this, 0.05f);
        }
    }
}

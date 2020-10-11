#include "World.hpp"

#include "Chunk.hpp"
#include "ChunkRequestHandler.hpp"
#include "Entity.hpp"
#include "WorldListener.hpp"
#include "WorldTask.hpp"

#include <thread>

void World::setCell(CellLoc loc, Cell cell) {
    ChunkIndex ind = Chunk::chunkIndexForCellLoc(loc);
    auto chunkIter = chunks.find(ind);
    if (chunkIter != chunks.end()) {
        chunkIter->second->setCell(Chunk::relativeCellLocForCellLoc(loc), cell);
        for (auto listener: listeners) {
            listener->setCell(*this, loc, cell);
        }
    }
}

Cell World::getCell(CellLoc loc) {
    Cell cell = 0;
    ChunkIndex ind = Chunk::chunkIndexForCellLoc(loc);
    auto chunkIter = chunks.find(ind);
    if (chunkIter != chunks.end()) {
        cell = chunkIter->second->getCell(Chunk::relativeCellLocForCellLoc(loc));
    }
    return cell;
}

void World::addEntity(std::unique_ptr<Entity> entity) {
    entities.push_back(std::move(entity));
}

Chunk *World::getChunk(ChunkIndex chunkInd) {
    // Chunk &chunk = *(chunks.find(chunkInd)->second);
    // return chunk;
    auto chunkIter = chunks.find(chunkInd);
    if (chunkIter != chunks.end()) {
        return chunkIter->second.get();
    }
    return nullptr;
}

void World::requestChunk(ChunkIndex chunkInd) {
    chunkRequestHandler->requestChunk(chunkInd);
}

void World::addChunk(std::unique_ptr<Chunk> chunk) {
    auto chunkIndex = chunk->index;
    auto chunkPtr = chunk.get();
    chunks[chunkIndex] = std::move(chunk);
    for (auto listener: listeners) {
        listener->addChunk(*this, *chunkPtr);
    }
}

void World::removeChunk(ChunkIndex chunkInd) {
    for (auto listener: listeners) {
        listener->removeChunk(*this, chunkInd);
    }
    chunks.erase(chunkInd);
}

void World::ensureChunks(const std::unordered_set<ChunkIndex> &chunkInds) {
    auto t1 = std::chrono::high_resolution_clock::now();
    for (auto chunkInd: chunkInds) {
        if (chunks.find(chunkInd) == chunks.end()) {
            requestChunk(chunkInd);
        }
    }
    std::vector<ChunkIndex> toRemove;
    for (const auto &chunkIter: chunks) {
        if (chunkInds.find(chunkIter.first) == chunkInds.end()) {
            toRemove.push_back(chunkIter.first);
        }
    }
    for (auto chunkIndex : toRemove) {
        removeChunk(chunkIndex);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::seconds::period>(t2 - t1).count();
    if (toRemove.size() > 0) {
        std::cout << "ensureChunks - " << dt << std::endl;
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

std::thread &World::run() {
    runThread = std::thread([](World *_this){
        auto startTime = std::chrono::high_resolution_clock::now();
        auto lastTime = startTime;

        while (!_this->stopped) {
            while (_this->chunkRequestHandler->hasChunk()) {
                auto chunk = _this->chunkRequestHandler->retrieveChunk();
                _this->addChunk(std::move(chunk));
            }
            auto currentTime = std::chrono::high_resolution_clock::now();
            float timeDelta = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;
            timeDelta = std::min(timeDelta, 0.1f);
            for (auto task: _this->tasks) {
                task->executeTask(*_this, timeDelta);
            }
        }
    }, this);
    return runThread;
}

void World::stop() {
    stopped = true;
}

CellLoc World::cellLocForWorldPos(WorldPos pos) {
    return CellLoc {
        static_cast<int>(pos.x),
        static_cast<int>(pos.y),
        static_cast<int>(pos.z),
        static_cast<int>(pos.u),
        static_cast<int>(pos.v),
   };
}

#pragma once

#include "types.hpp"

class Chunk;

class ChunkRequestHandler {
public:
    virtual ~ChunkRequestHandler() = default;
    virtual void requestChunk(ChunkIndex chunkInd) = 0;
    virtual bool hasChunk() = 0;
    virtual std::unique_ptr<Chunk> retrieveChunk() = 0;
};

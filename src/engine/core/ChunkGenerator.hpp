#pragma once

#include "Chunk.hpp"

class ChunkGenerator {
public:
    virtual ~ChunkGenerator() = default;
    virtual void fillChunk(Chunk &chunk) = 0;
};

#pragma once

#include "Chunk.hpp"

class WorldGenerator {
   public:
    virtual void fillChunk(Chunk &chunk) = 0;
};

#pragma once

#include "ChunkGenerator.hpp"

class FlatChunkGenerator : public ChunkGenerator {
public:
    void fillChunk(Chunk &chunk) override;
};

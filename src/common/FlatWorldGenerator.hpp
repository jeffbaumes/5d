#pragma once

#include "WorldGenerator.hpp"

class FlatWorldGenerator : public WorldGenerator {
   public:
	void fillChunk(Chunk &chunk);
};

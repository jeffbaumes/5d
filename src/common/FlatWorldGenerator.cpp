#include "FlatWorldGenerator.hpp"

void FlatWorldGenerator::fillChunk(Chunk &chunk) {
    for (int x = 0; x < CHUNK_SIZE_XZUV; x += 1) {
        for (int y = 0; y < CHUNK_SIZE_Y; y += 1) {
            for (int z = 0; z < CHUNK_SIZE_XZUV; z += 1) {
                for (int u = 0; u < CHUNK_SIZE_XZUV; u += 1) {
                    for (int v = 0; v < CHUNK_SIZE_XZUV; v += 1) {
                        int material = 0;
                        // if (y == 3) {
                        //     material = 3;
                        // }
                        // if (y == 2) {
                        //     material = 2;
                        // }
                        if (y == 0) {
                            material = 1;
                        }
                        chunk[{x, y, z, u, v}] = material;
                    }
                }
            }
        }
    }
}

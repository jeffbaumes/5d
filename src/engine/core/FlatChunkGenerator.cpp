#include "FlatChunkGenerator.hpp"

void FlatChunkGenerator::fillChunk(Chunk &chunk) {
    for (int x = 0; x < chunk.size.x; x += 1) {
        for (int y = 0; y < chunk.size.y; y += 1) {
            for (int z = 0; z < chunk.size.z; z += 1) {
                for (int u = 0; u < chunk.size.u; u += 1) {
                    for (int v = 0; v < chunk.size.v; v += 1) {
                        RelativeCellLoc loc {x, y, z, u, v};
                        Cell cell = 0;
                        if (y == 0) {
                            cell = 1;
                            if (x == 0 && z == 0) {
                                cell = 0;
                            }
                        }
                        chunk.setCell(loc, cell);
                    }
                }
            }
        }
    }
}

#include "Chunk.hpp"

Chunk::Chunk() {
    cells.resize(CHUNK_SIZE_XZUV * CHUNK_SIZE_Y * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV, 0);
}

Cell &Chunk::operator[](const RelativeCellLoc loc) {
    int size = CHUNK_SIZE_XZUV;
    return cells.at(loc.x + size * loc.y + size * CHUNK_SIZE_Y * loc.z + size * CHUNK_SIZE_Y * size * loc.u + size * CHUNK_SIZE_Y * size * size * loc.v);
}

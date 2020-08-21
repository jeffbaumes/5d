#include "Chunk.hpp"
#include "World.hpp"

Chunk::Chunk(World &w) : world(w) {
    cells.resize(CHUNK_SIZE_XZUV * CHUNK_SIZE_Y * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV, 0);
}

Cell Chunk::getCell(const RelativeCellLoc &loc) {
    int size = CHUNK_SIZE_XZUV;
    int index = loc.x + size * loc.y + size * CHUNK_SIZE_Y * loc.z + size * CHUNK_SIZE_Y * size * loc.u + size * CHUNK_SIZE_Y * size * size * loc.v;
    if (index < 0 || index > cells.size() - 1) {
        return 0;
    }
    return cells.at(index);
}

Cell &Chunk::operator[](const RelativeCellLoc &loc) {
    int size = CHUNK_SIZE_XZUV;
    return cells.at(loc.x + size * loc.y + size * CHUNK_SIZE_Y * loc.z + size * CHUNK_SIZE_Y * size * loc.u + size * CHUNK_SIZE_Y * size * size * loc.v);
}

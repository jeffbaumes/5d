#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include "vec5.hpp"

typedef int Cell;
const int CHUNK_SIZE_XZUV = 4;
const int CHUNK_SIZE_Y = 16;
const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;

struct Chunk {
    Chunk();
    std::vector<Cell> cells;
    Cell& operator[](const RelativeCellLoc loc);
};

#endif // CHUNK_HPP

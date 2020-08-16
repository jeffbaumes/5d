#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>

#include "vec5.hpp"

typedef int Cell;
typedef ivec5 CellLoc;
typedef ivec5 ChunkLoc;
typedef ivec5 RelativeCellLoc;
const int CHUNK_SIZE_XZUV = 4;
const int CHUNK_SIZE_Y = 16;
const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;

class Chunk {
   public:
    Chunk();
    std::vector<Cell> cells;
    Cell& operator[](const RelativeCellLoc loc);
    Cell getCell(const RelativeCellLoc loc);
};

#endif // CHUNK_HPP

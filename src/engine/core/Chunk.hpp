#pragma once

#include <memory>
#include <vector>

#include "types.hpp"

// const int CHUNK_SIZE_XZUV = 4;
// const int CHUNK_SIZE_Y = 16;
// const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;

struct Chunk {
    static ChunkSize size;
    static RelativeCellLoc relativeCellLocForCellLoc(CellLoc loc);
    static ChunkIndex chunkIndexForCellLoc(CellLoc loc);
    static CellLoc cellLocForRelativeCellLoc(RelativeCellLoc loc, ChunkIndex index);

    void allocateCells();
    void setCell(CellLoc loc, Cell cell);
    void setCell(RelativeCellLoc loc, Cell cell);
    Cell getCell(CellLoc loc);
    Cell getCell(RelativeCellLoc loc);

    ChunkIndex index = {};

private:
    int cellIndexForRelativeCellLoc(RelativeCellLoc loc);
    std::vector<Cell> cells;
};

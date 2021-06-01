#pragma once

#include <memory>
#include <vector>

#include "types.hpp"

// const int CHUNK_SIZE_XZUV = 4;
// const int CHUNK_SIZE_Y = 16;
// const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;

/**
 * Divide one integer by another, rounding towards minus infinity.
 * @param x the dividend
 * @param y the divisor
 * @return the quoitant, rounded towards minus infinity
 */
inline int div_floor(int x, int y) {
    int q = x / y;
    int r = x % y;
    if ((r != 0) && ((r < 0) != (y < 0))) --q;
    return q;
}

/**
 * Calculate the remainder after dividing one integer by another,
 * rounding the quoitant towards minus infinity.
 * @param x the dividend
 * @param y the divisor
 * @return the remainder
 */
inline int mod_floor(int x, int y) {
    int r = x % y;
    if ((r != 0) && ((r < 0) != (y < 0))) {
        r += y;
    }
    return r;
}

struct Chunk {
    static ChunkSize size;
    static RelativeCellLoc relativeCellLocForCellLoc(CellLoc loc) {
        return {
            mod_floor(loc.x, size.x),
            mod_floor(loc.y, size.y),
            mod_floor(loc.z, size.z),
            mod_floor(loc.u, size.u),
            mod_floor(loc.v, size.v),
        };
    }

    static ChunkIndex chunkIndexForCellLoc(CellLoc loc) {
        return {
            div_floor(loc.x, size.x),
            div_floor(loc.y, size.y),
            div_floor(loc.z, size.z),
            div_floor(loc.u, size.u),
            div_floor(loc.v, size.v),
        };
    }

    static CellLoc cellLocForRelativeCellLoc(RelativeCellLoc loc, ChunkIndex index) {
        return {
            index.x * size.x + loc.x,
            index.y * size.y + loc.y,
            index.z * size.z + loc.z,
            index.u * size.u + loc.u,
            index.v * size.v + loc.v,
        };
    }

    Chunk();
    // void setCell(CellLoc loc, Cell cell);
    void setCell(RelativeCellLoc loc, Cell cell);
    // Cell getCell(CellLoc loc);
    Cell getCell(RelativeCellLoc loc);

    ChunkIndex index = {};

    int cellIndexForRelativeCellLoc(RelativeCellLoc loc);
    std::vector<Cell> cells;
};

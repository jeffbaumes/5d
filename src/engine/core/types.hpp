#pragma once

#include "vector5.hpp"

typedef uint64_t Cell;
typedef vector5<int> ivec5;
typedef vector5<float> vec5;
// typedef std::array<int, 6> SideIndex;

vec5 normalize(vec5 a);
vec5 project(vec5 a, vec5 b);
vec5 round(vec5 a);
vec5 floor(vec5 a);

class CellLoc : public ivec5 { };
class ChunkIndex : public ivec5 { };
class ChunkSize : public ivec5 { };
class RelativeCellLoc : public ivec5 { };
class WorldPos : public vec5 { };

namespace std {
template <>
struct hash<ChunkIndex> {
    size_t operator()(ChunkIndex const &chunkInd) const {
        size_t h = chunkInd.x ^ (chunkInd.y << 1);
        h = h ^ (chunkInd.z << 1);
        h = h ^ (chunkInd.u << 1);
        h = h ^ (chunkInd.v << 1);
        return h;
    }
};
}

struct SurroundingCells {
    Cell cell;
    CellLoc loc;

    CellWithLoc negativeX;
    CellWithLoc negativeU;
    CellWithLoc negativeZ;
    CellWithLoc negativeV;
    CellWithLoc negativeY;

    CellWithLoc positiveX;
    CellWithLoc positiveU;
    CellWithLoc positiveZ;
    CellWithLoc positiveV;
    CellWithLoc positiveY;

    SurroundingCells(World &world, CellLoc loc);
};

struct CellWithLoc {
    Cell cell;
    CellLoc loc;

    CellWithLoc() = default;
    CellWithLoc(Cell cell, CellLoc loc);
    CellWithLoc(World &world, CellLoc loc);
};
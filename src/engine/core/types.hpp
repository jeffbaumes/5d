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

// From https://www.quora.com/How-can-I-declare-an-unordered-set-of-pair-of-int-int-in-C++11
template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
template <>
struct hash<ChunkIndex> {
    size_t operator()(ChunkIndex const &chunkInd) const {
        size_t seed = 0;
        ::hash_combine(seed, chunkInd.x);
        ::hash_combine(seed, chunkInd.y);
        ::hash_combine(seed, chunkInd.z);
        ::hash_combine(seed, chunkInd.u);
        ::hash_combine(seed, chunkInd.v);
        return seed;
    }
};
}

#include "Chunk.hpp"

// ChunkSize Chunk::size = {4, 16, 4, 4, 4};
ChunkSize Chunk::size = {16, 256, 16, 1, 1};
// ChunkSize Chunk::size = {16, 16, 256, 1, 1};

// RelativeCellLoc Chunk::relativeCellLocForCellLoc(CellLoc loc) {
//     return {
//         mod_floor(loc.x, size.x),
//         mod_floor(loc.y, size.y),
//         mod_floor(loc.z, size.z),
//         mod_floor(loc.u, size.u),
//         mod_floor(loc.v, size.v),
//     };
// }

// ChunkIndex Chunk::chunkIndexForCellLoc(CellLoc loc) {
//     return {
//         div_floor(loc.x, size.x),
//         div_floor(loc.y, size.y),
//         div_floor(loc.z, size.z),
//         div_floor(loc.u, size.u),
//         div_floor(loc.v, size.v),
//     };
// }

// CellLoc Chunk::cellLocForRelativeCellLoc(RelativeCellLoc loc, ChunkIndex index) {
//     return {
//         index.x * size.x + loc.x,
//         index.y * size.y + loc.y,
//         index.z * size.z + loc.z,
//         index.u * size.u + loc.u,
//         index.v * size.v + loc.v,
//     };
// }


Chunk::Chunk() {
    cells.resize(size.x * size.y * size.z * size.u * size.v, 0);
}

// void Chunk::setCell(CellLoc loc, Cell cell) {
//     setCell(relativeCellLocForCellLoc(loc), cell);
// }

void Chunk::setCell(RelativeCellLoc loc, Cell cell) {
    int index = cellIndexForRelativeCellLoc(loc);
    if (index >= 0 && index < cells.size()) {
        cells[index] = cell;
    }
}

// Cell Chunk::getCell(CellLoc loc) {
//     return getCell(relativeCellLocForCellLoc(loc));
// }

Cell Chunk::getCell(RelativeCellLoc loc) {
    int index = cellIndexForRelativeCellLoc(loc);
    if (index >= 0 && index < cells.size()) {
        return cells[index];
    }
    return UNLOADED;
}

int Chunk::cellIndexForRelativeCellLoc(RelativeCellLoc loc) {
    return loc.x
        + size.x * loc.y
        + size.x * size.y * loc.z
        + size.x * size.y * size.z * loc.u
        + size.x * size.y * size.z * size.u * loc.v;
}

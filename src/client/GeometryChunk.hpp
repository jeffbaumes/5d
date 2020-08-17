#ifndef D5_GEOMETRYCHUNK_H
#define D5_GEOMETRYCHUNK_H
#include <map>
#include <vector>

#include "../common/Chunk.hpp"
#include "VulkanUtil.hpp"
typedef std::array<int, 6> SideIndex;

class GeometryChunk: public Chunk {
   public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    void setCell(RelativeCellLoc loc, Cell cellData);
    void generateGeometry();

   private:
    std::vector<size_t> emptySideIndices;
    std::vector<size_t> emptySideVertices;
    std::map<SideIndex, size_t> sideIndices;
    std::map<SideIndex, size_t> sideVertices;
    std::vector<size_t> changedIndices;
    std::vector<size_t> changedVertices;
    CellLoc getCellLocFrom(RelativeCellLoc relCellLoc, ChunkLoc chunkLoc);
    void createSide(RelativeCellLoc loc, int side, Cell cellData = -1);
    void removeSide(RelativeCellLoc loc, int side);
};

#endif

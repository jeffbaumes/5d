#ifndef D5_GEOMETRYCHUNK_H
#define D5_GEOMETRYCHUNK_H
#include <map>
#include <vector>

#include "../common/Chunk.hpp"
#include "VulkanUtil.hpp"
typedef std::array<int, 6> SideIndex;

class World;

class GeometryChunk: public Chunk {
   public:
    GeometryChunk(World &w);
    std::vector<Vertex> vertices;
    void setCell(const RelativeCellLoc &loc, Cell cellData);
    void generateGeometry();
    void createSide(const RelativeCellLoc &loc, int side, Cell cellData = -1);
    void removeSide(const RelativeCellLoc &loc, int side);

   private:
    World &world;
    std::vector<size_t> emptySideVertices;
    std::map<SideIndex, size_t> sideVertices;
    std::vector<size_t> changedVertices;
    CellLoc getCellLocFrom(const RelativeCellLoc &relCellLoc, ChunkLoc chunkLoc);
    Cell getWorldCell(const RelativeCellLoc &loc);
    void createWorldSide(const RelativeCellLoc &loc, int side, Cell cellData = -1);
    void removeWorldSide(const RelativeCellLoc &loc, int side);
};

#endif

#pragma once

#include <vector>
#include "../core/glm_include.hpp"
#include "../core/types.hpp"
#include "VulkanRenderer.hpp"

enum Side {
    POS_XU = 1,
    NEG_XU = -1,
    POS_Y = 2,
    NEG_Y = -2,
    POS_ZV = 3,
    NEG_ZV = -3,
};

class SideCreator {
public:
    void createSide(CellLoc loc, Side side, Cell cell);
private:
    UnfinishedVertex unfinishedVertex;
    std::vector<int> order;
    SideIndex sideIndex;
    int verticesIndex;
    void createPosXUSide(CellLoc loc, Cell cellData);
    void createPosZVSide(CellLoc loc, Cell cellData);
    void createPosYSide(CellLoc loc, Cell cellData);
    void createNegXUSide(CellLoc loc, Cell cellData);
    void createNegZVSide(CellLoc loc, Cell cellData);
    void createNegYSide(CellLoc loc, Cell cellData);
    UnfinishedVertex getUnfinishedVertexForSide(SideIndex sideIndex, Cell cellData);
    int getVertexLocationForSideAndAllocateRoomInVertices(CellLoc cellLoc);
};

struct UnfinishedVertex {
    glm::i16vec3 xyz;
    glm::i16vec2 uv;
    uint16_t sp;

    Vertex toVertex(int spRest);
};

struct SideIndex {
    RelativeCellLoc cellLoc;
    int side;
};

namespace std {
template <>
struct hash<SideIndex> {
    size_t operator()(SideIndex const &sideIndex) const {
        RelativeCellLoc cellLoc = sideIndex.cellLoc;
        int side = sideIndex.side;
        size_t h = cellLoc.x ^ (cellLoc.y << 1);
        h = h ^ (cellLoc.z << 1);
        h = h ^ (cellLoc.u << 1);
        h = h ^ (cellLoc.v << 1);
        h = h ^ (side << 1);
        return h;
    }
};
}  // namespace std
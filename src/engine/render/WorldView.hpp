#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "../core/types.hpp"
#include "../core/WorldListener.hpp"
#include "../core/WorldTask.hpp"
#include "VulkanRenderer.hpp"

class Entity;
class World;

enum CELLTYPE {
    AIR,
};

enum SIDE {
    POS_XU = 1,
    NEG_XU = -1,
    POS_Y = 2,
    NEG_Y = -2,
    POS_ZV = 3,
    NEG_ZV = -3,
};

struct UnfinishedVertex {
    glm::i16vec3 xyz;
    glm::i16vec2 uv;
    uint16_t sp;

    Vertex toVertex(int side, int corner);
};

struct SideIndex {
    RelativeCellLoc cellLoc = {0, 0, 0, 0, 0};
    int side = 0;

    friend bool operator==(const SideIndex &a, const SideIndex &b) {
        return a.cellLoc == b.cellLoc && a.side == b.side;
    }
};

struct CellWithLoc {
    Cell cell = 0;
    CellLoc loc = {0, 0, 0, 0, 0};

    CellWithLoc() = default;
    CellWithLoc(Cell cell, CellLoc loc);
    CellWithLoc(World &world, CellLoc loc);
};

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

namespace std {
template <>
struct hash<SideIndex> {
    size_t operator()(SideIndex const &sideIndex) const {
        size_t h = sideIndex.cellLoc.x ^ (sideIndex.cellLoc.y << 1);
        h = h ^ (sideIndex.cellLoc.z << 1);
        h = h ^ (sideIndex.cellLoc.u << 1);
        h = h ^ (sideIndex.cellLoc.v << 1);
        h = h ^ (sideIndex.side << 1);
        // std::cout << "Here" << h << std::endl
        //           << std::flush;
        return h;
    }
};
}  // namespace std

struct GeometryChunk {
    std::vector<Vertex> vertices;
    std::vector<size_t> emptySideSlotIndices;
    std::unordered_map<SideIndex, size_t> sideIndices;
    std::vector<size_t> changedVertices;
};

class WorldView : public WorldListener, public WorldTask {
public:
    WorldView(std::vector<const char *> extensions);

    VkInstance getInstance();
    void initSurface(VkSurfaceKHR surface);

    // WorldListener
    void setCell(World &world, CellLoc loc, Cell cell) override;
    void updateEntity(World &world, Entity &entity, WorldPos pos) override;
    void addChunk(World &world, Chunk &chunk) override;
    void removeChunk(World &world, ChunkIndex chunkInd) override;

    // WorldTask
    void executeTask(World &world, float timeDelta) override;

    void setRenderDistanceXZ(float distance);
    void setRenderDistanceUV(float distance);
    void setCameraPosition(WorldPos pos);
    void setCameraLookAt(WorldPos pos);
    void setCameraViewAngle(float viewAngle);
    void setCameraUVView(float uvView);
    float getCameraUVView();
private:
    void createPosXUSide(const CellLoc &loc, const Cell &cell);
    void createPosZVSide(const CellLoc &loc, const Cell &cell);
    void createPosYSide(const CellLoc &loc, const Cell &cell);
    void createNegXUSide(const CellLoc &loc, const Cell &cell);
    void createNegZVSide(const CellLoc &loc, const Cell &cell);
    void createNegYSide(const CellLoc &loc, const Cell &cell);
    int sideSetup(GeometryChunk *chunk, const CellLoc &loc, const Cell &cell);
    UnfinishedVertex getUnfinishedVertex(const CellLoc &loc, const Cell &cell);
    // int getVertexLocationForSideAndAllocateRoomInVertices(CellLoc cellLoc);
    // void addSideVertices(std::vector<int> order, uint16_t packedInfo, glm::vec3 xyz, glm::vec2 uv);

    void removeSide(CellLoc loc, int side);

    void visibleChunkIndices(std::set<ChunkIndex> &chunkIndices);
    void updateUniforms(float timeDelta);

    float renderDistanceXZ = 20.0f;
    float renderDistanceUV = 10.0f;
    WorldPos cameraPosition = {2, 5, 2, 0.5, 0.5};
    WorldPos cameraLookAt = {3, 0, 3, 0.5, 0.5};
    float cameraViewAngle = 45.0;
    float cameraUVView = 0.0f;
    float cameraUVViewTween = 0.0f;
    float tweenTime = 2.0f;
    VulkanRenderer renderer;
    std::unordered_map<ChunkIndex, std::unique_ptr<GeometryChunk>> chunks;
};

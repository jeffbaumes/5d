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
private:
    void createSide(CellLoc loc, int side, Cell cell);
    void createPosXUSide(CellLoc loc, Cell cellData);
    void createPosZVSide(CellLoc loc, Cell cellData);
    void createPosYSide(CellLoc loc, Cell cellData);
    void createNegXUSide(CellLoc loc, Cell cellData);
    void createNegZVSide(CellLoc loc, Cell cellData);
    void createNegYSide(CellLoc loc, Cell cellData);
    UnfinishedVertex getUnfinishedVertexForSide(SideIndex sideIndex, Cell cellData);
    int getVertexLocationForSideAndAllocateRoomInVertices(CellLoc cellLoc);
    // void addSideVertices(std::vector<int> order, uint16_t packedInfo, glm::vec3 xyz, glm::vec2 uv);

    void removeSide(CellLoc loc, int side);
    void removePosXUSide(CellLoc loc, Cell cellData);
    void removePosZVSide(CellLoc loc, Cell cellData);
    void removePosYSide(CellLoc loc, Cell cellData);
    void removeNegXUSide(CellLoc loc, Cell cellData);
    void removeNegZVSide(CellLoc loc, Cell cellData);
    void removeNegYSide(CellLoc loc, Cell cellData);
    
    void visibleChunkIndices(std::set<ChunkIndex> &chunkIndices);
    void updateUniforms(float timeDelta);

    float renderDistanceXZ = 20.0f;
    float renderDistanceUV = 10.0f;
    WorldPos cameraPosition = {0, 0, 5, 0, 0};
    WorldPos cameraLookAt = {1, 0, 5, 0, 0};
    float cameraViewAngle = 45.0;
    float cameraUVView = 0.0f;
    float cameraUVViewTween = 0.0f;
    float tweenTime = 2.0f;
    VulkanRenderer renderer;
    std::unordered_map<ChunkIndex, std::shared_ptr<GeometryChunk>> chunks;
};

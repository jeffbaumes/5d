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
    void removeSide(CellLoc loc, int side);
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
    // std::unordered_map<ChunkIndex, std::vector<size_t>> allocations;
    // std::unordered_map<ChunkIndex, std::vector<Vertex>> vertices;
    // std::unordered_map<ChunkIndex, std::vector<size_t>> emptySides;
    // std::unordered_map<ChunkIndex, std::unordered_map<SideIndex, size_t>> sides;
    // std::unordered_map<ChunkIndex, std::vector<size_t>> changedVertices;
};

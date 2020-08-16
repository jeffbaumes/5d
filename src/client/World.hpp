#ifndef D5_WORLD_H_
#define D5_WORLD_H_

#include <vector>
#include <map>
// #include <sqlite3.h>

#include "../common/Chunk.hpp"
#include "VulkanUtil.hpp"
#include "Entity.hpp"
#include "../common/vec5.hpp"
#include "GeometryChunk.hpp"

const int CHUNK_BLOCK_SIZE = 4096;

struct ChunkNotLoadedException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Chunk is not loaded";
    }
};


const int TEX_WIDTH = 3;
const int MAX_INDIVIDUAL_CHANGES = 50;

typedef std::array<int, 6> SideIndex;

namespace std {
template <>
struct hash<ChunkLoc> {
    size_t operator()(ChunkLoc const &chunkLoc) const {
        size_t h1 = (chunkLoc.x);
        size_t h2 = (chunkLoc.y);
        size_t h3 = (chunkLoc.z);
        size_t h4 = (chunkLoc.u);
        size_t h5 = (chunkLoc.v);

        size_t h = h1 ^ (h2 << 1);
        h = h ^ (h3 << 1);
        h = h ^ (h4 << 1);
        h = h ^ (h5 << 1);

        return h;
    }
};
}

class WorldClient;

class World {
   public:
    const std::vector<uint32_t> EMPTY_INDICES_CHUNK_BLOCK;
    const std::vector<Vertex> EMPTY_VERTICES_CHUNK_BLOCK;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    int indicesIndex = 0;
    int verticesIndex = 0;

    std::vector<size_t> changedIndices;
    std::vector<size_t> changedVertices;

    std::vector<Entity> entities;
    std::vector<int> unusedEntityIDS;

    World(VulkanUtil *vulkan);
    World(VulkanUtil *vulkan, WorldClient *client);
    // World(VulkanUtil *vulkan, std::string dirname);
    ~World();

    void init();

    void pollEvents();

    Cell getCell(CellLoc loc);
    Cell getCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc);
    void setCell(CellLoc loc, Cell cellData);

    ChunkLoc chunkLocForCell(CellLoc loc);

    void loadChunk(ChunkLoc loc);
    void unloadChunk(ChunkLoc loc);
    // void saveChunk(ChunkLoc loc);
    // void generateChunk(ChunkLoc loc);

    void updateVulkan();

    void printStats();

    void updateUBO(UniformBufferObject *ubo);

   private:
    VulkanUtil *vulkan;
    WorldClient *client = nullptr;
    bool running = false;
    std::unordered_map<ChunkLoc, GeometryChunk> chunks;
    std::unordered_map<ChunkLoc, std::vector<uint>> chunkIndicesIndices;
    std::unordered_map<ChunkLoc, std::vector<uint>> chunkVerticesIndices;
    std::map<SideIndex, size_t> chunkIndices;
    std::map<SideIndex, size_t> chunkVertices;
    std::vector<size_t> emptyChunkIndices;
    std::vector<size_t> emptyChunkVertices;
    std::string dirname;



    void destroy();

    void writeFile(int startLoc, void* data, int size);
    void* readFile(int startLoc, int size);

};

#endif

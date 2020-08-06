#ifndef D5_WORLD_H_
#define D5_WORLD_H_

#include <vector>
#include <map>
#include <sqlite3.h>

#include "VulkanUtil.hpp"
#include "Entity.hpp"
#include "vec5.hpp"


struct ChunkNotLoadedException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Chunk is not loaded";
    }
};


const int CHUNK_SIZE_XZUV = 4;
const int CHUNK_SIZE_Y = 16;
const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;
const int TEX_WIDTH = 2;
const int MAX_INDIVIDUAL_CHANGES = 50;

struct CellLoc {
    CellLoc();
    CellLoc(vec5 loc);
    CellLoc(glm::vec3 xyz, glm::vec2 uv);
    CellLoc(int x, int y, int z, int u, int v);
    int x = 0;
    int y = 0;
    int z = 0;
    int u = 0;
    int v = 0;
    void print() {
        std::cerr << x << "," << y << "," << z << "," << u << "," << v << " " << std::endl;
    }
};

struct ChunkLoc {
    int x;
    int y;
    int z;
    int u;
    int v;

    bool operator==(const ChunkLoc& other) const;
};

typedef std::array<int, 6> SideIndex;

typedef int Cell;

typedef CellLoc RelativeCellLoc;

struct Chunk {
    Chunk();
    std::vector<Cell> cells;
    Cell & operator[](const RelativeCellLoc loc);
};

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

class World {
   public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    int indicesIndex = 0;
    int verticesIndex = 0;

    std::vector<size_t> changedIndices;
    std::vector<size_t> changedVertices;

    std::vector<Entity> entities;
    std::vector<int> unusedEntityIDS;

    World(VulkanUtil *vulkan);
    World(VulkanUtil *vulkan, std::string dirname);
    ~World();

    void init();

    Cell getCell(CellLoc loc);
    Cell getCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc);
    void setCell(CellLoc loc, Cell cellData);
    void setCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc, Cell cellData, bool sendVertices);

    ChunkLoc chunkLocForCell(CellLoc loc);

    void loadChunk(ChunkLoc loc);
    void unloadChunk(ChunkLoc loc);
    void saveChunk(ChunkLoc loc);
    void generateChunk(ChunkLoc loc);

    void sendVerticesAndIndicesToVulkan();

    void printStats();

    void updateUBO(UniformBufferObject *ubo);


    void createSide(CellLoc loc, int side, Cell cellData = -1);
    void removeSide(CellLoc loc, int side);

   private:
    VulkanUtil *vulkan;
    bool running = false;
    std::unordered_map<ChunkLoc, Chunk> chunks;
    std::map<SideIndex, size_t> sideIndices;
    std::map<SideIndex, size_t> sideVertices;
    std::vector<size_t> emptySideIndices;
    std::vector<size_t> emptySideVertices;
    std::string dirname;



    void destroy();

    void writeFile(int startLoc, void* data, int size);
    void* readFile(int startLoc, int size);

};

#endif

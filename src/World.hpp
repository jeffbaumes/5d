#ifndef D5_WORLD_H_
#define D5_WORLD_H_

#include <vector>
#include <map>
#include <sqlite3.h> 

#include "VulkanUtil.hpp"



struct ChunkNotLoadedException : public std::exception
{
	const char * what () const throw ()
    {
    	return "Chunk is not loaded";
    }
};


const int CHUNK_SIZE_XZUV = 4;
const int CHUNK_SIZE_Y = 8;
const int CHUNK_SIZE = CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_XZUV * CHUNK_SIZE_Y;
const int TEX_WIDTH = 2;

struct CellLoc {
    int x;
    int y;
    int z;
    int u;
    int v;
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
    std::vector<Cell> cells;
    std::vector<int> squareLocations;
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

    Cell getCell(CellLoc loc);
    Cell getCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc);

    void setCell(CellLoc loc, Cell cellData);

    void setCellInChunk(ChunkLoc chunkLoc, RelativeCellLoc loc, Cell cellData);

    void loadChunk(ChunkLoc loc);

    void unloadChunk(ChunkLoc loc);

    void saveChunk(ChunkLoc loc);

    void generateChunk(ChunkLoc loc);

    World(VulkanUtil vulkan);

    ~World();

    World(VulkanUtil vulkan, std::string dirname);

   private:
    VulkanUtil vulkan;
    bool running = false;
    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    std::unordered_map<ChunkLoc, Chunk> chunks;
    std::map<SideIndex, size_t> sideIndices;
    std::string dirname;

    int indicesIndex;
    int verticesIndex;

    void init();
    void destroy();

    void writeFile(int startLoc, void* data, int size);
    void* readFile(int startLoc, int size);

    void createSide(CellLoc loc, int side);
    void removeSide(CellLoc loc, int side);

    void addVertex(const Vertex &vertex);

    void sendVerticesAndIndicesToVulkan();

    void filterVertexArrayWithinChunk(ChunkLoc loc);

};

#endif

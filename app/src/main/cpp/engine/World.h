#pragma once
#include "Chunk.h"
#include "Shader.h"
#include <unordered_map>
#include <memory>
#include <cstdint>

struct ChunkKey {
    int x, z;
    bool operator==(const ChunkKey& o) const { return x == o.x && z == o.z; }
};
struct ChunkKeyHash {
    size_t operator()(const ChunkKey& k) const {
        return std::hash<int64_t>()((int64_t)k.x << 32 | (uint32_t)k.z);
    }
};

class World {
public:
    int renderDistance = 4;

    World();
    void update(float camX, float camZ);
    void render(const Shader& shader);
    void setRenderDistance(int d) { renderDistance = d; }

    BlockType getBlock(int wx, int wy, int wz) const;

private:
    std::unordered_map<ChunkKey, std::unique_ptr<Chunk>, ChunkKeyHash> chunks;

    Chunk* getChunk(int cx, int cz) const;
    void loadChunk(int cx, int cz);
    void generateChunk(Chunk& chunk);
    float terrainHeight(int wx, int wz) const;
    void plantTree(Chunk& chunk, int lx, int y, int lz);
};

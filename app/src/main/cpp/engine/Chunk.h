#pragma once
#include "Block.h"
#include <GLES3/gl3.h>
#include <vector>
#include <array>

static constexpr int CHUNK_SIZE = 16;
static constexpr int CHUNK_HEIGHT = 64;

class Chunk {
public:
    int chunkX = 0, chunkZ = 0;
    bool dirty = true;
    bool uploaded = false;

    std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE> blocks{};

    GLuint vao = 0, vbo = 0;
    int vertexCount = 0;

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType t);

    // Build mesh — needs neighbor chunks for face culling at edges
    void buildMesh(const Chunk* north, const Chunk* south,
                   const Chunk* east,  const Chunk* west);
    void uploadMesh(std::vector<float>& vertices);
    void destroy();

private:
    void addFace(std::vector<float>& verts,
                 float bx, float by, float bz,
                 int face, BlockType type);
};

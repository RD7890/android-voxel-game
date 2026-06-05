#include "Chunk.h"
#include <GLES3/gl3.h>
#include <cstring>

// Light multipliers per face (top, bottom, north, south, east, west)
static constexpr float FACE_LIGHT[6] = {1.0f, 0.50f, 0.72f, 0.72f, 0.86f, 0.86f};

// Face vertex offsets [6 faces][6 verts][3 coords]
// Each face = 2 triangles, wound CCW from outside
static const float FACE_VERTS[6][6][3] = {
    // TOP (y+1)
    {{0,1,0},{1,1,0},{1,1,1},{0,1,0},{1,1,1},{0,1,1}},
    // BOTTOM (y-0)
    {{0,0,1},{1,0,1},{1,0,0},{0,0,1},{1,0,0},{0,0,0}},
    // NORTH (z-0)
    {{1,1,0},{0,1,0},{0,0,0},{1,1,0},{0,0,0},{1,0,0}},
    // SOUTH (z+1)
    {{0,1,1},{1,1,1},{1,0,1},{0,1,1},{1,0,1},{0,0,1}},
    // EAST (x+1)
    {{1,1,0},{1,1,1},{1,0,1},{1,1,0},{1,0,1},{1,0,0}},
    // WEST (x-0)
    {{0,1,1},{0,1,0},{0,0,0},{0,1,1},{0,0,0},{0,0,1}},
};

static const int NEIGHBOR_DX[6] = { 0, 0, 0, 0, 1,-1};
static const int NEIGHBOR_DY[6] = { 1,-1, 0, 0, 0, 0};
static const int NEIGHBOR_DZ[6] = { 0, 0,-1, 1, 0, 0};

static inline int idx(int x, int y, int z) {
    return x * CHUNK_HEIGHT * CHUNK_SIZE + y * CHUNK_SIZE + z;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return BlockType::AIR;
    return blocks[idx(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, BlockType t) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE) return;
    blocks[idx(x, y, z)] = t;
    dirty = true;
}

void Chunk::addFace(std::vector<float>& verts,
                    float bx, float by, float bz,
                    int face, BlockType type) {
    BlockColor col = getBlockColor(type, face);
    float light = FACE_LIGHT[face];
    for (int v = 0; v < 6; v++) {
        verts.push_back(bx + FACE_VERTS[face][v][0]);
        verts.push_back(by + FACE_VERTS[face][v][1]);
        verts.push_back(bz + FACE_VERTS[face][v][2]);
        verts.push_back(col.r * light);
        verts.push_back(col.g * light);
        verts.push_back(col.b * light);
    }
}

void Chunk::buildMesh(const Chunk* north, const Chunk* south,
                      const Chunk* east,  const Chunk* west) {
    std::vector<float> verts;
    verts.reserve(8192);

    float ox = chunkX * CHUNK_SIZE;
    float oz = chunkZ * CHUNK_SIZE;

    for (int x = 0; x < CHUNK_SIZE; x++)
    for (int y = 0; y < CHUNK_HEIGHT; y++)
    for (int z = 0; z < CHUNK_SIZE; z++) {
        BlockType bt = getBlock(x, y, z);
        if (bt == BlockType::AIR) continue;

        // Check each of the 6 neighbors
        for (int f = 0; f < 6; f++) {
            int nx = x + NEIGHBOR_DX[f];
            int ny = y + NEIGHBOR_DY[f];
            int nz = z + NEIGHBOR_DZ[f];

            BlockType neighbor = BlockType::AIR;

            // Out-of-chunk neighbors: ask adjacent chunks
            if (nx < 0 && west)        neighbor = west->getBlock(CHUNK_SIZE-1, ny, nz);
            else if (nx >= CHUNK_SIZE && east) neighbor = east->getBlock(0, ny, nz);
            else if (nz < 0 && north)  neighbor = north->getBlock(nx, ny, CHUNK_SIZE-1);
            else if (nz >= CHUNK_SIZE && south) neighbor = south->getBlock(nx, ny, 0);
            else                       neighbor = getBlock(nx, ny, nz);

            bool showFace = (neighbor == BlockType::AIR) ||
                            (bt != BlockType::WATER && neighbor == BlockType::WATER) ||
                            (bt != BlockType::LEAVES && neighbor == BlockType::LEAVES);

            if (showFace) addFace(verts, ox + x, (float)y, oz + z, f, bt);
        }
    }

    uploadMesh(verts);
    dirty = false;
}

void Chunk::uploadMesh(std::vector<float>& vertices) {
    vertexCount = (int)(vertices.size() / 6);
    if (vertexCount == 0) { uploaded = true; return; }

    if (!vao) { glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // position: loc 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color: loc 1
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    uploaded = true;
}

void Chunk::destroy() {
    if (vao) { glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo); vao = vbo = 0; }
}

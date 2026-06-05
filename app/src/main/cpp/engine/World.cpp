#include "World.h"
#include <cmath>
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"MCME",__VA_ARGS__)

World::World() {}

static float fract(float x) { return x - floorf(x); }
static float hash(float n)  { return fract(sinf(n) * 43758.5453f); }
static float smoothNoise(float x, float z) {
    float ix = floorf(x), iz = floorf(z);
    float fx = fract(x),  fz = fract(z);
    fx = fx*fx*(3-2*fx); fz = fz*fz*(3-2*fz);
    float a = hash(ix + iz*57);
    float b = hash(ix+1 + iz*57);
    float c = hash(ix + (iz+1)*57);
    float d = hash(ix+1 + (iz+1)*57);
    return a + (b-a)*fx + (c-a)*fz + (a-b-c+d)*fx*fz;
}

float World::terrainHeight(int wx, int wz) const {
    float x = wx * 0.05f, z = wz * 0.05f;
    float h = 14.0f;
    h += 12.0f * smoothNoise(x * 1.0f,  z * 1.0f);
    h +=  6.0f * smoothNoise(x * 2.3f + 1.7f, z * 2.3f + 0.9f);
    h +=  3.0f * smoothNoise(x * 4.7f + 3.1f, z * 4.7f + 2.3f);
    return h;
}

void World::plantTree(Chunk& chunk, int lx, int y, int lz) {
    int trunkH = 4 + (int)(hash((float)(lx + lz * 13)) * 2);
    for (int i = 0; i < trunkH; i++) chunk.setBlock(lx, y + i, lz, BlockType::WOOD);
    int top = y + trunkH;
    for (int dx = -2; dx <= 2; dx++)
    for (int dz = -2; dz <= 2; dz++)
    for (int dy = -1; dy <= 2; dy++) {
        if (abs(dx)==2 && abs(dz)==2) continue;
        int bx = lx+dx, by = top+dy, bz = lz+dz;
        if (bx>=0&&bx<CHUNK_SIZE&&by>=0&&by<CHUNK_HEIGHT&&bz>=0&&bz<CHUNK_SIZE)
            if (chunk.getBlock(bx,by,bz)==BlockType::AIR)
                chunk.setBlock(bx,by,bz,BlockType::LEAVES);
    }
}

void World::generateChunk(Chunk& chunk) {
    int worldOffX = chunk.chunkX * CHUNK_SIZE;
    int worldOffZ = chunk.chunkZ * CHUNK_SIZE;
    static constexpr int SEA_LEVEL = 18;

    for (int x = 0; x < CHUNK_SIZE; x++)
    for (int z = 0; z < CHUNK_SIZE; z++) {
        int h = (int)terrainHeight(worldOffX + x, worldOffZ + z);
        if (h < 2) h = 2;
        if (h >= CHUNK_HEIGHT - 4) h = CHUNK_HEIGHT - 4;

        chunk.setBlock(x, 0, z, BlockType::BEDROCK);

        int stoneTop = h - 4;
        if (stoneTop < 1) stoneTop = 1;
        for (int y = 1; y < stoneTop; y++) chunk.setBlock(x, y, z, BlockType::STONE);
        for (int y = stoneTop; y < h; y++) chunk.setBlock(x, y, z, BlockType::DIRT);

        if (h <= SEA_LEVEL) {
            chunk.setBlock(x, h, z, BlockType::SAND);
            for (int y = h + 1; y <= SEA_LEVEL; y++) chunk.setBlock(x, y, z, BlockType::WATER);
        } else {
            chunk.setBlock(x, h, z, BlockType::GRASS);
            float r = hash((float)((worldOffX+x)*31 + (worldOffZ+z)*17));
            if (r > 0.93f && x >= 2 && x <= 13 && z >= 2 && z <= 13)
                plantTree(chunk, x, h + 1, z);
        }
    }
}

Chunk* World::getChunk(int cx, int cz) const {
    auto it = chunks.find({cx, cz});
    return it != chunks.end() ? it->second.get() : nullptr;
}

void World::loadChunk(int cx, int cz) {
    if (getChunk(cx, cz)) return;
    auto chunk = std::make_unique<Chunk>();
    chunk->chunkX = cx; chunk->chunkZ = cz;
    generateChunk(*chunk);
    chunks[{cx, cz}] = std::move(chunk);
}

void World::update(float camX, float camZ) {
    int cx = (int)floorf(camX / CHUNK_SIZE);
    int cz = (int)floorf(camZ / CHUNK_SIZE);
    int rd = renderDistance;
    for (int dx = -rd; dx <= rd; dx++)
    for (int dz = -rd; dz <= rd; dz++)
        loadChunk(cx + dx, cz + dz);

    for (auto& [key, chunk] : chunks) {
        if (!chunk->dirty) continue;
        chunk->buildMesh(
            getChunk(key.x, key.z - 1),
            getChunk(key.x, key.z + 1),
            getChunk(key.x + 1, key.z),
            getChunk(key.x - 1, key.z)
        );
    }
}

void World::render(const Shader& shader) {
    for (auto& [key, chunk] : chunks) {
        if (!chunk->uploaded || chunk->vertexCount == 0) continue;
        glBindVertexArray(chunk->vao);
        glDrawArrays(GL_TRIANGLES, 0, chunk->vertexCount);
    }
    glBindVertexArray(0);
}

BlockType World::getBlock(int wx, int wy, int wz) const {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return BlockType::AIR;
    int cx = (int)floorf((float)wx / CHUNK_SIZE);
    int cz = (int)floorf((float)wz / CHUNK_SIZE);
    Chunk* c = getChunk(cx, cz);
    if (!c) return BlockType::AIR;
    int lx = wx - cx * CHUNK_SIZE, lz = wz - cz * CHUNK_SIZE;
    return c->getBlock(lx, wy, lz);
}

int World::getSpawnHeight(int wx, int wz) const {
    for (int y = CHUNK_HEIGHT - 1; y >= 0; y--) {
        BlockType bt = getBlock(wx, y, wz);
        if (bt != BlockType::AIR && bt != BlockType::WATER) {
            return y + 1;
        }
    }
    return 20;
}

void World::setBlockWorld(int wx, int wy, int wz, BlockType t) {
    if (wy < 0 || wy >= CHUNK_HEIGHT) return;
    int cx = (int)floorf((float)wx / CHUNK_SIZE);
    int cz = (int)floorf((float)wz / CHUNK_SIZE);
    auto it = chunks.find({cx, cz});
    if (it == chunks.end()) return;
    Chunk* c = it->second.get();
    int lx = wx - cx * CHUNK_SIZE;
    int lz = wz - cz * CHUNK_SIZE;
    c->setBlock(lx, wy, lz, t);
    // Mark neighboring chunks dirty if on chunk border
    if (lx == 0)            { auto* n = getChunk(cx-1, cz); if (n) n->dirty = true; }
    if (lx == CHUNK_SIZE-1) { auto* n = getChunk(cx+1, cz); if (n) n->dirty = true; }
    if (lz == 0)            { auto* n = getChunk(cx, cz-1); if (n) n->dirty = true; }
    if (lz == CHUNK_SIZE-1) { auto* n = getChunk(cx, cz+1); if (n) n->dirty = true; }
}

RaycastResult World::raycast(float ox, float oy, float oz,
                              float dx, float dy, float dz,
                              float maxDist) const {
    float len = sqrtf(dx*dx + dy*dy + dz*dz);
    if (len < 1e-6f) return {};
    dx /= len; dy /= len; dz /= len;

    int bx = (int)floorf(ox);
    int by = (int)floorf(oy);
    int bz = (int)floorf(oz);

    int sX = dx > 0 ? 1 : -1;
    int sY = dy > 0 ? 1 : -1;
    int sZ = dz > 0 ? 1 : -1;

    float tDX = fabsf(dx) < 1e-6f ? 1e30f : 1.0f / fabsf(dx);
    float tDY = fabsf(dy) < 1e-6f ? 1e30f : 1.0f / fabsf(dy);
    float tDZ = fabsf(dz) < 1e-6f ? 1e30f : 1.0f / fabsf(dz);

    float tMX = (dx > 0) ? ((float)(bx+1) - ox) * tDX : (ox - (float)bx) * tDX;
    float tMY = (dy > 0) ? ((float)(by+1) - oy) * tDY : (oy - (float)by) * tDY;
    float tMZ = (dz > 0) ? ((float)(bz+1) - oz) * tDZ : (oz - (float)bz) * tDZ;

    int nx = 0, ny = 0, nz = 0;

    for (int i = 0; i < 100; i++) {
        if (tMX < tMY && tMX < tMZ) {
            if (tMX > maxDist) break;
            bx += sX; nx = -sX; ny = 0; nz = 0; tMX += tDX;
        } else if (tMY < tMZ) {
            if (tMY > maxDist) break;
            by += sY; nx = 0; ny = -sY; nz = 0; tMY += tDY;
        } else {
            if (tMZ > maxDist) break;
            bz += sZ; nx = 0; ny = 0; nz = -sZ; tMZ += tDZ;
        }
        if (by < 0 || by >= CHUNK_HEIGHT) continue;
        BlockType bt = getBlock(bx, by, bz);
        if (bt != BlockType::AIR && bt != BlockType::WATER) {
            return {true, bx, by, bz, nx, ny, nz};
        }
    }
    return {};
}

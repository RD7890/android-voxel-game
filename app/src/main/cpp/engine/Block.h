#pragma once
#include <cstdint>

enum class BlockType : uint8_t {
    AIR    = 0,
    GRASS  = 1,
    DIRT   = 2,
    STONE  = 3,
    WOOD   = 4,
    LEAVES = 5,
    SAND   = 6,
    WATER  = 7,
    BEDROCK = 8
};

struct BlockColor { float r, g, b; };

// face: 0=top, 1=bottom, 2=north(z-), 3=south(z+), 4=east(x+), 5=west(x-)
inline BlockColor getBlockColor(BlockType type, int face) {
    switch (type) {
        case BlockType::GRASS:
            if (face == 0) return {0.29f, 0.68f, 0.18f};
            if (face == 1) return {0.50f, 0.34f, 0.19f};
            return {0.42f, 0.54f, 0.22f};
        case BlockType::DIRT:    return {0.50f, 0.34f, 0.19f};
        case BlockType::STONE:   return {0.54f, 0.54f, 0.54f};
        case BlockType::WOOD:
            if (face == 0 || face == 1) return {0.56f, 0.42f, 0.16f};
            return {0.38f, 0.26f, 0.10f};
        case BlockType::LEAVES:  return {0.11f, 0.44f, 0.09f};
        case BlockType::SAND:    return {0.87f, 0.77f, 0.50f};
        case BlockType::WATER:   return {0.17f, 0.36f, 0.76f};
        case BlockType::BEDROCK: return {0.14f, 0.14f, 0.14f};
        default:                 return {1.0f, 0.0f, 1.0f};
    }
}

inline bool isOpaque(BlockType t) {
    return t != BlockType::AIR && t != BlockType::WATER && t != BlockType::LEAVES;
}
inline bool isSolid(BlockType t) { return t != BlockType::AIR; }

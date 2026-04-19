#ifndef BEVOID_WORLD_META_H
#define BEVOID_WORLD_META_H

#include <string>
#include <cstdint>

namespace be::void_::world {

struct PlayerPos {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;
    float pitch = -0.2f;
};

struct WorldMeta {
    std::string name;
    uint32_t seed;
    std::string created;
    int64_t lastPlayed;
    int playTimeSeconds;
    int previewWidth;
    int previewHeight;
    PlayerPos playerPos;
};

} // namespace

#endif

#ifndef BEVOID_WORLD_MANAGER_H
#define BEVOID_WORLD_MANAGER_H

#include "WorldMeta.h"
#include <vector>
#include <string>

namespace be::void_::world {

class WorldManager {
public:
    static std::string getWorldsDir();
    static std::vector<WorldMeta> listWorlds();
    static WorldMeta createWorld(const std::string& name, uint32_t seed);
    static bool deleteWorld(const std::string& name);
    static bool worldExists(const std::string& name);
    static uint32_t randomSeed();

    static void generatePreview(const WorldMeta& meta, const std::string& worldDir);
    static std::string getPreviewPath(const std::string& worldName);
    static std::string getWorldDir(const std::string& worldName);

    static void savePlayerPos(const std::string& worldName, const PlayerPos& pos);
    static PlayerPos loadPlayerPos(const std::string& worldName);
    static void saveApr(const std::string& worldName);
    static WorldMeta loadMeta(const std::string& worldName);
};

} // namespace

#endif

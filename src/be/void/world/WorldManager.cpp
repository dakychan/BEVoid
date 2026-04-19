#include "world/WorldManager.h"
#include "core/render/world/Biome.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <random>
#include <cstring>
#include <cstdlib>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace be::void_::world {

namespace fs = std::filesystem;

static std::string getHomeDir() {
#if defined(_WIN32)
    const char* home = std::getenv("USERPROFILE");
    if (!home) home = std::getenv("HOMEDRIVE");
    return home ? std::string(home) : "C:\\Users\\Default";
#else
    const char* home = std::getenv("HOME");
    return home ? std::string(home) : "/tmp";
#endif
}

std::string WorldManager::getWorldsDir() {
    return getHomeDir() + "/BEVoid/worlds";
}

std::string WorldManager::getWorldDir(const std::string& name) {
    return getWorldsDir() + "/" + name;
}

std::string WorldManager::getPreviewPath(const std::string& worldName) {
    return getWorldDir(worldName) + "/preview.png";
}

uint32_t WorldManager::randomSeed() {
    std::random_device rd;
    return rd();
}

static std::string currentTimeStr() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    struct tm tm_buf;
#if defined(_WIN32)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_buf);
    return buf;
}

static bool writeMetaJson(const std::string& dir, const WorldMeta& m) {
    std::string path = dir + "/meta.json";
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << "{\"name\":\"" << m.name
      << "\",\"seed\":" << m.seed
      << ",\"created\":\"" << m.created
      << "\",\"lastPlayed\":" << m.lastPlayed
      << ",\"playTime\":" << m.playTimeSeconds
      << ",\"px\":" << m.playerPos.x
      << ",\"py\":" << m.playerPos.y
      << ",\"pz\":" << m.playerPos.z
      << ",\"yaw\":" << m.playerPos.yaw
      << ",\"pitch\":" << m.playerPos.pitch
      << "}";
    return true;
}

static WorldMeta readMetaJson(const std::string& dir) {
    WorldMeta m;
    m.seed = 0;
    m.lastPlayed = 0;
    m.playTimeSeconds = 0;
    m.previewWidth = 0;
    m.previewHeight = 0;

    std::string path = dir + "/meta.json";
    std::ifstream f(path);
    if (!f.is_open()) return m;

    std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    auto findVal = [&](const char* key) -> std::string {
        std::string sk = "\"" + std::string(key) + "\"";
        auto pos = json.find(sk);
        if (pos == std::string::npos) return "";
        pos = json.find(':', pos + sk.size());
        if (pos == std::string::npos) return "";
        pos++;
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        if (pos < json.size() && json[pos] == '"') {
            pos++;
            auto end = json.find('"', pos);
            return json.substr(pos, end - pos);
        }
        auto end = json.find_first_of(",} \t\n", pos);
        return json.substr(pos, end - pos);
    };

    m.name = findVal("name");
    std::string seedStr = findVal("seed");
    if (!seedStr.empty()) m.seed = (uint32_t)std::stoul(seedStr);
    m.created = findVal("created");
    std::string lpStr = findVal("lastPlayed");
    if (!lpStr.empty()) m.lastPlayed = std::stoll(lpStr);
    std::string ptStr = findVal("playTime");
    if (!ptStr.empty()) m.playTimeSeconds = std::stoi(ptStr);

    std::string pxStr = findVal("px");
    if (!pxStr.empty()) m.playerPos.x = std::stof(pxStr);
    std::string pyStr = findVal("py");
    if (!pyStr.empty()) m.playerPos.y = std::stof(pyStr);
    std::string pzStr = findVal("pz");
    if (!pzStr.empty()) m.playerPos.z = std::stof(pzStr);
    std::string yawStr = findVal("yaw");
    if (!yawStr.empty()) m.playerPos.yaw = std::stof(yawStr);
    std::string pitchStr = findVal("pitch");
    if (!pitchStr.empty()) m.playerPos.pitch = std::stof(pitchStr);

    return m;
}

std::vector<WorldMeta> WorldManager::listWorlds() {
    std::vector<WorldMeta> result;
    std::string dir = getWorldsDir();
    
    if (!fs::exists(dir)) return result;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        std::string metaPath = entry.path().string() + "/meta.json";
        if (!fs::exists(metaPath)) continue;

        WorldMeta m = readMetaJson(entry.path().string());
        if (m.seed != 0 || !m.name.empty()) {
            result.push_back(m);
        }
    }

    std::sort(result.begin(), result.end(), [](const WorldMeta& a, const WorldMeta& b) {
        return a.lastPlayed > b.lastPlayed;
    });

    return result;
}

WorldMeta WorldManager::createWorld(const std::string& name, uint32_t seed) {
    std::string dir = getWorldDir(name);

    if (fs::exists(dir)) {
        std::cerr << "[World] Already exists: " << name << "\n";
        return readMetaJson(dir);
    }

    fs::create_directories(dir);

    WorldMeta m;
    m.name = name;
    m.seed = seed;
    m.created = currentTimeStr();
    m.lastPlayed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    m.playTimeSeconds = 0;
    m.previewWidth = 0;
    m.previewHeight = 0;

    writeMetaJson(dir, m);
    generatePreview(m, dir);

    std::cout << "[World] Created: " << name << " seed=" << seed << "\n";
    return m;
}

bool WorldManager::deleteWorld(const std::string& name) {
    std::string dir = getWorldDir(name);
    if (!fs::exists(dir)) return false;
    fs::remove_all(dir);
    std::cout << "[World] Deleted: " << name << "\n";
    return true;
}

bool WorldManager::worldExists(const std::string& name) {
    return fs::exists(getWorldDir(name));
}

void WorldManager::generatePreview(const WorldMeta& meta, const std::string& worldDir) {
    const int PW = 128, PH = 128;
    unsigned char data[PW * PH * 3];

    core::render::world::BiomeNoise noise(meta.seed);

    for (int y = 0; y < PH; y++) {
        for (int x = 0; x < PW; x++) {
            float wx = (float)x * 0.5f;
            float wz = (float)y * 0.5f;
            auto s = noise.sample(wx, wz);
            float h = s.height;
            float n = (h + 1.0f) * 0.5f;
            if (n < 0.0f) n = 0.0f;
            if (n > 1.0f) n = 1.0f;

            unsigned char v = (unsigned char)(n * 255.0f);
            int idx = (y * PW + x) * 3;

            if (n < 0.35f) {
                data[idx+0] = 30; data[idx+1] = 60; data[idx+2] = 150;
            } else if (n < 0.42f) {
                data[idx+0] = 50; data[idx+1] = 90; data[idx+2] = 170;
            } else if (n < 0.48f) {
                data[idx+0] = 160; data[idx+1] = 150; data[idx+2] = 100;
            } else if (n < 0.65f) {
                data[idx+0] = 40+v/4; data[idx+1] = 120+v/4; data[idx+2] = 30;
            } else if (n < 0.80f) {
                data[idx+0] = 100; data[idx+1] = 90; data[idx+2] = 70;
            } else {
                data[idx+0] = 200+v/5; data[idx+1] = 200+v/5; data[idx+2] = 210+v/5;
            }
        }
    }

    std::string path = worldDir + "/preview.png";
    stbi_write_png(path.c_str(), PW, PH, 3, data, PW * 3);
    std::cout << "[World] Preview saved: " << path << "\n";
}

void WorldManager::savePlayerPos(const std::string& worldName, const PlayerPos& pos) {
    std::string dir = getWorldDir(worldName);
    if (!fs::exists(dir)) return;

    WorldMeta m = readMetaJson(dir);
    m.playerPos = pos;
    m.lastPlayed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    writeMetaJson(dir, m);

    std::ofstream dat(dir + "/content.dat", std::ios::binary);
    if (dat.is_open()) {
        dat.write(reinterpret_cast<const char*>(&pos), sizeof(PlayerPos));
    }

    saveApr(worldName);
    std::cout << "[World] Player pos saved: " << worldName << "\n";
}

PlayerPos WorldManager::loadPlayerPos(const std::string& worldName) {
    std::string dir = getWorldDir(worldName);
    if (!fs::exists(dir)) return {};

    WorldMeta m = readMetaJson(dir);
    return m.playerPos;
}

void WorldManager::saveApr(const std::string& worldName) {
    std::string dir = getWorldDir(worldName);
    if (!fs::exists(dir)) return;

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    std::string tsPath = dir + "/timestamp.json";
    std::ofstream ts(tsPath);
    if (ts.is_open()) {
        ts << "{\"timestamp\":" << ms << ",\"worldName\":\"" << worldName << "\"}";
    }
}

WorldMeta WorldManager::loadMeta(const std::string& worldName) {
    return readMetaJson(getWorldDir(worldName));
}

} // namespace

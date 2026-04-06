/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

/*
 * BEVoid Project — com.bevoid.aporia.system
 *
 * Информация о системе.
 */

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <string>
#include <cstdint>

namespace com::bevoid::aporia::system {

enum class Platform {
    Windows,
    Linux,
    MacOS,
    Android,
    Unknown
};

struct SystemInfo {
    Platform    platform        = Platform::Unknown;
    std::string os_name;
    std::string os_version;
    std::string arch;
    int32_t     cpu_cores       = 0;
    int64_t     total_ram_mb    = 0;
    std::string gpu_vendor;
    std::string gpu_renderer;
    std::string gl_version;

    void setGpuInfo(const char* vendor, const char* renderer, const char* version) {
        gpu_vendor = vendor ? vendor : "N/A";
        gpu_renderer = renderer ? renderer : "N/A";
        gl_version = version ? version : "N/A";
    }
};

} // namespace com::bevoid::aporia::system

#endif // SYSTEM_INFO_H

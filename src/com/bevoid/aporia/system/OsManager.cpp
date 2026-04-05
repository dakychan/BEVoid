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
 * Реализация OsManager — детект платформы, CPU, RAM.
 */

#include "system/OsManager.h"
#include <thread>
#include <iostream>

#if defined(BEVOID_PLATFORM_WINDOWS)
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(BEVOID_PLATFORM_LINUX)
    #include <sys/utsname.h>
    #include <unistd.h>
#elif defined(BEVOID_PLATFORM_MACOS)
    #include <sys/sysctl.h>
    #include <sys/utsname.h>
    #include <unistd.h>
#elif defined(BEVOID_PLATFORM_ANDROID)
    #include <sys/utsname.h>
    #include <unistd.h>
    #include <cstdio>
#endif

namespace com::bevoid::aporia::system {

OsManager::OsManager() = default;
OsManager::~OsManager() = default;

void OsManager::detect() {
    /* --- Платформа --- */
#if defined(BEVOID_PLATFORM_WINDOWS)
    m_info.platform = Platform::Windows;
    m_info.os_name  = "Windows";

    OSVERSIONINFOEXW osvi = {};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    // RtlGetVersion не требует манифеста
    using RtlGetVersionPtr = LONG(WINAPI*)(OSVERSIONINFOEXW*);
    auto mod = GetModuleHandleW(L"ntdll.dll");
    if (mod) {
        auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(
            GetProcAddress(mod, "RtlGetVersion"));
        if (pRtlGetVersion && pRtlGetVersion(&osvi) == 0) {
            m_info.os_version = std::to_string(osvi.dwMajorVersion) + "."
                              + std::to_string(osvi.dwMinorVersion) + "."
                              + std::to_string(osvi.dwBuildNumber);
        }
    }

#elif defined(BEVOID_PLATFORM_LINUX)
    m_info.platform = Platform::Linux;
    m_info.os_name  = "Linux";
    struct utsname un;
    if (uname(&un) == 0) {
        m_info.os_name    = un.sysname;
        m_info.os_version = un.release;
    }

#elif defined(BEVOID_PLATFORM_MACOS)
    m_info.platform = Platform::MacOS;
    m_info.os_name  = "macOS";
    struct utsname un;
    if (uname(&un) == 0) {
        m_info.os_version = un.release;
    }

#elif defined(BEVOID_PLATFORM_ANDROID)
    m_info.platform = Platform::Android;
    m_info.os_name  = "Android";
    struct utsname un;
    if (uname(&un) == 0) {
        m_info.os_version = un.release;
    }

#else
    m_info.platform = Platform::Unknown;
    m_info.os_name  = "Unknown";
#endif

    /* --- Архитектура --- */
#if defined(_M_X64) || defined(__x86_64__)
    m_info.arch = "x86_64";
#elif defined(_M_IX86) || defined(__i386__)
    m_info.arch = "x86";
#elif defined(_M_ARM64) || defined(__aarch64__)
    m_info.arch = "arm64";
#elif defined(_M_ARM) || defined(__arm__)
    m_info.arch = "arm";
#else
    m_info.arch = "unknown";
#endif

    /* --- CPU cores --- */
    m_info.cpu_cores = static_cast<int32_t>(std::thread::hardware_concurrency());

    /* --- RAM --- */
#if defined(BEVOID_PLATFORM_WINDOWS)
    MEMORYSTATUSEX mem = {};
    mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        m_info.total_ram_mb = static_cast<int64_t>(mem.ullTotalPhys / (1024 * 1024));
    }
#elif defined(BEVOID_PLATFORM_LINUX)
    long pages  = sysconf(_SC_PHYS_PAGES);
    long psize  = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && psize > 0) {
        m_info.total_ram_mb = static_cast<int64_t>((pages * psize) / (1024 * 1024));
    }
#elif defined(BEVOID_PLATFORM_MACOS)
    int64_t ram = 0;
    size_t len = sizeof(ram);
    if (sysctlbyname("hw.memsize", &ram, &len, nullptr, 0) == 0) {
        m_info.total_ram_mb = static_cast<int64_t>(ram / (1024 * 1024));
    }
#elif defined(BEVOID_PLATFORM_ANDROID)
    /* На Android sysconf(_SC_PHYS_PAGES) недоступен — читаем /proc/meminfo */
    FILE* f = fopen("/proc/meminfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            long long kb = 0;
            if (sscanf(line, "MemTotal: %lld kB", &kb) == 1) {
                m_info.total_ram_mb = static_cast<int64_t>(kb / 1024);
                break;
            }
        }
        fclose(f);
    }
#endif

    /* --- Логируем --- */
    std::cout << "[OsManager] Platform : " << m_info.os_name
              << " " << m_info.os_version << "\n";
    std::cout << "[OsManager] Arch     : " << m_info.arch << "\n";
    std::cout << "[OsManager] CPU cores: " << m_info.cpu_cores << "\n";
    std::cout << "[OsManager] RAM      : " << m_info.total_ram_mb << " MB\n";
}

} // namespace com::bevoid::aporia::system

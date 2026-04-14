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
 * OsManager — резолвит информацию о системе.
 * ApiRender использует его для выбора правильного рендера.
 */

#ifndef OS_MANAGER_H
#define OS_MANAGER_H

#include "system/SystemInfo.h"

namespace com::bevoid::aporia::system {

class OsManager {
public:
    OsManager();
    ~OsManager();

    void detect();
    const SystemInfo& getInfo() const { return m_info; }
    SystemInfo&       getInfo()       { return m_info; }

    bool isWindows() const { return m_info.platform == Platform::Windows; }
    bool isLinux()   const { return m_info.platform == Platform::Linux;   }
    bool isMacOS()   const { return m_info.platform == Platform::MacOS;   }
    bool isAndroid() const { return m_info.platform == Platform::Android; }

private:
    SystemInfo m_info;
};

} // namespace com::bevoid::aporia::system

#endif // OS_MANAGER_H

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
 * BEVoid Project — drivers
 *
 * Абстрактный интерфейс оконного драйвера.
 * Каждая платформа реализует своё окно через нативный API.
 */

#ifndef I_WINDOW_DRIVER_H
#define I_WINDOW_DRIVER_H

#include <string>
#include <cstdint>

namespace drivers {

struct WindowConfig {
    std::string title  = "BEVoid";
    int32_t     width  = 1280;
    int32_t     height = 720;
    bool        fullscreen = false;
    bool        resizable  = true;
};

class IWindowDriver {
public:
    virtual ~IWindowDriver() = default;

    virtual bool   init(const WindowConfig& config) = 0;
    virtual void   shutdown() = 0;
    virtual bool   pollEvents() = 0;
    virtual bool   shouldClose() const = 0;
    virtual void   setTitle(const std::string& title) = 0;
    virtual void   swapBuffers() = 0;
    virtual int32_t getWidth() const = 0;
    virtual int32_t getHeight() const = 0;

    /* Нативный указатель на окно (HWND / Display* / ANativeWindow*) */
    virtual void* getNativeWindow() const = 0;

    /* Полноэкранный режим */
    virtual void   setFullscreen(bool fullscreen) = 0;
    virtual bool   isFullscreen() const = 0;
};

} // namespace drivers

#endif // I_WINDOW_DRIVER_H

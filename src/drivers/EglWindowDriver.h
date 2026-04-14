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
 * EGL window driver — Android (NativeActivity + EGL + GLES3).
 */

#ifndef EGL_WINDOW_DRIVER_H
#define EGL_WINDOW_DRIVER_H

#include "IWindowDriver.h"
#include <memory>

struct android_app;

namespace drivers {

class EglWindowDriver : public IWindowDriver {
public:
    EglWindowDriver();
    ~EglWindowDriver() override;

    bool   init(const WindowConfig& config) override;
    void   shutdown() override;
    bool   pollEvents() override;
    bool   shouldClose() const override;
    void   setTitle(const std::string& title) override;
    void   swapBuffers() override;
    int32_t getWidth() const override;
    int32_t getHeight() const override;
    void*  getNativeWindow() const override;
    void   setFullscreen(bool fullscreen) override;
    bool   isFullscreen() const override { return true; }

    void registerAppCallbacks(android_app* app);

    struct AndroidState;
    std::unique_ptr<AndroidState> m_state;

private:
    static void handleCmd(android_app* app, int32_t cmd);
};

} // namespace drivers

#endif // EGL_WINDOW_DRIVER_H

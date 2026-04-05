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
 * be.void.core
 *
 * Ядро игры — собирает Render, Movement, Network, Physics.
 * Платформо-независимый — ApiRender создаёт окно/контекст снаружи.
 */

#ifndef BEVOID_CORE_H
#define BEVOID_CORE_H

#include "core/render/Render.h"
#include "core/movement/Movement.h"
#include "network/Network.h"
#include "physics/Physics.h"
#include <memory>

namespace be::void_::core {

class Core {
public:
    Core();
    ~Core();

    /* Инициализация */
    bool init();
    void shutdown();

    /* Обновление и рендер */
    void update(float deltaTime);
    void render(float time);

    /* Доступ к подсистемам */
    render::Render&     getRender()   { return m_render;   }
    movement::Movement& getMovement() { return m_movement; }
    network::Network&   getNetwork()  { return m_network;  }
    physics::Physics&   getPhysics()  { return m_physics;  }

private:
    render::Render     m_render;
    movement::Movement m_movement;
    network::Network   m_network;
    physics::Physics   m_physics;
};

} // namespace be::void_::core

#endif // BEVOID_CORE_H

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
 * be.void.network
 *
 * Заглушка — в будущем сокеты, пакеты, мультиплеер.
 */

#include "network/Network.h"
#include <iostream>

namespace be::void_::network {

bool Network::connect(const char* host, int port) {
    std::cout << "[Network] connect(" << host << ":" << port << ") — stub\n";
    m_connected = true;
    return true;
}

void Network::disconnect() {
    m_connected = false;
}

void Network::update(float /*deltaTime*/) {
    /* TODO: обработка входящих пакетов */
}

} // namespace be::void_::network

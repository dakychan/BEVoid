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
 * Сетевой модуль. Заглушка — не реализовано.
 */

#ifndef BEVOID_NETWORK_H
#define BEVOID_NETWORK_H

namespace be::void_::network {

class Network {
public:
    Network() = default;
    ~Network() = default;

    bool connect(const char* host, int port);
    void disconnect();
    void update(float deltaTime);

private:
    bool m_connected = false;
};

} // namespace be::void_::network

#endif // BEVOID_NETWORK_H

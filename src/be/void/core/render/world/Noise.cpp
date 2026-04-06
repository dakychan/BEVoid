#include "Noise.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>
#include <random>

namespace be::void_::core::render::world {

static uint32_t hash(uint32_t x) {
    x ^= x >> 16; x *= 0x45d9f3bU;
    x ^= x >> 16; x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

Noise::Noise(uint32_t seed) {
    std::array<int, 256> p;
    for (int i = 0; i < 256; ++i) p[i] = i;
    std::mt19937 rng(hash(seed));
    for (int i = 255; i > 0; --i) {
        std::uniform_int_distribution<int> d(0, i);
        std::swap(p[i], p[d(rng)]);
    }
    for (int i = 0; i < 512; ++i) perm[i] = p[i & 255];
}

static float dot(const int* g, float x, float y) { return g[0]*x + g[1]*y; }

float Noise::sample(float x, float y) const {
    int i = (int)std::floor(x + y * F2);
    int j = (int)std::floor(y + x * F2);
    float t = (i + j) * G2;
    float x0 = x - (i - t), y0 = y - (j - t);
    int i1 = x0 > y0 ? 1 : 0, j1 = x0 > y0 ? 0 : 1;
    float x1 = x0 - i1 + G2, y1 = y0 - j1 + G2;
    float x2 = x0 - 1 + 2*G2,  y2 = y0 - 1 + 2*G2;
    int ii = i & 255, jj = j & 255;
    float n0=0, n1=0, n2=0;
    auto step = [&](float sx, float sy, int gi) {
        float tt = 0.5f - sx*sx - sy*sy;
        if (tt > 0) { tt *= tt; n0 += tt*tt*dot(&GRAD[gi*2], sx, sy); }
    };
    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 > 0) { t0*=t0; n0 = t0*t0*dot(&GRAD[(perm[ii+perm[jj]]&15)*2], x0, y0); }
    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 > 0) { t1*=t1; n1 = t1*t1*dot(&GRAD[(perm[ii+i1+perm[jj+j1]]&15)*2], x1, y1); }
    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 > 0) { t2*=t2; n2 = t2*t2*dot(&GRAD[(perm[ii+1+perm[jj+1]]&15)*2], x2, y2); }
    return 70.0f * (n0 + n1 + n2);
}

float Noise::octave(float x, float y, int octaves) const {
    float sum = 0, amp = 1, freq = 1, max = 0;
    for (int i = 0; i < octaves; ++i) {
        sum += sample(x * freq, y * freq) * amp;
        max += amp; amp *= PERSISTENCE; freq *= LACUNARITY;
    }
    return sum / max;
}

} // namespace

#ifndef BEVOID_WORLD_CHUNK_H
#define BEVOID_WORLD_CHUNK_H

#include <cstdint>
#include <vector>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render::world {

struct ChunkVert {
    float x, y, z;     // позиция
    float nx, ny, nz;  // нормаль
    float r, g, b;     // цвет
};

struct ChunkMesh {
    std::vector<ChunkVert> verts;
    std::vector<uint32_t>  idx;
};

class Chunk {
public:
    Chunk(int cx, int cz);
    ~Chunk();

    int cx() const { return m_cx; }
    int cz() const { return m_cz; }

    void load(const ChunkMesh& mesh);
    void draw() const;
    void unload();
    bool loaded() const { return m_loaded; }

private:
    int    m_cx, m_cz;
    GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
    int    m_idxCount = 0;
    bool   m_loaded = false;
};

} // namespace
#endif

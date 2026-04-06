#include "Chunk.h"

namespace be::void_::core::render::world {

Chunk::Chunk(int cx, int cz) : m_cx(cx), m_cz(cz) {}

Chunk::~Chunk() { unload(); }

void Chunk::load(const ChunkMesh& mesh) {
    if (m_loaded) unload();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        (GLsizeiptr)(mesh.verts.size() * sizeof(ChunkVert)),
        mesh.verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        (GLsizeiptr)(mesh.idx.size() * sizeof(uint32_t)),
        mesh.idx.data(), GL_STATIC_DRAW);

    // attrib 0 — pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVert), (void*)0);
    // attrib 1 — normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVert), (void*)12);
    // attrib 2 — color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVert), (void*)24);

    glBindVertexArray(0);
    m_idxCount = (int)mesh.idx.size();
    m_loaded = true;
}

void Chunk::draw() const {
    if (!m_loaded) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_idxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Chunk::unload() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    m_vao = 0; m_vbo = 0; m_ebo = 0;
    m_loaded = false;
}

} // namespace

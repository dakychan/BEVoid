#include "screens/WorldsScreen.h"
#include "world/WorldManager.h"
#include <stb_image.h>
#include <iostream>

namespace be::void_::screens {

void WorldsScreen::onEnter() {
    std::cout << "[WorldsScreen] Enter\n";
    m_nextScreen = ScreenID::None;
    m_selected = 0;
    m_selectedSeed = 0;

    if (!m_fontsLoaded) {
        m_fontRenderer.init();
        m_fontRenderer.loadFont("bold", "bold");
        m_fontsLoaded = true;
    }

    refreshWorlds();
}

void WorldsScreen::onExit() {
    for (auto& p : m_previews) {
        if (p.tex) glDeleteTextures(1, &p.tex);
    }
    m_previews.clear();
    std::cout << "[WorldsScreen] Exit\n";
}

void WorldsScreen::refreshWorlds() {
    for (auto& p : m_previews) {
        if (p.tex) glDeleteTextures(1, &p.tex);
    }
    m_previews.clear();

    m_worlds = world::WorldManager::listWorlds();

    for (auto& w : m_worlds) {
        m_previews.push_back({0, 0, 0});
        loadPreviewTexture((int)m_previews.size() - 1);
    }

    if (m_selected >= (int)m_worlds.size()) m_selected = std::max(0, (int)m_worlds.size() - 1);
}

void WorldsScreen::loadPreviewTexture(int idx) {
    if (idx >= (int)m_worlds.size()) return;
    auto& w = m_worlds[idx];
    std::string path = world::WorldManager::getPreviewPath(w.name);

    int ch;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &m_previews[idx].w, &m_previews[idx].h, &ch, 3);
    stbi_set_flip_vertically_on_load(0);

    if (data) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_previews[idx].w, m_previews[idx].h, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, data);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        m_previews[idx].tex = tex;
    }
}

void WorldsScreen::update(float /*dt*/) {
    if (m_nextScreen != ScreenID::None) return;

#if !defined(BEVOID_PLATFORM_ANDROID)
    auto* window = glfwGetCurrentContext();

    glfwGetCursorPos(window, &m_mouseX, &m_mouseY);
    int winW, winH;
    glfwGetWindowSize(window, &winW, &winH);
    float ndcX = (2.0f * (float)m_mouseX / (float)winW) - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)m_mouseY / (float)winH);

    m_hoveredItem = -1;
    for (size_t i = 0; i < m_worlds.size() && i < 8; i++) {
        float y = 0.55f - (float)i * 0.22f;
        if (ndcX >= -0.4f && ndcX <= 0.4f && ndcY >= y - 0.09f && ndcY <= y + 0.09f) {
            m_hoveredItem = (int)i;
            break;
        }
    }

    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (leftNow && !m_mouseLeftPressed) {
        if (m_hoveredItem >= 0) {
            m_selected = m_hoveredItem;
            m_selectedSeed = m_worlds[m_selected].seed;
            m_selectedName = m_worlds[m_selected].name;
            m_nextScreen = ScreenID::Game;
        }
        if (ndcX >= -0.4f && ndcX <= -0.12f && ndcY >= -0.96f && ndcY <= -0.86f) {
            m_nextScreen = ScreenID::NewWorld;
        }
        if (ndcX >= -0.08f && ndcX <= 0.12f && ndcY >= -0.96f && ndcY <= -0.86f) {
            if (!m_worlds.empty()) {
                world::WorldManager::deleteWorld(m_worlds[m_selected].name);
                refreshWorlds();
            }
        }
        if (ndcX >= 0.16f && ndcX <= 0.35f && ndcY >= -0.96f && ndcY <= -0.86f) {
            m_nextScreen = ScreenID::Menu;
        }
    }
    m_mouseLeftPressed = leftNow;
#else
    auto& ts = getTouchState();
    if (ts.tapped) {
        ts.tapped = false;
        for (size_t i = 0; i < m_worlds.size() && i < 8; i++) {
            float y = 0.55f - (float)i * 0.22f;
            if (ts.ndcX >= -0.4f && ts.ndcX <= 0.4f && ts.ndcY >= y - 0.09f && ts.ndcY <= y + 0.09f) {
                m_selected = (int)i;
                m_selectedSeed = m_worlds[m_selected].seed;
                m_selectedName = m_worlds[m_selected].name;
                m_nextScreen = ScreenID::Game;
                break;
            }
        }
        if (ts.ndcX >= -0.4f && ts.ndcX <= -0.12f && ts.ndcY >= -0.96f && ts.ndcY <= -0.86f) {
            m_nextScreen = ScreenID::NewWorld;
        }
        if (ts.ndcX >= 0.16f && ts.ndcX <= 0.35f && ts.ndcY >= -0.96f && ts.ndcY <= -0.86f) {
            m_nextScreen = ScreenID::Menu;
        }
    }
    if (ts.backPressed && !m_escPressed) {
        m_nextScreen = ScreenID::Menu;
    }
    m_escPressed = ts.backPressed;
#endif
}

void WorldsScreen::render(float time) {
    glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    if (m_fontsLoaded) {
        float tw = m_fontRenderer.getTextWidth("bold", "Worlds", 0.05f);
        m_fontRenderer.drawText("bold", "Worlds", -tw * 0.5f, 0.8f, 0.05f, 0.6f, 0.8f, 0.95f);

        if (m_worlds.empty()) {
            tw = m_fontRenderer.getTextWidth("bold", "No worlds found", 0.025f);
            m_fontRenderer.drawText("bold", "No worlds found", -tw * 0.5f, 0.0f, 0.025f, 0.5f, 0.5f, 0.5f);
        } else {
            for (size_t i = 0; i < m_worlds.size() && i < 8; i++) {
                drawWorldEntry((int)i, 0.55f - (float)i * 0.22f, (int)i == m_selected);
            }
        }

        drawBottomButton(-0.4f, -0.96f, -0.12f, -0.86f, "New World", 0.5f, 0.7f, 0.9f);
        drawBottomButton(-0.08f, -0.96f, 0.12f, -0.86f, "Delete", 0.8f, 0.3f, 0.3f);
        drawBottomButton(0.16f, -0.96f, 0.35f, -0.86f, "Back", 0.5f, 0.5f, 0.5f);
    }

    glEnable(GL_DEPTH_TEST);
}

static GLuint s_entryVao = 0, s_entryVbo = 0, s_entryEbo = 0;
static GLuint s_entryProg = 0;
static bool s_entryInit = false;

static void ensureEntryShader() {
    if (s_entryInit) return;
    s_entryInit = true;

    const char* vs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
#else
        "#version 330 core\n"
#endif
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUV;\n"
        "out vec2 vUV;\n"
        "void main(){gl_Position=vec4(aPos,0,1);vUV=aUV;}\n";
    const char* fs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
        "precision mediump float;\n"
#else
        "#version 330 core\n"
#endif
        "in vec2 vUV;\n"
        "out vec4 fc;\n"
        "uniform sampler2D uTex;\n"
        "void main(){fc=texture(uTex,vUV);}\n";

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, nullptr);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, nullptr);
    glCompileShader(f);
    s_entryProg = glCreateProgram();
    glAttachShader(s_entryProg, v);
    glAttachShader(s_entryProg, f);
    glLinkProgram(s_entryProg);
    glDeleteShader(v);
    glDeleteShader(f);

    glGenVertexArrays(1, &s_entryVao);
    glGenBuffers(1, &s_entryVbo);
    glGenBuffers(1, &s_entryEbo);
}

void WorldsScreen::drawWorldEntry(int idx, float y, bool selected) {
    ensureEntryShader();

    float bx1 = -0.4f, bx2 = 0.4f;
    float by1 = y - 0.09f, by2 = y + 0.09f;

    if (selected) {
        static GLuint bgVao = 0, bgVbo = 0, bgEbo = 0, bgProg = 0;
        if (!bgProg) {
            const char* vs =
#if defined(BEVOID_PLATFORM_ANDROID)
                "#version 300 es\n"
#else
                "#version 330 core\n"
#endif
                "layout(location=0) in vec2 p;\n"
                "layout(location=1) in vec3 c;\n"
                "out vec3 vc;\n"
                "void main(){gl_Position=vec4(p,0,1);vc=c;}\n";
            const char* fs =
#if defined(BEVOID_PLATFORM_ANDROID)
                "#version 300 es\n"
                "precision mediump float;\n"
#else
                "#version 330 core\n"
#endif
                "in vec3 vc;\n"
                "out vec4 fc;\n"
                "void main(){fc=vec4(vc,1);}\n";
            GLuint v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v,1,&vs,nullptr); glCompileShader(v);
            GLuint f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f,1,&fs,nullptr); glCompileShader(f);
            bgProg = glCreateProgram(); glAttachShader(bgProg,v); glAttachShader(bgProg,f); glLinkProgram(bgProg);
            glDeleteShader(v); glDeleteShader(f);
            glGenVertexArrays(1, &bgVao); glGenBuffers(1, &bgVbo); glGenBuffers(1, &bgEbo);
        }
        float bgV[] = { bx1,by1,0.12f,0.3f,0.5f, bx2,by1,0.12f,0.3f,0.5f, bx2,by2,0.12f,0.3f,0.5f, bx1,by2,0.12f,0.3f,0.5f };
        uint32_t bgI[] = {0,1,2,2,3,0};
        glBindVertexArray(bgVao);
        glBindBuffer(GL_ARRAY_BUFFER, bgVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(bgV), bgV, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bgI), bgI, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glUseProgram(bgProg);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    if (idx < (int)m_previews.size() && m_previews[idx].tex) {
        float px1 = bx1 + 0.02f, px2 = bx1 + 0.18f;
        float py1 = by1 + 0.01f, py2 = by2 - 0.01f;

        float quad[] = { px1,py1,0,1, px2,py1,1,1, px2,py2,1,0, px1,py2,0,0 };
        uint32_t qi[] = {0,1,2,2,3,0};

        glUseProgram(s_entryProg);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_previews[idx].tex);
        glUniform1i(glGetUniformLocation(s_entryProg, "uTex"), 0);

        glBindVertexArray(s_entryVao);
        glBindBuffer(GL_ARRAY_BUFFER, s_entryVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_entryEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(qi), qi, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    if (m_fontsLoaded && idx < (int)m_worlds.size()) {
        auto& w = m_worlds[idx];
        float textX = bx1 + 0.22f;
        m_fontRenderer.drawText("bold", w.name, textX, y + 0.03f, 0.025f, 1,1,1);
        std::string info = "seed: " + std::to_string(w.seed) + "  " + w.created;
        m_fontRenderer.drawText("bold", info, textX, y - 0.04f, 0.015f, 0.6f,0.6f,0.6f);
    }
}

void WorldsScreen::drawBottomButton(float x1, float y1, float x2, float y2, const std::string& text, float r, float g, float b) {
    float verts[] = {
        x1,y1, r*0.3f,g*0.3f,b*0.3f,  x2,y1, r*0.3f,g*0.3f,b*0.3f,
        x2,y2, r*0.3f,g*0.3f,b*0.3f,  x1,y2, r*0.3f,g*0.3f,b*0.3f,
    };
    uint32_t idx[] = {0,1,2,2,3,0};

    static GLuint bbVao = 0, bbVbo = 0, bbEbo = 0, bbProg = 0;
    if (!bbProg) {
        const char* vs =
#if defined(BEVOID_PLATFORM_ANDROID)
            "#version 300 es\n"
#else
            "#version 330 core\n"
#endif
            "layout(location=0) in vec2 p;layout(location=1) in vec3 c;out vec3 vc;void main(){gl_Position=vec4(p,0,1);vc=c;}";
        const char* fs =
#if defined(BEVOID_PLATFORM_ANDROID)
            "#version 300 es\nprecision mediump float;\n"
#else
            "#version 330 core\n"
#endif
            "in vec3 vc;out vec4 fc;void main(){fc=vec4(vc,1);}";
        GLuint v = glCreateShader(GL_VERTEX_SHADER); glShaderSource(v,1,&vs,nullptr); glCompileShader(v);
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f,1,&fs,nullptr); glCompileShader(f);
        bbProg = glCreateProgram(); glAttachShader(bbProg,v); glAttachShader(bbProg,f); glLinkProgram(bbProg);
        glDeleteShader(v); glDeleteShader(f);
        glGenVertexArrays(1, &bbVao); glGenBuffers(1, &bbVbo); glGenBuffers(1, &bbEbo);
    }

    glBindVertexArray(bbVao);
    glBindBuffer(GL_ARRAY_BUFFER, bbVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bbEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
    glUseProgram(bbProg);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    float ol[] = { x1,y1,r,g,b, x2,y1,r,g,b, x2,y1,r,g,b, x2,y2,r,g,b, x2,y2,r,g,b, x1,y2,r,g,b, x1,y2,r,g,b, x1,y1,r,g,b };
    glBufferData(GL_ARRAY_BUFFER, sizeof(ol), ol, GL_DYNAMIC_DRAW);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 8);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    if (m_fontsLoaded) {
        float tw = m_fontRenderer.getTextWidth("bold", text, 0.018f);
        float cx = (x1 + x2) * 0.5f;
        m_fontRenderer.drawText("bold", text, cx - tw * 0.5f, (y1+y2)*0.5f - 0.005f, 0.018f, r, g, b);
    }
}

} // namespace

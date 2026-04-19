#include "screens/NewWorldScreen.h"
#include "world/WorldManager.h"
#include <iostream>
#include <cmath>

#if !defined(BEVOID_PLATFORM_ANDROID)
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#else
    #include <GLES3/gl3.h>
#endif

namespace be::void_::screens {

void NewWorldScreen::onEnter() {
    std::cout << "[NewWorldScreen] Enter\n";
    m_nextScreen = ScreenID::None;
    m_editing = 0;
    m_name = "New World";

    if (!m_fontsLoaded) {
        m_fontRenderer.init();
        m_fontRenderer.loadFont("bold", "bold");
        m_fontsLoaded = true;
    }

    regenerateSeed();
}

void NewWorldScreen::onExit() {
    std::cout << "[NewWorldScreen] Exit\n";
}

void NewWorldScreen::regenerateSeed() {
    m_seed = world::WorldManager::randomSeed();
    m_seedStr = std::to_string(m_seed);
}

void NewWorldScreen::createAndPlay() {
    m_seed = 0;
    if (!m_seedStr.empty()) {
        try { m_seed = (uint32_t)std::stoul(m_seedStr); } catch (...) { m_seed = 0; }
    }
    if (m_seed == 0) regenerateSeed();
    world::WorldManager::createWorld(m_name, m_seed);
    m_nextScreen = ScreenID::Game;
}

void NewWorldScreen::update(float /*dt*/) {
    if (m_nextScreen != ScreenID::None) return;

#if !defined(BEVOID_PLATFORM_ANDROID)
    auto* window = glfwGetCurrentContext();

    glfwGetCursorPos(window, &m_mouseX, &m_mouseY);
    int winW, winH;
    glfwGetWindowSize(window, &winW, &winH);
    float ndcX = (2.0f * (float)m_mouseX / (float)winW) - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)m_mouseY / (float)winH);

    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (leftNow && !m_mouseLeftPressed) {
        if (ndcX >= FIELD_X1 && ndcX <= FIELD_X2 &&
            ndcY >= NAME_Y - FIELD_HALF_H && ndcY <= NAME_Y + FIELD_HALF_H) {
            m_editing = 0;
        } else if (ndcX >= FIELD_X1 && ndcX <= FIELD_X2 &&
                   ndcY >= SEED_Y - FIELD_HALF_H && ndcY <= SEED_Y + FIELD_HALF_H) {
            m_editing = 1;
        } else         if (ndcX >= -0.35f && ndcX <= -0.05f && ndcY >= 0.05f && ndcY <= 0.15f) {
            regenerateSeed();
        } else if (ndcX >= -0.35f && ndcX <= -0.05f && ndcY >= -0.05f && ndcY <= 0.05f) {
            createAndPlay();
        } else if (ndcX >= -0.35f && ndcX <= -0.05f && ndcY >= -0.15f && ndcY <= -0.05f) {
            m_nextScreen = ScreenID::Worlds;
        }
    }
    m_mouseLeftPressed = leftNow;

    if (m_editing == 0 || m_editing == 1) {
        std::string& field = (m_editing == 0) ? m_name : m_seedStr;
        int key = -1;
        for (int k = 32; k <= 96; k++) {
            if (glfwGetKey(window, k) == GLFW_PRESS) { key = k; break; }
        }
        static bool keyHeld = false;
        if (key >= 0 && !keyHeld) {
            if (key == GLFW_KEY_BACKSPACE) {
                if (!field.empty()) field.pop_back();
            } else if (field.size() < 30) {
                bool shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                             glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
                char ch = (char)key;
                if (m_editing == 0 && key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
                    ch = shift ? (char)key : (char)(key + 32);
                }
                if (m_editing == 1 && (ch < '0' || ch > '9') && ch != '-') {
                } else {
                    field += ch;
                }
            }
            keyHeld = true;
        } else if (key < 0) {
            keyHeld = false;
        }

        bool tabNow = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
        static bool tabPressed = false;
        if (tabNow && !tabPressed) {
            m_editing = 1 - m_editing;
        }
        tabPressed = tabNow;

        bool enterNow = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
        if (enterNow && !m_enterPressed) {
            createAndPlay();
        }
        m_enterPressed = enterNow;
    }

    bool escNow = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escNow && !m_escPressed) m_nextScreen = ScreenID::Worlds;
    m_escPressed = escNow;
#else
    auto& ts = getTouchState();
    if (ts.backPressed && !m_escPressed) {
        m_nextScreen = ScreenID::Worlds;
    }
    m_escPressed = ts.backPressed;

    if (ts.tapped) {
        ts.tapped = false;
        if (ts.ndcX >= FIELD_X1 && ts.ndcX <= FIELD_X2 &&
            ts.ndcY >= NAME_Y - FIELD_HALF_H && ts.ndcY <= NAME_Y + FIELD_HALF_H) {
            m_editing = 0;
        } else if (ts.ndcX >= FIELD_X1 && ts.ndcX <= FIELD_X2 &&
                   ts.ndcY >= SEED_Y - FIELD_HALF_H && ts.ndcY <= SEED_Y + FIELD_HALF_H) {
            m_editing = 1;
        } else if (ts.ndcX >= -0.35f && ts.ndcX <= -0.05f && ts.ndcY >= 0.05f && ts.ndcY <= 0.15f) {
            regenerateSeed();
        } else if (ts.ndcX >= -0.35f && ts.ndcX <= -0.05f && ts.ndcY >= -0.05f && ts.ndcY <= 0.05f) {
            createAndPlay();
        } else if (ts.ndcX >= -0.35f && ts.ndcX <= -0.05f && ts.ndcY >= -0.15f && ts.ndcY <= -0.05f) {
            m_nextScreen = ScreenID::Worlds;
        }
    }
#endif
}

void NewWorldScreen::render(float time) {
    glClearColor(0.03f, 0.03f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    if (m_fontsLoaded) {
        float tw = m_fontRenderer.getTextWidth("bold", "Create New World", 0.045f);
        m_fontRenderer.drawText("bold", "Create New World", -tw * 0.5f, 0.7f, 0.045f, 0.6f, 0.8f, 0.95f);

        m_fontRenderer.drawText("bold", "Name:", FIELD_X1, NAME_Y + 0.035f, 0.02f, 0.7f, 0.7f, 0.7f);
        {
            float fy1 = NAME_Y - FIELD_HALF_H, fy2 = NAME_Y + FIELD_HALF_H;
            float verts[] = {
                FIELD_X1,fy1,0.1f,0.1f,0.15f, FIELD_X2,fy1,0.1f,0.1f,0.15f,
                FIELD_X2,fy2,0.1f,0.1f,0.15f, FIELD_X1,fy2,0.1f,0.1f,0.15f,
            };
            static GLuint vao=0,vbo=0,ebo=0,prog=0;
            if (!prog) {
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
                GLuint v=glCreateShader(GL_VERTEX_SHADER);glShaderSource(v,1,&vs,nullptr);glCompileShader(v);
                GLuint f=glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(f,1,&fs,nullptr);glCompileShader(f);
                prog=glCreateProgram();glAttachShader(prog,v);glAttachShader(prog,f);glLinkProgram(prog);
                glDeleteShader(v);glDeleteShader(f);
                glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glGenBuffers(1,&ebo);
            }
            uint32_t idx[]={0,1,2,2,3,0};
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER,vbo);
            glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(2*sizeof(float)));
            glUseProgram(prog);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

            float or_ = (m_editing==0)?0.4f:0.2f, og_ = (m_editing==0)?0.7f:0.3f, ob_ = (m_editing==0)?1.0f:0.4f;
            float ol[] = {FIELD_X1,fy1,or_,og_,ob_, FIELD_X2,fy1,or_,og_,ob_, FIELD_X2,fy1,or_,og_,ob_, FIELD_X2,fy2,or_,og_,ob_, FIELD_X2,fy2,or_,og_,ob_, FIELD_X1,fy2,or_,og_,ob_, FIELD_X1,fy2,or_,og_,ob_, FIELD_X1,fy1,or_,og_,ob_};
            glBufferData(GL_ARRAY_BUFFER,sizeof(ol),ol,GL_DYNAMIC_DRAW);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES,0,8);
            glLineWidth(1.0f);
            glBindVertexArray(0);
        }
        char cursor = (std::sin(time * 4.0f) > 0) ? '|' : ' ';
        std::string nameDisplay = m_name + ((m_editing==0) ? cursor : ' ');
        m_fontRenderer.drawText("bold", nameDisplay, FIELD_X1 + 0.02f, NAME_Y + 0.008f, 0.02f, 1,1,1);

        m_fontRenderer.drawText("bold", "Seed:", FIELD_X1, SEED_Y + 0.035f, 0.02f, 0.7f, 0.7f, 0.7f);
        {
            float fy1 = SEED_Y - FIELD_HALF_H, fy2 = SEED_Y + FIELD_HALF_H;
            float verts2[] = {
                FIELD_X1,fy1,0.1f,0.1f,0.15f, FIELD_X2,fy1,0.1f,0.1f,0.15f,
                FIELD_X2,fy2,0.1f,0.1f,0.15f, FIELD_X1,fy2,0.1f,0.1f,0.15f,
            };
            static GLuint vao2=0,vbo2=0,ebo2=0,prog2=0;
            if (!prog2) {
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
                GLuint v=glCreateShader(GL_VERTEX_SHADER);glShaderSource(v,1,&vs,nullptr);glCompileShader(v);
                GLuint f=glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(f,1,&fs,nullptr);glCompileShader(f);
                prog2=glCreateProgram();glAttachShader(prog2,v);glAttachShader(prog2,f);glLinkProgram(prog2);
                glDeleteShader(v);glDeleteShader(f);
                glGenVertexArrays(1,&vao2);glGenBuffers(1,&vbo2);glGenBuffers(1,&ebo2);
            }
            uint32_t idx2[]={0,1,2,2,3,0};
            glBindVertexArray(vao2);
            glBindBuffer(GL_ARRAY_BUFFER,vbo2);
            glBufferData(GL_ARRAY_BUFFER,sizeof(verts2),verts2,GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo2);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx2),idx2,GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(2*sizeof(float)));
            glUseProgram(prog2);
            glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

            float or_ = (m_editing==1)?0.4f:0.2f, og_ = (m_editing==1)?0.7f:0.3f, ob_ = (m_editing==1)?1.0f:0.4f;
            float ol[] = {FIELD_X1,fy1,or_,og_,ob_, FIELD_X2,fy1,or_,og_,ob_, FIELD_X2,fy1,or_,og_,ob_, FIELD_X2,fy2,or_,og_,ob_, FIELD_X2,fy2,or_,og_,ob_, FIELD_X1,fy2,or_,og_,ob_, FIELD_X1,fy2,or_,og_,ob_, FIELD_X1,fy1,or_,og_,ob_};
            glBufferData(GL_ARRAY_BUFFER,sizeof(ol),ol,GL_DYNAMIC_DRAW);
            glLineWidth(2.0f);
            glDrawArrays(GL_LINES,0,8);
            glLineWidth(1.0f);
            glBindVertexArray(0);
        }
        std::string seedDisplay = m_seedStr + ((m_editing==1) ? cursor : ' ');
        m_fontRenderer.drawText("bold", seedDisplay, FIELD_X1 + 0.02f, SEED_Y + 0.008f, 0.02f, 1,1,1);

        drawButton(-0.35f, 0.05f, -0.05f, 0.15f, "Regenerate Seed", 0.5f, 0.7f, 0.9f);
        drawButton(-0.35f, -0.05f, -0.05f, 0.05f, "Create & Play", 0.5f, 0.9f, 0.5f);
        drawButton(-0.35f, -0.15f, -0.05f, -0.05f, "Back", 0.5f, 0.5f, 0.5f);
    }

    glEnable(GL_DEPTH_TEST);
}

void NewWorldScreen::drawButton(float x1, float y1, float x2, float y2, const std::string& text, float r, float g, float b) {
    float verts[] = {
        x1,y1, r*0.2f,g*0.2f,b*0.2f, x2,y1, r*0.2f,g*0.2f,b*0.2f,
        x2,y2, r*0.2f,g*0.2f,b*0.2f, x1,y2, r*0.2f,g*0.2f,b*0.2f,
    };
    uint32_t idx[] = {0,1,2,2,3,0};

    static GLuint vao=0,vbo=0,ebo=0,prog=0;
    if (!prog) {
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
        GLuint v=glCreateShader(GL_VERTEX_SHADER);glShaderSource(v,1,&vs,nullptr);glCompileShader(v);
        GLuint f=glCreateShader(GL_FRAGMENT_SHADER);glShaderSource(f,1,&fs,nullptr);glCompileShader(f);
        prog=glCreateProgram();glAttachShader(prog,v);glAttachShader(prog,f);glLinkProgram(prog);
        glDeleteShader(v);glDeleteShader(f);
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);glGenBuffers(1,&ebo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(2*sizeof(float)));
    glUseProgram(prog);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    float ol[] = {x1,y1,r,g,b, x2,y1,r,g,b, x2,y1,r,g,b, x2,y2,r,g,b, x2,y2,r,g,b, x1,y2,r,g,b, x1,y2,r,g,b, x1,y1,r,g,b};
    glBufferData(GL_ARRAY_BUFFER,sizeof(ol),ol,GL_DYNAMIC_DRAW);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES,0,8);
    glLineWidth(1.0f);
    glBindVertexArray(0);

    if (m_fontsLoaded) {
        float tw = m_fontRenderer.getTextWidth("bold", text, 0.018f);
        float cx = (x1 + x2) * 0.5f;
        float cy = (y1 + y2) * 0.5f - 0.005f;
        m_fontRenderer.drawText("bold", text, cx - tw * 0.5f, cy, 0.018f, r, g, b);
    }
}

} // namespace

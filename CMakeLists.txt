cmake_minimum_required(VERSION 3.20)
project(BEVoid VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Источники проекта
# ============================================================
set(SOURCES_GAME
    src/be/void/Game.cpp
    src/be/void/core/Core.cpp
    src/be/void/core/render/Render.cpp
    src/be/void/core/render/SkyRenderer.cpp
    src/be/void/core/render/world/Noise.cpp
    src/be/void/core/render/world/Biome.cpp
    src/be/void/core/render/world/Chunk.cpp
    src/be/void/core/render/world/ChunkManager.cpp
    src/be/void/core/movement/Movement.cpp
    src/be/void/core/input/Input.cpp
    src/be/void/network/Network.cpp
    src/be/void/physics/Physics.cpp
    src/be/void/physics/Cycles.cpp
)

set(HEADERS_GAME
    src/be/void/Game.h
    src/be/void/Vec3.h
    src/be/void/core/Core.h
    src/be/void/core/render/Render.h
    src/be/void/core/render/SkyRenderer.h
    src/be/void/core/render/world/Constants.h
    src/be/void/core/render/world/Noise.h
    src/be/void/core/render/world/BiomeTypes.h
    src/be/void/core/render/world/Biome.h
    src/be/void/core/render/world/Chunk.h
    src/be/void/core/render/world/ChunkManager.h
    src/be/void/physics/metrics/Units.h
    src/be/void/core/movement/Movement.h
    src/be/void/core/input/Input.h
    src/be/void/network/Network.h
    src/be/void/physics/Physics.h
    src/be/void/physics/Cycles.h
)

set(SOURCES_SYSTEM
    src/com/bevoid/aporia/system/OsManager.cpp
    src/com/bevoid/aporia/system/ApiRender.cpp
)

set(HEADERS_SYSTEM
    src/com/bevoid/aporia/system/OsManager.h
    src/com/bevoid/aporia/system/SystemInfo.h
    src/com/bevoid/aporia/system/ApiRender.h
)

# ============================================================
# Glad — OpenGL loader (всегда нужен)
# ============================================================
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/glad)
if(EXISTS "${GLAD_DIR}/glad.c")
    add_library(glad STATIC ${GLAD_DIR}/glad.c)
    target_include_directories(glad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
        ${GLAD_DIR}
    )
    if(NOT WIN32 AND NOT ANDROID)
        target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
    endif()
else()
    message(FATAL_ERROR "glad.c not found! Generate from https://glad.dav1d.de/")
endif()

# ============================================================
# Desktop: GLFW + OpenGL
# ============================================================
if(WIN32 OR (UNIX AND NOT ANDROID))
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11      ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# ============================================================
# Android: NativeActivity + EGL (проверяем ПЕРВЫМ!)
# ============================================================
if(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)

    add_library(bevoid SHARED
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
        ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )

    target_link_libraries(bevoid PRIVATE
        ${log-lib} ${android-lib} glad EGL GLESv3
    )

    target_include_directories(bevoid PRIVATE
        ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_ANDROID)

# ============================================================
# Desktop: Windows
# ============================================================
elseif(WIN32)
    # Copy .ico file to build directory for .rc file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/assets/bevoid.ico
        ${CMAKE_CURRENT_BINARY_DIR}/bevoid.ico
        COPYONLY
    )
    
    add_executable(bevoid WIN32
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
        src/assets/bevoid.rc
    )

    target_link_libraries(bevoid PRIVATE glfw glad user32)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_WINDOWS)

# ============================================================
# Desktop: macOS
# ============================================================
elseif(APPLE)
    add_executable(bevoid
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE glfw glad)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_MACOS)

# ============================================================
# Desktop: Linux
# ============================================================
elseif(UNIX)
    add_executable(bevoid
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE glfw glad)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_LINUX)

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# ============================================================
# Общие настройки
# ============================================================
target_include_directories(bevoid PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/be/void
    ${CMAKE_CURRENT_SOURCE_DIR}/src/com/bevoid/aporia
)

if(MSVC)
    target_compile_options(bevoid PRIVATE /W4 /permissive-)
else()
    target_compile_options(bevoid PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS bevoid DESTINATION bin)

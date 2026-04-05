cmake_minimum_required(VERSION 3.20)
project(BEVoid VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# GLFW — кроссплатформенное окно + OpenGL контекст
# ============================================================
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ============================================================
# Glad — OpenGL loader
# ============================================================
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/glad)
if(EXISTS "${GLAD_DIR}/glad.c")
    add_library(glad STATIC ${GLAD_DIR}/glad.c)
    target_include_directories(glad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party
        ${GLAD_DIR}
    )
    if(NOT WIN32)
        target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
    endif()
else()
    message(FATAL_ERROR "glad.c not found! Generate from https://glad.dav1d.de/")
endif()

# ============================================================
# Источники проекта
# ============================================================
set(SOURCES_GAME
    src/be/void/Game.cpp
)

set(HEADERS_GAME
    src/be/void/Game.h
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
# Desktop: GLFW + OpenGL
# ============================================================
if(WIN32)
    add_executable(bevoid WIN32
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE glfw glad)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_WINDOWS)

elseif(APPLE)
    add_executable(bevoid
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE glfw glad)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_MACOS)

elseif(UNIX AND NOT ANDROID)
    add_executable(bevoid
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE glfw glad)
    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_LINUX)

# ============================================================
# Android: NativeActivity + EGL
# ============================================================
elseif(ANDROID)
    find_library(log-lib log)
    find_library(android-lib android)

    add_library(bevoid SHARED
        ${SOURCES_GAME} ${HEADERS_GAME}
        ${SOURCES_SYSTEM} ${HEADERS_SYSTEM}
    )

    target_link_libraries(bevoid PRIVATE
        ${log-lib} ${android-lib} glad
    )

    target_include_directories(bevoid PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

    target_compile_definitions(bevoid PRIVATE BEVOID_PLATFORM_ANDROID)

else()
    message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

# ============================================================
# Общие настройки
# ============================================================
target_include_directories(bevoid PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/com/bevoid/aporia
)

if(MSVC)
    target_compile_options(bevoid PRIVATE /W4 /permissive-)
else()
    target_compile_options(bevoid PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS bevoid DESTINATION bin)

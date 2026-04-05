#!/bin/bash
# ============================================================
# BEVoid — Linux Build Script
# Запускать на Linux (Ubuntu/Debian/Fedora/Arch)
# ============================================================

set -e

echo "============================================"
echo " BEVoid — Linux Build"
echo "============================================"

# --- Зависимости ---
echo ""
echo "[1/5] Checking dependencies..."

if command -v apt-get &>/dev/null; then
    sudo apt-get update -qq
    sudo apt-get install -y -qq build-essential cmake libx11-dev libxrandr-dev \
        libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev xorg-dev 2>/dev/null || true
elif command -v dnf &>/dev/null; then
    sudo dnf install -y cmake gcc-c++ libX11-devel libXrandr-devel \
        libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel 2>/dev/null || true
elif command -v pacman &>/dev/null; then
    sudo pacman -S --noconfirm cmake gcc libx11 libxrandr libxinerama \
        libxcursor libxi mesa 2>/dev/null || true
fi

# --- Проверка glad ---
echo ""
echo "[2/5] Checking glad..."
if [ ! -f "src/third_party/glad/glad.c" ]; then
    echo "ERROR: glad.c not found!"
    echo "Generate from https://glad.dav1d.de/ (OpenGL 3.3 Core)"
    echo "Place glad.h and glad.c in src/third_party/glad/"
    exit 1
fi

# --- CMake ---
echo ""
echo "[3/5] Configuring CMake..."
mkdir -p build-linux
cd build-linux
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc

# --- Build ---
echo ""
echo "[4/5] Building..."
cmake --build . --config Release -j$(nproc)

# --- Done ---
echo ""
echo "[5/5] Build complete!"
echo "============================================"
echo " Binary: ./build-linux/bevoid"
echo " Run:   ./build-linux/bevoid"
echo "============================================"

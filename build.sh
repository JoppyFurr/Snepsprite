#!/bin/sh

# Select compiler
COMPILER=${1:-"gcc"}

if [ ${COMPILER} = "clang" ]
then
    echo "Using GCC"
    CC=gcc
    CXX=g++
else
    echo "Using GCC"
    CC=gcc
    CXX=g++
fi

# OS-specific compiler options
if [ $(uname) = "Darwin" ]
then
    # MacOS
    DATE=$(date "+%Y-%m-%d")
    OS_FLAGS="-framework OpenGL -framework CoreFoundation"
else
    # Linux
    DATE=$(date --rfc-3339=date)
    OS_FLAGS="$(pkg-config --libs gl)"
fi

# Compile
eval ${CXX} Source/main.cpp \
    -DIMGUI_IMPL_OPENGL_LOADER_GL3W \
    Libraries/imgui-1.76/*.cpp \
    Libraries/imgui-1.76/examples/libs/gl3w/GL/gl3w.c \
    Libraries/imgui-1.76/examples/imgui_impl_opengl3.cpp \
    Libraries/imgui-1.76/examples/imgui_impl_sdl.cpp \
    `sdl2-config --cflags` \
    -I Source/ \
    -I Libraries/imgui-1.76/ \
    -I Libraries/imgui-1.76/examples/libs/gl3w/ \
    `sdl2-config --libs` \
    -ldl -DBUILD_DATE=\\\"$DATE\\\" \
    ${OS_FLAGS} \
    -o Snepsprite -std=c++11



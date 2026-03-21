#!/bin/bash

# Proxima Compiler Build Script
# License: GPLv3

set -e

echo "======================================"
echo "Proxima Compiler Build Script"
echo "======================================"

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
ENABLE_CUDA=${ENABLE_CUDA:-OFF}
ENABLE_CURL=${ENABLE_CURL:-OFF}
BUILD_TESTS=${BUILD_TESTS:-ON}
BUILD_IDE=${BUILD_IDE:-OFF}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --with-cuda)
            ENABLE_CUDA=ON
            shift
            ;;
        --with-curl)
            ENABLE_CURL=ON
            shift
            ;;
        --without-tests)
            BUILD_TESTS=OFF
            shift
            ;;
        --with-ide)
            BUILD_IDE=ON
            shift
            ;;
        --prefix=*)
            INSTALL_PREFIX="${1#*=}"
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            rm -rf $BUILD_DIR
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug        Build in debug mode"
            echo "  --with-cuda    Enable CUDA support"
            echo "  --with-curl    Enable CURL for LLM"
            echo "  --without-tests Disable tests"
            echo "  --with-ide     Build Centauri IDE"
            echo "  --prefix=PATH  Installation prefix"
            echo "  --clean        Clean build directory"
            echo "  --help         Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
echo "Configuring build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -DENABLE_CUDA=$ENABLE_CUDA \
    -DENABLE_CURL=$ENABLE_CURL \
    -DBUILD_TESTS=$BUILD_TESTS \
    -DBUILD_IDE=$BUILD_IDE

# Build
echo "Building..."
make -j$(nproc)

# Test
if [ "$BUILD_TESTS" = "ON" ]; then
    echo "Running tests..."
    ctest --output-on-failure
fi

# Install
if [ "$1" = "install" ]; then
    echo "Installing..."
    sudo make install
fi

echo "======================================"
echo "Build completed successfully!"
echo "======================================"
echo ""
echo "To run the compiler:"
echo "  ./build/proxima --help"
echo ""
echo "To run examples:"
echo "  ./build/proxima run ../examples/hello.prx"
echo ""

#!/bin/bash

# Centauri IDE Build Script
# License: GPLv3

set -e

echo "======================================"
echo "Centauri IDE Build Script"
echo "======================================"

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build-ide}
INSTALL_PREFIX=${INSTALL_PREFIX:-/usr/local}
QT_PATH=${QT_PATH:-/opt/Qt5.15.2}
COMPILER_PATH=${COMPILER_PATH:-../proxima-compiler/build}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE=Debug
            shift
            ;;
        --qt-path=*)
            QT_PATH="${1#*=}"
            shift
            ;;
        --compiler-path=*)
            COMPILER_PATH="${1#*=}"
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
            echo "  --debug              Build in debug mode"
            echo "  --qt-path=PATH       Path to Qt installation"
            echo "  --compiler-path=PATH Path to Proxima Compiler"
            echo "  --prefix=PATH        Installation prefix"
            echo "  --clean              Clean build directory"
            echo "  --help               Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check Qt installation
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Qt not found at $QT_PATH"
    echo "Please set QT_PATH environment variable or use --qt-path option"
    exit 1
fi

# Set Qt paths
export CMAKE_PREFIX_PATH="$QT_PATH;$CMAKE_PREFIX_PATH"
export PATH="$QT_PATH/bin:$PATH"

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
echo "Configuring build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -DCMAKE_PREFIX_PATH="$QT_PATH" \
    -DProximaCompiler_DIR="$COMPILER_PATH"

# Build
echo "Building..."
make -j$(nproc)

# Install
if [ "$1" = "install" ]; then
    echo "Installing..."
    sudo make install
fi

echo "======================================"
echo "Build completed successfully!"
echo "======================================"
echo ""
echo "To run the IDE:"
echo "  ./build-ide/centauri"
echo ""

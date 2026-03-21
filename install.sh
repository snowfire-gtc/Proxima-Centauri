#!/bin/bash

# Proxima & Centauri Installation Script
# License: GPLv3

set -e

echo "=========================================="
echo "Proxima & Centauri Installation Script"
echo "=========================================="

# Configuration
PREFIX=${PREFIX:-/usr/local}
BUILD_COMPILER=${BUILD_COMPILER:-yes}
BUILD_IDE=${BUILD_IDE:-yes}
BUILD_TESTS=${BUILD_TESTS:-yes}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Functions
print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

check_dependency() {
    if ! command -v $1 &> /dev/null; then
        print_error "$1 is not installed"
        return 1
    fi
    print_success "$1 is installed ($( $1 --version | head -n1 ))"
    return 0
}

# Check dependencies
echo ""
echo "Checking dependencies..."
echo "------------------------"

check_dependency cmake || exit 1
check_dependency g++ || exit 1
check_dependency git || exit 1

# Check LLVM
if [ -d "/usr/lib/llvm-14" ]; then
    print_success "LLVM 14 found"
elif [ -d "/usr/lib/llvm-15" ]; then
    print_success "LLVM 15 found"
elif [ -d "/usr/lib/llvm-16" ]; then
    print_success "LLVM 16 found"
elif [ -d "/usr/lib/llvm-17" ]; then
    print_success "LLVM 17 found"
elif [ -d "/usr/lib/llvm-18" ]; then
    print_success "LLVM 18 found"
else
    print_warning "LLVM not found in standard locations"
    echo "Please install LLVM 14+ or set LLVM_DIR environment variable"
fi

# Check Qt
if [ dpkg -s qt6-base-dev >/dev/null 2>&1; ]; then
    print_success "Qt6 found"
    export QT_PATH="/usr/lib/x86_64-linux-gnu/qt6"
else
    print_warning "Qt not found"
    echo "Please install qt6-base-dev and qt6-5compat-dev or set QT_PATH environment variable"
fi

# Build compiler
if [ "$BUILD_COMPILER" = "yes" ]; then
    echo ""
    echo "Building Proxima Compiler..."
    echo "----------------------------"
    
    cd proxima-compiler
    
    if [ -d "build" ]; then
        echo "Cleaning previous build..."
        rm -rf build
    fi
    
    mkdir build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$PREFIX \
        -DBUILD_TESTS=$BUILD_TESTS
    
    make -j$(nproc)
    
    if [ "$1" = "install" ]; then
        echo "Installing compiler..."
        sudo make install
        print_success "Compiler installed to $PREFIX"
    fi
    
    cd ../..
    
    print_success "Compiler build completed"
fi

# Build IDE
if [ "$BUILD_IDE" = "yes" ]; then
    echo ""
    echo "Building Centauri IDE..."
    echo "------------------------"
    
    cd centauri-ide
    
    if [ -d "build" ]; then
        echo "Cleaning previous build..."
        rm -rf build
    fi
    
    mkdir build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$PREFIX \
        -DCMAKE_PREFIX_PATH=$QT_PATH
    
    make -j$(nproc)
    
    if [ "$1" = "install" ]; then
        echo "Installing IDE..."
        sudo make install
        print_success "IDE installed to $PREFIX"
    fi
    
    cd ../..
    
    print_success "IDE build completed"
fi

# Create desktop entry (Linux)
if [ "$(uname)" = "Linux" ] && [ "$1" = "install" ]; then
    echo ""
    echo "Creating desktop entry..."
    echo "-------------------------"
    
    cat > /usr/share/applications/centauri.desktop << EOF
[Desktop Entry]
Name=Centauri IDE
Comment=IDE for Proxima Programming Language
Exec=$PREFIX/bin/centauri
Icon=$PREFIX/share/icons/hicolor/256x256/apps/logo.png
Terminal=false
Type=Application
Categories=Development;IDE;
Keywords=Proxima;IDE;Development;
EOF
    
    print_success "Desktop entry created"
fi

# Create symlink
if [ "$1" = "install" ]; then
    echo ""
    echo "Creating symlinks..."
    echo "--------------------"
    
    sudo ln -sf $PREFIX/bin/proxima /usr/bin/proxima 2>/dev/null || true
    sudo ln -sf $PREFIX/bin/centauri /usr/bin/centauri 2>/dev/null || true
    
    print_success "Symlinks created"
fi

echo ""
echo "=========================================="
echo "Installation Complete!"
echo "=========================================="
echo ""
echo "To run the compiler:"
echo "  proxima --help"
echo ""
echo "To run the IDE:"
echo "  centauri"
echo ""
echo "Or from installation directory:"
echo "  $PREFIX/bin/proxima"
echo "  $PREFIX/bin/centauri"
echo ""

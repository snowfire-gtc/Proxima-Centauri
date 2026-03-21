#!/bin/bash

# Proxima & Centauri Deployment Script
# License: GPLv3

set -e

echo "=========================================="
echo "Proxima & Centauri Deployment Script"
echo "=========================================="

# Configuration
VERSION=${VERSION:-1.0.0}
BUILD_DIR=${BUILD_DIR:-deploy}
PACKAGE_NAME="proxima-centauri-${VERSION}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Functions
print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Clean build directory
clean_build() {
    print_info "Cleaning build directory..."
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    print_success "Build directory cleaned"
}

# Build compiler
build_compiler() {
    print_info "Building Proxima Compiler..."
    
    cd proxima
    
    if [ -d "build" ]; then
        rm -rf build
    fi
    
    mkdir build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBUILD_TESTS=ON
    
    make -j$(nproc)
    
    # Copy binaries
    cp proxima ../../$BUILD_DIR/
    
    cd ../..
    
    print_success "Proxima built"
}

# Build IDE
build_ide() {
    print_info "Building Centauri IDE..."
    
    cd centauri
    
    if [ -d "build" ]; then
        rm -rf build
    fi
    
    mkdir build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_PREFIX_PATH=/opt/Qt5.15.2
    
    make -j$(nproc)
    
    # Copy binaries
    cp centauri ../../$BUILD_DIR/
    
    cd ../..
    
    print_success "Centaury built"
}

# Create package structure
create_package() {
    print_info "Creating package structure..."
    
    mkdir -p $BUILD_DIR/bin
    mkdir -p $BUILD_DIR/lib
    mkdir -p $BUILD_DIR/share/proxima/examples
    mkdir -p $BUILD_DIR/share/proxima/docs
    mkdir -p $BUILD_DIR/share/centauri/resources
    mkdir -p $BUILD_DIR/share/applications
    mkdir -p $BUILD_DIR/share/icons/hicolor/256x256/apps
    
    # Copy binaries
    cp $BUILD_DIR/proxima $BUILD_DIR/bin/
    cp $BUILD_DIR/centauri $BUILD_DIR/bin/
    
    # Copy examples
    cp -r examples/* $BUILD_DIR/share/proxima/examples/ 2>/dev/null || true
    
    # Copy documentation
    cp -r docs/* $BUILD_DIR/share/proxima/docs/ 2>/dev/null || true
    
    # Copy resources
    cp -r centauri/resources/* $BUILD_DIR/share/centauri/resources/ 2>/dev/null || true
    
    # Copy icons
    cp centauri/assets/logo.png $BUILD_DIR/share/icons/hicolor/256x256/apps/ 2>/dev/null || true
    
    # Create desktop file
    cat > $BUILD_DIR/share/applications/centauri.desktop << EOF
[Desktop Entry]
Name=Centauri IDE
Comment=IDE for Proxima Programming Language
Exec=/usr/bin/centauri
Icon=centauri
Terminal=false
Type=Application
Categories=Development;IDE;
Keywords=Proxima;IDE;Development;
MimeType=text/x-proxima;
EOF
    
    # Create MIME type file
    cat > $BUILD_DIR/share/mime/packages/proxima.xml << EOF
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns='http://www.freedesktop.org/standards/shared-mime-info'>
  <mime-type type="text/x-proxima">
    <comment>Proxima Source Code</comment>
    <glob pattern="*.prx"/>
  </mime-type>
</mime-info>
EOF
    
    # Create README
    cat > $BUILD_DIR/README.txt << EOF
Proxima Programming Language & Centauri IDE
Version: $VERSION
License: GPLv3

Installation:
  sudo ./install.sh

Uninstallation:
  sudo ./uninstall.sh

Documentation:
  See share/proxima/docs/

Examples:
  See share/proxima/examples/

Website: https://proxima-lang.org
EOF
    
    # Create install script
    cat > $BUILD_DIR/install.sh << 'EOF'
#!/bin/bash
set -e

echo "Installing Proxima & Centauri..."

# Copy binaries
sudo cp bin/* /usr/bin/

# Copy libraries
sudo cp -r lib/* /usr/lib/ 2>/dev/null || true

# Copy shared files
sudo cp -r share/* /usr/share/

# Update MIME database
sudo update-mime-database /usr/share/mime 2>/dev/null || true

# Update desktop database
sudo update-desktop-database /usr/share/applications 2>/dev/null || true

echo "Installation complete!"
echo "Run 'centauri' to start the IDE"
echo "Run 'proxima --help' for compiler options"
EOF
    chmod +x $BUILD_DIR/install.sh
    
    # Create uninstall script
    cat > $BUILD_DIR/uninstall.sh << 'EOF'
#!/bin/bash
set -e

echo "Uninstalling Proxima & Centauri..."

# Remove binaries
sudo rm -f /usr/bin/proxima
sudo rm -f /usr/bin/centauri

# Remove libraries
sudo rm -rf /usr/lib/proxima 2>/dev/null || true

# Remove shared files
sudo rm -rf /usr/share/proxima
sudo rm -rf /usr/share/centauri
sudo rm -f /usr/share/applications/centauri.desktop

# Update MIME database
sudo update-mime-database /usr/share/mime 2>/dev/null || true

# Update desktop database
sudo update-desktop-database /usr/share/applications 2>/dev/null || true

echo "Uninstallation complete!"
EOF
    chmod +x $BUILD_DIR/uninstall.sh
    
    # Copy license
    cp LICENSE $BUILD_DIR/ 2>/dev/null || echo "GPLv3 License" > $BUILD_DIR/LICENSE
    
    print_success "Package structure created"
}

# Create archives
create_archives() {
    print_info "Creating archives..."
    
    # Create tar.gz
    cd $BUILD_DIR
    tar -czf ../${PACKAGE_NAME}.tar.gz *
    cd ..
    
    # Create zip (if zip is available)
    if command -v zip &> /dev/null; then
        cd $BUILD_DIR
        zip -r ../${PACKAGE_NAME}.zip *
        cd ..
    fi
    
    # Create deb package (on Debian/Ubuntu)
    if [ -f "/etc/debian_version" ]; then
        create_deb_package
    fi
    
    # Create rpm package (on RHEL/Fedora)
    if [ -f "/etc/redhat-release" ]; then
        create_rpm_package
    fi
    
    print_success "Archives created"
}

create_deb_package() {
    print_info "Creating DEB package..."
    
    mkdir -p $BUILD_DIR-deb/DEBIAN
    mkdir -p $BUILD_DIR-deb/usr/bin
    mkdir -p $BUILD_DIR-deb/usr/share
    
    # Copy files
    cp -r $BUILD_DIR/bin/* $BUILD_DIR-deb/usr/bin/
    cp -r $BUILD_DIR/share/* $BUILD_DIR-deb/usr/share/
    
    # Create control file
    cat > $BUILD_DIR-deb/DEBIAN/control << EOF
Package: proxima-centauri
Version: $VERSION
Section: devel
Priority: optional
Architecture: amd64
Maintainer: Serey Antonov & Proxima Development Team <dev@prxlang.ru>
Description: Proxima Programming Language and Centauri IDE
 A MATLAB-compatible programming language with full OOP support,
 GPU acceleration, and modern IDE integration.
EOF
    
    # Build package
    cd $BUILD_DIR-deb
    dpkg-deb --build . ../${PACKAGE_NAME}.deb
    cd ..
    
    print_success "DEB package created"
}

create_rpm_package() {
    print_info "Creating RPM package..."
    
    # This is a simplified RPM creation
    # Full implementation would use rpmbuild
    
    print_warning "RPM package creation is simplified"
}

# Generate checksums
generate_checksums() {
    print_info "Generating checksums..."
    
    cd $BUILD_DIR
    
    # MD5
    find . -type f -exec md5sum {} \; > ../MD5SUMS
    
    # SHA256
    find . -type f -exec sha256sum {} \; > ../SHA256SUMS
    
    cd ..
    
    print_success "Checksums generated"
}

# Create release notes
create_release_notes() {
    print_info "Creating release notes..."
    
    cat > $BUILD_DIR/RELEASE_NOTES.md << EOF
# Proxima & Centauri Release Notes

## Version $VERSION

**Release Date:** $(date +%Y-%m-%d)

**License:** GPLv3

## New Features

- Complete Proxima Compiler with LLVM backend
- Centauri IDE with advanced debugging
- GPU acceleration support (CUDA)
- CPU vectorization (AVX/SSE)
- Git integration
- LLM assistance
- Multi-user collaboration
- Auto-save system

## Installation

### Linux

\`\`\`bash
tar -xzf proxima-centauri-${VERSION}.tar.gz
cd proxima-centauri-${VERSION}
sudo ./install.sh
\`\`\`

### Uninstallation

\`\`\`bash
sudo ./uninstall.sh
\`\`\`

## Quick Start

\`\`\`proxima
// hello.prx
namespace("hello");

main(): int32
    print("Hello, Proxima!\\n");
    return 0;
end

end
\`\`\`

Run:
\`\`\`bash
proxima run hello.prx
\`\`\`

## Documentation

See \`share/proxima/docs/\` for complete documentation.

## Examples

See \`share/proxima/examples/\` for example projects.

## Support

- Website: https://prxlang.ru
- Email: dev@prxlang.ru
- Issues: https://github.com/snowfire-gtc/Proxima-Centauri/issues

## Changelog

### Version $VERSION
- Initial release
- Complete compiler implementation
- Complete IDE implementation
- Full documentation
- Test framework

---

**Proxima Development Team** © $(date +%Y)
EOF
    
    print_success "Release notes created"
}

# Main deployment
main() {
    echo ""
    print_info "Starting deployment process..."
    echo ""
    
    clean_build
    build_compiler
    build_ide
    create_package
    create_release_notes
    create_archives
    generate_checksums
    
    echo ""
    echo "=========================================="
    print_success "Deployment Complete!"
    echo "=========================================="
    echo ""
    echo "Package created: ${PACKAGE_NAME}.tar.gz"
    echo ""
    echo "Contents:"
    ls -lh $BUILD_DIR/
    echo ""
    echo "Archives:"
    ls -lh *.tar.gz *.zip 2>/dev/null || ls -lh *.tar.gz
    echo ""
    echo "To install:"
    echo "  cd $BUILD_DIR"
    echo "  sudo ./install.sh"
    echo ""
    echo "To test:"
    echo "  ./proxima --help"
    echo "  ./centauri"
    echo ""
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --version=*)
            VERSION="${1#*=}"
            shift
            ;;
        --clean)
            clean_build
            exit 0
            ;;
        --compiler-only)
            clean_build
            build_compiler
            exit 0
            ;;
        --ide-only)
            clean_build
            build_ide
            exit 0
            ;;
        --package-only)
            create_package
            exit 0
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --version=VERSION  Set version number"
            echo "  --clean            Clean build directory"
            echo "  --compiler-only    Build only compiler"
            echo "  --ide-only         Build only IDE"
            echo "  --package-only     Create package only"
            echo "  --help             Show this help"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Run main
main

# Proxima Programming Language & Centauri IDE

**Version:** 1.0.0  
**License:** GPLv3  
**Status:** Alpha

Complete development environment for the Proxima programming language - a MATLAB-compatible language with full OOP support, GPU acceleration, and modern IDE integration.

## Components

### Proxima Compiler
- LLVM-based compilation
- MATLAB-compatible syntax
- Full OOP with interfaces
- GPU acceleration (CUDA)
- CPU vectorization (AVX/SSE)
- Automatic type inference
- Garbage collection

### Centauri IDE
- Qt 5.15.2 based
- Advanced code editor
- Visual debugging
- Variable visualization
- Git integration
- LLM assistance
- Multi-user collaboration
- Auto-save system

## Quick Start

### Prerequisites

- CMake 3.15+
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- LLVM 14+
- Qt 5.15.2
- (Optional) CUDA Toolkit
- (Optional) CURL for LLM

### Build Compiler

```bash
cd proxima-compiler
./build.sh
sudo ./build.sh install

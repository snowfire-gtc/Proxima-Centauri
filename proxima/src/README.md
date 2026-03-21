# Proxima Programming Language

**Version:** 1.0.0  
**License:** GPLv3  
**Status:** Alpha

Proxima is a MATLAB-compatible programming language with full OOP support, GPU acceleration, and modern IDE integration.

## Features

- **MATLAB-compatible syntax** with blocks ended by `end`
- **Full OOP** with classes, interfaces, inheritance, and operator overloading
- **Optional typing** with automatic type inference
- **GPU acceleration** via CUDA
- **CPU vectorization** via AVX/SSE
- **LLVM-based** compilation and JIT execution
- **Rich type system** including vectors, matrices, layers, collections
- **Built-in documentation** generation with LaTeX support
- **Modern IDE** (Centauri) with debugging and visualization
- **Git integration** for version control
- **LLM integration** for AI-assisted coding
- **Multi-user collaboration** support

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/proxima-lang/proxima.git
cd proxima

# Build
./build.sh

# Install (optional)
sudo ./build.sh install

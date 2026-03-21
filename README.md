# Proxima Programming Language & Centauri IDE

[![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)](https://github.com/proxima-lang/proxima/releases)
[![Build Status](https://img.shields.io/github/actions/workflow/status/proxima-lang/proxima/build.yml)](https://github.com/proxima-lang/proxima/actions)
[![Issues](https://img.shields.io/github/issues/proxima-lang/proxima)](https://github.com/proxima-lang/proxima/issues)

**Complete development environment for scientific computing and data analysis**

Proxima is a MATLAB-compatible programming language with full OOP support, GPU acceleration, and modern IDE integration. Centauri is the accompanying IDE built with Qt.

## 🚀 Features

### Language (Proxima)
- ✅ MATLAB-compatible syntax with `end` blocks
- ✅ Full OOP with classes, interfaces, inheritance
- ✅ Optional typing with automatic inference
- ✅ GPU acceleration (CUDA)
- ✅ CPU vectorization (AVX/SSE)
- ✅ LLVM-based compilation
- ✅ Automatic memory management (GC)
- ✅ Rich type system (vectors, matrices, layers, collections)
- ✅ Built-in documentation generation
- ✅ Module testing framework
- ✅ C/C++ interface

### IDE (Centauri)
- ✅ Advanced code editor with syntax highlighting
- ✅ Visual debugging with breakpoints
- ✅ Variable visualization (vectors, matrices, layers)
- ✅ Git integration
- ✅ LLM assistance (LM-Studio)
- ✅ Auto-save system
- ✅ Multi-user collaboration
- ✅ Project management
- ✅ Build system integration
- ✅ Documentation generation

## 📦 Installation

### Prerequisites

- **CMake** 3.15+
- **C++ Compiler** (GCC 9+, Clang 10+, MSVC 2019+)
- **LLVM** 14+
- **Qt** 5.15.2+
- **Git**

Optional:

- **CUDA Toolkit** (for GPU support)
- **CURL** (for LLM integration)

### Quick Install

```bash
# Clone repository
git clone https://github.com/proxima-lang/proxima.git
cd proxima

# Build and install
./install.sh install

### Testing

``` bash
# Run compiler tests
cd proxima-compiler/build
ctest

# Run IDE tests
cd centauri-ide/build
ctest

# Run integration tests
./run-tests.sh
```

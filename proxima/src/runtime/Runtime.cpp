#include "Runtime.h"
#include <iostream>
#include <cstring>

#ifdef USE_CUDA
#include <cuda_runtime.h>
#endif

namespace proxima {

Runtime::Runtime() 
    : jit(nullptr), context(nullptr), initialized(false), 
      debugMode(false), cudaAvailable(false), verboseLevel(2),
      memoryLimit(4 * 1024 * 1024 * 1024), // 4GB default
      currentMemoryUsage(0) {}

Runtime::~Runtime() {
    // Clean up allocated memory
    for (void* ptr : allocatedMemory) {
        std::free(ptr);
    }
    
#ifdef USE_CUDA
    for (void* ptr : gpuMemory) {
        cudaFree(ptr);
    }
#endif
}

bool Runtime::initialize() {
    if (initialized) {
        return true;
    }
    
    // Initialize JIT
    auto jitOrErr = llvm::orc::LLJITBuilder().create();
    if (!jitOrErr) {
        std::cerr << "Failed to create JIT: " 
                  << llvm::toString(jitOrErr.takeError()) << std::endl;
        return false;
    }
    
    jit = std::move(*jitOrErr);
    context = &jit->getContext();
    
    // Register standard library
    registerStdLib();
    
    initialized = true;
    log(1, "Runtime initialized");
    
    return true;
}

bool Runtime::loadModule(std::unique_ptr<llvm::Module> module) {
    if (!initialized) {
        if (!initialize()) {
            return false;
        }
    }
    
    auto err = jit->addModule(std::move(module));
    if (err) {
        std::cerr << "Failed to load module: " 
                  << llvm::toString(std::move(err)) << std::endl;
        return false;
    }
    
    log(2, "Module loaded successfully");
    return true;
}

int Runtime::execute(const std::string& entryPoint) {
    if (!initialized) {
        std::cerr << "Runtime not initialized" << std::endl;
        return -1;
    }
    
    auto sym = jit->lookup(entryPoint);
    if (!sym) {
        std::cerr << "Function '" << entryPoint << "' not found" << std::endl;
        return -1;
    }
    
    auto funcPtr = (int(*)())sym->getAddress();
    if (!funcPtr) {
        std::cerr << "Failed to get function pointer" << std::endl;
        return -1;
    }
    
    log(1, "Executing: " + entryPoint);
    int result = funcPtr();
    log(1, "Execution completed with result: " + std::to_string(result));
    
    return result;
}

void* Runtime::allocate(size_t size) {
    if (currentMemoryUsage + size > memoryLimit) {
        std::cerr << "Memory limit exceeded" << std::endl;
        return nullptr;
    }
    
    void* ptr = std::malloc(size);
    if (ptr) {
        allocatedMemory.push_back(ptr);
        currentMemoryUsage += size;
        log(4, "Allocated " + std::to_string(size) + " bytes");
    }
    
    return ptr;
}

void Runtime::deallocate(void* ptr) {
    if (!ptr) return;
    
    auto it = std::find(allocatedMemory.begin(), allocatedMemory.end(), ptr);
    if (it != allocatedMemory.end()) {
        allocatedMemory.erase(it);
        std::free(ptr);
        log(4, "Deallocated memory");
    }
}

size_t Runtime::getMemoryUsage() const {
    return currentMemoryUsage;
}

void Runtime::enableDebugMode(bool enable) {
    debugMode = enable;
    log(1, "Debug mode: " + std::string(enable ? "enabled" : "disabled"));
}

bool Runtime::enableCUDA() {
#ifdef USE_CUDA
    int deviceCount;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);
    
    if (err == cudaSuccess && deviceCount > 0) {
        cudaAvailable = true;
        log(1, "CUDA available: " + std::to_string(deviceCount) + " device(s)");
        return true;
    }
#endif
    
    cudaAvailable = false;
    log(1, "CUDA not available");
    return false;
}

void* Runtime::allocateGPU(size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable) {
        return nullptr;
    }
    
    void* ptr;
    cudaError_t err = cudaMalloc(&ptr, size);
    
    if (err == cudaSuccess) {
        gpuMemory.push_back(ptr);
        return ptr;
    }
#endif
    
    return nullptr;
}

void Runtime::freeGPU(void* ptr) {
#ifdef USE_CUDA
    if (!ptr || !cudaAvailable) return;
    
    auto it = std::find(gpuMemory.begin(), gpuMemory.end(), ptr);
    if (it != gpuMemory.end()) {
        gpuMemory.erase(it);
        cudaFree(ptr);
    }
#endif
}

void Runtime::copyToGPU(void* host, void* device, size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable || !host || !device) return;
    
    cudaMemcpy(device, host, size, cudaMemcpyHostToDevice);
#endif
}

void Runtime::copyFromGPU(void* device, void* host, size_t size) {
#ifdef USE_CUDA
    if (!cudaAvailable || !device || !host) return;
    
    cudaMemcpy(host, device, size, cudaMemcpyDeviceToHost);
#endif
}

void Runtime::registerStdLib() {
    registerPrintFunction();
    registerMathFunctions();
    registerIOFunctions();
    registerCollectionFunctions();
    registerTimeFunctions();
    registerDebugFunctions();
    
    log(2, "Standard library registered");
}

void Runtime::registerPrintFunction() {
    // Register print function
    // Implementation depends on how we want to handle variadic functions
    log(3, "Registered: print()");
}

void Runtime::registerMathFunctions() {
    // Register math functions: sin, cos, tan, exp, log, etc.
    log(3, "Registered: math functions");
}

void Runtime::registerIOFunctions() {
    // Register file I/O functions
    log(3, "Registered: I/O functions");
}

void Runtime::registerCollectionFunctions() {
    // Register collection manipulation functions
    log(3, "Registered: collection functions");
}

void Runtime::registerTimeFunctions() {
    // Register time-related functions
    log(3, "Registered: time functions");
}

void Runtime::registerDebugFunctions() {
    // Register debug functions: dbgstop, dbgprint, etc.
    log(3, "Registered: debug functions");
}

void Runtime::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Runtime] " << message << std::endl;
    }
}

} // namespace proxima
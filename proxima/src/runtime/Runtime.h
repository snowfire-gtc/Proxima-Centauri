#ifndef PROXIMA_RUNTIME_H
#define PROXIMA_RUNTIME_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>

namespace proxima {

class Runtime {
public:
    Runtime();
    ~Runtime();
    
    bool initialize();
    bool loadModule(std::unique_ptr<llvm::Module> module);
    int execute(const std::string& entryPoint = "main");
    
    void* getFunctionPointer(const std::string& name);
    
    // Memory management
    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getMemoryUsage() const;
    size_t getMemoryLimit() const { return memoryLimit; }
    void setMemoryLimit(size_t limit) { memoryLimit = limit; }
    
    // Debug support
    void enableDebugMode(bool enable);
    bool isDebugMode() const { return debugMode; }
    void setVerboseLevel(int level) { verboseLevel = level; }
    
    // GPU support
    bool isCUDAAvailable() const { return cudaAvailable; }
    bool enableCUDA();
    void* allocateGPU(size_t size);
    void freeGPU(void* ptr);
    void copyToGPU(void* host, void* device, size_t size);
    void copyFromGPU(void* device, void* host, size_t size);
    
    // Standard library functions
    void registerStdLib();
    
private:
    std::unique_ptr<llvm::orc::LLJIT> jit;
    llvm::LLVMContext* context;
    bool initialized;
    bool debugMode;
    bool cudaAvailable;
    int verboseLevel;
    size_t memoryLimit;
    size_t currentMemoryUsage;
    
    std::unordered_map<std::string, void*> functionPointers;
    std::vector<void*> allocatedMemory;
    std::vector<void*> gpuMemory;
    
    void registerPrintFunction();
    void registerMathFunctions();
    void registerIOFunctions();
    void registerCollectionFunctions();
    void registerTimeFunctions();
    void registerDebugFunctions();
    
    void log(int level, const std::string& message);
};

} // namespace proxima

#endif // PROXIMA_RUNTIME_H
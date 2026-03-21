#ifndef PROXIMA_RUNTIME_H
#define PROXIMA_RUNTIME_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

#ifdef HAVE_LLVM
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#endif

#include "stdlib/Time.h"
#include "stdlib/Collection.h"

namespace proxima {

struct RuntimeStats {
    size_t memoryUsed;
    size_t memoryLimit;
    size_t allocations;
    size_t gpuAllocations;
    int64_t uptime;
    int verboseLevel;
    bool debugMode;
    bool cudaAvailable;
};

class Runtime {
public:
    Runtime();
    ~Runtime();
    
    // Initialization
    bool initialize();
    bool loadModule(std::unique_ptr<llvm::Module> module);
    int execute(const std::string& entryPoint = "main");
    void* getFunctionPointer(const std::string& name);
    
    // Memory management
    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getMemoryUsage() const;
    size_t getMemoryLimit() const { return memoryLimit; }
    void setMemoryLimit(size_t limit);
    
    // Debug support
    void enableDebugMode(bool enable);
    bool isDebugMode() const { return debugMode; }
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }
    
    // GPU support
    bool isCUDAAvailable() const { return cudaAvailable; }
    bool enableCUDA();
    void* allocateGPU(size_t size);
    void freeGPU(void* ptr);
    void copyToGPU(void* host, void* device, size_t size);
    void copyFromGPU(void* device, void* host, size_t size);
    
    // Standard library
    void registerStdLib();
    
    // System info
    size_t getSystemMemoryFree() const;
    size_t getSystemMemoryTotal() const;
    double getCPUUsage() const;
    size_t getDiskFree(const std::string& path) const;
    std::string getOSInfo() const;
    std::string getCPUInfo() const;
    int getCPUCount() const;
    
    // Debug functions (language.txt #43)
    void dbgstop();
    void dbgprint(const std::string& message, int level = 0);
    void dbgcontext();
    void dbgstack();
    
    // GEM interface (language.txt #44)
    void gem_init(void* object);
    void gem_reset(void* object);
    std::pair<Time, bool> gem_update(void* object, const Time& currentTime);
    void gem_show(void* object);
    Collection gem_get_metrics(void* object);
    void gem_set_params(void* object, const Collection& params);
    Collection gem_get_params(void* object);
    std::string gem_get_name(void* object);
    void gem_publish(void* object, void* document);
    Collection gem_store(void* object);
    void gem_restore(void* object, const Collection& state);
    
    // Parallel execution
    void parallelFor(int start, int end, int step, 
                    std::function<void(int)> body, 
                    int threads = 4, void* array = nullptr);
    
    // Serialization
    std::string serialize(const void* data, size_t size, const std::string& type);
    void* deserialize(const std::string& data, size_t& size, const std::string& type);
    
    // Utilities
    void sleep(int64_t milliseconds);
    int64_t getTimestamp() const;
    void exit(int code);
    int getpid() const;
    std::string getCWD() const;
    bool setCWD(const std::string& path);
    bool fileExists(const std::string& path) const;
    bool directoryExists(const std::string& path) const;
    bool createDirectory(const std::string& path);
    bool deleteFile(const std::string& path);
    std::vector<std::string> listDirectory(const std::string& path);
    
    // Configuration
    void setAllowCUDA(bool allow);
    void setAllowAVX2(bool allow);
    void setAllowSSE4(bool allow);
    bool isCUDAAllowed() const;
    bool isAVX2Allowed() const;
    bool isSSE4Allowed() const;
    
    // Statistics
    RuntimeStats getStats() const;
    void resetStats();
    
    // Timing
    void startTimer(const std::string& name);
    double stopTimer(const std::string& name);
    double getTimer(const std::string& name) const;
    
    // Error handling
    void setError(const std::string& error);
    std::string getError() const;
    bool hasError() const;
    void clearError();
    
private:
    void registerPrintFunction();
    void registerMathFunctions();
    void registerIOFunctions();
    void registerCollectionFunctions();
    void registerTimeFunctions();
    void registerDebugFunctions();
    void registerSystemFunctions();
    void registerMemoryFunctions();
    
    void log(int level, const std::string& message);
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    void logDebug(const std::string& message);
    
    #ifdef HAVE_LLVM
    std::unique_ptr<llvm::orc::LLJIT> jit;
    llvm::LLVMContext* context;
    #endif
    
    bool initialized;
    bool debugMode;
    bool cudaAvailable;
    bool cudaAllowed;
    bool avx2Allowed;
    bool sse4Allowed;
    int verboseLevel;
    size_t memoryLimit;
    size_t currentMemoryUsage;
    int64_t startTime;
    
    std::vector<void*> allocatedMemory;
    std::vector<void*> gpuMemory;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> timers;
    
    std::mutex memoryMutex;
    std::mutex debugMutex;
    std::condition_variable debugCondition;
    bool stopRequested;
    
    std::string lastError;
};

} // namespace proxima

#endif // PROXIMA_RUNTIME_H
#ifndef PROXIMA_PROFILER_H
#define PROXIMA_PROFILER_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>

namespace proxima {

struct FunctionProfile {
    std::string name;
    std::string file;
    int line;
    int callCount;
    double totalTime;
    double minTime;
    double maxTime;
    double avgTime;
    double selfTime;
    std::vector<std::string> callers;
    std::vector<std::string> callees;
};

struct LineProfile {
    int line;
    int executionCount;
    double totalTime;
    bool isHotspot;
};

struct MemoryProfile {
    size_t totalAllocated;
    size_t totalFreed;
    size_t peakUsage;
    size_t currentUsage;
    int allocationCount;
    int deallocationCount;
};

class Profiler {
public:
    static Profiler& getInstance();
    
    // Control
    void start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const { return running; }
    
    // Function profiling
    void enterFunction(const std::string& name, const std::string& file, int line);
    void exitFunction(const std::string& name);
    
    // Line profiling
    void recordLineExecution(const std::string& file, int line);
    
    // Memory profiling
    void recordAllocation(size_t size);
    void recordDeallocation(size_t size);
    
    // GPU profiling
    void recordGPUKernelLaunch(const std::string& kernel, double time);
    void recordGPUTransfer(const std::string& direction, size_t bytes, double time);
    
    // Results
    std::vector<FunctionProfile> getFunctionProfiles() const;
    std::map<std::string, std::vector<LineProfile>> getLineProfiles() const;
    MemoryProfile getMemoryProfile() const;
    
    // Reporting
    std::string generateReport() const;
    std::string generateFlameGraph() const;
    std::string generateCallGraph() const;
    void saveReport(const std::string& path);
    void saveFlameGraph(const std::string& path);
    
    // Configuration
    void setEnabled(bool enable);
    bool isEnabled() const { return enabled; }
    void setSamplingInterval(int microseconds);
    int getSamplingInterval() const { return samplingInterval; }
    
    // Hotspot detection
    std::vector<std::pair<std::string, int>> getHotspots(int count = 10) const;
    std::vector<std::string> getBottlenecks() const;
    
private:
    Profiler();
    ~Profiler();
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;
    
    void calculateStatistics();
    std::string formatTime(double seconds) const;
    std::string formatSize(size_t bytes) const;
    
    bool running;
    bool enabled;
    bool paused;
    int samplingInterval;
    
    std::map<std::string, FunctionProfile> functionProfiles;
    std::map<std::string, std::map<int, LineProfile>> lineProfiles;
    MemoryProfile memoryProfile;
    
    std::vector<std::pair<std::string, double>> callStack;
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point pauseTime;
    
    std::mutex profilerMutex;
    
    // GPU profiling
    std::map<std::string, double> gpuKernelTimes;
    std::map<std::string, std::pair<size_t, double>> gpuTransferStats;
};

// RAII helper for function profiling
class ProfileScope {
public:
    ProfileScope(const std::string& name, const std::string& file, int line)
        : name(name), file(file), line(line) {
        Profiler::getInstance().enterFunction(name, file, line);
    }
    
    ~ProfileScope() {
        Profiler::getInstance().exitFunction(name);
    }
    
private:
    std::string name;
    std::string file;
    int line;
};

#define PROFILE_FUNCTION() \
    proxima::ProfileScope _profile_scope(__FUNCTION__, __FILE__, __LINE__)

#define PROFILE_SCOPE(name) \
    proxima::ProfileScope _profile_scope(name, __FILE__, __LINE__)

} // namespace proxima

#endif // PROXIMA_PROFILER_H
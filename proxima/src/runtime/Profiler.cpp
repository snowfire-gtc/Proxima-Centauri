#include "Profiler.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include "utils/Logger.h"

namespace proxima {

Profiler& Profiler::getInstance() {
    static Profiler instance;
    return instance;
}

Profiler::Profiler()
    : running(false)
    , enabled(true)
    , paused(false)
    , samplingInterval(1000) {
    
    memoryProfile.totalAllocated = 0;
    memoryProfile.totalFreed = 0;
    memoryProfile.peakUsage = 0;
    memoryProfile.currentUsage = 0;
    memoryProfile.allocationCount = 0;
    memoryProfile.deallocationCount = 0;
}

Profiler::~Profiler() {
    if (running) {
        stop();
    }
}

void Profiler::start() {
    if (running) return;
    
    running = true;
    paused = false;
    startTime = std::chrono::high_resolution_clock::now();
    
    LOG_INFO("Profiler started");
}

void Profiler::stop() {
    if (!running) return;
    
    running = false;
    calculateStatistics();
    
    LOG_INFO("Profiler stopped");
    LOG_INFO("Total profiling time: " + formatTime(getMemoryProfile().peakUsage));
}

void Profiler::pause() {
    if (!running || paused) return;
    
    paused = true;
    pauseTime = std::chrono::high_resolution_clock::now();
}

void Profiler::resume() {
    if (!running || !paused) return;
    
    paused = false;
    auto resumeTime = std::chrono::high_resolution_clock::now();
    auto pauseDuration = std::chrono::duration<double>(resumeTime - pauseTime).count();
    
    // Adjust all function times
    for (auto& pair : functionProfiles) {
        pair.second.totalTime -= pauseDuration;
    }
}

void Profiler::enterFunction(const std::string& name, const std::string& file, int line) {
    if (!enabled || paused) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    auto now = std::chrono::high_resolution_clock::now();
    
    // Initialize profile if needed
    if (functionProfiles.find(name) == functionProfiles.end()) {
        FunctionProfile profile;
        profile.name = name;
        profile.file = file;
        profile.line = line;
        profile.callCount = 0;
        profile.totalTime = 0;
        profile.minTime = 1e10;
        profile.maxTime = 0;
        profile.avgTime = 0;
        profile.selfTime = 0;
        functionProfiles[name] = profile;
    }
    
    // Track caller-callee relationship
    if (!callStack.empty()) {
        std::string caller = callStack.back().first;
        functionProfiles[caller].callees.push_back(name);
        functionProfiles[name].callers.push_back(caller);
    }
    
    // Push to call stack
    callStack.push_back({name, std::chrono::duration<double>(
        now - startTime).count()});
}

void Profiler::exitFunction(const std::string& name) {
    if (!enabled || paused) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    auto now = std::chrono::high_resolution_clock::now();
    
    if (callStack.empty()) return;
    
    // Find matching function in call stack
    for (auto it = callStack.rbegin(); it != callStack.rend(); ++it) {
        if (it->first == name) {
            double endTime = std::chrono::duration<double>(now - startTime).count();
            double duration = endTime - it->second;
            
            FunctionProfile& profile = functionProfiles[name];
            profile.callCount++;
            profile.totalTime += duration;
            profile.minTime = std::min(profile.minTime, duration);
            profile.maxTime = std::max(profile.maxTime, duration);
            
            callStack.erase(std::next(it).base());
            break;
        }
    }
}

void Profiler::recordLineExecution(const std::string& file, int line) {
    if (!enabled || paused) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    auto& fileProfiles = lineProfiles[file];
    if (fileProfiles.find(line) == fileProfiles.end()) {
        LineProfile profile;
        profile.line = line;
        profile.executionCount = 0;
        profile.totalTime = 0;
        profile.isHotspot = false;
        fileProfiles[line] = profile;
    }
    
    fileProfiles[line].executionCount++;
}

void Profiler::recordAllocation(size_t size) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    memoryProfile.totalAllocated += size;
    memoryProfile.currentUsage += size;
    memoryProfile.allocationCount++;
    
    if (memoryProfile.currentUsage > memoryProfile.peakUsage) {
        memoryProfile.peakUsage = memoryProfile.currentUsage;
    }
}

void Profiler::recordDeallocation(size_t size) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    memoryProfile.totalFreed += size;
    memoryProfile.currentUsage -= size;
    memoryProfile.deallocationCount++;
}

void Profiler::recordGPUKernelLaunch(const std::string& kernel, double time) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    gpuKernelTimes[kernel] += time;
}

void Profiler::recordGPUTransfer(const std::string& direction, size_t bytes, double time) {
    if (!enabled) return;
    
    std::lock_guard<std::mutex> lock(profilerMutex);
    
    std::string key = direction;
    gpuTransferStats[key].first += bytes;
    gpuTransferStats[key].second += time;
}

void Profiler::calculateStatistics() {
    for (auto& pair : functionProfiles) {
        FunctionProfile& profile = pair.second;
        if (profile.callCount > 0) {
            profile.avgTime = profile.totalTime / profile.callCount;
        }
        
        // Remove duplicates from callers/callees
        std::sort(profile.callers.begin(), profile.callers.end());
        profile.callers.erase(std::unique(profile.callers.begin(), profile.callers.end()), 
                             profile.callers.end());
        
        std::sort(profile.callees.begin(), profile.callees.end());
        profile.callees.erase(std::unique(profile.callees.begin(), profile.callees.end()), 
                             profile.callees.end());
    }
    
    // Detect hotspots
    for (auto& filePair : lineProfiles) {
        for (auto& linePair : filePair.second) {
            LineProfile& profile = linePair.second;
            // Mark as hotspot if execution count is above average
            profile.isHotspot = true; // Simplified
        }
    }
}

std::vector<FunctionProfile> Profiler::getFunctionProfiles() const {
    std::vector<FunctionProfile> profiles;
    for (const auto& pair : functionProfiles) {
        profiles.push_back(pair.second);
    }
    
    // Sort by total time
    std::sort(profiles.begin(), profiles.end(),
        [](const FunctionProfile& a, const FunctionProfile& b) {
            return a.totalTime > b.totalTime;
        });
    
    return profiles;
}

std::map<std::string, std::vector<LineProfile>> Profiler::getLineProfiles() const {
    std::map<std::string, std::vector<LineProfile>> result;
    
    for (const auto& filePair : lineProfiles) {
        for (const auto& linePair : filePair.second) {
            result[filePair.first].push_back(linePair.second);
        }
    }
    
    return result;
}

MemoryProfile Profiler::getMemoryProfile() const {
    return memoryProfile;
}

std::string Profiler::generateReport() const {
    std::ostringstream oss;
    
    oss << "========================================\n";
    oss << "Proxima Performance Report\n";
    oss << "========================================\n\n";
    
    // Function profiles
    oss << "Function Profiles (sorted by total time):\n";
    oss << "----------------------------------------\n";
    
    auto profiles = getFunctionProfiles();
    for (const auto& profile : profiles) {
        oss << profile.name << " (" << profile.file << ":" << profile.line << ")\n";
        oss << "  Calls: " << profile.callCount << "\n";
        oss << "  Total: " << formatTime(profile.totalTime) << "\n";
        oss << "  Avg: " << formatTime(profile.avgTime) << "\n";
        oss << "  Min: " << formatTime(profile.minTime) << "\n";
        oss << "  Max: " << formatTime(profile.maxTime) << "\n";
        oss << "\n";
    }
    
    // Memory profile
    oss << "\nMemory Profile:\n";
    oss << "----------------------------------------\n";
    oss << "Total Allocated: " << formatSize(memoryProfile.totalAllocated) << "\n";
    oss << "Total Freed: " << formatSize(memoryProfile.totalFreed) << "\n";
    oss << "Peak Usage: " << formatSize(memoryProfile.peakUsage) << "\n";
    oss << "Current Usage: " << formatSize(memoryProfile.currentUsage) << "\n";
    oss << "Allocations: " << memoryProfile.allocationCount << "\n";
    oss << "Deallocations: " << memoryProfile.deallocationCount << "\n";
    
    // GPU profile
    oss << "\nGPU Profile:\n";
    oss << "----------------------------------------\n";
    for (const auto& pair : gpuKernelTimes) {
        oss << pair.first << ": " << formatTime(pair.second) << "\n";
    }
    
    for (const auto& pair : gpuTransferStats) {
        oss << pair.first << ": " << formatSize(pair.second.first) 
            << " in " << formatTime(pair.second.second) << "\n";
    }
    
    return oss.str();
}

std::string Profiler::generateFlameGraph() const {
    // Generate folded stack format for flame graph
    std::ostringstream oss;
    
    for (const auto& pair : functionProfiles) {
        const FunctionProfile& profile = pair.second;
        
        // Simple single-function stacks
        oss << profile.name << " " << profile.callCount << "\n";
        
        // Caller-callee stacks
        for (const auto& caller : profile.callers) {
            oss << caller << ";" << profile.name << " " << profile.callCount << "\n";
        }
    }
    
    return oss.str();
}

std::string Profiler::generateCallGraph() const {
    std::ostringstream oss;
    
    oss << "digraph CallGraph {\n";
    oss << "  rankdir=TB;\n";
    oss << "  node [shape=box];\n\n";
    
    for (const auto& pair : functionProfiles) {
        const FunctionProfile& profile = pair.second;
        
        for (const auto& callee : profile.callees) {
            oss << "  \"" << profile.name << "\" -> \"" << callee << "\";\n";
        }
    }
    
    oss << "}\n";
    
    return oss.str();
}

void Profiler::saveReport(const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << generateReport();
        file.close();
        LOG_INFO("Profile report saved: " + path);
    }
}

void Profiler::saveFlameGraph(const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << generateFlameGraph();
        file.close();
        LOG_INFO("Flame graph data saved: " + path);
    }
}

void Profiler::setEnabled(bool enable) {
    enabled = enable;
}

void Profiler::setSamplingInterval(int microseconds) {
    samplingInterval = microseconds;
}

std::vector<std::pair<std::string, int>> Profiler::getHotspots(int count) const {
    std::vector<std::pair<std::string, int>> hotspots;
    
    for (const auto& pair : functionProfiles) {
        hotspots.push_back({pair.first, pair.second.callCount});
    }
    
    std::sort(hotspots.begin(), hotspots.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
    
    if (hotspots.size() > static_cast<size_t>(count)) {
        hotspots.resize(count);
    }
    
    return hotspots;
}

std::vector<std::string> Profiler::getBottlenecks() const {
    std::vector<std::string> bottlenecks;
    
    auto profiles = getFunctionProfiles();
    double totalTime = 0;
    
    for (const auto& profile : profiles) {
        totalTime += profile.totalTime;
    }
    
    // Functions taking more than 10% of total time
    for (const auto& profile : profiles) {
        if (totalTime > 0 && (profile.totalTime / totalTime) > 0.1) {
            bottlenecks.push_back(profile.name);
        }
    }
    
    return bottlenecks;
}

std::string Profiler::formatTime(double seconds) const {
    std::ostringstream oss;
    
    if (seconds < 0.001) {
        oss << std::fixed << std::setprecision(3) << (seconds * 1000000) << " μs";
    } else if (seconds < 1) {
        oss << std::fixed << std::setprecision(3) << (seconds * 1000) << " ms";
    } else {
        oss << std::fixed << std::setprecision(3) << seconds << " s";
    }
    
    return oss.str();
}

std::string Profiler::formatSize(size_t bytes) const {
    std::ostringstream oss;
    
    if (bytes < 1024) {
        oss << bytes << " B";
    } else if (bytes < 1024 * 1024) {
        oss << std::fixed << std::setprecision(2) << (bytes / 1024.0) << " KB";
    } else if (bytes < 1024 * 1024 * 1024) {
        oss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0)) << " MB";
    } else {
        oss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    }
    
    return oss.str();
}

} // namespace proxima
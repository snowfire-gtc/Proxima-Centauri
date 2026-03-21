#ifndef PROXIMA_CONFIG_H
#define PROXIMA_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>

namespace proxima {

struct Config {
    // Build settings
    std::string outputPath;
    std::string sourceFile;
    std::vector<std::string> sourceFiles;
    std::vector<std::string> includePaths;
    std::vector<std::string> defines;
    
    // Optimization
    int optimizationLevel;
    bool debugSymbols;
    bool releaseMode;
    
    // Hardware acceleration
    bool enableCUDA;
    bool enableAVX2;
    bool enableSSE4;
    bool enableOpenMP;
    
    // Memory limits
    size_t maxMemory;
    
    // Verbose level
    int verboseLevel;
    
    // Target
    std::string targetArch;
    std::string targetOS;
    
    // Output format
    std::string outputFormat; // "executable", "library", "bitcode"
    
    // IDE integration
    bool ideMode;
    std::string ideProtocol;
    
    Config() 
        : optimizationLevel(2),
          debugSymbols(true),
          releaseMode(false),
          enableCUDA(false),
          enableAVX2(false),
          enableSSE4(false),
          enableOpenMP(false),
          maxMemory(4 * 1024 * 1024 * 1024),
          verboseLevel(2),
          targetArch("x86_64"),
          targetOS("linux"),
          outputFormat("executable"),
          ideMode(false) {}
    
    void loadFromFile(const std::string& path);
    void saveToFile(const std::string& path) const;
    
    std::string getOptimizationFlags() const;
    std::string getTargetFlags() const;
};

} // namespace proxima

#endif // PROXIMA_CONFIG_H
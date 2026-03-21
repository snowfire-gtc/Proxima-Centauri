#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace proxima {

void Config::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << path << std::endl;
        return;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    // Parse collection format
    // Simple parser for build.rules format
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Skip comments
        if (line.find("//") == 0 || line.find("/*") == 0) continue;
        
        // Parse key-value pairs
        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t\""));
            key.erase(key.find_last_not_of(" \t\"") + 1);
            value.erase(0, value.find_first_not_of(" \t\""));
            value.erase(value.find_last_not_of(" \t\"") + 1);
            
            if (key == "compiler_version") {
                // Version check
            } else if (key == "optimization_level") {
                optimizationLevel = std::stoi(value);
            } else if (key == "target_arch") {
                targetArch = value;
            } else if (key == "enable_cuda") {
                enableCUDA = (value == "true" || value == "1");
            } else if (key == "enable_avx2") {
                enableAVX2 = (value == "true" || value == "1");
            } else if (key == "enable_sse4") {
                enableSSE4 = (value == "true" || value == "1");
            } else if (key == "max_memory") {
                // Parse memory size (e.g., "4GB")
                if (value.find("GB") != std::string::npos) {
                    maxMemory = std::stoull(value) * 1024 * 1024 * 1024;
                } else if (value.find("MB") != std::string::npos) {
                    maxMemory = std::stoull(value) * 1024 * 1024;
                }
            } else if (key == "debug_symbols") {
                debugSymbols = (value == "true" || value == "1");
            } else if (key == "output_format") {
                outputFormat = value;
            }
        }
    }
}

void Config::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write config file: " << path << std::endl;
        return;
    }
    
    file << "[\n";
    file << "    \"compiler_version\", \"1.0.0\",\n";
    file << "    \"optimization_level\", \"" << optimizationLevel << "\",\n";
    file << "    \"target_arch\", \"" << targetArch << "\",\n";
    file << "    \"enable_cuda\", " << (enableCUDA ? "true" : "false") << ",\n";
    file << "    \"enable_avx2\", " << (enableAVX2 ? "true" : "false") << ",\n";
    file << "    \"enable_sse4\", " << (enableSSE4 ? "true" : "false") << ",\n";
    file << "    \"max_memory\", \"" << (maxMemory / 1024 / 1024 / 1024) << "GB\",\n";
    file << "    \"debug_symbols\", " << (debugSymbols ? "true" : "false") << ",\n";
    file << "    \"output_format\", \"" << outputFormat << "\"\n";
    file << "]\n";
    
    file.close();
}

std::string Config::getOptimizationFlags() const {
    switch (optimizationLevel) {
        case 0: return "-O0";
        case 1: return "-O1";
        case 2: return "-O2";
        case 3: return "-O3";
        default: return "-O2";
    }
}

std::string Config::getTargetFlags() const {
    std::string flags = "-target " + targetArch;
    
    if (enableAVX2) {
        flags += " -mavx2";
    }
    if (enableSSE4) {
        flags += " -msse4";
    }
    
    return flags;
}

} // namespace proxima
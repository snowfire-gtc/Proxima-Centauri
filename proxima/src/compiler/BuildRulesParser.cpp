#include "BuildRulesParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace proxima {

BuildRulesParser::BuildRulesParser() {}

bool BuildRulesParser::parse(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        lastError = "Could not open file: " + filename;
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    return parseString(content);
}

bool BuildRulesParser::parseString(const std::string& content) {
    lastError.clear();
    modules.clear();
    
    // Remove comments
    std::string cleaned;
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        // Remove single-line comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        cleaned += line + "\n";
    }
    
    // Parse collection format
    if (!parseCollection(cleaned)) {
        return false;
    }
    
    return true;
}

bool BuildRulesParser::parseCollection(const std::string& content) {
    // Simple parser for collection format
    // [ "key", "value",, "key2", "value2",, ... ]
    
    std::regex keyValuePattern(R"("([^"]+)"\s*,\s*"([^"]*)")");
    
    std::string currentKey;
    std::string currentValue;
    bool expectKey = true;
    
    std::istringstream iss(content);
    std::string token;
    
    // Remove brackets
    std::string text = content;
    text.erase(std::remove(text.begin(), text.end(), '['), text.end());
    text.erase(std::remove(text.begin(), text.end(), ']'), text.end());
    
    // Split by commas
    std::vector<std::string> tokens;
    std::stringstream ss(text);
    
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty() && token != ",") {
            tokens.push_back(token);
        }
    }
    
    // Parse key-value pairs
    for (size_t i = 0; i < tokens.size(); i++) {
        std::string t = tokens[i];
        
        // Skip double commas
        if (t == ",") continue;
        
        // Extract string value
        if (t.front() == '"' && t.back() == '"') {
            t = t.substr(1, t.length() - 2);
        }
        
        if (expectKey) {
            currentKey = t;
            expectKey = false;
        } else {
            currentValue = t;
            extractConfigValue(currentKey, currentValue);
            expectKey = true;
        }
    }
    
    return true;
}

void BuildRulesParser::extractConfigValue(const std::string& key, const std::string& value) {
    if (key == "compiler_version") {
        // Version check could go here
    }
    else if (key == "optimization_level") {
        config.optimizationLevel = std::stoi(value);
    }
    else if (key == "target_arch") {
        config.targetArch = value;
    }
    else if (key == "enable_cuda") {
        config.enableCUDA = (value == "true" || value == "1");
    }
    else if (key == "enable_avx2") {
        config.enableAVX2 = (value == "true" || value == "1");
    }
    else if (key == "enable_sse4") {
        config.enableSSE4 = (value == "true" || value == "1");
    }
    else if (key == "max_memory") {
        if (value.find("GB") != std::string::npos) {
            config.maxMemory = std::stoull(value) * 1024 * 1024 * 1024;
        } else if (value.find("MB") != std::string::npos) {
            config.maxMemory = std::stoull(value) * 1024 * 1024;
        } else {
            config.maxMemory = std::stoull(value);
        }
    }
    else if (key == "debug_symbols") {
        config.debugSymbols = (value == "true" || value == "1");
    }
    else if (key == "output_format") {
        config.outputFormat = value;
    }
    else if (key == "output") {
        config.outputPath = value;
    }
    else if (key == "modules") {
        modules = parseStringArray(value);
        config.sourceFiles = modules;
    }
    else if (key == "defines") {
        config.defines = parseStringArray(value);
    }
    else if (key == "include_paths") {
        config.includePaths = parseStringArray(value);
    }
}

std::string BuildRulesParser::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\"");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> BuildRulesParser::parseStringArray(const std::string& str) {
    std::vector<std::string> result;
    
    // Simple array parser
    std::string text = str;
    text.erase(std::remove(text.begin(), text.end(), '['), text.end());
    text.erase(std::remove(text.begin(), text.end(), ']'), text.end());
    
    std::istringstream iss(text);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        token = trim(token);
        if (!token.empty() && token != ",") {
            result.push_back(token);
        }
    }
    
    return result;
}

} // namespace proxima
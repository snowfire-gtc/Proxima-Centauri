#ifndef PROXIMA_BUILD_RULES_PARSER_H
#define PROXIMA_BUILD_RULES_PARSER_H

#include "../stdlib/Collection.h"
#include "Config.h"
#include <string>
#include <vector>

namespace proxima {

class BuildRulesParser {
public:
    BuildRulesParser();
    
    bool parse(const std::string& filename);
    bool parseString(const std::string& content);
    
    Config getConfig() const { return config; }
    stdlib::Collection getRules() const { return rules; }
    
    std::string getLastError() const { return lastError; }
    bool hasErrors() const { return !lastError.empty(); }
    
    // Accessors
    std::string getOutputPath() const { return config.outputPath; }
    int getOptimizationLevel() const { return config.optimizationLevel; }
    bool isDebugSymbols() const { return config.debugSymbols; }
    bool isCUDAEnabled() const { return config.enableCUDA; }
    bool isAVX2Enabled() const { return config.enableAVX2; }
    std::vector<std::string> getModules() const { return modules; }
    std::vector<std::string> getDefines() const { return config.defines; }
    std::vector<std::string> getIncludePaths() const { return config.includePaths; }
    
private:
    Config config;
    stdlib::Collection rules;
    std::string lastError;
    std::vector<std::string> modules;
    
    bool parseCollection(const std::string& content);
    void extractConfigValue(const std::string& key, const std::string& value);
    std::string trim(const std::string& str);
    std::vector<std::string> parseStringArray(const std::string& str);
};

} // namespace proxima

#endif // PROXIMA_BUILD_RULES_PARSER_H
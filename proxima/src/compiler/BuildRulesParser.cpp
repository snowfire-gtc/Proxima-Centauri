#include "BuildRulesParser.h"
#include "../utils/CollectionParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace proxima {

// Helper function to parse memory size string (e.g., "4GB", "512MB")
static size_t parseMemorySize(const std::string& str) {
    // Simple implementation - can be extended
    size_t value = std::stoull(str);
    if (str.find("GB") != std::string::npos || str.find("gb") != std::string::npos) {
        return value * 1024 * 1024 * 1024;
    } else if (str.find("MB") != std::string::npos || str.find("mb") != std::string::npos) {
        return value * 1024 * 1024;
    } else if (str.find("KB") != std::string::npos || str.find("kb") != std::string::npos) {
        return value * 1024;
    }
    return value;
}

BuildRulesParser::BuildRulesParser() {}

bool BuildRulesParser::parse(const std::string& filename) {
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parseFile(filename);

    if (!result.success) {
        lastError = result.error;
        return false;
    }

    // Parse collection
    const auto& collection = result.value;

    // Extract values
    config.optimizationLevel = static_cast<int>(collection.get("optimization_level").asNumber());
    config.targetArch = collection.get("target_arch").asString();
    config.enableCUDA = collection.get("enable_cuda").asBoolean();
    config.enableAVX2 = collection.get("enable_avx2").asBoolean();
    config.maxMemory = parseMemorySize(collection.get("max_memory").asString());
    config.debugSymbols = collection.get("debug_symbols").asBoolean();
    config.outputFormat = collection.get("output_format").asString();

    // Parse modules array
    CollectionParser::Value modulesValue = collection.get("modules");
    if (modulesValue.isArray()) {
        for (const auto& moduleValue : modulesValue.asArray()) {
            modules.push_back(moduleValue.asString());
        }
    }

    // Parse defines array
    CollectionParser::Value definesValue = collection.get("defines");
    if (definesValue.isArray()) {
        for (const auto& defineValue : definesValue.asArray()) {
            config.defines.push_back(defineValue.asString());
        }
    }

    // Parse include_paths array
    CollectionParser::Value pathsValue = collection.get("include_paths");
    if (pathsValue.isArray()) {
        for (const auto& pathValue : pathsValue.asArray()) {
            config.includePaths.push_back(pathValue.asString());
        }
    }

    return true;
}

} // namespace proxima

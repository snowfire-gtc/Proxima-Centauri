#include "BuildRulesParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace proxima {

BuildRulesParser::BuildRulesParser() {}

bool BuildRulesParser::parse(const QString& filename) {
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parseFile(filename);

    if (!result.success) {
        lastError = result.error;
        return false;
    }

    // Parse collection
    const auto& collection = result.value;

    // Extract values
    config.optimizationLevel = collection.get("optimization_level").asNumber();
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
            modules.append(moduleValue.asString());
        }
    }

    // Parse defines array
    CollectionParser::Value definesValue = collection.get("defines");
    if (definesValue.isArray()) {
        for (const auto& defineValue : definesValue.asArray()) {
            config.defines.append(defineValue.asString());
        }
    }

    // Parse include_paths array
    CollectionParser::Value pathsValue = collection.get("include_paths");
    if (pathsValue.isArray()) {
        for (const auto& pathValue : pathsValue.asArray()) {
            config.includePaths.append(pathValue.asString());
        }
    }

    return true;
}

} // namespace proxima

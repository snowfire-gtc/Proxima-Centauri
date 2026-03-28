#include "Protocol.h"
#include "utils/CollectionParser.h"
#include "utils/Logger.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cmath>

namespace proxima {

// ============================================================================
// Collection Class Implementation
// ============================================================================

Collection::Collection()
    : m_type(CollectionType::Collection) {
}

Collection::Collection(const Collection& other)
    : m_type(other.m_type)
    , m_stringValue(other.m_stringValue)
    , m_numberValue(other.m_numberValue)
    , m_boolValue(other.m_boolValue)
    , m_arrayValue(other.m_arrayValue)
    , m_objectValue(other.m_objectValue)
    , m_collectionPairs(other.m_collectionPairs) {
}

Collection& Collection::operator=(const Collection& other) {
    if (this != &other) {
        m_type = other.m_type;
        m_stringValue = other.m_stringValue;
        m_numberValue = other.m_numberValue;
        m_boolValue = other.m_boolValue;
        m_arrayValue = other.m_arrayValue;
        m_objectValue = other.m_objectValue;
        m_collectionPairs = other.m_collectionPairs;
    }
    return *this;
}

Collection::~Collection() {
    clear();
}

void Collection::clear() {
    m_stringValue.clear();
    m_numberValue = 0.0;
    m_boolValue = false;
    m_arrayValue.clear();
    m_objectValue.clear();
    m_collectionPairs.clear();
}

Collection Collection::fromString(const std::string& value) {
    Collection c;
    c.m_type = CollectionType::String;
    c.m_stringValue = value;
    return c;
}

Collection Collection::fromNumber(double value) {
    Collection c;
    c.m_type = CollectionType::Number;
    c.m_numberValue = value;
    return c;
}

Collection Collection::fromBoolean(bool value) {
    Collection c;
    c.m_type = CollectionType::Boolean;
    c.m_boolValue = value;
    return c;
}

Collection Collection::fromArray(const CollectionArray& array) {
    Collection c;
    c.m_type = CollectionType::Array;
    c.m_arrayValue = array;
    return c;
}

Collection Collection::fromObject(const CollectionObject& object) {
    Collection c;
    c.m_type = CollectionType::Object;
    c.m_objectValue = object;
    return c;
}

Collection Collection::fromCollection(const CollectionPairs& pairs) {
    Collection c;
    c.m_type = CollectionType::Collection;
    c.m_collectionPairs = pairs;
    return c;
}

Collection Collection::fromCollection(std::initializer_list<std::pair<std::string, Collection>> pairs) {
    Collection c;
    c.m_type = CollectionType::Collection;
    for (const auto& pair : pairs) {
        c.m_collectionPairs.push_back(pair);
    }
    return c;
}

CollectionType Collection::type() const {
    return m_type;
}

std::string Collection::toString() const {
    if (m_type == CollectionType::String) {
        return m_stringValue;
    }
    return std::string();
}

double Collection::toNumber() const {
    if (m_type == CollectionType::Number) {
        return m_numberValue;
    }
    return 0.0;
}

bool Collection::toBoolean() const {
    if (m_type == CollectionType::Boolean) {
        return m_boolValue;
    }
    return false;
}

CollectionArray Collection::toArray() const {
    if (m_type == CollectionType::Array) {
        return m_arrayValue;
    }
    return CollectionArray();
}

CollectionObject Collection::toObject() const {
    if (m_type == CollectionType::Object) {
        return m_objectValue;
    }
    return CollectionObject();
}

CollectionPairs Collection::toCollectionPairs() const {
    if (m_type == CollectionType::Collection) {
        return m_collectionPairs;
    }
    return CollectionPairs();
}

Collection Collection::get(const std::string& key) const {
    if (m_type == CollectionType::Collection) {
        for (const auto& pair : m_collectionPairs) {
            if (pair.first == key) {
                return pair.second;
            }
        }
    } else if (m_type == CollectionType::Object) {
        auto it = m_objectValue.find(key);
        if (it != m_objectValue.end()) {
            return it->second;
        }
    }
    return Collection();
}

Collection Collection::get(int index) const {
    if (m_type == CollectionType::Array && index >= 0 && static_cast<size_t>(index) < m_arrayValue.size()) {
        return m_arrayValue[index];
    }
    return Collection();
}

bool Collection::has(const std::string& key) const {
    if (m_type == CollectionType::Collection) {
        for (const auto& pair : m_collectionPairs) {
            if (pair.first == key) {
                return true;
            }
        }
    } else if (m_type == CollectionType::Object) {
        return m_objectValue.find(key) != m_objectValue.end();
    }
    return false;
}

int Collection::size() const {
    switch (m_type) {
        case CollectionType::Array:
            return static_cast<int>(m_arrayValue.size());
        case CollectionType::Object:
            return static_cast<int>(m_objectValue.size());
        case CollectionType::Collection:
            return static_cast<int>(m_collectionPairs.size());
        default:
            return 0;
    }
}

bool Collection::isEmpty() const {
    return size() == 0;
}

void Collection::set(const std::string& key, const Collection& value) {
    if (m_type == CollectionType::Collection) {
        for (size_t i = 0; i < m_collectionPairs.size(); i++) {
            if (m_collectionPairs[i].first == key) {
                m_collectionPairs[i].second = value;
                return;
            }
        }
        m_collectionPairs.push_back(std::make_pair(key, value));
    }
}

void Collection::append(const Collection& value) {
    if (m_type == CollectionType::Array) {
        m_arrayValue.push_back(value);
    }
}

void Collection::append(const std::string& key, const Collection& value) {
    if (m_type == CollectionType::Collection) {
        m_collectionPairs.push_back(std::make_pair(key, value));
    }
}

std::string Collection::serialize(int indent) const {
    return toCollectionString(indent);
}

Collection Collection::deserialize(const std::string& input) {
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(input);
    
    if (result.success) {
        return collectionValueToCollection(result.value);
    }
    
    LOG_ERROR("Collection deserialize error: " + result.error);
    return Collection();
}

bool Collection::isValid(const std::string& input) {
    return CollectionParser::isValid(input);
}

std::string Collection::toCollectionString(int indent) const {
    std::string result;
    std::string indentStr(indent * 4, ' ');
    std::string nextIndentStr((indent + 1) * 4, ' ');
    
    switch (m_type) {
        case CollectionType::String:
            result = "\"" + escapeString(m_stringValue) + "\"";
            break;
            
        case CollectionType::Number: {
            // Проверка на целое число
            if (m_numberValue == static_cast<int64_t>(m_numberValue)) {
                result = std::to_string(static_cast<int64_t>(m_numberValue));
            } else {
                std::ostringstream oss;
                oss.precision(15);
                oss << m_numberValue;
                result = oss.str();
            }
            break;
        }
            
        case CollectionType::Boolean:
            result = m_boolValue ? "true" : "false";
            break;
            
        case CollectionType::Null:
            result = "null";
            break;
            
        case CollectionType::Array: {
            result += "[";
            for (size_t i = 0; i < m_arrayValue.size(); i++) {
                if (i > 0) {
                    result += ", ";
                }
                result += m_arrayValue[i].toCollectionString(indent + 1);
            }
            result += "]";
            break;
        }
            
        case CollectionType::Object: {
            result += "{\n";
            int count = 0;
            for (const auto& kv : m_objectValue) {
                if (count > 0) {
                    result += ",\n";
                }
                result += nextIndentStr + "\"" + escapeString(kv.first) + "\": " + 
                         kv.second.toCollectionString(indent + 1);
                count++;
            }
            if (count > 0) {
                result += "\n";
            }
            result += indentStr + "}";
            break;
        }
            
        case CollectionType::Collection: {
            result += "[\n";
            int count = 0;
            for (const auto& pair : m_collectionPairs) {
                if (count > 0) {
                    result += ",,\n";
                }
                result += nextIndentStr + "\"" + escapeString(pair.first) + "\", " + 
                         pair.second.toCollectionString(indent + 1);
                count++;
            }
            if (count > 0) {
                result += "\n";
            }
            result += indentStr + "]";
            break;
        }
            
        default:
            result = "null";
            break;
    }
    
    return result;
}

std::string Collection::escapeString(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

Collection Collection::collectionValueToCollection(const CollectionParser::Value& value) {
    Collection c;
    
    switch (value.type) {
        case CollectionParser::ValueType::String:
            c = Collection::fromString(value.stringValue);
            break;
            
        case CollectionParser::ValueType::Number:
            c = Collection::fromNumber(value.numberValue);
            break;
            
        case CollectionParser::ValueType::Boolean:
            c = Collection::fromBoolean(value.boolValue);
            break;
            
        case CollectionParser::ValueType::Null:
            c = Collection();
            break;
            
        case CollectionParser::ValueType::Array: {
            CollectionArray array;
            for (const auto& item : value.arrayValue) {
                array.push_back(collectionValueToCollection(item));
            }
            c = Collection::fromArray(array);
            break;
        }
            
        case CollectionParser::ValueType::Object: {
            CollectionObject object;
            for (const auto& kv : value.objectValue) {
                object[kv.first] = collectionValueToCollection(kv.second);
            }
            c = Collection::fromObject(object);
            break;
        }
            
        case CollectionParser::ValueType::Collection: {
            CollectionPairs pairs;
            for (const auto& kv : value.objectValue) {
                pairs.push_back(std::make_pair(kv.first, collectionValueToCollection(kv.second)));
            }
            c = Collection::fromCollection(pairs);
            break;
        }
            
        default:
            break;
    }
    
    return c;
}

CollectionParser::Value Collection::collectionToCollectionValue(const Collection& collection) {
    CollectionParser::Value value;
    
    switch (collection.m_type) {
        case CollectionType::String:
            value = CollectionParser::Value::fromString(collection.m_stringValue);
            break;
            
        case CollectionType::Number:
            value = CollectionParser::Value::fromNumber(collection.m_numberValue);
            break;
            
        case CollectionType::Boolean:
            value = CollectionParser::Value::fromBoolean(collection.m_boolValue);
            break;
            
        case CollectionType::Null:
            value = CollectionParser::Value();
            break;
            
        case CollectionType::Array: {
            std::vector<CollectionParser::Value> array;
            for (const auto& item : collection.m_arrayValue) {
                array.push_back(collectionToCollectionValue(item));
            }
            value = CollectionParser::Value::fromArray(array);
            break;
        }
            
        case CollectionType::Object: {
            std::map<std::string, CollectionParser::Value> object;
            for (const auto& kv : collection.m_objectValue) {
                object[kv.first] = collectionToCollectionValue(kv.second);
            }
            value = CollectionParser::Value::fromObject(object);
            break;
        }
            
        case CollectionType::Collection: {
            std::map<std::string, CollectionParser::Value> collectionMap;
            for (const auto& pair : collection.m_collectionPairs) {
                collectionMap[pair.first] = collectionToCollectionValue(pair.second);
            }
            value = CollectionParser::Value::fromObject(collectionMap);
            break;
        }
            
        default:
            value = CollectionParser::Value();
            break;
    }
    
    return value;
}

bool Collection::operator==(const Collection& other) const {
    if (m_type != other.m_type) return false;
    
    switch (m_type) {
        case CollectionType::String:
            return m_stringValue == other.m_stringValue;
        case CollectionType::Number:
            return std::abs(m_numberValue - other.m_numberValue) < 1e-10;
        case CollectionType::Boolean:
            return m_boolValue == other.m_boolValue;
        case CollectionType::Null:
            return true;
        case CollectionType::Array:
            return m_arrayValue == other.m_arrayValue;
        case CollectionType::Object:
            return m_objectValue == other.m_objectValue;
        case CollectionType::Collection:
            return m_collectionPairs == other.m_collectionPairs;
        default:
            return false;
    }
}

// ============================================================================
// Message Implementation
// ============================================================================

namespace {

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm_now);
    return std::string(buffer);
}

std::string generateUuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    
    return ss.str();
}

} // anonymous namespace

Message::Message()
    : type(MessageType::ERROR_RESPONSE)
    , requestId("")
    , timestamp(getCurrentTimestamp()) {
}

Message::Message(MessageType t)
    : type(t)
    , requestId(generateUuid())
    , timestamp(getCurrentTimestamp()) {
}

Message::~Message() {
}

std::string Message::serialize() const {
    CollectionParser parser;
    
    // Создаём collection для сообщения
    CollectionPairs pairs;
    
    // Тип сообщения
    pairs.push_back(std::make_pair("type", 
                          Collection::fromNumber(static_cast<double>(type))));
    
    // ID запроса
    pairs.push_back(std::make_pair("requestId", 
                          Collection::fromString(requestId)));
    
    // Временная метка
    pairs.push_back(std::make_pair("timestamp", 
                          Collection::fromString(timestamp)));
    
    // Данные
    pairs.push_back(std::make_pair("data", data));
    
    Collection messageCollection = Collection::fromCollection(pairs);
    return parser.serialize(messageCollection, 4);
}

Message Message::deserialize(const std::string& data) {
    Message msg;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        // Извлекаем тип сообщения
        Collection typeValue = collection.get("type");
        if (typeValue.type() == CollectionType::Number) {
            msg.type = static_cast<MessageType>(static_cast<int>(typeValue.toNumber()));
        }
        
        // Извлекаем requestId
        Collection requestIdValue = collection.get("requestId");
        if (requestIdValue.type() == CollectionType::String) {
            msg.requestId = requestIdValue.toString();
        }
        
        // Извлекаем timestamp
        Collection timestampValue = collection.get("timestamp");
        if (timestampValue.type() == CollectionType::String) {
            msg.timestamp = timestampValue.toString();
        }
        
        // Извлекаем данные
        msg.data = collection.get("data");
    } else {
        LOG_ERROR("Message deserialize error: " + result.error);
        msg.type = MessageType::ERROR_RESPONSE;
        msg.data = Collection::fromString(result.error);
    }
    
    return msg;
}

bool Message::isValid() const {
    return type != MessageType::ERROR_RESPONSE || !data.isEmpty();
}


// ============================================================================
// AnalysisRequest Implementation
// ============================================================================

AnalysisRequest::AnalysisRequest()
    : inferTypes(true)
    , highlightTiming(false)
    , branch("main") {
}

std::string AnalysisRequest::serialize() const {
    CollectionParser parser;
    
    CollectionPairs pairs;
    pairs.push_back(std::make_pair("file", Collection::fromString(file)));
    pairs.push_back(std::make_pair("infer_types", Collection::fromBoolean(inferTypes)));
    pairs.push_back(std::make_pair("highlight_timing", Collection::fromBoolean(highlightTiming)));
    pairs.push_back(std::make_pair("branch", Collection::fromString(branch)));
    
    Collection collection = Collection::fromCollection(pairs);
    return parser.serialize(collection, 4);
}

AnalysisRequest AnalysisRequest::deserialize(const std::string& data) {
    AnalysisRequest request;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        Collection fileValue = collection.get("file");
        if (fileValue.type() == CollectionType::String) {
            request.file = fileValue.toString();
        }
        
        Collection inferTypesValue = collection.get("infer_types");
        if (inferTypesValue.type() == CollectionType::Boolean) {
            request.inferTypes = inferTypesValue.toBoolean();
        }
        
        Collection highlightTimingValue = collection.get("highlight_timing");
        if (highlightTimingValue.type() == CollectionType::Boolean) {
            request.highlightTiming = highlightTimingValue.toBoolean();
        }
        
        Collection branchValue = collection.get("branch");
        if (branchValue.type() == CollectionType::String) {
            request.branch = branchValue.toString();
        }
    }
    
    return request;
}

// ============================================================================
// AnalysisResponse Implementation
// ============================================================================

AnalysisResponse::AnalysisResponse()
    : status("error") {
}

std::string AnalysisResponse::serialize() const {
    CollectionParser parser;
    
    CollectionPairs pairs;
    pairs.push_back(std::make_pair("status", Collection::fromString(status)));
    
    // Сериализация symbols
    CollectionArray symbolsArray;
    for (const SymbolInfo& symbol : symbols) {
        CollectionPairs symbolPairs;
        symbolPairs.push_back(std::make_pair("name", Collection::fromString(symbol.name)));
        symbolPairs.push_back(std::make_pair("type", Collection::fromString(symbol.type)));
        symbolPairs.push_back(std::make_pair("line", Collection::fromNumber(symbol.line)));
        if (!symbol.returnType.empty()) {
            symbolPairs.push_back(std::make_pair("return_type", Collection::fromString(symbol.returnType)));
        }
        symbolsArray.push_back(Collection::fromCollection(symbolPairs));
    }
    pairs.push_back(std::make_pair("symbols", Collection::fromArray(symbolsArray)));
    
    // Сериализация warnings
    CollectionArray warningsArray;
    for (const std::string& warning : warnings) {
        warningsArray.push_back(Collection::fromString(warning));
    }
    pairs.push_back(std::make_pair("warnings", Collection::fromArray(warningsArray)));
    
    // Сериализация timingHints
    Collection timingHintsObj;
    std::map<std::string, Collection> timingMap;
    auto it = timingHints.find("hotspot_lines");
    if (it != timingHints.end()) {
        CollectionArray linesArray;
        for (int line : it->second) {
            linesArray.push_back(Collection::fromNumber(line));
        }
        timingMap["hotspot_lines"] = Collection::fromArray(linesArray);
    }
    pairs.push_back(std::make_pair("timing_hints", Collection::fromObject(timingMap)));
    
    Collection collection = Collection::fromCollection(pairs);
    return parser.serialize(collection, 4);
}

AnalysisResponse AnalysisResponse::deserialize(const std::string& data) {
    AnalysisResponse response;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        Collection statusValue = collection.get("status");
        if (statusValue.type() == CollectionType::String) {
            response.status = statusValue.toString();
        }
        
        // Десериализация symbols
        Collection symbolsValue = collection.get("symbols");
        if (symbolsValue.type() == CollectionType::Array) {
            for (const auto& symbolItem : symbolsValue.toArray()) {
                SymbolInfo symbol;
                symbol.name = symbolItem.get("name").toString();
                symbol.type = symbolItem.get("type").toString();
                symbol.line = static_cast<int>(symbolItem.get("line").toNumber());
                symbol.returnType = symbolItem.get("return_type").toString();
                response.symbols.append(symbol);
            }
        }
        
        // Десериализация warnings
        Collection warningsValue = collection.get("warnings");
        if (warningsValue.type() == CollectionType::Array) {
            for (const auto& warningItem : warningsValue.toArray()) {
                response.warnings.push_back(warningItem.toString());
            }
        }
        
        // Десериализация timingHints
        Collection timingHintsValue = collection.get("timing_hints");
        if (timingHintsValue.type() == CollectionType::Object) {
            Collection hotspotsValue = timingHintsValue.get("hotspot_lines");
            if (hotspotsValue.type() == CollectionType::Array) {
                std::vector<int> hotspots;
                for (const auto& lineItem : hotspotsValue.toArray()) {
                    hotspots.push_back(static_cast<int>(lineItem.toNumber()));
                }
                response.timingHints["hotspot_lines"] = hotspots;
            }
        }
    }
    
    return response;
}

// ============================================================================
// Protocol Implementation
// ============================================================================

Protocol& Protocol::getInstance() {
    static Protocol instance;
    return instance;
}

Protocol::Protocol()
    : connected(false)
    , port(9090) {
}

Protocol::~Protocol() {
    disconnect();
}

void Protocol::sendMessage(const Message& msg) {
    LOG_ERROR("Protocol::sendMessage not implemented - requires network library");
}

Message Protocol::receiveMessage() {
    LOG_ERROR("Protocol::receiveMessage not implemented - requires network library");
    return Message();
}

Message Protocol::createAnalysisRequest(const AnalysisRequest& req) {
    Message msg(MessageType::ANALYZE_FILE);
    msg.data = Collection::fromString(req.file);
    return msg;
}

Message Protocol::createAnalysisResponse(const AnalysisResponse& resp) {
    Message msg(MessageType::ANALYSIS_RESULT);
    msg.data = Collection::fromString(resp.status);
    return msg;
}

Message Protocol::createError(const std::string& error, const std::string& requestId) {
    Message msg(MessageType::ERROR_RESPONSE);
    msg.data = Collection::fromString(error);
    msg.requestId = requestId;
    return msg;
}

void Protocol::setConnection(const std::string& host_, int port_) {
    host = host_;
    port = port_;
}

void Protocol::disconnect() {
    connected = false;
    LOG_INFO("Protocol disconnected");
}

std::string Protocol::readFromSocket() {
    return "";
}

void Protocol::writeToSocket(const std::string& data) {
    (void)data;
}

// ============================================================================
// Utility Functions
// ============================================================================

Collection createMessage(MessageType type, const CollectionPairs& data) {
    Message msg(type);
    msg.data = Collection::fromCollection(data);
    return msg.data;
}

Collection createAnalysisRequest(const std::string& file, bool inferTypes, 
                                 bool highlightTiming, const std::string& branch) {
    CollectionPairs pairs;
    pairs.push_back(std::make_pair("file", Collection::fromString(file)));
    pairs.push_back(std::make_pair("infer_types", Collection::fromBoolean(inferTypes)));
    pairs.push_back(std::make_pair("highlight_timing", Collection::fromBoolean(highlightTiming)));
    pairs.push_back(std::make_pair("branch", Collection::fromString(branch)));
    
    return Collection::fromCollection(pairs);
}

Collection createCompileRequest(const std::string& projectPath, const CompilerConfig& config) {
    CollectionPairs pairs;
    pairs.push_back(std::make_pair("project_path", Collection::fromString(projectPath)));
    pairs.push_back(std::make_pair("mode", Collection::fromNumber(static_cast<double>(config.mode))));
    pairs.push_back(std::make_pair("verbose_level", Collection::fromNumber(config.verboseLevel)));
    pairs.push_back(std::make_pair("max_memory", Collection::fromNumber(static_cast<double>(config.maxMemory))));
    pairs.push_back(std::make_pair("enable_cuda", Collection::fromBoolean(config.enableCUDA)));
    pairs.push_back(std::make_pair("enable_avx2", Collection::fromBoolean(config.enableAVX2))));
    pairs.push_back(std::make_pair("enable_sse4", Collection::fromBoolean(config.enableSSE4)));
    pairs.push_back(std::make_pair("debug_symbols", Collection::fromBoolean(config.debugSymbols)));
    pairs.push_back(std::make_pair("optimization_level", Collection::fromNumber(config.optimizationLevel)));
    
    if (!config.outputPath.empty()) {
        pairs.push_back(std::make_pair("output_path", Collection::fromString(config.outputPath)));
    }
    
    return Collection::fromCollection(pairs);
}

Collection createDebugRequest(const std::string& action, const std::string& file, int line) {
    CollectionPairs pairs;
    pairs.push_back(std::make_pair("action", Collection::fromString(action)));
    
    if (!file.empty()) {
        pairs.push_back(std::make_pair("file", Collection::fromString(file)));
        pairs.push_back(std::make_pair("line", Collection::fromNumber(line)));
    }
    
    return Collection::fromCollection(pairs);
}

} // namespace proxima
        pairs.append(qMakePair("file", Collection::fromString(file)));
    }
    
    if (line > 0) {
        pairs.append(qMakePair("line", Collection::fromNumber(line)));
    }
    
    return Collection::fromCollection(pairs);
}

CompilerConfig configFromCollection(const Collection& collection) {
    CompilerConfig config;
    
    Collection modeValue = collection.get("mode");
    if (modeValue.type() == CollectionType::Number) {
        config.mode = static_cast<RunMode>(static_cast<int>(modeValue.toNumber()));
    }
    
    Collection verboseValue = collection.get("verbose_level");
    if (verboseValue.type() == CollectionType::Number) {
        config.verboseLevel = static_cast<int>(verboseValue.toNumber());
    }
    
    Collection memoryValue = collection.get("max_memory");
    if (memoryValue.type() == CollectionType::Number) {
        config.maxMemory = static_cast<size_t>(memoryValue.toNumber());
    }
    
    Collection cudaValue = collection.get("enable_cuda");
    if (cudaValue.type() == CollectionType::Boolean) {
        config.enableCUDA = cudaValue.toBoolean();
    }
    
    Collection avxValue = collection.get("enable_avx2");
    if (avxValue.type() == CollectionType::Boolean) {
        config.enableAVX2 = avxValue.toBoolean();
    }
    
    Collection sseValue = collection.get("enable_sse4");
    if (sseValue.type() == CollectionType::Boolean) {
        config.enableSSE4 = sseValue.toBoolean();
    }
    
    Collection debugValue = collection.get("debug_symbols");
    if (debugValue.type() == CollectionType::Boolean) {
        config.debugSymbols = debugValue.toBoolean();
    }
    
    Collection optValue = collection.get("optimization_level");
    if (optValue.type() == CollectionType::Number) {
        config.optimizationLevel = static_cast<int>(optValue.toNumber());
    }
    
    Collection outputValue = collection.get("output_path");
    if (outputValue.type() == CollectionType::String) {
        config.outputPath = outputValue.toString();
    }
    
    return config;
}

} // namespace proxima
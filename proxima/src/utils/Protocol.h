#ifndef PROXIMA_PROTOCOL_H
#define PROXIMA_PROTOCOL_H

#include "../stdlib/Collection.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace proxima {

// Forward declaration
class CollectionValue;

using CollectionArray = std::vector<Collection>;
using CollectionObject = std::map<std::string, Collection>;
using CollectionPairs = std::vector<std::pair<std::string, Collection>>;

enum class CollectionType {
    Null,
    String,
    Number,
    Boolean,
    Array,
    Object,
    Collection
};

class Collection {
public:
    Collection();
    Collection(const Collection& other);
    Collection& operator=(const Collection& other);
    ~Collection();
    
    static Collection fromString(const std::string& value);
    static Collection fromNumber(double value);
    static Collection fromBoolean(bool value);
    static Collection fromArray(const CollectionArray& array);
    static Collection fromObject(const CollectionObject& object);
    static Collection fromCollection(const CollectionPairs& pairs);
    static Collection fromCollection(std::initializer_list<std::pair<std::string, Collection>> pairs);
    
    CollectionType type() const;
    std::string toString() const;
    double toNumber() const;
    bool toBoolean() const;
    CollectionArray toArray() const;
    CollectionObject toObject() const;
    CollectionPairs toCollectionPairs() const;
    
    Collection get(const std::string& key) const;
    Collection get(int index) const;
    bool has(const std::string& key) const;
    int size() const;
    bool isEmpty() const;
    
    void set(const std::string& key, const Collection& value);
    void append(const Collection& value);
    void append(const std::string& key, const Collection& value);
    
    std::string serialize(int indent = 0) const;
    static Collection deserialize(const std::string& input);
    static bool isValid(const std::string& input);
    
    bool operator==(const Collection& other) const;
    
private:
    void clear();
    std::string toCollectionString(int indent) const;
    std::string escapeString(const std::string& str) const;
    
    CollectionType m_type;
    std::string m_stringValue;
    double m_numberValue;
    bool m_boolValue;
    CollectionArray m_arrayValue;
    CollectionObject m_objectValue;
    CollectionPairs m_collectionPairs;
};

enum class MessageType {
    // IDE -> Compiler
    ANALYZE_FILE,
    COMPILE_MODULE,
    RUN_PROGRAM,
    DEBUG_START,
    DEBUG_STEP,
    DEBUG_CONTINUE,
    DEBUG_STOP,
    GET_VARIABLES,
    GET_CALLSTACK,
    FORMAT_CODE,
    GENERATE_DOC,
    
    // Compiler -> IDE
    ANALYSIS_RESULT,
    COMPILE_RESULT,
    RUN_RESULT,
    DEBUG_EVENT,
    VARIABLES_DATA,
    CALLSTACK_DATA,
    FORMATTED_CODE,
    DOCUMENTATION,
    
    // Error
    ERROR_RESPONSE
};

struct Message {
    MessageType type;
    Collection data;
    std::string requestId;
    std::string timestamp;
    
    Message() : type(MessageType::ERROR_RESPONSE) {}
    Message(MessageType t) : type(t) {}
    
    std::string serialize() const;
    static Message deserialize(const std::string& data);
    bool isValid() const;
};

struct AnalysisRequest {
    std::string file;
    bool inferTypes;
    bool highlightTiming;
    std::string branch;
    
    AnalysisRequest();
    std::string serialize() const;
    static AnalysisRequest deserialize(const std::string& data);
};

struct SymbolInfo {
    std::string name;
    std::string type;
    int line;
    std::string returnType;
};

struct AnalysisResponse {
    std::string status;
    std::vector<SymbolInfo> symbols;
    std::vector<std::string> warnings;
    std::map<std::string, std::vector<int>> timingHints;
    
    AnalysisResponse();
    std::string serialize() const;
    static AnalysisResponse deserialize(const std::string& data);
};

class Protocol {
public:
    static Protocol& getInstance();
    
    void sendMessage(const Message& msg);
    Message receiveMessage();
    
    Message createAnalysisRequest(const AnalysisRequest& req);
    Message createAnalysisResponse(const AnalysisResponse& resp);
    Message createError(const std::string& error, const std::string& requestId);
    
    void setConnection(const std::string& host, int port);
    void disconnect();
    bool isConnected() const { return connected; }
    
private:
    Protocol();
    ~Protocol();
    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;
    
    std::string host;
    int port;
    bool connected;
    
    std::string readFromSocket();
    void writeToSocket(const std::string& data);
};

// Utility functions
Collection createMessage(MessageType type, const CollectionPairs& data);
Collection createAnalysisRequest(const std::string& file, bool inferTypes, 
                                 bool highlightTiming, const std::string& branch);
Collection createCompileRequest(const std::string& projectPath, const struct CompilerConfig& config);
Collection createDebugRequest(const std::string& action, const std::string& file, int line);

} // namespace proxima

#endif // PROXIMA_PROTOCOL_H
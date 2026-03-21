#ifndef PROXIMA_PROTOCOL_H
#define PROXIMA_PROTOCOL_H

#include "../stdlib/Collection.h"
#include <string>
#include <vector>
#include <memory>

namespace proxima {

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
    stdlib::Collection data;
    std::string requestId;
    std::string timestamp;
    
    Message() : type(MessageType::ERROR_RESPONSE) {}
    Message(MessageType t) : type(t) {}
    
    std::string serialize() const;
    static Message deserialize(const std::string& data);
};

struct AnalysisRequest {
    std::string file;
    bool inferTypes;
    bool highlightTiming;
    std::string branch;
    
    std::string serialize() const;
    static AnalysisRequest deserialize(const std::string& data);
};

struct AnalysisResponse {
    std::string status;
    std::vector<std::map<std::string, std::string>> symbols;
    std::vector<std::string> warnings;
    std::map<std::string, std::vector<int>> timingHints;
    
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
    std::string buffer;
    
    std::string readFromSocket();
    void writeToSocket(const std::string& data);
};

} // namespace proxima

#endif // PROXIMA_PROTOCOL_H
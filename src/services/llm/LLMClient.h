#ifndef PROXIMA_LLM_CLIENT_H
#define PROXIMA_LLM_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace proxima {

struct CodeSuggestion {
    int blockId;
    int startLine;
    int endLine;
    std::string originalCode;
    std::string suggestedCode;
    std::string explanation;
    double confidence;
};

struct LLMRequest {
    std::string type;  // "code_modification", "explanation", "completion"
    std::string file;
    int startLine;
    int endLine;
    std::string prompt;
    std::string codeContext;
    std::map<std::string, std::string> options;
};

struct LLMResponse {
    bool success;
    std::string errorMessage;
    std::vector<CodeSuggestion> suggestions;
    std::string explanation;
    double processingTime;
};

class LLMClient {
public:
    LLMClient();
    ~LLMClient();
    
    void setServerURL(const std::string& url);
    void setModel(const std::string& model);
    void setAPIKey(const std::string& key);
    void setMaxTokens(int tokens);
    
    LLMResponse sendRequest(const LLMRequest& request);
    
    // Convenience methods
    std::vector<CodeSuggestion> suggestModifications(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& prompt
    );
    
    std::string explainCode(
        const std::string& file,
        int startLine,
        int endLine
    );
    
    std::string completeCode(
        const std::string& file,
        int line,
        int column,
        const std::string& prefix
    );
    
    bool isConnected() const { return connected; }
    std::string getLastError() const { return lastError; }
    
    // Callbacks
    using ResponseCallback = std::function<void(const LLMResponse&)>;
    void onResponse(ResponseCallback callback);
    
private:
    std::string serverURL;
    std::string model;
    std::string apiKey;
    int maxTokens;
    bool connected;
    std::string lastError;
    ResponseCallback responseCallback;
    
    std::string sendHTTPRequest(const std::string& endpoint, const std::string& data);
    LLMResponse parseResponse(const std::string& responseData);
    std::string buildRequestJSON(const LLMRequest& request);
};

} // namespace proxima

#endif // PROXIMA_LLM_CLIENT_H
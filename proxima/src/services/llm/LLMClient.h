#ifndef PROXIMA_LLM_CLIENT_H
#define PROXIMA_LLM_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <mutex>
#include <queue>

namespace proxima {

struct CodeSuggestion {
    int id;
    int blockId;
    int startLine;
    int endLine;
    std::string originalCode;
    std::string suggestedCode;
    std::string explanation;
    double confidence;
    bool accepted;
    
    CodeSuggestion() : id(0), blockId(0), startLine(0), endLine(0),
                       confidence(0.0), accepted(false) {}
};

struct LLMRequest {
    std::string type;  // "code_modification", "explanation", "completion", etc.
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
    std::string completion;
    double processingTime;
    
    LLMResponse() : success(false), processingTime(0.0) {}
};

struct ConversationMessage {
    std::string role;  // "user", "assistant", "system"
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

struct CacheEntry {
    LLMResponse response;
    std::chrono::system_clock::time_point timestamp;
};

class LLMClient {
public:
    explicit LLMClient();
    ~LLMClient();
    
    // Configuration
    void setServerURL(const std::string& url);
    void setModel(const std::string& model);
    void setAPIKey(const std::string& key);
    void setMaxTokens(int tokens);
    void setTemperature(double temp);
    void setTimeout(int ms);
    
    // Connection
    bool checkConnection();
    bool isConnected() const;
    
    // Core request
    LLMResponse sendRequest(const LLMRequest& request);
    
    // Code assistance methods
    std::vector<CodeSuggestion> suggestModifications(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code,
        const std::string& prompt = "");
    
    std::string explainCode(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code);
    
    std::string completeCode(
        const std::string& file,
        int line,
        int column,
        const std::string& prefix,
        const std::string& suffix = "");
    
    LLMResponse refactorCode(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code,
        const std::string& goal);
    
    LLMResponse generateDocumentation(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code);
    
    LLMResponse fixBugs(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code,
        const std::string& errorMessage);
    
    LLMResponse generateTests(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code);
    
    LLMResponse optimizeCode(
        const std::string& file,
        int startLine,
        int endLine,
        const std::string& code,
        const std::string& optimizationGoal);
    
    // Apply suggestions
    void applySuggestion(int suggestionId);
    void applyAllSuggestions();
    void rejectSuggestion(int suggestionId);
    void rejectAllSuggestions();
    std::vector<CodeSuggestion> getAcceptedSuggestions() const;
    
    // Conversation history
    void addToConversationHistory(const std::string& role, const std::string& content);
    void clearConversationHistory();
    std::vector<ConversationMessage> getConversationHistory() const;
    
    // Settings persistence
    void saveSettings();
    void loadSettings();
    
    // Callbacks
    using ResponseCallback = std::function<void(const LLMResponse&)>;
    void onResponse(ResponseCallback callback);
    
    // Batch operations
    LLMResponse requestMultipleSuggestions(
        const std::vector<std::pair<int, int>>& ranges,
        const std::string& file,
        const std::string& code,
        const std::string& prompt);
    
    // Cache
    void clearCache();
    
    // Status
    int getPendingRequests() const;
    std::string getLastError() const;
    void clearError();
    std::vector<CodeSuggestion> getLastSuggestions() const;
    std::string getLastExplanation() const;
    std::string getLastCompletion() const;
    
private:
    std::string buildRequestBody(const LLMRequest& request);
    LLMResponse parseResponse(const std::string& responseData);
    std::string getCacheKey(const LLMRequest& request) const;
    bool getCachedResponse(const std::string& cacheKey, LLMResponse& response);
    void cacheResponse(const std::string& cacheKey, const LLMResponse& response);
    std::string escapeString(const std::string& str) const;
    std::string unescapeString(const std::string& str) const;
    void log(int level, const std::string& message);
    
    std::string serverURL;
    std::string model;
    std::string apiKey;
    int maxTokens;
    double temperature;
    int timeout;
    bool connected;
    bool processing;
    int pendingRequests;
    std::string lastError;
    
    std::vector<CodeSuggestion> lastSuggestions;
    std::string lastExplanation;
    std::string lastCompletion;
    
    std::vector<ConversationMessage> conversationHistory;
    std::map<std::string, CacheEntry> responseCache;
    
    ResponseCallback responseCallback;
    mutable std::mutex mutex_;
};

} // namespace proxima

#endif // PROXIMA_LLM_CLIENT_H
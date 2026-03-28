#include "LLMClient.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include "utils/Logger.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

LLMClient::LLMClient()
    : serverURL("http://localhost:1234")
    , model("local-model")
    , maxTokens(2048)
    , temperature(0.7)
    , timeout(30000)
    , connected(false)
    , processing(false)
    , pendingRequests(0) {

    // Проверка соединения при создании
    checkConnection();

    LOG_INFO("LLMClient initialized");
}

LLMClient::~LLMClient() {
    LOG_INFO("LLMClient destroyed");
}

// ============================================================================
// Configuration
// ============================================================================

void LLMClient::setServerURL(const std::string& url) {
    std::lock_guard<std::mutex> lock(mutex_);
    serverURL = url;
    LOG_INFO("Server URL set to: " + url);
}

void LLMClient::setModel(const std::string& mdl) {
    std::lock_guard<std::mutex> lock(mutex_);
    model = mdl;
    LOG_INFO("Model set to: " + mdl);
}

void LLMClient::setAPIKey(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    apiKey = key;
}

void LLMClient::setMaxTokens(int tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxTokens = tokens;
}

void LLMClient::setTemperature(double temp) {
    std::lock_guard<std::mutex> lock(mutex_);
    temperature = temp;
}

void LLMClient::setTimeout(int ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    timeout = ms;
}

// ============================================================================
// Connection
// ============================================================================

bool LLMClient::checkConnection() {
    // Заглушка - в реальной реализации здесь будет HTTP запрос
    connected = true;
    return connected;
}

bool LLMClient::isConnected() const {
    return connected;
}

// ============================================================================
// Core request
// ============================================================================

LLMResponse LLMClient::sendRequest(const LLMRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    LLMResponse response;
    response.success = false;
    response.errorMessage = "Not implemented - requires HTTP client";
    
    return response;
}

// ============================================================================
// Code assistance methods
// ============================================================================

std::vector<CodeSuggestion> LLMClient::suggestModifications(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code,
    const std::string& prompt) {
    
    LLMRequest request;
    request.type = "code_modification";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = prompt;
    
    LLMResponse response = sendRequest(request);
    lastSuggestions = response.suggestions;
    
    return response.suggestions;
}

std::string LLMClient::explainCode(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code) {
    
    LLMRequest request;
    request.type = "explanation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Explain this code";
    
    LLMResponse response = sendRequest(request);
    lastExplanation = response.explanation;
    
    return response.explanation;
}

std::string LLMClient::completeCode(
    const std::string& file,
    int line,
    int column,
    const std::string& prefix,
    const std::string& suffix) {
    
    LLMRequest request;
    request.type = "completion";
    request.file = file;
    request.codeContext = prefix + suffix;
    request.prompt = "Complete the code";
    
    LLMResponse response = sendRequest(request);
    lastCompletion = response.completion;
    
    return response.completion;
}

LLMResponse LLMClient::refactorCode(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code,
    const std::string& goal) {
    
    LLMRequest request;
    request.type = "refactoring";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Refactor: " + goal;
    
    return sendRequest(request);
}

LLMResponse LLMClient::generateDocumentation(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code) {
    
    LLMRequest request;
    request.type = "documentation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Generate documentation";
    
    return sendRequest(request);
}

LLMResponse LLMClient::fixBugs(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code,
    const std::string& errorMessage) {
    
    LLMRequest request;
    request.type = "bug_fix";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Fix bug: " + errorMessage;
    
    return sendRequest(request);
}

LLMResponse LLMClient::generateTests(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code) {
    
    LLMRequest request;
    request.type = "test_generation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Generate tests";
    
    return sendRequest(request);
}

LLMResponse LLMClient::optimizeCode(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& code,
    const std::string& optimizationGoal) {
    
    LLMRequest request;
    request.type = "optimization";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.codeContext = code;
    request.prompt = "Optimize: " + optimizationGoal;
    
    return sendRequest(request);
}

// ============================================================================
// Apply suggestions
// ============================================================================

void LLMClient::applySuggestion(int suggestionId) {
    for (auto& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = true;
            LOG_INFO("Applied suggestion " + std::to_string(suggestionId));
            break;
        }
    }
}

void LLMClient::applyAllSuggestions() {
    for (auto& suggestion : lastSuggestions) {
        suggestion.accepted = true;
    }
    LOG_INFO("Applied all suggestions");
}

void LLMClient::rejectSuggestion(int suggestionId) {
    for (auto& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = false;
            LOG_INFO("Rejected suggestion " + std::to_string(suggestionId));
            break;
        }
    }
}

void LLMClient::rejectAllSuggestions() {
    for (auto& suggestion : lastSuggestions) {
        suggestion.accepted = false;
    }
    LOG_INFO("Rejected all suggestions");
}

std::vector<CodeSuggestion> LLMClient::getAcceptedSuggestions() const {
    std::vector<CodeSuggestion> accepted;
    for (const auto& suggestion : lastSuggestions) {
        if (suggestion.accepted) {
            accepted.push_back(suggestion);
        }
    }
    return accepted;
}

// ============================================================================
// Conversation history
// ============================================================================

void LLMClient::addToConversationHistory(const std::string& role, const std::string& content) {
    std::lock_guard<std::mutex> lock(mutex_);
    ConversationMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = std::chrono::system_clock::now();
    conversationHistory.push_back(msg);
}

void LLMClient::clearConversationHistory() {
    std::lock_guard<std::mutex> lock(mutex_);
    conversationHistory.clear();
}

std::vector<ConversationMessage> LLMClient::getConversationHistory() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return conversationHistory;
}

// ============================================================================
// Settings persistence
// ============================================================================

void LLMClient::saveSettings() {
    // Заглушка - сохранить настройки в файл
    LOG_INFO("Settings saved");
}

void LLMClient::loadSettings() {
    // Заглушка - загрузить настройки из файла
    LOG_INFO("Settings loaded");
}

// ============================================================================
// Callbacks
// ============================================================================

void LLMClient::onResponse(ResponseCallback callback) {
    responseCallback = callback;
}

// ============================================================================
// Batch operations
// ============================================================================

LLMResponse LLMClient::requestMultipleSuggestions(
    const std::vector<std::pair<int, int>>& ranges,
    const std::string& file,
    const std::string& code,
    const std::string& prompt) {
    
    LLMResponse combinedResponse;
    combinedResponse.success = true;
    
    for (const auto& range : ranges) {
        LLMRequest request;
        request.type = "code_modification";
        request.file = file;
        request.startLine = range.first;
        request.endLine = range.second;
        request.codeContext = code;
        request.prompt = prompt;
        
        LLMResponse response = sendRequest(request);
        if (response.success) {
            combinedResponse.suggestions.insert(
                combinedResponse.suggestions.end(),
                response.suggestions.begin(),
                response.suggestions.end());
        } else {
            combinedResponse.success = false;
            combinedResponse.errorMessage = response.errorMessage;
        }
    }
    
    return combinedResponse;
}

// ============================================================================
// Cache
// ============================================================================

void LLMClient::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    responseCache.clear();
    LOG_INFO("Cache cleared");
}

// ============================================================================
// Status
// ============================================================================

int LLMClient::getPendingRequests() const {
    return pendingRequests;
}

std::string LLMClient::getLastError() const {
    return lastError;
}

void LLMClient::clearError() {
    lastError.clear();
}

std::vector<CodeSuggestion> LLMClient::getLastSuggestions() const {
    return lastSuggestions;
}

std::string LLMClient::getLastExplanation() const {
    return lastExplanation;
}

std::string LLMClient::getLastCompletion() const {
    return lastCompletion;
}

// ============================================================================
// Private methods
// ============================================================================

std::string LLMClient::buildRequestBody(const LLMRequest& request) {
    std::ostringstream oss;
    oss << "{\"model\": \"" << model << "\", ";
    oss << "\"prompt\": \"" << escapeString(request.prompt) << "\", ";
    oss << "\"max_tokens\": " << maxTokens << ", ";
    oss << "\"temperature\": " << temperature << "}";
    return oss.str();
}

LLMResponse LLMClient::parseResponse(const std::string& responseData) {
    LLMResponse response;
    // Заглушка - нужен JSON парсер
    response.success = false;
    response.errorMessage = "JSON parsing not implemented";
    return response;
}

std::string LLMClient::getCacheKey(const LLMRequest& request) const {
    return request.type + ":" + request.file + ":" + 
           std::to_string(request.startLine) + ":" + 
           std::to_string(request.endLine);
}

bool LLMClient::getCachedResponse(const std::string& cacheKey, LLMResponse& response) {
    auto it = responseCache.find(cacheKey);
    if (it != responseCache.end()) {
        response = it->second.response;
        return true;
    }
    return false;
}

void LLMClient::cacheResponse(const std::string& cacheKey, const LLMResponse& response) {
    CacheEntry entry;
    entry.response = response;
    entry.timestamp = std::chrono::system_clock::now();
    responseCache[cacheKey] = entry;
}

std::string LLMClient::escapeString(const std::string& str) const {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string LLMClient::unescapeString(const std::string& str) const {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case 'n': result += '\n'; ++i; break;
                case 'r': result += '\r'; ++i; break;
                case 't': result += '\t'; ++i; break;
                default: result += str[i];
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

void LLMClient::log(int level, const std::string& message) {
    if (level <= 2) {
        LOG_INFO("[LLM] " + message);
    }
}

} // namespace proxima

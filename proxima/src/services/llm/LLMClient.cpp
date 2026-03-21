#include "LLMClient.h"
#include <iostream>
#include <sstream>
#include <chrono>

#ifdef USE_CURL
#include <curl/curl.h>
#endif

namespace proxima {

LLMClient::LLMClient() 
    : serverURL("http://localhost:1234"), model("local-model"),
      maxTokens(2048), connected(false) {}

LLMClient::~LLMClient() {}

void LLMClient::setServerURL(const std::string& url) {
    serverURL = url;
}

void LLMClient::setModel(const std::string& model) {
    this->model = model;
}

void LLMClient::setAPIKey(const std::string& key) {
    apiKey = key;
}

void LLMClient::setMaxTokens(int tokens) {
    maxTokens = tokens;
}

LLMResponse LLMClient::sendRequest(const LLMRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    LLMResponse response;
    response.success = false;
    response.processingTime = 0;
    
#ifdef USE_CURL
    std::string endpoint = "/v1/chat/completions";
    std::string jsonData = buildRequestJSON(request);
    
    std::string responseData = sendHTTPRequest(endpoint, jsonData);
    
    if (!responseData.empty()) {
        response = parseResponse(responseData);
        response.success = true;
    } else {
        response.errorMessage = lastError;
    }
#else
    // Mock response for development
    response.success = true;
    CodeSuggestion suggestion;
    suggestion.blockId = 1;
    suggestion.startLine = request.startLine;
    suggestion.endLine = request.endLine;
    suggestion.originalCode = "// Original code";
    suggestion.suggestedCode = "// Suggested code";
    suggestion.explanation = "This is a mock suggestion";
    suggestion.confidence = 0.9;
    response.suggestions.push_back(suggestion);
#endif
    
    auto endTime = std::chrono::high_resolution_clock::now();
    response.processingTime = std::chrono::duration<double>(endTime - startTime).count();
    
    if (responseCallback) {
        responseCallback(response);
    }
    
    return response;
}

std::vector<CodeSuggestion> LLMClient::suggestModifications(
    const std::string& file,
    int startLine,
    int endLine,
    const std::string& prompt
) {
    LLMRequest request;
    request.type = "code_modification";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = prompt;
    
    LLMResponse response = sendRequest(request);
    
    if (response.success) {
        return response.suggestions;
    }
    
    return std::vector<CodeSuggestion>();
}

std::string LLMClient::explainCode(
    const std::string& file,
    int startLine,
    int endLine
) {
    LLMRequest request;
    request.type = "explanation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Explain this code in detail";
    
    LLMResponse response = sendRequest(request);
    
    if (response.success) {
        return response.explanation;
    }
    
    return "Could not get explanation: " + response.errorMessage;
}

std::string LLMClient::completeCode(
    const std::string& file,
    int line,
    int column,
    const std::string& prefix
) {
    LLMRequest request;
    request.type = "completion";
    request.file = file;
    request.startLine = line;
    request.endLine = line;
    request.prompt = "Complete this code";
    request.codeContext = prefix;
    
    LLMResponse response = sendRequest(request);
    
    if (response.success && !response.suggestions.empty()) {
        return response.suggestions[0].suggestedCode;
    }
    
    return "";
}

void LLMClient::onResponse(ResponseCallback callback) {
    responseCallback = callback;
}

std::string LLMClient::sendHTTPRequest(const std::string& endpoint, const std::string& data) {
#ifdef USE_CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return "";
    }
    
    std::string url = serverURL + endpoint;
    std::string responseData;
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Set callback to collect response
    // Implementation would use WRITEFUNCTION callback
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        lastError = curl_easy_strerror(res);
        connected = false;
    } else {
        connected = true;
    }
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return responseData;
#else
    lastError = "CURL support not compiled in";
    return "";
#endif
}

LLMResponse LLMClient::parseResponse(const std::string& responseData) {
    LLMResponse response;
    
    // Parse JSON response
    // This is a simplified parser - would use proper JSON library in production
    
    response.success = responseData.find("\"error\"") == std::string::npos;
    
    if (!response.success) {
        response.errorMessage = "Error in response";
    }
    
    // Extract suggestions
    // Implementation would parse JSON structure
    
    return response;
}

std::string LLMClient::buildRequestJSON(const LLMRequest& request) {
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"model\": \"" << model << "\",\n";
    oss << "  \"messages\": [\n";
    oss << "    {\n";
    oss << "      \"role\": \"user\",\n";
    oss << "      \"content\": \"" << request.prompt << "\\n\\nCode:\\n" 
        << request.codeContext << "\"\n";
    oss << "    }\n";
    oss << "  ],\n";
    oss << "  \"max_tokens\": " << maxTokens << ",\n";
    oss << "  \"temperature\": 0.7\n";
    oss << "}\n";
    
    return oss.str();
}

} // namespace proxima
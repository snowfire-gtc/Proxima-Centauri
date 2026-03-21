#ifndef PROXIMA_LLM_CLIENT_H
#define PROXIMA_LLM_CLIENT_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QNetworkReply>
#include <functional>
#include <chrono>

namespace proxima {

struct CodeSuggestion {
    int id;
    int blockId;
    int startLine;
    int endLine;
    QString originalCode;
    QString suggestedCode;
    QString explanation;
    double confidence;
    bool accepted;
    
    CodeSuggestion() : id(0), blockId(0), startLine(0), endLine(0),
                       confidence(0.0), accepted(false) {}
};

struct LLMRequest {
    QString type;  // "code_modification", "explanation", "completion", etc.
    QString file;
    int startLine;
    int endLine;
    QString prompt;
    QString codeContext;
    QMap<QString, QString> options;
};

struct LLMResponse {
    bool success;
    QString errorMessage;
    QVector<CodeSuggestion> suggestions;
    QString explanation;
    QString completion;
    double processingTime;
    
    LLMResponse() : success(false), processingTime(0.0) {}
};

struct ConversationMessage {
    QString role;  // "user", "assistant", "system"
    QString content;
    QDateTime timestamp;
};

struct CacheEntry {
    LLMResponse response;
    QDateTime timestamp;
};

class LLMClient : public QObject {
    Q_OBJECT
    
public:
    explicit LLMClient(QObject *parent = nullptr);
    ~LLMClient();
    
    // Configuration
    void setServerURL(const QString& url);
    void setModel(const QString& model);
    void setAPIKey(const QString& key);
    void setMaxTokens(int tokens);
    void setTemperature(double temp);
    void setTimeout(int ms);
    
    // Connection
    bool checkConnection();
    bool isConnected() const;
    
    // Core request
    LLMResponse sendRequest(const LLMRequest& request);
    
    // Code assistance methods
    QVector<CodeSuggestion> suggestModifications(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code,
        const QString& prompt = "");
    
    QString explainCode(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code);
    
    QString completeCode(
        const QString& file,
        int line,
        int column,
        const QString& prefix,
        const QString& suffix = "");
    
    LLMResponse refactorCode(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code,
        const QString& goal);
    
    LLMResponse generateDocumentation(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code);
    
    LLMResponse fixBugs(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code,
        const QString& errorMessage);
    
    LLMResponse generateTests(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code);
    
    LLMResponse optimizeCode(
        const QString& file,
        int startLine,
        int endLine,
        const QString& code,
        const QString& optimizationGoal);
    
    // Apply suggestions
    void applySuggestion(int suggestionId);
    void applyAllSuggestions();
    void rejectSuggestion(int suggestionId);
    void rejectAllSuggestions();
    QVector<CodeSuggestion> getAcceptedSuggestions() const;
    
    // Conversation history
    void addToConversationHistory(const QString& role, const QString& content);
    void clearConversationHistory();
    QVector<ConversationMessage> getConversationHistory() const;
    
    // Settings persistence
    void saveSettings();
    void loadSettings();
    
    // Callbacks
    using ResponseCallback = std::function<void(const LLMResponse&)>;
    void onResponse(ResponseCallback callback);
    
    // Batch operations
    LLMResponse requestMultipleSuggestions(
        const QVector<QPair<int, int>>& ranges,
        const QString& file,
        const QString& code,
        const QString& prompt);
    
    // Cache
    void clearCache();
    
    // Status
    int getPendingRequests() const;
    QString getLastError() const;
    void clearError();
    QVector<CodeSuggestion> getLastSuggestions() const;
    QString getLastExplanation() const;
    QString getLastCompletion() const;
    
signals:
    void suggestionsReady(const QVector<CodeSuggestion>& suggestions);
    void explanationReady(const QString& explanation);
    void completionReady(const QString& completion);
    void errorOccurred(const QString& error);
    void processingStarted();
    void processingFinished();
    void connectionStatusChanged(bool connected);
    
private slots:
    void onNetworkReplyFinished(QNetworkReply* reply);
    void onNetworkError(QNetworkReply::NetworkError error);
    void onTimeout();
    
private:
    QString buildRequestBody(const LLMRequest& request);
    LLMResponse parseResponse(const QString& responseData);
    QString getCacheKey(const LLMRequest& request) const;
    bool getCachedResponse(const QString& cacheKey, LLMResponse& response);
    void cacheResponse(const QString& cacheKey, const LLMResponse& response);
    QString escapeString(const QString& str) const;
    QString unescapeString(const QString& str) const;
    void log(int level, const std::string& message);
    
    QNetworkAccessManager* networkManager;
    QString serverURL;
    QString model;
    QString apiKey;
    int maxTokens;
    double temperature;
    int timeout;
    bool connected;
    bool processing;
    int pendingRequests;
    QString lastError;
    
    QVector<CodeSuggestion> lastSuggestions;
    QString lastExplanation;
    QString lastCompletion;
    
    QVector<ConversationMessage> conversationHistory;
    QMap<QString, CacheEntry> responseCache;
    
    QList<QNetworkReply*> pendingReplies;
    ResponseCallback responseCallback;
};

} // namespace proxima

#endif // PROXIMA_LLM_CLIENT_H
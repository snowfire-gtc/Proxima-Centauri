#ifndef CENTAURI_LLMSERVICE_H
#define CENTAURI_LLMSERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QMap>

namespace proxima {

struct CodeSuggestion {
    int id;
    int startLine;
    int endLine;
    QString originalCode;
    QString suggestedCode;
    QString explanation;
    double confidence;
    bool accepted;
};

struct LLMConfig {
    QString serverURL;
    QString model;
    int maxTokens;
    double temperature;
    int timeout;
    QString apiKey;
};

class LLMService : public QObject {
    Q_OBJECT
    
public:
    explicit LLMService(QObject *parent = nullptr);
    ~LLMService();
    
    // Configuration
    void setConfig(const LLMConfig& config);
    LLMConfig getConfig() const { return config; }
    void setServerURL(const QString& url);
    void setModel(const QString& model);
    void setAPIKey(const QString& key);
    
    // Code assistance
    void requestSuggestions(const QString& file, int startLine, int endLine, 
                           const QString& code, const QString& prompt = "");
    void requestExplanation(const QString& file, int startLine, int endLine,
                           const QString& code);
    void requestCompletion(const QString& file, int line, int column,
                          const QString& prefix, const QString& suffix = "");
    void requestRefactoring(const QString& file, int startLine, int endLine,
                           const QString& code, const QString& goal);
    void requestDocumentation(const QString& file, int startLine, int endLine,
                             const QString& code);
    void requestBugFix(const QString& file, int startLine, int endLine,
                      const QString& code, const QString& errorMessage);
    
    // Batch operations
    void requestMultipleSuggestions(const QVector<QPair<int, int>>& ranges,
                                   const QString& file, const QString& code);
    
    // Status
    bool isConnected() const { return connected; }
    bool isProcessing() const { return processing; }
    QString getLastError() const { return lastError; }
    int getPendingRequests() const { return pendingRequests; }
    
    // Results
    QVector<CodeSuggestion> getLastSuggestions() const { return lastSuggestions; }
    QString getLastExplanation() const { return lastExplanation; }
    
    // Apply suggestions
    void applySuggestion(int suggestionId);
    void applyAllSuggestions();
    void rejectSuggestion(int suggestionId);
    void rejectAllSuggestions();
    
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
    void setupNetwork();
    QNetworkRequest createRequest(const QString& endpoint);
    QJsonObject buildRequestBody(const QString& prompt, const QString& context);
    QVector<CodeSuggestion> parseSuggestions(const QJsonObject& response);
    QString parseExplanation(const QJsonObject& response);
    QString parseCompletion(const QJsonObject& response);
    void sendRequest(const QString& endpoint, const QJsonObject& body);
    QString buildContext(const QString& file, int startLine, int endLine, 
                        const QString& code);
    QString buildPrompt(const QString& task, const QString& context, 
                       const QString& userPrompt);
    
    LLMConfig config;
    QNetworkAccessManager* networkManager;
    bool connected;
    bool processing;
    int pendingRequests;
    QString lastError;
    
    QVector<CodeSuggestion> lastSuggestions;
    QString lastExplanation;
    QString lastCompletion;
    
    QTimer* timeoutTimer;
    QMap<QNetworkReply*, QString> requestTypes;
};

} // namespace proxima

#endif // CENTAURI_LLMSERVICE_H
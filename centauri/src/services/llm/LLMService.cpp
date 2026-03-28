#include "LLMService.h"
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonValue>
#include <QCryptographicHash>
#include <QUuid>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

LLMService::LLMService(QObject *parent)
    : QObject(parent)
    , connected(false)
    , processing(false)
    , pendingRequests(0)
    , networkManager(nullptr)
    , timeoutTimer(nullptr)
{
    // Конфигурация по умолчанию
    config.serverURL = "http://localhost:1234/v1";  // LM Studio по умолчанию
    config.model = "default";
    config.maxTokens = 2048;
    config.temperature = 0.7;
    config.timeout = 30000; // 30 секунд
    config.apiKey = "";
    
    setupNetwork();
}

LLMService::~LLMService() {
    if (timeoutTimer) {
        timeoutTimer->stop();
        delete timeoutTimer;
    }
    
    if (networkManager) {
        networkManager->deleteLater();
    }
}

// ============================================================================
// Инициализация сети
// ============================================================================

void LLMService::setupNetwork() {
    networkManager = new QNetworkAccessManager(this);
    
    timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &LLMService::onTimeout);
    
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &LLMService::onNetworkReplyFinished);
}

// ============================================================================
// Конфигурация
// ============================================================================

void LLMService::setConfig(const LLMConfig& newConfig) {
    config = newConfig;
    LOG_INFO("LLM configuration updated: " + config.serverURL.toStdString());
}

void LLMService::setServerURL(const QString& url) {
    config.serverURL = url;
    LOG_INFO("LLM server URL set to: " + url.toStdString());
}

void LLMService::setModel(const QString& model) {
    config.model = model;
    LOG_INFO("LLM model set to: " + model.toStdString());
}

void LLMService::setAPIKey(const QString& key) {
    config.apiKey = key;
}

// ============================================================================
// Создание запроса
// ============================================================================

QNetworkRequest LLMService::createRequest(const QString& endpoint) {
    QUrl url(config.serverURL + "/" + endpoint);
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "Centauri-IDE/1.0");
    
    if (!config.apiKey.isEmpty()) {
        request.setRawHeader("Authorization", 
                            ("Bearer " + config.apiKey).toUtf8());
    }
    
    return request;
}

// ============================================================================
// Построение контекста
// ============================================================================

QString LLMService::buildContext(const QString& file, int startLine, int endLine, 
                                 const QString& code) {
    QString context;
    
    // Добавляем информацию о файле
    context += QString("File: %1\n").arg(file);
    context += QString("Lines: %1-%2\n\n").arg(startLine).arg(endLine);
    
    // Добавляем код
    context += code;
    
    return context;
}

// ============================================================================
// Построение промпта
// ============================================================================

QString LLMService::buildPrompt(const QString& task, const QString& context, 
                                const QString& userPrompt) {
    QString prompt;
    
    // Системная инструкция
    prompt += "You are an expert Proxima programming language assistant. ";
    prompt += "Help with code suggestions, explanations, and improvements.\n\n";
    
    // Задача
    prompt += QString("Task: %1\n\n").arg(task);
    
    // Контекст
    if (!context.isEmpty()) {
        prompt += QString("Context:\n%1\n\n").arg(context);
    }
    
    // Пользовательский запрос
    if (!userPrompt.isEmpty()) {
        prompt += QString("Request: %1\n\n").arg(userPrompt);
    }
    
    // Инструкция по формату ответа
    prompt += "Please provide your response in the following format:\n";
    prompt += "1. Brief explanation\n";
    prompt += "2. Code suggestion (if applicable)\n";
    prompt += "3. Additional notes (if any)";
    
    return prompt;
}

// ============================================================================
// Построение тела запроса
// ============================================================================

QJsonObject LLMService::buildRequestBody(const QString& prompt, const QString& context) {
    QJsonObject requestBody;
    
    requestBody["model"] = config.model;
    
    QJsonArray messages;
    
    // Системное сообщение
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "You are an expert Proxima programming language assistant.";
    messages.append(systemMessage);
    
    // Пользовательское сообщение
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt;
    messages.append(userMessage);
    
    requestBody["messages"] = messages;
    requestBody["max_tokens"] = config.maxTokens;
    requestBody["temperature"] = config.temperature;
    requestBody["stream"] = false;
    
    return requestBody;
}

// ============================================================================
// Отправка запроса
// ============================================================================

void LLMService::sendRequest(const QString& endpoint, const QJsonObject& body) {
    QNetworkRequest request = createRequest(endpoint);
    
    QJsonDocument doc(body);
    QByteArray jsonData = doc.toJson();
    
    QNetworkReply* reply = networkManager->post(request, jsonData);
    
    pendingRequests++;
    emit processingStarted();
    
    // Запуск таймера таймаута
    timeoutTimer->start(config.timeout);
    
    LOG_DEBUG("Request sent to " + endpoint.toStdString());
}

// ============================================================================
// Запрос предложений кода
// ============================================================================

void LLMService::requestSuggestions(const QString& file, int startLine, int endLine,
                                   const QString& code, const QString& prompt) {
    QString context = buildContext(file, startLine, endLine, code);
    QString fullPrompt = buildPrompt("Provide code suggestions and improvements", 
                                     context, prompt);
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Code suggestions requested for " + file.toStdString());
}

// ============================================================================
// Запрос объяснения
// ============================================================================

void LLMService::requestExplanation(const QString& file, int startLine, int endLine,
                                   const QString& code) {
    QString context = buildContext(file, startLine, endLine, code);
    QString fullPrompt = buildPrompt("Explain this code", context, 
                                    "Please explain what this code does, its purpose, and how it works.");
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Code explanation requested for " + file.toStdString());
}

// ============================================================================
// Завершение кода
// ============================================================================

void LLMService::requestCompletion(const QString& file, int line, int column,
                                  const QString& prefix, const QString& suffix) {
    QString context = buildContext(file, line, column, prefix);
    if (!suffix.isEmpty()) {
        context += "\n\nSuffix:\n" + suffix;
    }
    
    QString fullPrompt = buildPrompt("Complete this code", context,
                                    "Please complete the code at the cursor position. Only return the completion, no explanation.");
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Code completion requested for " + file.toStdString());
}

// ============================================================================
// Рефакторинг
// ============================================================================

void LLMService::requestRefactoring(const QString& file, int startLine, int endLine,
                                   const QString& code, const QString& goal) {
    QString context = buildContext(file, startLine, endLine, code);
    QString fullPrompt = buildPrompt("Refactor this code", context,
                                    QString("Goal: %1\nPlease refactor the code to achieve this goal.").arg(goal));
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Code refactoring requested for " + file.toStdString());
}

// ============================================================================
// Документация
// ============================================================================

void LLMService::requestDocumentation(const QString& file, int startLine, int endLine,
                                     const QString& code) {
    QString context = buildContext(file, startLine, endLine, code);
    QString fullPrompt = buildPrompt("Generate documentation", context,
                                    "Please generate comprehensive documentation for this code including function descriptions, parameters, return values, and usage examples.");
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Documentation generation requested for " + file.toStdString());
}

// ============================================================================
// Исправление ошибок
// ============================================================================

void LLMService::requestBugFix(const QString& file, int startLine, int endLine,
                              const QString& code, const QString& errorMessage) {
    QString context = buildContext(file, startLine, endLine, code);
    QString fullPrompt = buildPrompt("Fix bugs in this code", context,
                                    QString("Error message: %1\nPlease identify and fix the bug(s) in this code.").arg(errorMessage));
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Bug fix requested for " + file.toStdString());
}

// ============================================================================
// Множественные предложения
// ============================================================================

void LLMService::requestMultipleSuggestions(const QVector<QPair<int, int>>& ranges,
                                           const QString& file, const QString& code) {
    // Для множественных диапазонов создаём комплексный запрос
    QString context = buildContext(file, 0, 0, code);
    
    QString rangesStr;
    for (const auto& range : ranges) {
        rangesStr += QString("Lines %1-%2\n").arg(range.first).arg(range.second);
    }
    
    QString fullPrompt = buildPrompt("Provide suggestions for multiple code ranges", 
                                     context,
                                     QString("Please review and suggest improvements for these code ranges:\n%1").arg(rangesStr));
    
    QJsonObject body = buildRequestBody(fullPrompt, context);
    sendRequest("chat/completions", body);
    
    LOG_INFO("Multiple suggestions requested for " + file.toStdString());
}

// ============================================================================
// Обработка ответов сети
// ============================================================================

void LLMService::onNetworkReplyFinished(QNetworkReply* reply) {
    timeoutTimer->stop();
    pendingRequests--;
    
    if (reply->error() != QNetworkReply::NoError) {
        lastError = reply->errorString();
        LOG_ERROR("Network error: " + lastError.toStdString());
        emit errorOccurred(lastError);
        
        if (pendingRequests == 0) {
            emit processingFinished();
            processing = false;
        }
        
        reply->deleteLater();
        return;
    }
    
    // Чтение ответа
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull()) {
        lastError = "Invalid JSON response";
        emit errorOccurred(lastError);
        
        if (pendingRequests == 0) {
            emit processingFinished();
            processing = false;
        }
        
        reply->deleteLater();
        return;
    }
    
    QJsonObject response = doc.object();
    
    // Парсинг в зависимости от типа запроса
    // В реальном приложении нужно отслеживать тип запроса
    
    // Попытка распарсить предложения
    QVector<CodeSuggestion> suggestions = parseSuggestions(response);
    if (!suggestions.isEmpty()) {
        lastSuggestions = suggestions;
        emit suggestionsReady(suggestions);
    }
    
    // Попытка распарсить объяснение
    QString explanation = parseExplanation(response);
    if (!explanation.isEmpty()) {
        lastExplanation = explanation;
        emit explanationReady(explanation);
    }
    
    // Попытка распарсить завершение
    QString completion = parseCompletion(response);
    if (!completion.isEmpty()) {
        lastCompletion = completion;
        emit completionReady(completion);
    }
    
    if (pendingRequests == 0) {
        emit processingFinished();
        processing = false;
    }
    
    reply->deleteLater();
}

void LLMService::onNetworkError(QNetworkReply::NetworkError error) {
    lastError = QString("Network error: %1").arg(static_cast<int>(error));
    LOG_ERROR(lastError.toStdString());
    emit errorOccurred(lastError);
}

void LLMService::onTimeout() {
    lastError = "Request timeout";
    LOG_ERROR("LLM request timeout");
    emit errorOccurred(lastError);
    
    // Прерывание всех текущих запросов
    networkManager->clearAccessCache();
    
    if (pendingRequests > 0) {
        pendingRequests = 0;
        emit processingFinished();
        processing = false;
    }
}

// ============================================================================
// Парсинг ответов
// ============================================================================

QVector<CodeSuggestion> LLMService::parseSuggestions(const QJsonObject& response) {
    QVector<CodeSuggestion> suggestions;
    
    // Парсинг ответа от LLM API
    // Формат зависит от конкретного API
    
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        
        for (int i = 0; i < choices.size(); ++i) {
            QJsonObject choice = choices[i].toObject();
            
            if (choice.contains("message")) {
                QJsonObject message = choice["message"].toObject();
                QString content = message["content"].toString();
                
                CodeSuggestion suggestion;
                suggestion.id = i;
                suggestion.suggestedCode = content;
                suggestion.confidence = 0.8; // По умолчанию
                suggestion.accepted = false;
                
                suggestions.append(suggestion);
            }
        }
    }
    
    return suggestions;
}

QString LLMService::parseExplanation(const QJsonObject& response) {
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            
            if (choice.contains("message")) {
                QJsonObject message = choice["message"].toObject();
                return message["content"].toString();
            }
        }
    }
    
    return QString();
}

QString LLMService::parseCompletion(const QJsonObject& response) {
    return parseExplanation(response);
}

// ============================================================================
// Применение предложений
// ============================================================================

void LLMService::applySuggestion(int suggestionId) {
    for (auto& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = true;
            LOG_INFO("Suggestion " + QString::number(suggestionId).toStdString() + " accepted");
            break;
        }
    }
}

void LLMService::applyAllSuggestions() {
    for (auto& suggestion : lastSuggestions) {
        suggestion.accepted = true;
    }
    LOG_INFO("All suggestions accepted");
}

void LLMService::rejectSuggestion(int suggestionId) {
    for (auto& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = false;
            LOG_INFO("Suggestion " + QString::number(suggestionId).toStdString() + " rejected");
            break;
        }
    }
}

void LLMService::rejectAllSuggestions() {
    for (auto& suggestion : lastSuggestions) {
        suggestion.accepted = false;
    }
    LOG_INFO("All suggestions rejected");
}

} // namespace proxima

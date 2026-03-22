#include "LLMClient.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include "utils/Logger.h"
#include "utils/Protocol.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

LLMClient::LLMClient(QObject *parent)
    : QObject(parent)
    , serverURL("http://localhost:1234")
    , model("local-model")
    , maxTokens(2048)
    , temperature(0.7)
    , timeout(30000)
    , connected(false)
    , processing(false)
    , pendingRequests(0) {

    networkManager = new QNetworkAccessManager(this);

    // Проверка соединения при создании
    checkConnection();

    LOG_INFO("LLMClient initialized");
}

LLMClient::~LLMClient() {
    // Отмена всех_pending запросов
    for (QNetworkReply* reply : pendingReplies) {
        if (reply) {
            reply->abort();
            reply->deleteLater();
        }
    }
    pendingReplies.clear();

    LOG_INFO("LLMClient destroyed");
}

// ============================================================================
// Конфигурация
// ============================================================================

void LLMClient::setServerURL(const QString& url) {
    serverURL = url;
    LOG_INFO("LLM server URL set to: " + url.toStdString());
    checkConnection();
}

void LLMClient::setModel(const QString& modelName) {
    model = modelName;
    LOG_INFO("LLM model set to: " + modelName.toStdString());
}

void LLMClient::setAPIKey(const QString& key) {
    apiKey = key;
    LOG_DEBUG("API key set");
}

void LLMClient::setMaxTokens(int tokens) {
    maxTokens = tokens;
    LOG_DEBUG("Max tokens set to: " + std::to_string(tokens));
}

void LLMClient::setTemperature(double temp) {
    temperature = temp;
    LOG_DEBUG("Temperature set to: " + std::to_string(temp));
}

void LLMClient::setTimeout(int ms) {
    timeout = ms;
    LOG_DEBUG("Timeout set to: " + std::to_string(ms) + " ms");
}

// ============================================================================
// Проверка соединения
// ============================================================================

bool LLMClient::checkConnection() {
    QUrl url(serverURL + "/v1/models");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!apiKey.isEmpty()) {
        request.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
    }

    QNetworkReply* reply = networkManager->get(request);

    // Ждём ответа
    QEventLoop loop;
    QTimer timeoutTimer;
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeoutTimer.start(5000);

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if (timeoutTimer.isActive()) {
        timeoutTimer.stop();
    }

    bool success = false;

    if (reply->error() == QNetworkReply::NoError) {
        connected = true;
        success = true;
        LOG_INFO("LLM server connection successful");
    } else {
        connected = false;
        lastError = reply->errorString();
        LOG_WARNING("LLM server connection failed: " + lastError.toStdString());
    }

    reply->deleteLater();
    emit connectionStatusChanged(connected);

    return success;
}

bool LLMClient::isConnected() const {
    return connected;
}

// ============================================================================
// Основной запрос к LLM
// ============================================================================

LLMResponse LLMClient::sendRequest(const LLMRequest& request) {
    if (!connected) {
        LLMResponse response;
        response.success = false;
        response.errorMessage = "Not connected to LLM server";
        return response;
    }

    processing = true;
    pendingRequests++;
    emit processingStarted();

    auto startTime = std::chrono::high_resolution_clock::now();

    // Build request body using collection format (not JSON)
    QString requestBody = buildRequestBody(request);

    QUrl url(serverURL + "/v1/chat/completions");
    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-proxima-collection");
    networkRequest.setRawHeader("Accept", "application/x-proxima-collection");

    if (!apiKey.isEmpty()) {
        networkRequest.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
    }

    QNetworkReply* reply = networkManager->post(networkRequest, requestBody.toUtf8());
    pendingReplies.push_back(reply);

    // Setup timeout
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    timeoutTimer->start(timeout);

    // Wait for response
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);

    loop.exec();

    auto endTime = std::chrono::high_resolution_clock::now();

    LLMResponse response;
    response.processingTime = std::chrono::duration<double>(endTime - startTime).count();

    if (timeoutTimer->isActive()) {
        timeoutTimer->stop();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            response = parseResponse(QString::fromUtf8(responseData));
            response.success = true;

            LOG_INFO("LLM request completed in " + std::to_string(response.processingTime) + "s");
        } else {
            response.success = false;
            response.errorMessage = reply->errorString();
            lastError = response.errorMessage;
            LOG_ERROR("LLM request failed: " + response.errorMessage.toStdString());
        }
    } else {
        response.success = false;
        response.errorMessage = "Request timeout";
        lastError = response.errorMessage;
        reply->abort();
        LOG_ERROR("LLM request timeout");
    }

    timeoutTimer->deleteLater();
    reply->deleteLater();
    pendingReplies.removeOne(reply);
    pendingRequests--;
    processing = false;

    emit processingFinished();

    if (responseCallback) {
        responseCallback(response);
    }

    return response;
}

// ============================================================================
// Построение запроса в формате collection
// ============================================================================

QString LLMClient::buildRequestBody(const LLMRequest& request) {
    CollectionParser parser;

    QVector<QPair<QString, CollectionParser::Value>> pairs;
    pairs.append(qMakePair("type", CollectionParser::Value::fromString(request.type)));
    pairs.append(qMakePair("model", CollectionParser::Value::fromString(model)));
    pairs.append(qMakePair("max_tokens", CollectionParser::Value::fromNumber(maxTokens)));
    pairs.append(qMakePair("temperature", CollectionParser::Value::fromNumber(temperature)));
    pairs.append(qMakePair("file", CollectionParser::Value::fromString(request.file)));
    pairs.append(qMakePair("start_line", CollectionParser::Value::fromNumber(request.startLine)));
    pairs.append(qMakePair("end_line", CollectionParser::Value::fromNumber(request.endLine)));
    pairs.append(qMakePair("prompt", CollectionParser::Value::fromString(request.prompt)));
    pairs.append(qMakePair("code_context", CollectionParser::Value::fromString(request.codeContext)));

    // Options как вложенная collection
    QVector<QPair<QString, CollectionParser::Value>> optionPairs;
    for (auto it = request.options.begin(); it != request.options.end(); ++it) {
        optionPairs.append(qMakePair(it.key(), CollectionParser::Value::fromString(it.value())));
    }
    pairs.append(qMakePair("options", CollectionParser::createCollection(optionPairs)));

    CollectionParser::Value collection = CollectionParser::createCollection(pairs);
    return parser.serialize(collection);
}

// ============================================================================
// Парсинг ответа в формате collection
// ============================================================================

LLMResponse LLMClient::parseResponse(const QString& responseData) {
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(responseData);

    LLMResponse response;

    if (result.success) {
        response.success = result.value.get("status").asString() == "ok";
        response.errorMessage = result.value.get("error_message").asString();
        response.explanation = result.value.get("explanation").asString();
        response.processingTime = result.value.get("processing_time").asNumber();

        // Parse suggestions array
        CollectionParser::Value suggestionsValue = result.value.get("suggestions");
        if (suggestionsValue.isArray()) {
            for (const auto& suggestionValue : suggestionsValue.asArray()) {
                CodeSuggestion suggestion;
                suggestion.blockId = suggestionValue.get("block_id").asNumber();
                suggestion.startLine = suggestionValue.get("start_line").asNumber();
                suggestion.endLine = suggestionValue.get("end_line").asNumber();
                suggestion.originalCode = suggestionValue.get("original_code").asString();
                suggestion.suggestedCode = suggestionValue.get("suggested_code").asString();
                suggestion.explanation = suggestionValue.get("explanation").asString();
                suggestion.confidence = suggestionValue.get("confidence").asNumber();
                response.suggestions.append(suggestion);
            }
        }
    }

    return response;
}

// ============================================================================
// Методы помощи в кодировании
// ============================================================================

QVector<CodeSuggestion> LLMClient::suggestModifications(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code,
    const QString& prompt) {

    LLMRequest request;
    request.type = "code_modification";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = prompt.isEmpty() ? "Improve this code" : prompt;
    request.codeContext = code;

    LLMResponse response = sendRequest(request);

    if (response.success) {
        return response.suggestions;
    } else {
        LOG_ERROR("Code modification request failed: " + response.errorMessage.toStdString());
        return QVector<CodeSuggestion>();
    }
}

QString LLMClient::explainCode(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code) {

    LLMRequest request;
    request.type = "explanation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Explain this code in detail, including its purpose, logic, and potential improvements";
    request.codeContext = code;

    LLMResponse response = sendRequest(request);

    if (response.success) {
        return response.explanation;
    } else {
        return "Error: " + response.errorMessage;
    }
}

QString LLMClient::completeCode(
    const QString& file,
    int line,
    int column,
    const QString& prefix,
    const QString& suffix) {

    LLMRequest request;
    request.type = "completion";
    request.file = file;
    request.startLine = line;
    request.endLine = line;
    request.prompt = "Complete this code";
    request.codeContext = prefix;
    request.options["suffix"] = suffix;

    LLMResponse response = sendRequest(request);

    if (response.success && !response.suggestions.isEmpty()) {
        return response.suggestions[0].suggestedCode;
    }

    return "";
}

LLMResponse LLMClient::refactorCode(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code,
    const QString& goal) {

    LLMRequest request;
    request.type = "refactoring";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Refactor this code: " + goal;
    request.codeContext = code;
    request.options["goal"] = goal;

    return sendRequest(request);
}

LLMResponse LLMClient::generateDocumentation(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code) {

    LLMRequest request;
    request.type = "documentation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Generate comprehensive documentation for this code including @param, @return, and @method tags";
    request.codeContext = code;

    return sendRequest(request);
}

LLMResponse LLMClient::fixBugs(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code,
    const QString& errorMessage) {

    LLMRequest request;
    request.type = "bug_fix";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Fix bugs in this code. Error message: " + errorMessage;
    request.codeContext = code;
    request.options["error_message"] = errorMessage;

    return sendRequest(request);
}

LLMResponse LLMClient::generateTests(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code) {

    LLMRequest request;
    request.type = "test_generation";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Generate comprehensive unit tests for this code including edge cases";
    request.codeContext = code;

    return sendRequest(request);
}

LLMResponse LLMClient::optimizeCode(
    const QString& file,
    int startLine,
    int endLine,
    const QString& code,
    const QString& optimizationGoal) {

    LLMRequest request;
    request.type = "optimization";
    request.file = file;
    request.startLine = startLine;
    request.endLine = endLine;
    request.prompt = "Optimize this code for: " + optimizationGoal;
    request.codeContext = code;
    request.options["optimization_goal"] = optimizationGoal;

    return sendRequest(request);
}

// ============================================================================
// Применение изменений
// ============================================================================

void LLMClient::applySuggestion(int suggestionId) {
    for (CodeSuggestion& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = true;
            LOG_INFO("Suggestion " + std::to_string(suggestionId) + " accepted");
            break;
        }
    }
}

void LLMClient::applyAllSuggestions() {
    for (CodeSuggestion& suggestion : lastSuggestions) {
        suggestion.accepted = true;
    }
    LOG_INFO("All suggestions accepted");
}

void LLMClient::rejectSuggestion(int suggestionId) {
    for (CodeSuggestion& suggestion : lastSuggestions) {
        if (suggestion.id == suggestionId) {
            suggestion.accepted = false;
            LOG_INFO("Suggestion " + std::to_string(suggestionId) + " rejected");
            break;
        }
    }
}

void LLMClient::rejectAllSuggestions() {
    for (CodeSuggestion& suggestion : lastSuggestions) {
        suggestion.accepted = false;
    }
    LOG_INFO("All suggestions rejected");
}

QVector<CodeSuggestion> LLMClient::getAcceptedSuggestions() const {
    QVector<CodeSuggestion> accepted;
    for (const CodeSuggestion& suggestion : lastSuggestions) {
        if (suggestion.accepted) {
            accepted.append(suggestion);
        }
    }
    return accepted;
}

// ============================================================================
// Управление историей диалога
// ============================================================================

void LLMClient::addToConversationHistory(const QString& role, const QString& content) {
    ConversationMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime();

    conversationHistory.append(msg);

    // Ограничение размера истории
    if (conversationHistory.size() > 20) {
        conversationHistory.removeFirst();
    }
}

void LLMClient::clearConversationHistory() {
    conversationHistory.clear();
    LOG_DEBUG("Conversation history cleared");
}

QVector<ConversationMessage> LLMClient::getConversationHistory() const {
    return conversationHistory;
}

// ============================================================================
// Сохранение/загрузка настроек
// ============================================================================

void LLMClient::saveSettings() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
                        "/llm_settings.conf";

    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << "[LLM]\n";
        out << "server_url=" << serverURL << "\n";
        out << "model=" << model << "\n";
        out << "max_tokens=" << maxTokens << "\n";
        out << "temperature=" << temperature << "\n";
        out << "timeout=" << timeout << "\n";
        file.close();
        LOG_INFO("LLM settings saved");
    }
}

void LLMClient::loadSettings() {
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
                        "/llm_settings.conf";

    QFile file(configPath);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString line;

        while (!in.atEnd()) {
            line = in.readLine();

            if (line.startsWith("server_url=")) {
                serverURL = line.mid(11);
            } else if (line.startsWith("model=")) {
                model = line.mid(6);
            } else if (line.startsWith("max_tokens=")) {
                maxTokens = line.mid(11).toInt();
            } else if (line.startsWith("temperature=")) {
                temperature = line.mid(12).toDouble();
            } else if (line.startsWith("timeout=")) {
                timeout = line.mid(8).toInt();
            }
        }

        file.close();
        LOG_INFO("LLM settings loaded");
    }
}

// ============================================================================
// Callbacks
// ============================================================================

void LLMClient::onResponse(ResponseCallback callback) {
    responseCallback = callback;
}

// ============================================================================
// Обработка сетевых ответов
// ============================================================================

void LLMClient::onNetworkReplyFinished(QNetworkReply* reply) {
    pendingReplies.removeOne(reply);
    reply->deleteLater();

    LOG_DEBUG("Network reply finished");
}

void LLMClient::onNetworkError(QNetworkReply::NetworkError error) {
    lastError = "Network error: " + QString::number(error);
    LOG_ERROR(lastError.toStdString());

    emit errorOccurred(lastError);
}

void LLMClient::onTimeout() {
    lastError = "Request timeout";
    LOG_ERROR(lastError.toStdString());

    emit errorOccurred(lastError);
}

// ============================================================================
// Утилиты
// ============================================================================

QString LLMClient::escapeString(const QString& str) const {
    QString escaped = str;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

QString LLMClient::unescapeString(const QString& str) const {
    QString unescaped = str;
    unescaped.replace("\\n", "\n");
    unescaped.replace("\\r", "\r");
    unescaped.replace("\\t", "\t");
    unescaped.replace("\\\"", "\"");
    unescaped.replace("\\\\", "\\");
    return unescaped;
}

int LLMClient::getPendingRequests() const {
    return pendingRequests;
}

QString LLMClient::getLastError() const {
    return lastError;
}

void LLMClient::clearError() {
    lastError.clear();
}

QVector<CodeSuggestion> LLMClient::getLastSuggestions() const {
    return lastSuggestions;
}

QString LLMClient::getLastExplanation() const {
    return lastExplanation;
}

QString LLMClient::getLastCompletion() const {
    return lastCompletion;
}

// ============================================================================
// Batch операции
// ============================================================================

LLMResponse LLMClient::requestMultipleSuggestions(
    const QVector<QPair<int, int>>& ranges,
    const QString& file,
    const QString& code,
    const QString& prompt) {

    LLMResponse combinedResponse;
    combinedResponse.success = true;

    for (const auto& range : ranges) {
        LLMRequest request;
        request.type = "code_modification";
        request.file = file;
        request.startLine = range.first;
        request.endLine = range.second;
        request.prompt = prompt;
        request.codeContext = code;

        LLMResponse response = sendRequest(request);

        if (response.success) {
            combinedResponse.suggestions.append(response.suggestions);
        } else {
            combinedResponse.success = false;
            combinedResponse.errorMessage += response.errorMessage + "; ";
        }
    }

    return combinedResponse;
}

// ============================================================================
// Кэширование запросов
// ============================================================================

QString LLMClient::getCacheKey(const LLMRequest& request) const {
    QString key = request.type + "|" +
                  request.file + "|" +
                  QString::number(request.startLine) + "|" +
                  QString::number(request.endLine) + "|" +
                  request.prompt;

    // MD5 хеш для ключа
    QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5);
    return QString::fromUtf8(hash.toHex());
}

bool LLMClient::getCachedResponse(const QString& cacheKey, LLMResponse& response) {
    auto it = responseCache.find(cacheKey);
    if (it != responseCache.end()) {
        // Проверка времени жизни кэша (5 минут)
        if (it->timestamp.secsTo(QDateTime::currentDateTime()) < 300) {
            response = it->response;
            return true;
        } else {
            responseCache.remove(cacheKey);
        }
    }
    return false;
}

void LLMClient::cacheResponse(const QString& cacheKey, const LLMResponse& response) {
    CacheEntry entry;
    entry.response = response;
    entry.timestamp = QDateTime::currentDateTime();
    responseCache[cacheKey] = entry;

    // Очистка старого кэша
    if (responseCache.size() > 100) {
        auto it = responseCache.begin();
        responseCache.erase(it);
    }
}

void LLMClient::clearCache() {
    responseCache.clear();
    LOG_DEBUG("LLM response cache cleared");
}

// ============================================================================
// Логирование
// ============================================================================

void LLMClient::log(int level, const std::string& message) {
    if (level <= 3) {  // 3 = INFO
        std::cout << "[LLMClient] " << message << std::endl;
        LOG_INFO("[LLMClient] " + QString::fromStdString(message));
    }
}

} // namespace proxima

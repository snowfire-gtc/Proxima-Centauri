#include "Protocol.h"
#include "utils/CollectionParser.h"
#include "utils/Logger.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStack>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QVariant>
#include <QIODevice>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

namespace proxima {

// ============================================================================
// Collection Class Implementation
// ============================================================================

Collection::Collection()
    : m_type(CollectionType::Collection) {
}

Collection::Collection(const Collection& other)
    : m_type(other.m_type)
    , m_stringValue(other.m_stringValue)
    , m_numberValue(other.m_numberValue)
    , m_boolValue(other.m_boolValue)
    , m_arrayValue(other.m_arrayValue)
    , m_objectValue(other.m_objectValue)
    , m_collectionPairs(other.m_collectionPairs) {
}

Collection& Collection::operator=(const Collection& other) {
    if (this != &other) {
        m_type = other.m_type;
        m_stringValue = other.m_stringValue;
        m_numberValue = other.m_numberValue;
        m_boolValue = other.m_boolValue;
        m_arrayValue = other.m_arrayValue;
        m_objectValue = other.m_objectValue;
        m_collectionPairs = other.m_collectionPairs;
    }
    return *this;
}

Collection::~Collection() {
    clear();
}

void Collection::clear() {
    m_stringValue.clear();
    m_numberValue = 0.0;
    m_boolValue = false;
    m_arrayValue.clear();
    m_objectValue.clear();
    m_collectionPairs.clear();
}

Collection Collection::fromString(const QString& value) {
    Collection c;
    c.m_type = CollectionType::String;
    c.m_stringValue = value;
    return c;
}

Collection Collection::fromNumber(double value) {
    Collection c;
    c.m_type = CollectionType::Number;
    c.m_numberValue = value;
    return c;
}

Collection Collection::fromBoolean(bool value) {
    Collection c;
    c.m_type = CollectionType::Boolean;
    c.m_boolValue = value;
    return c;
}

Collection Collection::fromArray(const QVector<Collection>& array) {
    Collection c;
    c.m_type = CollectionType::Array;
    c.m_arrayValue = array;
    return c;
}

Collection Collection::fromObject(const QMap<QString, Collection>& object) {
    Collection c;
    c.m_type = CollectionType::Object;
    c.m_objectValue = object;
    return c;
}

Collection Collection::fromCollection(const QVector<QPair<QString, Collection>>& pairs) {
    Collection c;
    c.m_type = CollectionType::Collection;
    c.m_collectionPairs = pairs;
    return c;
}

Collection Collection::fromCollection(const std::initializer_list<QPair<QString, Collection>>& pairs) {
    Collection c;
    c.m_type = CollectionType::Collection;
    for (const auto& pair : pairs) {
        c.m_collectionPairs.append(pair);
    }
    return c;
}

CollectionType Collection::type() const {
    return m_type;
}

QString Collection::toString() const {
    if (m_type == CollectionType::String) {
        return m_stringValue;
    }
    return QString();
}

double Collection::toNumber() const {
    if (m_type == CollectionType::Number) {
        return m_numberValue;
    }
    return 0.0;
}

bool Collection::toBoolean() const {
    if (m_type == CollectionType::Boolean) {
        return m_boolValue;
    }
    return false;
}

QVector<Collection> Collection::toArray() const {
    if (m_type == CollectionType::Array) {
        return m_arrayValue;
    }
    return QVector<Collection>();
}

QMap<QString, Collection> Collection::toObject() const {
    if (m_type == CollectionType::Object) {
        return m_objectValue;
    }
    return QMap<QString, Collection>();
}

QVector<QPair<QString, Collection>> Collection::toCollectionPairs() const {
    if (m_type == CollectionType::Collection) {
        return m_collectionPairs;
    }
    return QVector<QPair<QString, Collection>>();
}

Collection Collection::get(const QString& key) const {
    if (m_type == CollectionType::Collection) {
        for (const auto& pair : m_collectionPairs) {
            if (pair.first == key) {
                return pair.second;
            }
        }
    } else if (m_type == CollectionType::Object) {
        auto it = m_objectValue.find(key);
        if (it != m_objectValue.end()) {
            return it.value();
        }
    }
    return Collection();
}

Collection Collection::get(int index) const {
    if (m_type == CollectionType::Array && index >= 0 && index < m_arrayValue.size()) {
        return m_arrayValue[index];
    }
    return Collection();
}

bool Collection::has(const QString& key) const {
    if (m_type == CollectionType::Collection) {
        for (const auto& pair : m_collectionPairs) {
            if (pair.first == key) {
                return true;
            }
        }
    } else if (m_type == CollectionType::Object) {
        return m_objectValue.contains(key);
    }
    return false;
}

int Collection::size() const {
    switch (m_type) {
        case CollectionType::Array:
            return m_arrayValue.size();
        case CollectionType::Object:
            return m_objectValue.size();
        case CollectionType::Collection:
            return m_collectionPairs.size();
        default:
            return 0;
    }
}

bool Collection::isEmpty() const {
    return size() == 0;
}

void Collection::set(const QString& key, const Collection& value) {
    if (m_type == CollectionType::Collection) {
        for (int i = 0; i < m_collectionPairs.size(); i++) {
            if (m_collectionPairs[i].first == key) {
                m_collectionPairs[i].second = value;
                return;
            }
        }
        m_collectionPairs.append(qMakePair(key, value));
    }
}

void Collection::append(const Collection& value) {
    if (m_type == CollectionType::Array) {
        m_arrayValue.append(value);
    }
}

void Collection::append(const QString& key, const Collection& value) {
    if (m_type == CollectionType::Collection) {
        m_collectionPairs.append(qMakePair(key, value));
    }
}

QString Collection::serialize(int indent) const {
    return toCollectionString(indent);
}

Collection Collection::deserialize(const QString& input) {
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(input);
    
    if (result.success) {
        return collectionValueToCollection(result.value);
    }
    
    LOG_ERROR("Collection deserialize error: " + result.error.toStdString());
    return Collection();
}

bool Collection::isValid(const QString& input) {
    return CollectionParser::isValid(input);
}

QString Collection::toCollectionString(int indent) const {
    QString result;
    QString indentStr(indent * 4, ' ');
    QString nextIndentStr((indent + 1) * 4, ' ');
    
    switch (m_type) {
        case CollectionType::String:
            result = "\"" + escapeString(m_stringValue) + "\"";
            break;
            
        case CollectionType::Number: {
            // Проверка на целое число
            if (m_numberValue == static_cast<int64_t>(m_numberValue)) {
                result = QString::number(static_cast<int64_t>(m_numberValue));
            } else {
                result = QString::number(m_numberValue, 'g', 15);
            }
            break;
        }
            
        case CollectionType::Boolean:
            result = m_boolValue ? "true" : "false";
            break;
            
        case CollectionType::Null:
            result = "null";
            break;
            
        case CollectionType::Array: {
            result += "[";
            for (int i = 0; i < m_arrayValue.size(); i++) {
                if (i > 0) {
                    result += ", ";
                }
                result += m_arrayValue[i].toCollectionString(indent + 1);
            }
            result += "]";
            break;
        }
            
        case CollectionType::Object: {
            result += "{\n";
            int count = 0;
            for (auto it = m_objectValue.begin(); it != m_objectValue.end(); ++it) {
                if (count > 0) {
                    result += ",\n";
                }
                result += nextIndentStr + "\"" + escapeString(it.key()) + "\": " + 
                         it.value().toCollectionString(indent + 1);
                count++;
            }
            if (count > 0) {
                result += "\n";
            }
            result += indentStr + "}";
            break;
        }
            
        case CollectionType::Collection: {
            result += "[\n";
            int count = 0;
            for (const auto& pair : m_collectionPairs) {
                if (count > 0) {
                    result += ",,\n";
                }
                result += nextIndentStr + "\"" + escapeString(pair.first) + "\", " + 
                         pair.second.toCollectionString(indent + 1);
                count++;
            }
            if (count > 0) {
                result += "\n";
            }
            result += indentStr + "]";
            break;
        }
            
        default:
            result = "null";
            break;
    }
    
    return result;
}

QString Collection::escapeString(const QString& str) const {
    QString result;
    for (int i = 0; i < str.size(); i++) {
        QChar c = str[i];
        switch (c.unicode()) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (c.unicode() < 32) {
                    result += QString("\\u%1").arg(c.unicode(), 4, 16, QChar('0'));
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

Collection Collection::collectionValueToCollection(const CollectionParser::Value& value) {
    Collection c;
    
    switch (value.type) {
        case CollectionParser::ValueType::String:
            c = Collection::fromString(value.stringValue);
            break;
            
        case CollectionParser::ValueType::Number:
            c = Collection::fromNumber(value.numberValue);
            break;
            
        case CollectionParser::ValueType::Boolean:
            c = Collection::fromBoolean(value.boolValue);
            break;
            
        case CollectionParser::ValueType::Null:
            c = Collection();
            break;
            
        case CollectionParser::ValueType::Array: {
            QVector<Collection> array;
            for (const auto& item : value.arrayValue) {
                array.append(collectionValueToCollection(item));
            }
            c = Collection::fromArray(array);
            break;
        }
            
        case CollectionParser::ValueType::Object: {
            QMap<QString, Collection> object;
            for (auto it = value.objectValue.begin(); it != value.objectValue.end(); ++it) {
                object[it.key()] = collectionValueToCollection(it.value());
            }
            c = Collection::fromObject(object);
            break;
        }
            
        case CollectionParser::ValueType::Collection: {
            QVector<QPair<QString, Collection>> pairs;
            for (auto it = value.objectValue.begin(); it != value.objectValue.end(); ++it) {
                pairs.append(qMakePair(it.key(), collectionValueToCollection(it.value())));
            }
            c = Collection::fromCollection(pairs);
            break;
        }
            
        default:
            break;
    }
    
    return c;
}

CollectionParser::Value Collection::collectionToCollectionValue(const Collection& collection) {
    CollectionParser::Value value;
    
    switch (collection.m_type) {
        case CollectionType::String:
            value = CollectionParser::Value::fromString(collection.m_stringValue);
            break;
            
        case CollectionType::Number:
            value = CollectionParser::Value::fromNumber(collection.m_numberValue);
            break;
            
        case CollectionType::Boolean:
            value = CollectionParser::Value::fromBoolean(collection.m_boolValue);
            break;
            
        case CollectionType::Null:
            value = CollectionParser::Value();
            break;
            
        case CollectionType::Array: {
            QVector<CollectionParser::Value> array;
            for (const auto& item : collection.m_arrayValue) {
                array.append(collectionToCollectionValue(item));
            }
            value = CollectionParser::Value::fromArray(array);
            break;
        }
            
        case CollectionType::Object: {
            QMap<QString, CollectionParser::Value> object;
            for (auto it = collection.m_objectValue.begin(); it != collection.m_objectValue.end(); ++it) {
                object[it.key()] = collectionToCollectionValue(it.value());
            }
            value = CollectionParser::Value::fromObject(object);
            break;
        }
            
        case CollectionType::Collection: {
            QMap<QString, CollectionParser::Value> collectionMap;
            for (const auto& pair : collection.m_collectionPairs) {
                collectionMap[pair.first] = collectionToCollectionValue(pair.second);
            }
            value = CollectionParser::Value::fromObject(collectionMap);
            break;
        }
            
        default:
            value = CollectionParser::Value();
            break;
    }
    
    return value;
}

bool Collection::operator==(const Collection& other) const {
    if (m_type != other.m_type) return false;
    
    switch (m_type) {
        case CollectionType::String:
            return m_stringValue == other.m_stringValue;
        case CollectionType::Number:
            return qAbs(m_numberValue - other.m_numberValue) < 1e-10;
        case CollectionType::Boolean:
            return m_boolValue == other.m_boolValue;
        case CollectionType::Null:
            return true;
        case CollectionType::Array:
            return m_arrayValue == other.m_arrayValue;
        case CollectionType::Object:
            return m_objectValue == other.m_objectValue;
        case CollectionType::Collection:
            return m_collectionPairs == other.m_collectionPairs;
        default:
            return false;
    }
}

// ============================================================================
// Message Implementation
// ============================================================================

Message::Message()
    : type(MessageType::ERROR_RESPONSE)
    , requestId("")
    , timestamp(QDateTime::currentDateTime().toString(Qt::ISODate)) {
}

Message::Message(MessageType t)
    : type(t)
    , requestId(QUuid::createUuid().toString())
    , timestamp(QDateTime::currentDateTime().toString(Qt::ISODate)) {
}

Message::~Message() {
}

QString Message::serialize() const {
    CollectionParser parser;
    
    // Создаём collection для сообщения
    QVector<QPair<QString, Collection>> pairs;
    
    // Тип сообщения
    pairs.append(qMakePair(QString("type"), 
                          Collection::fromNumber(static_cast<double>(type))));
    
    // ID запроса
    pairs.append(qMakePair(QString("requestId"), 
                          Collection::fromString(requestId)));
    
    // Временная метка
    pairs.append(qMakePair(QString("timestamp"), 
                          Collection::fromString(timestamp)));
    
    // Данные
    pairs.append(qMakePair(QString("data"), data));
    
    Collection messageCollection = Collection::fromCollection(pairs);
    return parser.serialize(messageCollection, 4);
}

Message Message::deserialize(const QString& data) {
    Message msg;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        // Извлекаем тип сообщения
        Collection typeValue = collection.get("type");
        if (typeValue.type() == CollectionType::Number) {
            msg.type = static_cast<MessageType>(static_cast<int>(typeValue.toNumber()));
        }
        
        // Извлекаем requestId
        Collection requestIdValue = collection.get("requestId");
        if (requestIdValue.type() == CollectionType::String) {
            msg.requestId = requestIdValue.toString();
        }
        
        // Извлекаем timestamp
        Collection timestampValue = collection.get("timestamp");
        if (timestampValue.type() == CollectionType::String) {
            msg.timestamp = timestampValue.toString();
        }
        
        // Извлекаем данные
        msg.data = collection.get("data");
    } else {
        LOG_ERROR("Message deserialize error: " + result.error.toStdString());
        msg.type = MessageType::ERROR_RESPONSE;
        msg.data = Collection::fromString(result.error);
    }
    
    return msg;
}

bool Message::isValid() const {
    return type != MessageType::ERROR_RESPONSE || !data.isEmpty();
}

// ============================================================================
// AnalysisRequest Implementation
// ============================================================================

AnalysisRequest::AnalysisRequest()
    : inferTypes(true)
    , highlightTiming(false)
    , branch("main") {
}

QString AnalysisRequest::serialize() const {
    CollectionParser parser;
    
    QVector<QPair<QString, Collection>> pairs;
    pairs.append(qMakePair("file", Collection::fromString(file)));
    pairs.append(qMakePair("infer_types", Collection::fromBoolean(inferTypes)));
    pairs.append(qMakePair("highlight_timing", Collection::fromBoolean(highlightTiming)));
    pairs.append(qMakePair("branch", Collection::fromString(branch)));
    
    Collection collection = Collection::fromCollection(pairs);
    return parser.serialize(collection, 4);
}

AnalysisRequest AnalysisRequest::deserialize(const QString& data) {
    AnalysisRequest request;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        Collection fileValue = collection.get("file");
        if (fileValue.type() == CollectionType::String) {
            request.file = fileValue.toString();
        }
        
        Collection inferTypesValue = collection.get("infer_types");
        if (inferTypesValue.type() == CollectionType::Boolean) {
            request.inferTypes = inferTypesValue.toBoolean();
        }
        
        Collection highlightTimingValue = collection.get("highlight_timing");
        if (highlightTimingValue.type() == CollectionType::Boolean) {
            request.highlightTiming = highlightTimingValue.toBoolean();
        }
        
        Collection branchValue = collection.get("branch");
        if (branchValue.type() == CollectionType::String) {
            request.branch = branchValue.toString();
        }
    }
    
    return request;
}

// ============================================================================
// AnalysisResponse Implementation
// ============================================================================

AnalysisResponse::AnalysisResponse()
    : status("error") {
}

QString AnalysisResponse::serialize() const {
    CollectionParser parser;
    
    QVector<QPair<QString, Collection>> pairs;
    pairs.append(qMakePair("status", Collection::fromString(status)));
    
    // Сериализация symbols
    QVector<Collection> symbolsArray;
    for (const SymbolInfo& symbol : symbols) {
        QVector<QPair<QString, Collection>> symbolPairs;
        symbolPairs.append(qMakePair("name", Collection::fromString(symbol.name)));
        symbolPairs.append(qMakePair("type", Collection::fromString(symbol.type)));
        symbolPairs.append(qMakePair("line", Collection::fromNumber(symbol.line)));
        if (!symbol.returnType.isEmpty()) {
            symbolPairs.append(qMakePair("return_type", Collection::fromString(symbol.returnType)));
        }
        symbolsArray.append(Collection::fromCollection(symbolPairs));
    }
    pairs.append(qMakePair("symbols", Collection::fromArray(symbolsArray)));
    
    // Сериализация warnings
    QVector<Collection> warningsArray;
    for (const QString& warning : warnings) {
        warningsArray.append(Collection::fromString(warning));
    }
    pairs.append(qMakePair("warnings", Collection::fromArray(warningsArray)));
    
    // Сериализация timingHints
    Collection timingHintsObj;
    QMap<QString, Collection> timingMap;
    if (timingHints.contains("hotspot_lines")) {
        QVector<Collection> linesArray;
        for (int line : timingHints["hotspot_lines"]) {
            linesArray.append(Collection::fromNumber(line));
        }
        timingMap["hotspot_lines"] = Collection::fromArray(linesArray);
    }
    pairs.append(qMakePair("timing_hints", Collection::fromObject(timingMap)));
    
    Collection collection = Collection::fromCollection(pairs);
    return parser.serialize(collection, 4);
}

AnalysisResponse AnalysisResponse::deserialize(const QString& data) {
    AnalysisResponse response;
    
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(data);
    
    if (result.success) {
        const auto& collection = result.value;
        
        Collection statusValue = collection.get("status");
        if (statusValue.type() == CollectionType::String) {
            response.status = statusValue.toString();
        }
        
        // Десериализация symbols
        Collection symbolsValue = collection.get("symbols");
        if (symbolsValue.type() == CollectionType::Array) {
            for (const auto& symbolItem : symbolsValue.toArray()) {
                SymbolInfo symbol;
                symbol.name = symbolItem.get("name").toString();
                symbol.type = symbolItem.get("type").toString();
                symbol.line = static_cast<int>(symbolItem.get("line").toNumber());
                symbol.returnType = symbolItem.get("return_type").toString();
                response.symbols.append(symbol);
            }
        }
        
        // Десериализация warnings
        Collection warningsValue = collection.get("warnings");
        if (warningsValue.type() == CollectionType::Array) {
            for (const auto& warningItem : warningsValue.toArray()) {
                response.warnings.append(warningItem.toString());
            }
        }
        
        // Десериализация timingHints
        Collection timingHintsValue = collection.get("timing_hints");
        if (timingHintsValue.type() == CollectionType::Object) {
            Collection hotspotsValue = timingHintsValue.get("hotspot_lines");
            if (hotspotsValue.type() == CollectionType::Array) {
                QVector<int> hotspots;
                for (const auto& lineItem : hotspotsValue.toArray()) {
                    hotspots.append(static_cast<int>(lineItem.toNumber()));
                }
                response.timingHints["hotspot_lines"] = hotspots;
            }
        }
    }
    
    return response;
}

// ============================================================================
// Protocol Implementation
// ============================================================================

Protocol& Protocol::getInstance() {
    static Protocol instance;
    return instance;
}

Protocol::Protocol()
    : server(nullptr)
    , socket(nullptr)
    , connected(false)
    , hostMode(false)
    , port(9090) {
}

Protocol::~Protocol() {
    disconnect();
}

bool Protocol::connectToCompiler(const QString& host, int port) {
    if (connected) {
        disconnect();
    }
    
    socket = new QTcpSocket(this);
    
    connect(socket, &QTcpSocket::connected, this, &Protocol::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Protocol::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &Protocol::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Protocol::onError);
    
    socket->connectToHost(host, port);
    
    if (socket->waitForConnected(5000)) {
        connected = true;
        hostMode = false;
        this->port = port;
        LOG_INFO("Connected to compiler at " + host.toStdString() + ":" + QString::number(port).toStdString());
        return true;
    }
    
    LOG_ERROR("Failed to connect to compiler");
    delete socket;
    socket = nullptr;
    return false;
}

bool Protocol::startServer(int port) {
    if (connected) {
        disconnect();
    }
    
    server = new QTcpServer(this);
    
    if (server->listen(QHostAddress::Any, port)) {
        connected = true;
        hostMode = true;
        this->port = port;
        
        connect(server, &QTcpServer::newConnection, this, &Protocol::onNewConnection);
        
        LOG_INFO("Protocol server started on port " + QString::number(port).toStdString());
        return true;
    }
    
    LOG_ERROR("Failed to start protocol server: " + server->errorString().toStdString());
    delete server;
    server = nullptr;
    return false;
}

void Protocol::disconnect() {
    if (socket) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
        delete socket;
        socket = nullptr;
    }
    
    if (server) {
        server->close();
        delete server;
        server = nullptr;
    }
    
    connected = false;
    hostMode = false;
    
    LOG_INFO("Protocol disconnected");
}

void Protocol::sendMessage(const Message& message) {
    if (!connected || !socket) {
        LOG_ERROR("Cannot send message - not connected");
        return;
    }
    
    QString serialized = message.serialize();
    QByteArray data = serialized.toUtf8();
    
    // Отправляем длину сообщения (4 байта) + данные
    quint32 length = static_cast<quint32>(data.size());
    QByteArray header;
    header.append(static_cast<char>((length >> 24) & 0xFF));
    header.append(static_cast<char>((length >> 16) & 0xFF));
    header.append(static_cast<char>((length >> 8) & 0xFF));
    header.append(static_cast<char>(length & 0xFF));
    
    socket->write(header);
    socket->write(data);
    socket->flush();
    
    LOG_DEBUG("Message sent: " + message.requestId.toStdString());
}

Message Protocol::receiveMessage() {
    if (!connected || !socket) {
        return Message();
    }
    
    // Читаем длину сообщения (4 байта)
    if (socket->bytesAvailable() < 4) {
        return Message();
    }
    
    QByteArray header = socket->read(4);
    quint32 length = 0;
    length |= (static_cast<quint8>(header[0]) << 24);
    length |= (static_cast<quint8>(header[1]) << 16);
    length |= (static_cast<quint8>(header[2]) << 8);
    length |= static_cast<quint8>(header[3]);
    
    // Ждём данные
    while (socket->bytesAvailable() < static_cast<qint64>(length)) {
        socket->waitForReadyRead(1000);
    }
    
    QByteArray data = socket->read(length);
    QString serialized = QString::fromUtf8(data);
    
    Message message = Message::deserialize(serialized);
    
    LOG_DEBUG("Message received: " + message.requestId.toStdString());
    
    return message;
}

void Protocol::sendRequest(const Message& request) {
    sendMessage(request);
}

Message Protocol::receiveResponse(int timeout) {
    QElapsedTimer timer;
    timer.start();
    
    while (timer.elapsed() < timeout) {
        if (socket && socket->bytesAvailable() >= 4) {
            Message response = receiveMessage();
            if (response.isValid()) {
                return response;
            }
        }
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    
    return Message();
}

bool Protocol::isConnected() const {
    return connected;
}

bool Protocol::isHost() const {
    return hostMode;
}

int Protocol::getPort() const {
    return port;
}

void Protocol::onConnected() {
    LOG_INFO("Protocol connection established");
    emit connectionStatusChanged(true);
}

void Protocol::onDisconnected() {
    LOG_INFO("Protocol connection lost");
    connected = false;
    emit connectionStatusChanged(false);
}

void Protocol::onReadyRead() {
    while (socket && socket->bytesAvailable() >= 4) {
        Message message = receiveMessage();
        if (message.isValid()) {
            emit messageReceived(message);
        }
    }
}

void Protocol::onError(QAbstractSocket::SocketError socketError) {
    QString errorStr;
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            errorStr = "Connection refused";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorStr = "Remote host closed";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorStr = "Host not found";
            break;
        case QAbstractSocket::SocketAccessError:
            errorStr = "Socket access error";
            break;
        case QAbstractSocket::SocketResourceError:
            errorStr = "Socket resource error";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorStr = "Socket timeout";
            break;
        case QAbstractSocket::DatagramTooLargeError:
            errorStr = "Datagram too large";
            break;
        case QAbstractSocket::NetworkError:
            errorStr = "Network error";
            break;
        case QAbstractSocket::AddressInUseError:
            errorStr = "Address in use";
            break;
        case QAbstractSocket::SocketAddressNotAvailableError:
            errorStr = "Socket address not available";
            break;
        case QAbstractSocket::UnsupportedSocketOperationError:
            errorStr = "Unsupported socket operation";
            break;
        case QAbstractSocket::UnfinishedSocketOperationError:
            errorStr = "Unfinished socket operation";
            break;
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            errorStr = "Proxy authentication required";
            break;
        case QAbstractSocket::SslHandshakeFailedError:
            errorStr = "SSL handshake failed";
            break;
        case QAbstractSocket::ProxyConnectionRefusedError:
            errorStr = "Proxy connection refused";
            break;
        case QAbstractSocket::ProxyConnectionClosedError:
            errorStr = "Proxy connection closed";
            break;
        case QAbstractSocket::ProxyConnectionTimeoutError:
            errorStr = "Proxy connection timeout";
            break;
        case QAbstractSocket::ProxyConnectionHostNotFoundError:
            errorStr = "Proxy host not found";
            break;
        case QAbstractSocket::ProxyConnectionSocketError:
            errorStr = "Proxy socket error";
            break;
        case QAbstractSocket::ProxyConnectionServiceNotFoundError:
            errorStr = "Proxy service not found";
            break;
        case QAbstractSocket::ProxyConnectionAuthenticationRequiredError:
            errorStr = "Proxy authentication required";
            break;
        default:
            errorStr = "Unknown error";
            break;
    }
    
    LOG_ERROR("Protocol error: " + errorStr.toStdString());
    emit errorOccurred(errorStr);
}

void Protocol::onNewConnection() {
    if (!server) return;
    
    QTcpSocket* clientSocket = server->nextPendingConnection();
    
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
        while (clientSocket && clientSocket->bytesAvailable() >= 4) {
            // Чтение сообщения от клиента
            QByteArray header = clientSocket->read(4);
            if (header.size() < 4) return;
            
            quint32 length = 0;
            length |= (static_cast<quint8>(header[0]) << 24);
            length |= (static_cast<quint8>(header[1]) << 16);
            length |= (static_cast<quint8>(header[2]) << 8);
            length |= static_cast<quint8>(header[3]);
            
            while (clientSocket->bytesAvailable() < static_cast<qint64>(length)) {
                clientSocket->waitForReadyRead(1000);
            }
            
            QByteArray data = clientSocket->read(length);
            QString serialized = QString::fromUtf8(data);
            
            Message message = Message::deserialize(serialized);
            if (message.isValid()) {
                emit messageReceived(message);
            }
        }
    });
    
    LOG_INFO("New client connection");
}

// ============================================================================
// Utility Functions
// ============================================================================

Collection createMessage(MessageType type, const QVector<QPair<QString, Collection>>& data) {
    Message msg(type);
    msg.data = Collection::fromCollection(data);
    return msg.data;
}

Collection createAnalysisRequest(const QString& file, bool inferTypes, 
                                 bool highlightTiming, const QString& branch) {
    QVector<QPair<QString, Collection>> pairs;
    pairs.append(qMakePair("file", Collection::fromString(file)));
    pairs.append(qMakePair("infer_types", Collection::fromBoolean(inferTypes)));
    pairs.append(qMakePair("highlight_timing", Collection::fromBoolean(highlightTiming)));
    pairs.append(qMakePair("branch", Collection::fromString(branch)));
    
    return Collection::fromCollection(pairs);
}

Collection createCompileRequest(const QString& projectPath, const CompilerConfig& config) {
    QVector<QPair<QString, Collection>> pairs;
    pairs.append(qMakePair("project_path", Collection::fromString(projectPath)));
    pairs.append(qMakePair("mode", Collection::fromNumber(static_cast<double>(config.mode))));
    pairs.append(qMakePair("verbose_level", Collection::fromNumber(config.verboseLevel)));
    pairs.append(qMakePair("max_memory", Collection::fromNumber(static_cast<double>(config.maxMemory))));
    pairs.append(qMakePair("enable_cuda", Collection::fromBoolean(config.enableCUDA)));
    pairs.append(qMakePair("enable_avx2", Collection::fromBoolean(config.enableAVX2)));
    pairs.append(qMakePair("enable_sse4", Collection::fromBoolean(config.enableSSE4)));
    pairs.append(qMakePair("debug_symbols", Collection::fromBoolean(config.debugSymbols)));
    pairs.append(qMakePair("optimization_level", Collection::fromNumber(config.optimizationLevel)));
    
    if (!config.outputPath.isEmpty()) {
        pairs.append(qMakePair("output_path", Collection::fromString(config.outputPath)));
    }
    
    return Collection::fromCollection(pairs);
}

Collection createDebugRequest(const QString& action, const QString& file, int line) {
    QVector<QPair<QString, Collection>> pairs;
    pairs.append(qMakePair("action", Collection::fromString(action)));
    
    if (!file.isEmpty()) {
        pairs.append(qMakePair("file", Collection::fromString(file)));
    }
    
    if (line > 0) {
        pairs.append(qMakePair("line", Collection::fromNumber(line)));
    }
    
    return Collection::fromCollection(pairs);
}

CompilerConfig configFromCollection(const Collection& collection) {
    CompilerConfig config;
    
    Collection modeValue = collection.get("mode");
    if (modeValue.type() == CollectionType::Number) {
        config.mode = static_cast<RunMode>(static_cast<int>(modeValue.toNumber()));
    }
    
    Collection verboseValue = collection.get("verbose_level");
    if (verboseValue.type() == CollectionType::Number) {
        config.verboseLevel = static_cast<int>(verboseValue.toNumber());
    }
    
    Collection memoryValue = collection.get("max_memory");
    if (memoryValue.type() == CollectionType::Number) {
        config.maxMemory = static_cast<size_t>(memoryValue.toNumber());
    }
    
    Collection cudaValue = collection.get("enable_cuda");
    if (cudaValue.type() == CollectionType::Boolean) {
        config.enableCUDA = cudaValue.toBoolean();
    }
    
    Collection avxValue = collection.get("enable_avx2");
    if (avxValue.type() == CollectionType::Boolean) {
        config.enableAVX2 = avxValue.toBoolean();
    }
    
    Collection sseValue = collection.get("enable_sse4");
    if (sseValue.type() == CollectionType::Boolean) {
        config.enableSSE4 = sseValue.toBoolean();
    }
    
    Collection debugValue = collection.get("debug_symbols");
    if (debugValue.type() == CollectionType::Boolean) {
        config.debugSymbols = debugValue.toBoolean();
    }
    
    Collection optValue = collection.get("optimization_level");
    if (optValue.type() == CollectionType::Number) {
        config.optimizationLevel = static_cast<int>(optValue.toNumber());
    }
    
    Collection outputValue = collection.get("output_path");
    if (outputValue.type() == CollectionType::String) {
        config.outputPath = outputValue.toString();
    }
    
    return config;
}

} // namespace proxima
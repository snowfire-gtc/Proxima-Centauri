#include "NetworkAuthService.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSslKey>
#include <QSslConfiguration>
#include <QDataStream>
#include <QRandomGenerator>
#include <QHostAddress>
#include <QUdpSocket>

NetworkAuthService::NetworkAuthService(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_discoveryTimer(new QTimer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &NetworkAuthService::onNewConnection);
    connect(m_discoveryTimer, &QTimer::timeout, this, &NetworkAuthService::onDiscoveryTimeout);
}

NetworkAuthService::~NetworkAuthService()
{
    stopDiscovery();
    m_server->close();
}

void NetworkAuthService::initialize(const QString &storagePath)
{
    m_storagePath = storagePath;
    QDir dir(m_storagePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    loadTrustStore();
    
    // Генерируем ключи для локального узла, если их нет
    if (m_localNodeId.isEmpty()) {
        registerLocalNode(QHostInfo::localHostName());
    }
}

bool NetworkAuthService::registerLocalNode(const QString &hostname)
{
    m_localNodeId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_localPrivateKey = generateKeyPair(m_localPublicKey);
    
    RemoteNodeInfo localInfo;
    localInfo.id = m_localNodeId;
    localInfo.hostname = hostname;
    localInfo.ipAddress = QHostAddress::LocalHost.toString();
    localInfo.publicKey = m_localPublicKey;
    localInfo.registeredAt = QDateTime::currentMSecsSinceEpoch();
    localInfo.isActive = true;
    
    m_trustedNodes[m_localNodeId] = localInfo;
    saveTrustStore();
    
    return true;
}

bool NetworkAuthService::addRemoteUser(const QString &accessKey, const QString &ipAddress)
{
    // Парсим ключ доступа (формат: PUBKEY-HASH-SIGNATURE)
    QStringList parts = accessKey.split('-');
    if (parts.size() != 3) {
        emit errorOccurred("Invalid access key format");
        return false;
    }
    
    QByteArray publicKey = QByteArray::fromBase64(parts[0].toUtf8());
    if (publicKey.isEmpty()) {
        emit errorOccurred("Invalid public key in access key");
        return false;
    }
    
    // Проверяем, не добавлен ли уже этот ключ
    for (auto it = m_trustedNodes.begin(); it != m_trustedNodes.end(); ++it) {
        if (it.value().publicKey == publicKey) {
            emit errorOccurred("User with this key already exists");
            return false;
        }
    }
    
    QString nodeId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    RemoteNodeInfo nodeInfo;
    nodeInfo.id = nodeId;
    nodeInfo.hostname = "Remote-" + nodeId.left(8);
    nodeInfo.ipAddress = ipAddress;
    nodeInfo.publicKey = publicKey;
    nodeInfo.registeredAt = QDateTime::currentMSecsSinceEpoch();
    nodeInfo.isActive = true;
    
    m_trustedNodes[nodeId] = nodeInfo;
    saveTrustStore();
    
    emit nodeDiscovered(nodeInfo);
    return true;
}

void NetworkAuthService::startDiscovery(quint16 port)
{
    m_discoveryTimer->start(5000); // Проверка каждые 5 секунд
    m_server->listen(QHostAddress::Any, port);
}

void NetworkAuthService::stopDiscovery()
{
    m_discoveryTimer->stop();
}

QList<RemoteNodeInfo> NetworkAuthService::trustedNodes() const
{
    return m_trustedNodes.values();
}

bool NetworkAuthService::verifyAccessKey(const QString &key) const
{
    QStringList parts = key.split('-');
    if (parts.size() != 3) {
        return false;
    }
    
    QByteArray publicKey = QByteArray::fromBase64(parts[0].toUtf8());
    if (publicKey.isEmpty()) {
        return false;
    }
    
    // Проверяем наличие в доверенных узлах
    for (auto it = m_trustedNodes.begin(); it != m_trustedNodes.end(); ++it) {
        if (it.value().publicKey == publicKey && it.value().isActive) {
            return true;
        }
    }
    
    return false;
}

QString NetworkAuthService::generateAccessKeyForSharing() const
{
    if (m_localPublicKey.isEmpty()) {
        return QString();
    }
    
    // Формируем ключ: PUBKEY-HASH-SIGNATURE
    QString pubKeyB64 = m_localPublicKey.toBase64();
    QByteArray hash = QCryptographicHash::hash(m_localPublicKey, QCryptographicHash::Sha256).toHex();
    
    // Простая подпись (в продакшене использовать асимметричную криптографию)
    QByteArray signature = QCryptographicHash::hash(m_localPublicKey + hash, QCryptographicHash::Sha256).toHex();
    
    return QString("%1-%2-%3").arg(pubKeyB64, QString(hash), QString(signature));
}

QByteArray NetworkAuthService::generateKeyPair(QByteArray &publicKey)
{
    // Генерация пары ключей RSA через QSsl
    QSslKey privateKey = QSslKey::generateKey(QSsl::Rsa, QSsl::PrivateKey, 2048);
    QSslKey pubKey = privateKey.toPublic();
    
    publicKey = pubKey.toDer();
    return privateKey.toDer();
}

QByteArray NetworkAuthService::encryptData(const QByteArray &data, const QByteArray &key)
{
    // Упрощённое шифрование (в продакшене использовать полноценное асимметричное шифрование)
    QByteArray result = data;
    for (int i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    return result.toBase64();
}

QByteArray NetworkAuthService::decryptData(const QByteArray &data, const QByteArray &key)
{
    QByteArray decoded = QByteArray::fromBase64(data);
    QByteArray result = decoded;
    for (int i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

QString NetworkAuthService::generateAccessKey(const QByteArray &publicKey)
{
    QString pubKeyB64 = publicKey.toBase64();
    QByteArray hash = QCryptographicHash::hash(publicKey, QCryptographicHash::Sha256).toHex();
    QByteArray signature = QCryptographicHash::hash(publicKey + hash, QCryptographicHash::Sha256).toHex();
    
    return QString("%1-%2-%3").arg(pubKeyB64, QString(hash), QString(signature));
}

bool NetworkAuthService::loadTrustStore()
{
    QString filePath = getStorageFilePath();
    QFile file(filePath);
    
    if (!file.exists()) {
        return true; // Пустое хранилище - это нормально
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open trust store: " + file.errorString());
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (doc.isNull()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    m_trustedNodes.clear();
    
    // Загружаем локальный ID и ключи
    m_localNodeId = root["localNodeId"].toString();
    m_localPublicKey = QByteArray::fromBase64(root["localPublicKey"].toString().toUtf8());
    m_localPrivateKey = QByteArray::fromBase64(root["localPrivateKey"].toString().toUtf8());
    
    // Загружаем доверенные узлы
    QJsonArray nodesArray = root["nodes"].toArray();
    for (const QJsonValue &value : nodesArray) {
        QJsonObject nodeObj = value.toObject();
        
        RemoteNodeInfo info;
        info.id = nodeObj["id"].toString();
        info.hostname = nodeObj["hostname"].toString();
        info.ipAddress = nodeObj["ipAddress"].toString();
        info.publicKey = QByteArray::fromBase64(nodeObj["publicKey"].toString().toUtf8());
        info.registeredAt = nodeObj["registeredAt"].toVariant().toLongLong();
        info.isActive = nodeObj["isActive"].toBool();
        
        m_trustedNodes[info.id] = info;
    }
    
    return true;
}

bool NetworkAuthService::saveTrustStore()
{
    QString filePath = getStorageFilePath();
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to save trust store: " + file.errorString());
        return false;
    }
    
    QJsonObject root;
    root["localNodeId"] = m_localNodeId;
    root["localPublicKey"] = QString(m_localPublicKey.toBase64());
    root["localPrivateKey"] = QString(m_localPrivateKey.toBase64());
    
    QJsonArray nodesArray;
    for (auto it = m_trustedNodes.begin(); it != m_trustedNodes.end(); ++it) {
        QJsonObject nodeObj;
        nodeObj["id"] = it.value().id;
        nodeObj["hostname"] = it.value().hostname;
        nodeObj["ipAddress"] = it.value().ipAddress;
        nodeObj["publicKey"] = QString(it.value().publicKey.toBase64());
        nodeObj["registeredAt"] = it.value().registeredAt;
        nodeObj["isActive"] = it.value().isActive;
        
        nodesArray.append(nodeObj);
    }
    
    root["nodes"] = nodesArray;
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

QString NetworkAuthService::getStorageFilePath() const
{
    return QDir(m_storagePath).filePath("trust_store.json");
}

void NetworkAuthService::onNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &NetworkAuthService::onSocketReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkAuthService::onSocketDisconnected);
    
    // Отправляем наш публичный ключ для рукопожатия
    sendHandshake(socket, m_localPublicKey);
}

void NetworkAuthService::onSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    m_handshakeBuffer[socket] += data;
    
    // Проверяем завершённость рукопожатия
    if (processHandshake(socket, m_handshakeBuffer[socket])) {
        m_handshakeBuffer.remove(socket);
    }
}

void NetworkAuthService::onSocketDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_handshakeBuffer.remove(socket);
        socket->deleteLater();
    }
}

void NetworkAuthService::onDiscoveryTimeout()
{
    // Периодическая проверка активных соединений
    for (auto it = m_trustedNodes.begin(); it != m_trustedNodes.end(); ++it) {
        // Здесь можно реализовать ping-проверку активности узлов
        Q_UNUSED(it);
    }
}

void NetworkAuthService::sendHandshake(QTcpSocket *socket, const QByteArray &myPublicKey)
{
    QJsonObject handshake;
    handshake["type"] = "handshake";
    handshake["publicKey"] = QString(myPublicKey.toBase64());
    handshake["nodeId"] = m_localNodeId;
    
    QJsonDocument doc(handshake);
    socket->write(doc.toJson());
    socket->flush();
}

bool NetworkAuthService::processHandshake(QTcpSocket *socket, const QByteArray &receivedData)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(receivedData, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false; // Данные ещё не полные
    }
    
    QJsonObject obj = doc.object();
    if (obj["type"].toString() != "handshake") {
        socket->disconnectFromHost();
        return true;
    }
    
    QByteArray remotePublicKey = QByteArray::fromBase64(obj["publicKey"].toString().toUtf8());
    QString remoteNodeId = obj["nodeId"].toString();
    
    // Проверяем, есть ли этот узел в доверенных
    bool isTrusted = false;
    for (auto it = m_trustedNodes.begin(); it != m_trustedNodes.end(); ++it) {
        if (it.value().publicKey == remotePublicKey) {
            isTrusted = true;
            emit newConnectionEstablished(it.value().id);
            break;
        }
    }
    
    if (!isTrusted) {
        // Узел не доверенный - отключаемся
        emit authenticationFailed(socket->peerAddress().toString(), "Untrusted node");
        socket->disconnectFromHost();
    }
    
    return true;
}

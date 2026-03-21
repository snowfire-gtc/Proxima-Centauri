#include "CollaborationService.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QTimer>
#include "utils/Logger.h"

namespace proxima {

CollaborationService::CollaborationService(QObject *parent)
    : QObject(parent)
    , server(nullptr)
    , socket(nullptr)
    , hosting(false)
    , joined(false)
    , connected(false)
    , authenticated(false)
    , currentVersion(0)
    , autoSync(true)
    , syncInterval(1000)
    , encryptionEnabled(true)
    , maxHistorySize(1000)
    , port(8080) {
    
    keepAliveTimer = new QTimer(this);
    connect(keepAliveTimer, &QTimer::timeout, this, &CollaborationService::sendKeepAlive);
    
    timeoutTimer = new QTimer(this);
    connect(timeoutTimer, &QTimer::timeout, this, &CollaborationService::checkTimeout);
}

CollaborationService::~CollaborationService() {
    leaveSession();
    logout();
}

bool CollaborationService::hostSession(const QString& projectName, int port) {
    if (hosting) {
        lastError = "Already hosting a session";
        return false;
    }
    
    server = new QTcpServer(this);
    
    if (!server->listen(QHostAddress::Any, port)) {
        lastError = "Failed to start server: " + server->errorString();
        return false;
    }
    
    connect(server, &QTcpServer::newConnection, this, &CollaborationService::onHostConnected);
    
    hosting = true;
    joined = false;
    this->port = port;
    projectName = projectName;
    sessionId = QUuid::createUuid().toString();
    currentVersion = 0;
    
    LOG_INFO("Hosting session: " + sessionId.toStdString() + " on port " + std::to_string(port));
    
    emit sessionStarted(sessionId);
    emit connectionStatusChanged(true);
    
    keepAliveTimer->start(30000); // 30 seconds
    timeoutTimer->start(60000); // 60 seconds
    
    return true;
}

bool CollaborationService::joinSession(const QString& hostAddress, int port, const QString& password) {
    if (joined) {
        lastError = "Already joined a session";
        return false;
    }
    
    socket = new QTcpSocket(this);
    
    connect(socket, &QTcpSocket::connected, this, &CollaborationService::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &CollaborationService::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &CollaborationService::onReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &CollaborationService::onError);
    
    socket->connectToHost(hostAddress, port);
    
    if (!socket->waitForConnected(5000)) {
        lastError = "Failed to connect to host: " + socket->errorString();
        return false;
    }
    
    this->hostAddress = hostAddress;
    this->port = port;
    
    // Send authentication if password provided
    if (!password.isEmpty()) {
        QJsonObject authMsg = createMessage("auth");
        authMsg["password_hash"] = encryptPassword(password);
        sendMessage(authMsg);
    }
    
    return true;
}

bool CollaborationService::leaveSession() {
    if (hosting) {
        // Notify all clients
        QJsonObject msg = createMessage("session_ended");
        broadcastMessage(msg);
        
        // Close all client connections
        for (QTcpSocket* client : clients) {
            client->disconnectFromHost();
            client->deleteLater();
        }
        clients.clear();
        
        if (server) {
            server->close();
            server->deleteLater();
            server = nullptr;
        }
        
        hosting = false;
        emit sessionEnded();
    }
    
    if (joined && socket) {
        QJsonObject msg = createMessage("leave_session");
        sendMessage(msg);
        
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(3000);
        }
        
        socket->deleteLater();
        socket = nullptr;
        
        joined = false;
        emit sessionEnded();
    }
    
    connected = false;
    emit connectionStatusChanged(false);
    
    keepAliveTimer->stop();
    timeoutTimer->stop();
    
    LOG_INFO("Left session");
    
    return true;
}

bool CollaborationService::registerUser(const QString& username, const QString& email, 
                                        const QString& password) {
    if (users.contains(username)) {
        lastError = "Username already exists";
        return false;
    }
    
    UserInfo user;
    user.id = QUuid::createUuid().toString();
    user.username = username;
    user.email = email;
    user.isOnline = false;
    user.color = generateUserColor(user.id);
    
    users[username] = user;
    passwordHashes[username] = encryptPassword(password);
    
    LOG_INFO("User registered: " + username.toStdString());
    
    return true;
}

bool CollaborationService::login(const QString& username, const QString& password) {
    if (!users.contains(username)) {
        lastError = "User not found";
        return false;
    }
    
    if (!verifyPassword(password, passwordHashes[username])) {
        lastError = "Invalid password";
        return false;
    }
    
    currentUser = users[username];
    currentUser.isOnline = true;
    currentUser.lastSeen = QDateTime::currentDateTime().toString(Qt::ISODate);
    authenticated = true;
    
    LOG_INFO("User logged in: " + username.toStdString());
    
    emit authenticationStatusChanged(true);
    
    return true;
}

bool CollaborationService::logout() {
    if (authenticated) {
        currentUser.isOnline = false;
        currentUser.lastSeen = QDateTime::currentDateTime().toString(Qt::ISODate);
        authenticated = false;
        
        emit authenticationStatusChanged(false);
    }
    
    return true;
}

void CollaborationService::sendChange(const DocumentChange& change) {
    currentVersion++;
    
    DocumentChange versionedChange = change;
    versionedChange.version = currentVersion;
    versionedChange.timestamp = QDateTime::currentMSecsSinceEpoch();
    versionedChange.userId = currentUser.id;
    
    changeHistory.append(versionedChange);
    
    // Limit history size
    if (changeHistory.size() > maxHistorySize) {
        changeHistory.removeFirst();
    }
    
    // Track per-file changes
    if (!fileChanges.contains(change.file)) {
        fileChanges[change.file] = QVector<DocumentChange>();
    }
    fileChanges[change.file].append(versionedChange);
    
    // Notify participants
    QJsonObject msg = createMessage("change");
    msg["change_id"] = change.id;
    msg["user_id"] = currentUser.id;
    msg["file"] = change.file;
    msg["start_line"] = change.startLine;
    msg["end_line"] = change.endLine;
    msg["old_content"] = change.oldContent;
    msg["new_content"] = change.newContent;
    msg["version"] = currentVersion;
    msg["timestamp"] = versionedChange.timestamp;
    
    if (hosting) {
        broadcastMessage(msg);
    } else if (socket) {
        sendMessage(msg);
    }
    
    emit versionUpdated(currentVersion);
}

void CollaborationService::requestFullSync(const QString& file) {
    QJsonObject msg = createMessage("sync_request");
    msg["file"] = file;
    msg["version"] = currentVersion;
    
    if (hosting) {
        // Send full file content to requester
        // Implementation would send complete file
    } else if (socket) {
        sendMessage(msg);
    }
}

void CollaborationService::requestVersion(int version) {
    QJsonObject msg = createMessage("version_request");
    msg["version"] = version;
    
    sendMessage(msg);
}

void CollaborationService::acknowledgeVersion(int version) {
    QJsonObject msg = createMessage("version_ack");
    msg["version"] = version;
    
    sendMessage(msg);
}

QVector<UserInfo> CollaborationService::getParticipants() const {
    return participants;
}

UserInfo CollaborationService::getUserInfo(const QString& userId) const {
    for (const UserInfo& user : participants) {
        if (user.id == userId) {
            return user;
        }
    }
    return UserInfo();
}

void CollaborationService::setUserColor(const QString& color) {
    currentUser.color = color;
    
    QJsonObject msg = createMessage("user_info");
    msg["user_id"] = currentUser.id;
    msg["color"] = color;
    
    broadcastMessage(msg);
}

void CollaborationService::openFile(const QString& file) {
    QJsonObject msg = createMessage("file_open");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    
    broadcastMessage(msg);
}

void CollaborationService::closeFile(const QString& file) {
    QJsonObject msg = createMessage("file_close");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    
    broadcastMessage(msg);
}

void CollaborationService::saveFile(const QString& file) {
    QJsonObject msg = createMessage("file_save");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    msg["version"] = currentVersion;
    
    broadcastMessage(msg);
}

void CollaborationService::lockFile(const QString& file) {
    if (fileLocks.contains(file)) {
        lastError = "File already locked by " + fileLocks[file];
        return;
    }
    
    fileLocks[file] = currentUser.id;
    
    QJsonObject msg = createMessage("file_lock");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    
    broadcastMessage(msg);
    
    emit fileLocked(file, currentUser.id);
}

void CollaborationService::unlockFile(const QString& file) {
    if (!fileLocks.contains(file) || fileLocks[file] != currentUser.id) {
        lastError = "File not locked by current user";
        return;
    }
    
    fileLocks.remove(file);
    
    QJsonObject msg = createMessage("file_unlock");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    
    broadcastMessage(msg);
    
    emit fileUnlocked(file);
}

bool CollaborationService::isFileLocked(const QString& file) const {
    return fileLocks.contains(file);
}

QString CollaborationService::getFileLocker(const QString& file) const {
    return fileLocks.value(file, "");
}

void CollaborationService::updateCursor(const QString& file, int line, int column) {
    if (!remoteCursors.contains(file)) {
        remoteCursors[file] = QMap<QString, QVector<int>>();
    }
    
    remoteCursors[file][currentUser.id] = QVector<int>() << line << column;
    
    QJsonObject msg = createMessage("cursor");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    msg["line"] = line;
    msg["column"] = column;
    
    broadcastMessage(msg);
}

void CollaborationService::updateSelection(const QString& file, int startLine, int startCol,
                                           int endLine, int endCol) {
    if (!remoteSelections.contains(file)) {
        remoteSelections[file] = QMap<QString, QVector<int>>();
    }
    
    remoteSelections[file][currentUser.id] = QVector<int>() << startLine << startCol << endLine << endCol;
    
    QJsonObject msg = createMessage("selection");
    msg["file"] = file;
    msg["user_id"] = currentUser.id;
    msg["start_line"] = startLine;
    msg["start_col"] = startCol;
    msg["end_line"] = endLine;
    msg["end_col"] = endCol;
    
    broadcastMessage(msg);
}

QMap<QString, QVector<int>> CollaborationService::getRemoteCursors(const QString& file) const {
    return remoteCursors.value(file);
}

QVector<DocumentChange> CollaborationService::getChangeHistory(const QString& file) const {
    return fileChanges.value(file);
}

bool CollaborationService::rollbackToVersion(const QString& file, int version) {
    // Find the change at that version
    for (const DocumentChange& change : changeHistory) {
        if (change.file == file && change.version == version) {
            // Send rollback command
            QJsonObject msg = createMessage("rollback");
            msg["file"] = file;
            msg["to_version"] = version;
            msg["scope"] = "full";
            
            broadcastMessage(msg);
            
            return true;
        }
    }
    
    lastError = "Version not found";
    return false;
}

bool CollaborationService::rollbackChanges(const QString& file, int changeId) {
    // Find the specific change
    for (const DocumentChange& change : changeHistory) {
        if (change.id == changeId) {
            // Send rollback command
            QJsonObject msg = createMessage("rollback");
            msg["file"] = file;
            msg["change_id"] = changeId;
            msg["scope"] = "partial";
            
            broadcastMessage(msg);
            
            return true;
        }
    }
    
    lastError = "Change not found";
    return false;
}

bool CollaborationService::rollbackAll(const QString& userId) {
    // Rollback all changes by a specific user
    QJsonObject msg = createMessage("rollback");
    msg["user_id"] = userId;
    msg["scope"] = "all";
    
    broadcastMessage(msg);
    
    return true;
}

void CollaborationService::setAutoSync(bool enable) {
    autoSync = enable;
}

void CollaborationService::setSyncInterval(int ms) {
    syncInterval = ms;
}

void CollaborationService::setMaxHistory(int count) {
    maxHistorySize = count;
}

void CollaborationService::setEncryptionEnabled(bool enable) {
    encryptionEnabled = enable;
}

void CollaborationService::setupServer() {
    // Server setup handled in hostSession
}

void CollaborationService::setupClient() {
    // Client setup handled in joinSession
}

void CollaborationService::sendMessage(const QJsonObject& message) {
    if (!socket || !connected) return;
    
    QJsonDocument doc(message);
    socket->write(doc.toJson());
    socket->write("\n");
}

void CollaborationService::broadcastMessage(const QJsonObject& message) {
    if (!hosting) return;
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson();
    
    for (QTcpSocket* client : clients) {
        if (client->state() == QTcpSocket::ConnectedState) {
            client->write(data);
            client->write("\n");
        }
    }
}

void CollaborationService::processMessage(const QJsonObject& message) {
    QString type = message["type"].toString();
    
    if (type == "change") {
        handleChange(message);
    } else if (type == "sync_request") {
        handleSyncRequest(message);
    } else if (type == "cursor") {
        handleCursor(message);
    } else if (type == "selection") {
        handleSelection(message);
    } else if (type == "file_lock") {
        handleLock(message);
    } else if (type == "auth") {
        handleAuthentication(message);
    } else if (type == "user_joined") {
        UserInfo user;
        user.id = message["user_id"].toString();
        user.username = message["username"].toString();
        user.color = message["color"].toString();
        user.isOnline = true;
        
        participants.append(user);
        emit participantJoined(user);
    } else if (type == "user_left") {
        QString userId = message["user_id"].toString();
        for (int i = 0; i < participants.size(); i++) {
            if (participants[i].id == userId) {
                UserInfo user = participants.takeAt(i);
                user.isOnline = false;
                emit participantLeft(user);
                break;
            }
        }
    }
}

void CollaborationService::handleSyncRequest(const QJsonObject& message) {
    QString file = message["file"].toString();
    int version = message["version"].toInt();
    
    // Send full sync or delta
    QJsonObject response = createMessage("sync");
    response["file"] = file;
    response["version"] = currentVersion;
    // response["content"] = getFileContent(file);
    
    sendMessage(response);
}

void CollaborationService::handleChange(const QJsonObject& message) {
    DocumentChange change;
    change.id = message["change_id"].toString();
    change.userId = message["user_id"].toString();
    change.file = message["file"].toString();
    change.startLine = message["start_line"].toInt();
    change.endLine = message["end_line"].toInt();
    change.oldContent = message["old_content"].toString();
    change.newContent = message["new_content"].toString();
    change.version = message["version"].toInt();
    change.timestamp = message["timestamp"].toDouble();
    
    changeHistory.append(change);
    
    if (!fileChanges.contains(change.file)) {
        fileChanges[change.file] = QVector<DocumentChange>();
    }
    fileChanges[change.file].append(change);
    
    emit changeReceived(change);
}

void CollaborationService::handleCursor(const QJsonObject& message) {
    QString userId = message["user_id"].toString();
    QString file = message["file"].toString();
    int line = message["line"].toInt();
    int column = message["column"].toInt();
    
    if (!remoteCursors.contains(file)) {
        remoteCursors[file] = QMap<QString, QVector<int>>();
    }
    remoteCursors[file][userId] = QVector<int>() << line << column;
    
    emit cursorUpdated(userId, file, line, column);
}

void CollaborationService::handleSelection(const QJsonObject& message) {
    QString userId = message["user_id"].toString();
    QString file = message["file"].toString();
    int startLine = message["start_line"].toInt();
    int startCol = message["start_col"].toInt();
    int endLine = message["end_line"].toInt();
    int endCol = message["end_col"].toInt();
    
    if (!remoteSelections.contains(file)) {
        remoteSelections[file] = QMap<QString, QVector<int>>();
    }
    remoteSelections[file][userId] = QVector<int>() << startLine << startCol << endLine << endCol;
    
    emit selectionUpdated(userId, file, startLine, startCol, endLine, endCol);
}

void CollaborationService::handleLock(const QJsonObject& message) {
    QString file = message["file"].toString();
    QString userId = message["user_id"].toString();
    
    fileLocks[file] = userId;
    
    UserInfo user = getUserInfo(userId);
    emit fileLocked(file, user.username);
}

void CollaborationService::handleAuthentication(const QJsonObject& message) {
    // Handle authentication for hosted sessions
    QString passwordHash = message["password_hash"].toString();
    
    // Verify and respond
    QJsonObject response = createMessage("auth_response");
    response["success"] = true; // Simplified
    
    sendMessage(response);
}

QString CollaborationService::encryptPassword(const QString& password) const {
    if (!encryptionEnabled) {
        return password;
    }
    
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromUtf8(hash.toHex());
}

bool CollaborationService::verifyPassword(const QString& password, const QString& hash) const {
    return encryptPassword(password) == hash;
}

QJsonObject CollaborationService::createMessage(const QString& type) {
    QJsonObject msg;
    msg["type"] = type;
    msg["session_id"] = sessionId;
    msg["user_id"] = currentUser.id;
    msg["username"] = currentUser.username;
    msg["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    return msg;
}

void CollaborationService::sendKeepAlive() {
    QJsonObject msg = createMessage("keepalive");
    
    if (hosting) {
        broadcastMessage(msg);
    } else if (socket) {
        sendMessage(msg);
    }
}

void CollaborationService::checkTimeout() {
    // Check for timed out connections
    // Implementation would remove inactive participants
}

QString CollaborationService::generateUserColor(const QString& userId) const {
    // Generate a consistent color based on user ID
    int hash = qHash(userId);
    int r = (hash >> 16) & 0xFF;
    int g = (hash >> 8) & 0xFF;
    int b = hash & 0xFF;
    
    // Ensure good visibility
    r = 100 + (r % 155);
    g = 100 + (g % 155);
    b = 100 + (b % 155);
    
    return QString("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0'));
}

// Socket event handlers
void CollaborationService::onConnected() {
    connected = true;
    joined = true;
    
    LOG_INFO("Connected to collaboration session");
    
    emit connectionStatusChanged(true);
    
    keepAliveTimer->start(30000);
    timeoutTimer->start(60000);
}

void CollaborationService::onDisconnected() {
    connected = false;
    joined = false;
    
    LOG_INFO("Disconnected from collaboration session");
    
    emit connectionStatusChanged(false);
    emit sessionEnded();
    
    keepAliveTimer->stop();
    timeoutTimer->stop();
}

void CollaborationService::onReadyRead() {
    if (!socket) return;
    
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        
        if (error.error == QJsonParseError::NoError) {
            processMessage(doc.object());
        } else {
            LOG_ERROR("JSON parse error: " + error.errorString().toStdString());
        }
    }
}

void CollaborationService::onError(QAbstractSocket::SocketError socketError) {
    lastError = socket->errorString();
    LOG_ERROR("Socket error: " + lastError.toStdString());
    emit errorOccurred(lastError);
}

void CollaborationService::onHostConnected() {
    QTcpSocket* client = server->nextPendingConnection();
    clients.append(client);
    
    connect(client, &QTcpSocket::readyRead, this, &CollaborationService::onHostReadyRead);
    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        clients.removeOne(client);
        client->deleteLater();
        
        // Notify other participants
        QJsonObject msg = createMessage("user_left");
        // msg["user_id"] = clientId;
        broadcastMessage(msg);
    });
    
    // Send welcome message
    QJsonObject welcome = createMessage("welcome");
    welcome["session_id"] = sessionId;
    welcome["version"] = currentVersion;
    
    QJsonDocument doc(welcome);
    client->write(doc.toJson());
    client->write("\n");
}

void CollaborationService::onHostReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;
    
    while (client->canReadLine()) {
        QByteArray line = client->readLine();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        
        if (error.error == QJsonParseError::NoError) {
            // Broadcast to all other clients
            QJsonObject msg = doc.object();
            
            QJsonDocument outDoc(msg);
            QByteArray data = outDoc.toJson();
            
            for (QTcpSocket* otherClient : clients) {
                if (otherClient != client && otherClient->state() == QTcpSocket::ConnectedState) {
                    otherClient->write(data);
                    otherClient->write("\n");
                }
            }
            
            // Also process locally
            processMessage(msg);
        }
    }
}

} // namespace proxima
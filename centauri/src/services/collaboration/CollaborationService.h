#ifndef CENTAURI_COLLABORATIONSERVICE_H
#define CENTAURI_COLLABORATIONSERVICE_H

#include <QObject>
#include <QTcpSocket>
#include <QMap>
#include <QVector>
#include <QCryptographicHash>

namespace proxima {

struct UserInfo {
    QString id;
    QString username;
    QString email;
    QString color;
    bool isOnline;
    QString lastSeen;
};

struct DocumentChange {
    int id;
    QString userId;
    QString file;
    int startLine;
    int endLine;
    QString oldContent;
    QString newContent;
    qint64 timestamp;
    int version;
};

struct CollaborationSession {
    QString sessionId;
    QString hostAddress;
    int port;
    QString projectName;
    QVector<UserInfo> participants;
    qint64 startTime;
    int version;
};

class CollaborationService : public QObject {
    Q_OBJECT
    
public:
    explicit CollaborationService(QObject *parent = nullptr);
    ~CollaborationService();
    
    // Session management
    bool hostSession(const QString& projectName, int port = 8080);
    bool joinSession(const QString& hostAddress, int port, const QString& password);
    bool leaveSession();
    bool isHosting() const { return hosting; }
    bool isJoined() const { return joined; }
    QString getSessionId() const { return sessionId; }
    
    // Authentication
    bool registerUser(const QString& username, const QString& email, 
                     const QString& password);
    bool login(const QString& username, const QString& password);
    bool logout();
    bool isAuthenticated() const { return authenticated; }
    UserInfo getCurrentUser() const { return currentUser; }
    
    // Document synchronization
    void sendChange(const DocumentChange& change);
    void requestFullSync(const QString& file);
    void requestVersion(int version);
    void acknowledgeVersion(int version);
    
    // Participants
    QVector<UserInfo> getParticipants() const { return participants; }
    UserInfo getUserInfo(const QString& userId) const;
    void setUserColor(const QString& color);
    
    // File operations
    void openFile(const QString& file);
    void closeFile(const QString& file);
    void saveFile(const QString& file);
    void lockFile(const QString& file);
    void unlockFile(const QString& file);
    bool isFileLocked(const QString& file) const;
    QString getFileLocker(const QString& file) const;
    
    // Cursors and selections
    void updateCursor(const QString& file, int line, int column);
    void updateSelection(const QString& file, int startLine, int startCol,
                        int endLine, int endCol);
    QMap<QString, QVector<int>> getRemoteCursors(const QString& file) const;
    
    // History and rollback
    QVector<DocumentChange> getChangeHistory(const QString& file) const;
    bool rollbackToVersion(const QString& file, int version);
    bool rollbackChanges(const QString& file, int changeId);
    bool rollbackAll(const QString& userId);
    
    // Configuration
    void setAutoSync(bool enable);
    void setSyncInterval(int ms);
    void setMaxHistory(int count);
    void setEncryptionEnabled(bool enable);
    
    // Status
    bool isConnected() const { return connected; }
    QString getLastError() const { return lastError; }
    int getVersion() const { return currentVersion; }
    
signals:
    void sessionStarted(const QString& sessionId);
    void sessionJoined(const QString& sessionId);
    void sessionEnded();
    void participantJoined(const UserInfo& user);
    void participantLeft(const UserInfo& user);
    void changeReceived(const DocumentChange& change);
    void syncReceived(const QString& file, const QString& content);
    void cursorUpdated(const QString& userId, const QString& file, 
                      int line, int column);
    void selectionUpdated(const QString& userId, const QString& file,
                         int startLine, int startCol, int endLine, int endCol);
    void fileLocked(const QString& file, const QString& userId);
    void fileUnlocked(const QString& file);
    void versionUpdated(int version);
    void errorOccurred(const QString& error);
    void connectionStatusChanged(bool connected);
    void authenticationStatusChanged(bool authenticated);
    
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void onHostConnected();
    void onHostReadyRead();
    
private:
    void setupServer();
    void setupClient();
    void sendMessage(const QJsonObject& message);
    void broadcastMessage(const QJsonObject& message);
    void processMessage(const QJsonObject& message);
    void handleSyncRequest(const QJsonObject& message);
    void handleChange(const QJsonObject& message);
    void handleCursor(const QJsonObject& message);
    void handleSelection(const QJsonObject& message);
    void handleLock(const QJsonObject& message);
    void handleAuthentication(const QJsonObject& message);
    
    QString encryptPassword(const QString& password) const;
    bool verifyPassword(const QString& password, const QString& hash) const;
    QJsonObject createMessage(const QString& type);
    void sendKeepAlive();
    void checkTimeout();
    
    // Server
    QTcpServer* server;
    QVector<QTcpSocket*> clients;
    bool hosting;
    
    // Client
    QTcpSocket* socket;
    bool joined;
    bool connected;
    
    // Authentication
    bool authenticated;
    UserInfo currentUser;
    QMap<QString, UserInfo> users;
    QMap<QString, QString> passwordHashes;
    
    // Session
    QString sessionId;
    QString projectName;
    QVector<UserInfo> participants;
    int currentVersion;
    
    // Changes
    QVector<DocumentChange> changeHistory;
    QMap<QString, QVector<DocumentChange>> fileChanges;
    int maxHistorySize;
    
    // File locks
    QMap<QString, QString> fileLocks;
    
    // Cursors
    QMap<QString, QMap<QString, QVector<int>>> remoteCursors;
    QMap<QString, QMap<QString, QVector<int>>> remoteSelections;
    
    // Configuration
    bool autoSync;
    int syncInterval;
    bool encryptionEnabled;
    
    // Timers
    QTimer* keepAliveTimer;
    QTimer* timeoutTimer;
    
    QString lastError;
    QString hostAddress;
    int port;
};

} // namespace proxima

#endif // CENTAURI_COLLABORATIONSERVICE_H
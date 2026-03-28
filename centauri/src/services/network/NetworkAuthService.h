#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QUuid>
#include <QByteArray>
#include <QSslSocket>
#include <QTcpServer>
#include <QTimer>
#include <QMap>

/**
 * @brief Структура, описывающая зарегистрированного удалённого пользователя/узел.
 */
struct RemoteNodeInfo {
    QString id;             // Уникальный идентификатор узла (UUID)
    QString hostname;       // Имя хоста
    QString ipAddress;      // IP-адрес в локальной сети
    QByteArray publicKey;   // Публичный ключ для шифрования
    qint64 registeredAt;    // Время регистрации
    bool isActive;          // Статус активности
};

/**
 * @brief Сервис аутентификации и управления доступом в локальной сети.
 * 
 * Отвечает за регистрацию новых узлов, генерацию ключей доступа,
 * безопасное хранение учетных данных и установление защищённых соединений.
 */
class NetworkAuthService : public QObject
{
    Q_OBJECT
public:
    explicit NetworkAuthService(QObject *parent = nullptr);
    ~NetworkAuthService();

    // Инициализация сервиса
    void initialize(const QString &storagePath);

    // Регистрация текущего узла в сети (генерация ключей)
    bool registerLocalNode(const QString &hostname);

    // Добавление удалённого пользователя по ключу доступа (ручная регистрация)
    bool addRemoteUser(const QString &accessKey, const QString &ipAddress);

    // Запрос на регистрацию от удалённого узла (автоматическое обнаружение)
    void startDiscovery(quint16 port = 53530);
    void stopDiscovery();

    // Получение списка доверенных узлов
    QList<RemoteNodeInfo> trustedNodes() const;

    // Проверка валидности ключа
    bool verifyAccessKey(const QString &key) const;

    // Экспорт ключа доступа для передачи другому пользователю
    QString generateAccessKeyForSharing() const;

signals:
    void nodeDiscovered(const RemoteNodeInfo &node);
    void authenticationFailed(const QString &ip, const QString &reason);
    void newConnectionEstablished(const QString &nodeId);
    void errorOccurred(const QString &message);

private slots:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onDiscoveryTimeout();

private:
    // Криптография
    QByteArray generateKeyPair(QByteArray &publicKey);
    QByteArray encryptData(const QByteArray &data, const QByteArray &key);
    QByteArray decryptData(const QByteArray &data, const QByteArray &key);
    QString generateAccessKey(const QByteArray &publicKey);

    // Хранение
    bool loadTrustStore();
    bool saveTrustStore();
    QString getStorageFilePath() const;

    // Сеть
    void sendHandshake(QTcpSocket *socket, const QByteArray &myPublicKey);
    bool processHandshake(QTcpSocket *socket, const QByteArray &receivedData);

    QMap<QString, RemoteNodeInfo> m_trustedNodes;
    QString m_localNodeId;
    QByteArray m_localPrivateKey;
    QByteArray m_localPublicKey;
    
    QTcpServer *m_server;
    QTimer *m_discoveryTimer;
    QString m_storagePath;
    
    // Временное хранилище для процесса рукопожатия
    QMap<QTcpSocket*, QByteArray> m_handshakeBuffer;
};

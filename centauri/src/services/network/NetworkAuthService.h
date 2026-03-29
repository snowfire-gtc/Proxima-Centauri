#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <functional>

/**
 * @brief Структура, описывающая зарегистрированного удалённого пользователя/узел.
 */
struct RemoteNodeInfo {
    std::string id;             // Уникальный идентификатор узла (UUID)
    std::string hostname;       // Имя хоста
    std::string ipAddress;      // IP-адрес в локальной сети
    std::vector<uint8_t> publicKey;   // Публичный ключ для шифрования
    int64_t registeredAt;    // Время регистрации
    bool isActive;          // Статус активности
};

// Типы callback'ов для замены сигналов Qt
using NodeDiscoveredCallback = std::function<void(const RemoteNodeInfo&)>;
using AuthenticationFailedCallback = std::function<void(const std::string&, const std::string&)>;
using NewConnectionCallback = std::function<void(const std::string&)>;
using ErrorCallback = std::function<void(const std::string&)>;

/**
 * @brief Сервис аутентификации и управления доступом в локальной сети.
 * 
 * Отвечает за регистрацию новых узлов, генерацию ключей доступа,
 * безопасное хранение учетных данных и установление защищённых соединений.
 */
class NetworkAuthService
{
public:
    NetworkAuthService();
    ~NetworkAuthService();

    // Callback'и для замены сигналов Qt
    void setNodeDiscoveredCallback(NodeDiscoveredCallback callback);
    void setAuthenticationFailedCallback(AuthenticationFailedCallback callback);
    void setNewConnectionCallback(NewConnectionCallback callback);
    void setErrorCallback(ErrorCallback callback);

    // Инициализация сервиса
    void initialize(const std::string &storagePath);

    // Регистрация текущего узла в сети (генерация ключей)
    bool registerLocalNode(const std::string &hostname);

    // Добавление удалённого пользователя по ключу доступа (ручная регистрация)
    bool addRemoteUser(const std::string &accessKey, const std::string &ipAddress);

    // Запрос на регистрацию от удалённого узла (автоматическое обнаружение)
    void startDiscovery(uint16_t port = 53530);
    void stopDiscovery();

    // Получение списка доверенных узлов
    std::vector<RemoteNodeInfo> trustedNodes() const;

    // Проверка валидности ключа
    bool verifyAccessKey(const std::string &key) const;

    // Экспорт ключа доступа для передачи другому пользователю
    std::string generateAccessKeyForSharing() const;

private:
    // Криптография
    std::vector<uint8_t> generateKeyPair(std::vector<uint8_t> &publicKey);
    std::vector<uint8_t> encryptData(const std::vector<uint8_t> &data, const std::vector<uint8_t> &key);
    std::vector<uint8_t> decryptData(const std::vector<uint8_t> &data, const std::vector<uint8_t> &key);
    std::string generateAccessKey(const std::vector<uint8_t> &publicKey);

    // Хранение
    bool loadTrustStore();
    bool saveTrustStore();
    std::string getStorageFilePath() const;

    // Сеть
    void sendHandshake(void *socket, const std::vector<uint8_t> &myPublicKey);
    bool processHandshake(void *socket, const std::vector<uint8_t> &receivedData);

    // Обработчики событий (замена слотов Qt)
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onDiscoveryTimeout();

    std::map<std::string, RemoteNodeInfo> m_trustedNodes;
    std::string m_localNodeId;
    std::vector<uint8_t> m_localPrivateKey;
    std::vector<uint8_t> m_localPublicKey;
    
    void* m_server;           // Заглушка для QTcpServer
    void* m_discoveryTimer;   // Заглушка для QTimer
    std::string m_storagePath;
    
    // Временное хранилище для процесса рукопожатия
    std::map<void*, std::vector<uint8_t>> m_handshakeBuffer;

    // Callback'и
    NodeDiscoveredCallback m_onNodeDiscovered;
    AuthenticationFailedCallback m_onAuthenticationFailed;
    NewConnectionCallback m_onNewConnection;
    ErrorCallback m_onError;
};

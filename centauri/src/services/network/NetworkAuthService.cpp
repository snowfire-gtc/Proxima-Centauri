#include "NetworkAuthService.h"
#include <cstring>
#include <sstream>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// Вспомогательные функции для кодирования/декодирования Base64
static std::string base64_encode(const std::vector<uint8_t>& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    for (size_t i = 0; i < data.size(); i += 3) {
        uint32_t n = (data[i] << 16) + 
                     ((i + 1 < data.size() ? data[i + 1] : 0) << 8) + 
                     (i + 2 < data.size() ? data[i + 2] : 0);
        result += chars[(n >> 18) & 0x3F];
        result += chars[(n >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? chars[n & 0x3F] : '=';
    }
    return result;
}

static std::vector<uint8_t> base64_decode(const std::string& encoded) {
    static const int chars[] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
    };
    std::vector<uint8_t> result;
    for (size_t i = 0; i < encoded.size(); i += 4) {
        uint32_t n = (chars[encoded[i]] << 18) + 
                     (chars[encoded[i + 1]] << 12) + 
                     (chars[encoded[i + 2]] << 6) + 
                     chars[encoded[i + 3]];
        result.push_back((n >> 16) & 0xFF);
        if (encoded[i + 2] != '=') result.push_back((n >> 8) & 0xFF);
        if (encoded[i + 3] != '=') result.push_back(n & 0xFF);
    }
    return result;
}

// Простая хеш-функция (замена QCryptographicHash)
static std::vector<uint8_t> sha256_hash(const std::vector<uint8_t>& data) {
    // Упрощённая заглушка - в продакшене использовать OpenSSL или аналог
    std::vector<uint8_t> hash(32, 0);
    for (size_t i = 0; i < data.size() && i < 32; ++i) {
        hash[i] = data[i] ^ (data.size() & 0xFF);
    }
    return hash;
}

static std::string to_hex(const std::vector<uint8_t>& data) {
    static const char* hex_chars = "0123456789abcdef";
    std::string result;
    result.reserve(data.size() * 2);
    for (uint8_t byte : data) {
        result += hex_chars[(byte >> 4) & 0x0F];
        result += hex_chars[byte & 0x0F];
    }
    return result;
}

// Генерация UUID (замена QUuid)
static std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    
    uint64_t a = dis(gen);
    uint64_t b = dis(gen);
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    oss << std::setw(8) << (a >> 32) << '-';
    oss << std::setw(4) << ((a >> 16) & 0xFFFF) << '-';
    oss << std::setw(4) << (a & 0xFFFF) << '-';
    oss << std::setw(4) << (b >> 48) << '-';
    oss << std::setw(12) << (b & 0xFFFFFFFFFFFF);
    
    return oss.str();
}

NetworkAuthService::NetworkAuthService()
    : m_server(nullptr)
    , m_discoveryTimer(nullptr)
{
}

NetworkAuthService::~NetworkAuthService()
{
    stopDiscovery();
}

void NetworkAuthService::setNodeDiscoveredCallback(NodeDiscoveredCallback callback) {
    m_onNodeDiscovered = callback;
}

void NetworkAuthService::setAuthenticationFailedCallback(AuthenticationFailedCallback callback) {
    m_onAuthenticationFailed = callback;
}

void NetworkAuthService::setNewConnectionCallback(NewConnectionCallback callback) {
    m_onNewConnection = callback;
}

void NetworkAuthService::setErrorCallback(ErrorCallback callback) {
    m_onError = callback;
}

void NetworkAuthService::initialize(const std::string &storagePath)
{
    m_storagePath = storagePath;
    fs::create_directories(m_storagePath);
    
    loadTrustStore();
    
    // Генерируем ключи для локального узла, если их нет
    if (m_localNodeId.empty()) {
        // Получаем имя хоста
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        registerLocalNode(std::string(hostname));
    }
}

bool NetworkAuthService::registerLocalNode(const std::string &hostname)
{
    m_localNodeId = generate_uuid();
    m_localPrivateKey = generateKeyPair(m_localPublicKey);
    
    RemoteNodeInfo localInfo;
    localInfo.id = m_localNodeId;
    localInfo.hostname = hostname;
    localInfo.ipAddress = "127.0.0.1";
    localInfo.publicKey = m_localPublicKey;
    localInfo.registeredAt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    localInfo.isActive = true;
    
    m_trustedNodes[m_localNodeId] = localInfo;
    saveTrustStore();
    
    return true;
}

bool NetworkAuthService::addRemoteUser(const std::string &accessKey, const std::string &ipAddress)
{
    // Парсим ключ доступа (формат: PUBKEY-HASH-SIGNATURE)
    size_t pos1 = accessKey.find('-');
    size_t pos2 = accessKey.find('-', pos1 + 1);
    
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        if (m_onError) m_onError("Invalid access key format");
        return false;
    }
    
    std::string pubKeyStr = accessKey.substr(0, pos1);
    std::vector<uint8_t> publicKey = base64_decode(pubKeyStr);
    
    if (publicKey.empty()) {
        if (m_onError) m_onError("Invalid public key in access key");
        return false;
    }
    
    // Проверяем, не добавлен ли уже этот ключ
    for (const auto& pair : m_trustedNodes) {
        if (pair.second.publicKey == publicKey) {
            if (m_onError) m_onError("User with this key already exists");
            return false;
        }
    }
    
    std::string nodeId = generate_uuid();
    
    RemoteNodeInfo nodeInfo;
    nodeInfo.id = nodeId;
    nodeInfo.hostname = "Remote-" + nodeId.substr(0, 8);
    nodeInfo.ipAddress = ipAddress;
    nodeInfo.publicKey = publicKey;
    nodeInfo.registeredAt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    nodeInfo.isActive = true;
    
    m_trustedNodes[nodeId] = nodeInfo;
    saveTrustStore();
    
    if (m_onNodeDiscovered) m_onNodeDiscovered(nodeInfo);
    return true;
}

void NetworkAuthService::startDiscovery(uint16_t port)
{
    // Заглушка - в продакшене реализовать сетевой сервер
    Q_UNUSED(port);
}

void NetworkAuthService::stopDiscovery()
{
    // Заглушка
}

std::vector<RemoteNodeInfo> NetworkAuthService::trustedNodes() const
{
    std::vector<RemoteNodeInfo> result;
    for (const auto& pair : m_trustedNodes) {
        result.push_back(pair.second);
    }
    return result;
}

bool NetworkAuthService::verifyAccessKey(const std::string &key) const
{
    size_t pos1 = key.find('-');
    size_t pos2 = key.find('-', pos1 + 1);
    
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return false;
    }
    
    std::string pubKeyStr = key.substr(0, pos1);
    std::vector<uint8_t> publicKey = base64_decode(pubKeyStr);
    
    if (publicKey.empty()) {
        return false;
    }
    
    // Проверяем наличие в доверенных узлах
    for (const auto& pair : m_trustedNodes) {
        if (pair.second.publicKey == publicKey && pair.second.isActive) {
            return true;
        }
    }
    
    return false;
}

std::string NetworkAuthService::generateAccessKeyForSharing() const
{
    if (m_localPublicKey.empty()) {
        return std::string();
    }
    
    // Формируем ключ: PUBKEY-HASH-SIGNATURE
    std::string pubKeyB64 = base64_encode(m_localPublicKey);
    std::vector<uint8_t> hash = sha256_hash(m_localPublicKey);
    std::string hashHex = to_hex(hash);
    
    // Простая подпись (в продакшене использовать асимметричную криптографию)
    std::vector<uint8_t> sigData = m_localPublicKey;
    sigData.insert(sigData.end(), hash.begin(), hash.end());
    std::vector<uint8_t> signature = sha256_hash(sigData);
    std::string sigHex = to_hex(signature);
    
    return pubKeyB64 + "-" + hashHex + "-" + sigHex;
}

std::vector<uint8_t> NetworkAuthService::generateKeyPair(std::vector<uint8_t> &publicKey)
{
    // Заглушка - в продакшене использовать OpenSSL для генерации RSA ключей
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    // Генерируем случайные данные для ключей
    std::vector<uint8_t> privateKey(256);
    publicKey.resize(256);
    
    for (size_t i = 0; i < 256; ++i) {
        privateKey[i] = static_cast<uint8_t>(dis(gen));
        publicKey[i] = static_cast<uint8_t>(dis(gen));
    }
    
    return privateKey;
}

std::vector<uint8_t> NetworkAuthService::encryptData(const std::vector<uint8_t> &data, const std::vector<uint8_t> &key)
{
    // Упрощённое шифрование XOR (в продакшене использовать полноценное шифрование)
    std::vector<uint8_t> result = data;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    // Возвращаем как есть (в продакшене добавить Base64 кодирование если нужно)
    return result;
}

std::vector<uint8_t> NetworkAuthService::decryptData(const std::vector<uint8_t> &data, const std::vector<uint8_t> &key)
{
    // XOR дешифрование (аналогично шифрованию)
    std::vector<uint8_t> result = data;
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }
    return result;
}

std::string NetworkAuthService::generateAccessKey(const std::vector<uint8_t> &publicKey)
{
    std::string pubKeyB64 = base64_encode(publicKey);
    std::vector<uint8_t> hash = sha256_hash(publicKey);
    std::string hashHex = to_hex(hash);
    
    std::vector<uint8_t> sigData = publicKey;
    sigData.insert(sigData.end(), hash.begin(), hash.end());
    std::vector<uint8_t> signature = sha256_hash(sigData);
    std::string sigHex = to_hex(signature);
    
    return pubKeyB64 + "-" + hashHex + "-" + sigHex;
}

bool NetworkAuthService::loadTrustStore()
{
    std::string filePath = getStorageFilePath();
    std::ifstream file(filePath);
    
    if (!file.exists()) {
        return true; // Пустое хранилище - это нормально
    }
    
    if (!file.is_open()) {
        if (m_onError) m_onError("Failed to open trust store");
        return false;
    }
    
    // Читаем JSON вручную (заглушка - в продакшене использовать nlohmann/json)
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    if (content.empty()) {
        return false;
    }
    
    // Простой парсинг JSON (заглушка)
    m_trustedNodes.clear();
    
    // TODO: Реализовать полноценный парсинг JSON
    // Здесь нужна библиотека nlohmann/json или аналог
    
    return true;
}

bool NetworkAuthService::saveTrustStore()
{
    std::string filePath = getStorageFilePath();
    std::ofstream file(filePath);
    
    if (!file.is_open()) {
        if (m_onError) m_onError("Failed to save trust store");
        return false;
    }
    
    // Простая генерация JSON (заглушка - в продакшене использовать nlohmann/json)
    std::ostringstream json;
    json << "{\n";
    json << "  \"localNodeId\": \"" << m_localNodeId << "\",\n";
    json << "  \"localPublicKey\": \"" << base64_encode(m_localPublicKey) << "\",\n";
    json << "  \"localPrivateKey\": \"" << base64_encode(m_localPrivateKey) << "\",\n";
    json << "  \"nodes\": [\n";
    
    bool first = true;
    for (const auto& pair : m_trustedNodes) {
        if (!first) json << ",\n";
        first = false;
        
        const auto& node = pair.second;
        json << "    {\n";
        json << "      \"id\": \"" << node.id << "\",\n";
        json << "      \"hostname\": \"" << node.hostname << "\",\n";
        json << "      \"ipAddress\": \"" << node.ipAddress << "\",\n";
        json << "      \"publicKey\": \"" << base64_encode(node.publicKey) << "\",\n";
        json << "      \"registeredAt\": " << node.registeredAt << ",\n";
        json << "      \"isActive\": " << (node.isActive ? "true" : "false") << "\n";
        json << "    }";
    }
    
    json << "\n  ]\n";
    json << "}\n";
    
    file << json.str();
    file.close();
    
    return true;
}

std::string NetworkAuthService::getStorageFilePath() const
{
    return fs::path(m_storagePath) / "trust_store.json";
}

void NetworkAuthService::onNewConnection()
{
    // Заглушка - в продакшене реализовать обработку соединений
}

void NetworkAuthService::onSocketReadyRead()
{
    // Заглушка
}

void NetworkAuthService::onSocketDisconnected()
{
    // Заглушка
}

void NetworkAuthService::onDiscoveryTimeout()
{
    // Периодическая проверка активных соединений
    // В продакшене реализовать ping-проверку
}

void NetworkAuthService::sendHandshake(void* socket, const std::vector<uint8_t>& myPublicKey)
{
    // Заглушка - в продакшене реализовать отправку handshake
    Q_UNUSED(socket);
    Q_UNUSED(myPublicKey);
}

bool NetworkAuthService::processHandshake(void* socket, const std::vector<uint8_t>& receivedData)
{
    // Заглушка - в продакшене реализовать обработку handshake
    Q_UNUSED(socket);
    Q_UNUSED(receivedData);
    return true;
}

// Оставшиеся методы - заглушки для сетевой функциональности
// В продакшене здесь будет полноценная реализация с использованием boost.asio или аналогичной библиотеки

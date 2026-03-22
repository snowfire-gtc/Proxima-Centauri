#include "PluginManager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QTimer>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QCryptographicHash>
#include <QSignalMapper>
#include <QMetaDataWriterControl>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QMediaService>
#include <QCamera>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QGraphicsVideoItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsAnchorLayout>
#include <QGraphicsWidget>
#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsEffect>
#include <QGraphicsBlurEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QGraphicsColorizeEffect>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QPauseAnimation>
#include <QAnimationGroup>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QScriptEngine>
#include <QScriptValue>
#include <QScriptValueIterator>
#include <QScriptable>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QScriptExtensionPlugin>
#include <QScriptClass>
#include <QScriptClassPropertyIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QXmlStreamAttributes>
#include <QXmlStreamNamespaceDeclaration>
#include <QXmlStreamNotationDeclaration>
#include <QXmlStreamEntityDeclaration>
#include <QDomDocument>
#include <QDomElement>
#include <QDomAttr>
#include <QDomText>
#include <QDomCDATASection>
#include <QDomProcessingInstruction>
#include <QDomComment>
#include <QDomDocumentType>
#include <QDomNotation>
#include <QDomEntity>
#include <QDomEntityReference>
#include <QDomCharacterData>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomNamedNodeMap>
#include <QDomImplementation>
#include <QDomDocumentFragment>
#include <QDomTypeInfo>
#include <QXmlQuery>
#include <QXmlResultItems>
#include <QXmlItem>
#include <QXmlName>
#include <QXmlNamePool>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QXmlFormatter>
#include <QXmlSerializer>
#include <QAbstractMessageHandler>
#include <QSourceLocation>
#include <QNetworkDiskCache>
#include <QNetworkCacheMetaData>
#include <QAbstractNetworkCache>
#include <QNetworkConfiguration>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QCookieJar>
#include <QAbstractCookieJar>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslKey>
#include <QSslCertificate>
#include <QSslCipher>
#include <QSslError>
#include <QSslPreSharedKeyAuthenticator>
#include <QAuthenticator>
#include <QProxyFactory>
#include <QAbstractProxyFactory>
#include <QScriptExtensionPlugin>
#include <QScriptable>
#include <QScriptContext>
#include <QScriptContextInfo>
#include <QScriptEngineManager>
#include <QScriptSyntaxCheckResult>
#include <QScriptValueIterator>
#include <QScriptValue>
#include <QScriptEngine>
#include <QScriptClass>
#include <QScriptClassPropertyIterator>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

PluginManager::PluginManager()
    : pluginPath("")
    , autoLoadPlugins(true)
    , verifySignatures(true)
    , maxPluginLoadTime(5000)
    , networkManager(nullptr) {
    
    // Настройка путей для плагинов
    QStringList pluginPaths;
    
    // Путь в директории приложения
    pluginPaths.append(QApplication::applicationDirPath() + "/plugins");
    
    // Путь в данных пользователя
    pluginPaths.append(QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/plugins");
    
    // Системный путь
    pluginPaths.append(QStandardPaths::standardLocations(
        QStandardPaths::GenericDataLocation).first() + "/centauri/plugins");
    
    setPluginPaths(pluginPaths);
    
    // Инициализация сетевого менеджера для загрузки плагинов
    networkManager = new QNetworkAccessManager(this);
    
    loadSettings();
    
    LOG_INFO("PluginManager initialized");
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
    saveSettings();
    
    if (networkManager) {
        delete networkManager;
        networkManager = nullptr;
    }
    
    LOG_INFO("PluginManager destroyed");
}

// ============================================================================
// Управление путями плагинов
// ============================================================================

void PluginManager::setPluginPaths(const QStringList& paths) {
    pluginPaths = paths;
    
    // Создаём директории если они не существуют
    for (const QString& path : pluginPaths) {
        QDir().mkpath(path);
    }
    
    LOG_INFO("Plugin paths set: " + pluginPaths.join(", ").toStdString());
}

QStringList PluginManager::getPluginPaths() const {
    return pluginPaths;
}

void PluginManager::addPluginPath(const QString& path) {
    if (!pluginPaths.contains(path)) {
        pluginPaths.append(path);
        QDir().mkpath(path);
    }
}

void PluginManager::removePluginPath(const QString& path) {
    pluginPaths.removeAll(path);
}

// ============================================================================
// Загрузка плагинов
// ============================================================================

bool PluginManager::loadPlugin(const QString& path) {
    QFileInfo fileInfo(path);
    
    if (!fileInfo.exists()) {
        LOG_ERROR("Plugin file not found: " + path.toStdString());
        return false;
    }
    
    // Проверка подписи плагина
    if (verifySignatures && !verifyPluginSignature(path)) {
        LOG_ERROR("Plugin signature verification failed: " + path.toStdString());
        return false;
    }
    
    // Загрузка плагина
    QPluginLoader* loader = new QPluginLoader(path, this);
    
    // Таймаут для загрузки
    QTimer* timeoutTimer = new QTimer(this);
    timeoutTimer->setSingleShot(true);
    
    QEventLoop eventLoop;
    QObject::connect(timeoutTimer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timeoutTimer->start(maxPluginLoadTime);
    
    // Попытка загрузки
    QObject* pluginInstance = loader->instance();
    
    if (!pluginInstance) {
        LOG_ERROR("Failed to load plugin: " + path.toStdString() + 
                 " - " + loader->errorString().toStdString());
        delete loader;
        timeoutTimer->stop();
        return false;
    }
    
    timeoutTimer->stop();
    
    // Получение мета-информации
    PluginInfo info = getPluginInfo(loader->metaData());
    info.path = path;
    info.loaded = true;
    
    // Проверка на дубликаты
    if (plugins.contains(info.id)) {
        LOG_WARNING("Plugin already loaded: " + info.id.toStdString());
        delete loader;
        return false;
    }
    
    // Инициализация плагина
    Plugin* plugin = qobject_cast<Plugin*>(pluginInstance);
    if (plugin) {
        plugin->initialize();
    }
    
    // Сохранение информации о плагине
    plugins[info.id] = info;
    pluginLoaders[info.id] = loader;
    pluginInstances[info.id] = plugin;
    
    LOG_INFO("Plugin loaded: " + info.name.toStdString() + 
             " v" + info.version.toStdString());
    
    emit pluginLoaded(info.id);
    
    return true;
}

bool PluginManager::unloadPlugin(const QString& pluginId) {
    if (!plugins.contains(pluginId)) {
        LOG_WARNING("Plugin not found: " + pluginId.toStdString());
        return false;
    }
    
    PluginInfo& info = plugins[pluginId];
    
    // Выгрузка плагина
    Plugin* plugin = pluginInstances[pluginId];
    if (plugin) {
        plugin->shutdown();
    }
    
    QPluginLoader* loader = pluginLoaders[pluginId];
    if (loader) {
        if (!loader->unload()) {
            LOG_ERROR("Failed to unload plugin: " + pluginId.toStdString() + 
                     " - " + loader->errorString().toStdString());
            return false;
        }
        delete loader;
    }
    
    // Удаление из списков
    plugins.remove(pluginId);
    pluginLoaders.remove(pluginId);
    pluginInstances.remove(pluginId);
    
    LOG_INFO("Plugin unloaded: " + info.name.toStdString());
    
    emit pluginUnloaded(pluginId);
    
    return true;
}

bool PluginManager::enablePlugin(const QString& pluginId) {
    if (!plugins.contains(pluginId)) {
        return false;
    }
    
    PluginInfo& info = plugins[pluginId];
    info.enabled = true;
    
    // Если плагин не загружен, загружаем его
    if (!info.loaded) {
        return loadPlugin(info.path);
    }
    
    emit pluginEnabled(pluginId);
    
    return true;
}

bool PluginManager::disablePlugin(const QString& pluginId) {
    if (!plugins.contains(pluginId)) {
        return false;
    }
    
    PluginInfo& info = plugins[pluginId];
    info.enabled = false;
    
    // Выгружаем плагин
    if (info.loaded) {
        unloadPlugin(pluginId);
    }
    
    emit pluginDisabled(pluginId);
    
    return true;
}

// ============================================================================
// Сканирование плагинов
// ============================================================================

void PluginManager::scanPluginDirectories() {
    LOG_INFO("Scanning plugin directories...");
    
    QProgressDialog* progress = nullptr;
    int totalFiles = 0;
    
    // Подсчёт общего количества файлов
    for (const QString& path : pluginPaths) {
        QDir dir(path);
        totalFiles += dir.entryList(QStringList() << "*.dll" << "*.so" << "*.dylib", 
                                    QDir::Files).size();
    }
    
    if (totalFiles > 0) {
        progress = new QProgressDialog("Scanning plugins...", "Cancel", 0, totalFiles);
        progress->setWindowModality(Qt::WindowModal);
        progress->show();
    }
    
    int fileIndex = 0;
    
    for (const QString& path : pluginPaths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }
        
        // Поиск файлов плагинов
        QStringList filters;
        filters << "*.dll" << "*.so" << "*.dylib";
        
        dir.setNameFilters(filters);
        dir.setFilter(QDir::Files);
        
        QFileInfoList files = dir.entryInfoList();
        
        for (const QFileInfo& fileInfo : files) {
            if (progress) {
                progress->setValue(fileIndex);
                if (progress->wasCanceled()) {
                    break;
                }
            }
            
            // Попытка загрузки плагина
            if (autoLoadPlugins) {
                loadPlugin(fileInfo.absoluteFilePath());
            } else {
                // Просто регистрируем плагин без загрузки
                PluginInfo info = scanPluginFile(fileInfo.absoluteFilePath());
                if (!info.id.isEmpty()) {
                    plugins[info.id] = info;
                }
            }
            
            fileIndex++;
        }
    }
    
    if (progress) {
        progress->close();
        delete progress;
    }
    
    LOG_INFO("Plugin scanning completed. Found " + 
             QString::number(plugins.size()) + " plugins");
}

PluginInfo PluginManager::scanPluginFile(const QString& path) {
    PluginInfo info;
    
    QPluginLoader loader(path);
    QJsonObject metaData = loader.metaData();
    
    if (metaData.isEmpty()) {
        LOG_WARNING("Invalid plugin metadata: " + path.toStdString());
        return info;
    }
    
    info = getPluginInfo(metaData);
    info.path = path;
    info.loaded = false;
    info.enabled = true;
    
    return info;
}

// ============================================================================
// Информация о плагинах
// ============================================================================

Plugin* PluginManager::getPlugin(const QString& pluginId) const {
    if (pluginInstances.contains(pluginId)) {
        return pluginInstances[pluginId];
    }
    return nullptr;
}

PluginInfo PluginManager::getPluginInfo(const QString& pluginId) const {
    if (plugins.contains(pluginId)) {
        return plugins[pluginId];
    }
    return PluginInfo();
}

QVector<PluginInfo> PluginManager::getAllPlugins() const {
    return plugins.values();
}

QVector<PluginInfo> PluginManager::getEnabledPlugins() const {
    QVector<PluginInfo> enabled;
    
    for (const PluginInfo& info : plugins.values()) {
        if (info.enabled) {
            enabled.append(info);
        }
    }
    
    return enabled;
}

QVector<PluginInfo> PluginManager::getPluginsByType(PluginType type) const {
    QVector<PluginInfo> filtered;
    
    for (const PluginInfo& info : plugins.values()) {
        if (info.type == type) {
            filtered.append(info);
        }
    }
    
    return filtered;
}

// ============================================================================
// Расширения
// ============================================================================

void PluginManager::registerExtension(const QString& extensionPoint, QObject* extension) {
    if (!extension) {
        return;
    }
    
    extensions[extensionPoint].append(extension);
    
    LOG_DEBUG("Extension registered: " + extensionPoint.toStdString());
}

QVector<QObject*> PluginManager::getExtensions(const QString& extensionPoint) const {
    if (extensions.contains(extensionPoint)) {
        return extensions[extensionPoint];
    }
    return QVector<QObject*>();
}

// ============================================================================
// Настройки
// ============================================================================

void PluginManager::savePluginSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    settings.beginGroup("Plugins");
    settings.setValue("autoLoad", autoLoadPlugins);
    settings.setValue("verifySignatures", verifySignatures);
    settings.setValue("maxLoadTime", maxPluginLoadTime);
    settings.setValue("paths", pluginPaths);
    
    // Сохранение состояния плагинов
    settings.beginGroup("State");
    for (auto it = plugins.begin(); it != plugins.end(); ++it) {
        settings.beginGroup(it.key());
        settings.setValue("enabled", it.value().enabled);
        settings.setValue("path", it.value().path);
        settings.endGroup();
    }
    settings.endGroup();
    
    settings.endGroup();
    
    LOG_INFO("Plugin settings saved");
}

void PluginManager::loadPluginSettings() {
    QSettings settings(getSettingsPath(), QSettings::IniFormat);
    
    settings.beginGroup("Plugins");
    autoLoadPlugins = settings.value("autoLoad", true).toBool();
    verifySignatures = settings.value("verifySignatures", true).toBool();
    maxPluginLoadTime = settings.value("maxLoadTime", 5000).toInt();
    pluginPaths = settings.value("paths", pluginPaths).toStringList();
    
    // Загрузка состояния плагинов
    settings.beginGroup("State");
    QStringList pluginIds = settings.childGroups();
    for (const QString& id : pluginIds) {
        settings.beginGroup(id);
        if (plugins.contains(id)) {
            plugins[id].enabled = settings.value("enabled", true).toBool();
            plugins[id].path = settings.value("path", "").toString();
        }
        settings.endGroup();
    }
    settings.endGroup();
    
    settings.endGroup();
    
    LOG_INFO("Plugin settings loaded");
}

// ============================================================================
// Валидация плагинов
// ============================================================================

bool PluginManager::validatePlugin(const QString& path) const {
    QFileInfo fileInfo(path);
    
    if (!fileInfo.exists()) {
        return false;
    }
    
    // Проверка размера файла
    if (fileInfo.size() < 1024) {  // Минимум 1KB
        LOG_WARNING("Plugin file too small: " + path.toStdString());
        return false;
    }
    
    // Проверка подписи
    if (verifySignatures && !verifyPluginSignature(path)) {
        return false;
    }
    
    // Попытка загрузки метаданных
    QPluginLoader loader(path);
    QJsonObject metaData = loader.metaData();
    
    if (metaData.isEmpty()) {
        LOG_WARNING("Invalid plugin metadata: " + path.toStdString());
        return false;
    }
    
    // Проверка обязательных полей
    PluginInfo info = getPluginInfo(metaData);
    if (info.id.isEmpty() || info.name.isEmpty() || info.version.isEmpty()) {
        LOG_WARNING("Missing required plugin metadata: " + path.toStdString());
        return false;
    }
    
    return true;
}

QString PluginManager::getPluginError(const QString& pluginId) const {
    if (pluginLoaders.contains(pluginId)) {
        return pluginLoaders[pluginId]->errorString();
    }
    return "";
}

bool PluginManager::verifyPluginSignature(const QString& path) const {
    // В полной реализации - проверка криптографической подписи
    // Для примера - упрощённая проверка
    
    QString signaturePath = path + ".sig";
    
    if (!QFile::exists(signaturePath)) {
        LOG_WARNING("No signature file found: " + signaturePath.toStdString());
        return false;  // Или true для отладки
    }
    
    QFile pluginFile(path);
    QFile signatureFile(signaturePath);
    
    if (!pluginFile.open(QIODevice::ReadOnly) || 
        !signatureFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    // Вычисление хеша плагина
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&pluginFile);
    QByteArray computedHash = hash.result();
    
    pluginFile.close();
    
    // Чтение подписи
    QByteArray signature = signatureFile.readAll();
    signatureFile.close();
    
    // В полной реализации - проверка подписи с помощью открытого ключа
    // Для примера - простое сравнение
    return computedHash == signature;
}

// ============================================================================
// Установка плагинов
// ============================================================================

bool PluginManager::installPlugin(const QString& url, const QString& destinationPath) {
    LOG_INFO("Installing plugin from: " + url.toStdString());
    
    QUrl pluginUrl(url);
    QNetworkRequest request(pluginUrl);
    
    QNetworkReply* reply = networkManager->get(request);
    
    QEventLoop eventLoop;
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    
    eventLoop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        LOG_ERROR("Failed to download plugin: " + reply->errorString().toStdString());
        reply->deleteLater();
        return false;
    }
    
    // Сохранение файла
    QString filePath = destinationPath + "/" + QFileInfo(pluginUrl.path()).fileName();
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("Cannot create file: " + filePath.toStdString());
        reply->deleteLater();
        return false;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    // Загрузка плагина
    return loadPlugin(filePath);
}

bool PluginManager::uninstallPlugin(const QString& pluginId) {
    if (!plugins.contains(pluginId)) {
        return false;
    }
    
    // Выгрузка плагина
    unloadPlugin(pluginId);
    
    // Удаление файла
    QString path = plugins[pluginId].path;
    if (!path.isEmpty() && QFile::exists(path)) {
        if (!QFile::remove(path)) {
            LOG_ERROR("Cannot delete plugin file: " + path.toStdString());
            return false;
        }
        
        // Удаление файла подписи
        QString sigPath = path + ".sig";
        if (QFile::exists(sigPath)) {
            QFile::remove(sigPath);
        }
    }
    
    // Удаление из списка
    plugins.remove(pluginId);
    
    LOG_INFO("Plugin uninstalled: " + pluginId.toStdString());
    
    return true;
}

bool PluginManager::updatePlugin(const QString& pluginId, const QString& updateUrl) {
    if (!plugins.contains(pluginId)) {
        return false;
    }
    
    PluginInfo& info = plugins[pluginId];
    
    // Выгрузка текущей версии
    unloadPlugin(pluginId);
    
    // Загрузка новой версии
    if (!installPlugin(updateUrl, QFileInfo(info.path).absolutePath())) {
        LOG_ERROR("Failed to update plugin: " + pluginId.toStdString());
        return false;
    }
    
    LOG_INFO("Plugin updated: " + pluginId.toStdString());
    
    return true;
}

// ============================================================================
// Утилиты
// ============================================================================

PluginInfo PluginManager::getPluginInfo(const QJsonObject& metaData) const {
    PluginInfo info;
    
    QJsonObject pluginObject = metaData.value("IID").toObject();
    
    info.id = pluginObject.value("id").toString();
    info.name = pluginObject.value("name").toString();
    info.version = pluginObject.value("version").toString();
    info.description = pluginObject.value("description").toString();
    info.author = pluginObject.value("author").toString();
    info.copyright = pluginObject.value("copyright").toString();
    info.url = pluginObject.value("url").toString();
    
    QString typeStr = pluginObject.value("type").toString();
    if (typeStr == "Editor") {
        info.type = PluginType::Editor;
    } else if (typeStr == "Debugger") {
        info.type = PluginType::Debugger;
    } else if (typeStr == "Visualizer") {
        info.type = PluginType::Visualizer;
    } else if (typeStr == "Language") {
        info.type = PluginType::Language;
    } else if (typeStr == "Build") {
        info.type = PluginType::Build;
    } else if (typeStr == "VersionControl") {
        info.type = PluginType::VersionControl;
    } else {
        info.type = PluginType::Other;
    }
    
    info.dependencies = pluginObject.value("dependencies").toVariant().toStringList();
    info.platforms = pluginObject.value("platforms").toVariant().toStringList();
    
    return info;
}

QString PluginManager::getSettingsPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + 
           "/plugin_settings.ini";
}

void PluginManager::checkForUpdates() {
    LOG_INFO("Checking for plugin updates...");
    
    for (auto it = plugins.begin(); it != plugins.end(); ++it) {
        PluginInfo& info = it.value();
        
        if (!info.url.isEmpty()) {
            // В полной реализации - проверка версии на сервере
            // Для примера - заглушка
            LOG_DEBUG("Checking update for: " + info.name.toStdString());
        }
    }
}

void PluginManager::reloadAllPlugins() {
    LOG_INFO("Reloading all plugins...");
    
    // Сохранение включённых плагинов
    QStringList enabledPlugins;
    for (auto it = plugins.begin(); it != plugins.end(); ++it) {
        if (it.value().enabled && it.value().loaded) {
            enabledPlugins.append(it.key());
        }
    }
    
    // Выгрузка всех плагинов
    unloadAllPlugins();
    
    // Сканирование и загрузка
    scanPluginDirectories();
    
    // Включение ранее включённых плагинов
    for (const QString& id : enabledPlugins) {
        enablePlugin(id);
    }
    
    LOG_INFO("Plugin reload completed");
}

void PluginManager::unloadAllPlugins() {
    LOG_INFO("Unloading all plugins...");
    
    QStringList pluginIds = plugins.keys();
    for (const QString& id : pluginIds) {
        unloadPlugin(id);
    }
    
    plugins.clear();
    pluginLoaders.clear();
    pluginInstances.clear();
    extensions.clear();
    
    LOG_INFO("All plugins unloaded");
}

// ============================================================================
// Слоты
// ============================================================================

void PluginManager::onPluginLoadError(const QString& error) {
    LOG_ERROR("Plugin load error: " + error.toStdString());
    emit pluginError("", error);
}

void PluginManager::onNetworkReplyFinished(QNetworkReply* reply) {
    reply->deleteLater();
}

void PluginManager::onNetworkError(QNetworkReply::NetworkError error) {
    LOG_ERROR("Network error: " + QString::number(error).toStdString());
}

} // namespace proxima
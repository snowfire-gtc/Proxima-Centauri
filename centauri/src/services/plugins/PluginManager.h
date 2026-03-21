#ifndef CENTAURI_PLUGINMANAGER_H
#define CENTAURI_PLUGINMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QPluginLoader>
#include <QJsonObject>

namespace proxima {

enum class PluginType {
    Editor,
    Debugger,
    Visualizer,
    Language,
    Build,
    VersionControl,
    Other
};

struct PluginInfo {
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    PluginType type;
    QString path;
    bool enabled;
    bool loaded;
    QStringList dependencies;
    QJsonObject metadata;
};

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual QString getId() const = 0;
    virtual QString getName() const = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    virtual PluginInfo getInfo() const = 0;
};

class PluginManager : public QObject {
    Q_OBJECT
    
public:
    static PluginManager& getInstance();
    
    // Plugin management
    bool loadPlugin(const QString& path);
    bool unloadPlugin(const QString& pluginId);
    bool enablePlugin(const QString& pluginId);
    bool disablePlugin(const QString& pluginId);
    
    // Discovery
    void scanPluginDirectories();
    void addPluginDirectory(const QString& path);
    QVector<QString> getPluginDirectories() const { return pluginDirectories; }
    
    // Queries
    Plugin* getPlugin(const QString& pluginId) const;
    PluginInfo getPluginInfo(const QString& pluginId) const;
    QVector<PluginInfo> getAllPlugins() const;
    QVector<PluginInfo> getEnabledPlugins() const;
    QVector<PluginInfo> getPluginsByType(PluginType type) const;
    
    // Extension points
    void registerExtension(const QString& extensionPoint, QObject* extension);
    QVector<QObject*> getExtensions(const QString& extensionPoint) const;
    
    // Settings
    void savePluginSettings();
    void loadPluginSettings();
    
    // Validation
    bool validatePlugin(const QString& path) const;
    QString getPluginError(const QString& pluginId) const;
    
signals:
    void pluginLoaded(const QString& pluginId);
    void pluginUnloaded(const QString& pluginId);
    void pluginEnabled(const QString& pluginId);
    void pluginDisabled(const QString& pluginId);
    void pluginError(const QString& pluginId, const QString& error);
    
private:
    PluginManager();
    ~PluginManager();
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    bool loadPluginMetadata(const QString& path, PluginInfo& info);
    bool checkDependencies(const PluginInfo& info) const;
    void sortPluginsByDependency(QVector<PluginInfo>& plugins);
    
    QMap<QString, PluginInfo> plugins;
    QMap<QString, Plugin*> loadedPlugins;
    QMap<QString, QPluginLoader*> pluginLoaders;
    QVector<QString> pluginDirectories;
    QMap<QString, QVector<QObject*>> extensions;
    QMap<QString, QString> pluginErrors;
    
    QString pluginDir;
    QString userPluginDir;
};

} // namespace proxima

#endif // CENTAURI_PLUGINMANAGER_H
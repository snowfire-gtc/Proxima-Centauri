#ifndef CENTAURI_PROJECT_H
#define CENTAURI_PROJECT_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include "Module.h"
#include "compiler/Config.h"

namespace proxima {

enum class ProjectStatus {
    New,
    Open,
    Modified,
    Building,
    Running,
    Error
};

struct ProjectInfo {
    QString name;
    QString version;
    QString entryPoint;
    QString requiredIDEVersion;
    QStringList capabilities;
    QString license;
    QStringList authors;
    QString buildHash;
    QString buildTimestamp;
};

class Project : public QObject {
    Q_OBJECT
    
public:
    explicit Project(QObject *parent = nullptr);
    ~Project();
    
    // Project management
    bool create(const QString& path, const QString& name);
    bool load(const QString& path);
    bool save();
    bool close();
    
    // Accessors
    QString getName() const { return info.name; }
    QString getVersion() const { return info.version; }
    QString getPath() const { return projectPath; }
    QString getEntryPoint() const { return info.entryPoint; }
    ProjectStatus getStatus() const { return status; }
    ProjectInfo getInfo() const { return info; }
    
    // Module management
    bool addModule(const QString& path);
    bool removeModule(const QString& path);
    Module* getModule(const QString& path) const;
    QVector<Module*> getAllModules() const { return modules; }
    QVector<Module*> getSubprojectModules(const QString& subproject) const;
    bool hasModule(const QString& path) const;
    
    // Configuration
    bool loadBuildRules();
    bool saveBuildRules();
    Config getBuildConfig() const { return buildConfig; }
    void setBuildConfig(const Config& config) { buildConfig = config; }
    
    // Build
    bool build();
    bool clean();
    bool run();
    
    // Git
    bool isGitRepository() const { return gitInitialized; }
    void initializeGit();
    
    // Auto-save
    QString getAutoSavePath(const QString& modulePath) const;
    void setAutoSaveEnabled(bool enable) { autoSaveEnabled = enable; }
    bool isAutoSaveEnabled() const { return autoSaveEnabled; }
    
    // Statistics
    int getModuleCount() const { return modules.size(); }
    int getTotalLines() const;
    qint64 getLastModified() const { return lastModified; }
    
signals:
    void projectCreated(const QString& path);
    void projectLoaded(const QString& path);
    void projectSaved();
    void projectClosed();
    void moduleAdded(Module* module);
    void moduleRemoved(const QString& path);
    void moduleModified(Module* module);
    void buildStarted();
    void buildCompleted(bool success);
    void statusChanged(ProjectStatus status);
    void infoChanged(const ProjectInfo& info);
    
private:
    void setupProjectStructure();
    bool loadManifest();
    bool saveManifest();
    bool loadModules();
    void updateStatus(ProjectStatus newStatus);
    void calculateBuildHash();
    QString generateBuildTimestamp() const;
    
    QString projectPath;
    ProjectInfo info;
    ProjectStatus status;
    Config buildConfig;
    
    QVector<Module*> modules;
    QMap<QString, Module*> modulesByPath;
    
    bool gitInitialized;
    bool autoSaveEnabled;
    qint64 lastModified;
    
    QString buildHash;
    QString buildTimestamp;
};

} // namespace proxima

#endif // CENTAURI_PROJECT_H

#ifndef CENTAURI_AUTOSAVEMANAGER_H
#define CENTAURI_AUTOSAVEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QDateTime>

namespace proxima {

struct AutoSaveEntry {
    QString filePath;
    QString autosavePath;
    QString content;
    QDateTime lastEdit;
    QDateTime lastAutosave;
    bool isDirty;
    bool isAutosaved;
    int editCount;
};

class AutoSaveManager : public QObject {
    Q_OBJECT
    
public:
    explicit AutoSaveManager(QObject *parent = nullptr);
    ~AutoSaveManager();
    
    // Configuration
    void setAutosaveInterval(int minutes);
    int getAutosaveInterval() const { return intervalMinutes; }
    void setAutosaveDir(const QString& dir);
    QString getAutosaveDir() const { return autosaveDir; }
    void setProjectPath(const QString& path);
    QString getProjectPath() const { return projectPath; }
    void setEnabled(bool enable);
    bool isEnabled() const { return enabled; }
    
    // File management
    void registerFile(const QString& filePath);
    void unregisterFile(const QString& filePath);
    bool isRegistered(const QString& filePath) const;
    
    // Content tracking
    void markEdited(const QString& filePath, const QString& content);
    void markSaved(const QString& filePath);
    void markClosed(const QString& filePath);
    
    // Save operations
    void saveNow(const QString& filePath);
    void saveAll();
    void saveDirtyFiles();
    
    // Recovery
    QString getAutosavePath(const QString& filePath) const;
    QString loadAutosave(const QString& filePath) const;
    bool hasAutosave(const QString& filePath) const;
    QDateTime getAutosaveTime(const QString& filePath) const;
    QVector<AutoSaveEntry> getAutosaveHistory(const QString& filePath) const;
    
    // Cleanup
    void cleanupOldAutosaves(int daysToKeep);
    void clearAllAutosaves();
    
    // Status
    QVector<AutoSaveEntry> getAllEntries() const { return files.values(); }
    int getDirtyFileCount() const;
    int getTotalFileCount() const { return files.size(); }
    qint64 getTotalAutosaveSize() const;
    
    // Start/Stop
    void start();
    void stop();
    bool isRunning() const { return running; }
    
signals:
    void fileAutosaved(const QString& filePath, const QString& autosavePath);
    void fileRecovered(const QString& filePath);
    void autosaveError(const QString& filePath, const QString& error);
    void statusChanged();
    
private slots:
    void onAutosaveTimer();
    void onCleanupTimer();
    
private:
    void setupTimers();
    bool shouldSave(const AutoSaveEntry& entry) const;
    bool performAutosave(AutoSaveEntry& entry);
    QString calculateAutosavePath(const QString& filePath) const;
    void ensureDirectoryExists(const QString& path);
    bool writeAutosave(const QString& path, const QString& content);
    QString readAutosave(const QString& path) const;
    void removeAutosave(const QString& path);
    void log(const QString& message);
    
    int intervalMinutes;
    QString autosaveDir;
    QString projectPath;
    bool enabled;
    bool running;
    
    QMap<QString, AutoSaveEntry> files;
    
    QTimer* autosaveTimer;
    QTimer* cleanupTimer;
    
    int cleanupIntervalHours;
    int maxAutosaveVersions;
    qint64 maxTotalSize;
};

} // namespace proxima

#endif // CENTAURI_AUTOSAVEMANAGER_H
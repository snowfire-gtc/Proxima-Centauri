#include "AutoSaveManager.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <QStandardPaths>
#include "utils/Logger.h"

namespace proxima {

AutoSaveManager::AutoSaveManager(QObject *parent)
    : QObject(parent)
    , intervalMinutes(5)
    , autosaveDir("./autosave")
    , enabled(true)
    , running(false)
    , cleanupIntervalHours(24)
    , maxAutosaveVersions(10)
    , maxTotalSize(1024 * 1024 * 1024) { // 1GB
    
    setupTimers();
}

AutoSaveManager::~AutoSaveManager() {
    stop();
}

void AutoSaveManager::setupTimers() {
    autosaveTimer = new QTimer(this);
    connect(autosaveTimer, &QTimer::timeout, this, &AutoSaveManager::onAutosaveTimer);
    
    cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, &AutoSaveManager::onCleanupTimer);
}

void AutoSaveManager::setAutosaveInterval(int minutes) {
    intervalMinutes = qMax(1, minutes);
    
    if (running) {
        autosaveTimer->setInterval(intervalMinutes * 60 * 1000);
    }
    
    log("Autosave interval set to " + QString::number(intervalMinutes) + " minutes");
}

void AutoSaveManager::setAutosaveDir(const QString& dir) {
    autosaveDir = dir;
    ensureDirectoryExists(autosaveDir);
}

void AutoSaveManager::setProjectPath(const QString& path) {
    projectPath = path;
    
    if (!projectPath.isEmpty()) {
        setAutosaveDir(projectPath + "/autosave");
    }
}

void AutoSaveManager::setEnabled(bool enable) {
    enabled = enable;
    
    if (!enabled && running) {
        stop();
    } else if (enabled && !running) {
        start();
    }
    
    emit statusChanged();
}

void AutoSaveManager::registerFile(const QString& filePath) {
    if (files.contains(filePath)) {
        return;
    }
    
    AutoSaveEntry entry;
    entry.filePath = filePath;
    entry.autosavePath = calculateAutosavePath(filePath);
    entry.isDirty = false;
    entry.isAutosaved = false;
    entry.lastEdit = QDateTime::currentDateTime();
    entry.lastAutosave = entry.lastEdit;
    entry.editCount = 0;
    
    files[filePath] = entry;
    
    log("File registered for autosave: " + filePath);
    emit statusChanged();
}

void AutoSaveManager::unregisterFile(const QString& filePath) {
    if (!files.contains(filePath)) {
        return;
    }
    
    // Save before unregistering
    if (files[filePath].isDirty) {
        performAutosave(files[filePath]);
    }
    
    files.remove(filePath);
    
    log("File unregistered from autosave: " + filePath);
    emit statusChanged();
}

bool AutoSaveManager::isRegistered(const QString& filePath) const {
    return files.contains(filePath);
}

void AutoSaveManager::markEdited(const QString& filePath, const QString& content) {
    if (!files.contains(filePath)) {
        registerFile(filePath);
    }
    
    AutoSaveEntry& entry = files[filePath];
    entry.content = content;
    entry.lastEdit = QDateTime::currentDateTime();
    entry.isDirty = true;
    entry.isAutosaved = false;
    entry.editCount++;
    
    emit statusChanged();
}

void AutoSaveManager::markSaved(const QString& filePath) {
    if (!files.contains(filePath)) return;
    
    AutoSaveEntry& entry = files[filePath];
    entry.isDirty = false;
    entry.isAutosaved = true;
    entry.lastAutosave = QDateTime::currentDateTime();
    
    emit statusChanged();
}

void AutoSaveManager::markClosed(const QString& filePath) {
    unregisterFile(filePath);
}

void AutoSaveManager::saveNow(const QString& filePath) {
    if (!files.contains(filePath)) return;
    
    AutoSaveEntry& entry = files[filePath];
    if (performAutosave(entry)) {
        log("Manual autosave completed: " + filePath);
    }
}

void AutoSaveManager::saveAll() {
    int savedCount = 0;
    
    for (auto it = files.begin(); it != files.end(); ++it) {
        if (it->isDirty) {
            if (performAutosave(*it)) {
                savedCount++;
            }
        }
    }
    
    if (savedCount > 0) {
        log("Saved " + QString::number(savedCount) + " files");
    }
}

void AutoSaveManager::saveDirtyFiles() {
    saveAll();
}

QString AutoSaveManager::getAutosavePath(const QString& filePath) const {
    return calculateAutosavePath(filePath);
}

QString AutoSaveManager::loadAutosave(const QString& filePath) const {
    QString autosavePath = calculateAutosavePath(filePath);
    return readAutosave(autosavePath);
}

bool AutoSaveManager::hasAutosave(const QString& filePath) const {
    QString autosavePath = calculateAutosavePath(filePath);
    return QFile::exists(autosavePath);
}

QDateTime AutoSaveManager::getAutosaveTime(const QString& filePath) const {
    QString autosavePath = calculateAutosavePath(filePath);
    QFileInfo info(autosavePath);
    return info.lastModified();
}

QVector<AutoSaveEntry> AutoSaveManager::getAutosaveHistory(const QString& filePath) const {
    // Would return version history
    // Simplified for now
    QVector<AutoSaveEntry> history;
    if (files.contains(filePath)) {
        history.append(files[filePath]);
    }
    return history;
}

void AutoSaveManager::cleanupOldAutosaves(int daysToKeep) {
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-daysToKeep);
    int removedCount = 0;
    
    QDir autosaveDirectory(autosaveDir);
    QFileInfoList files = autosaveDirectory.entryInfoList(QDir::Files, QDir::Time);
    
    for (const QFileInfo& file : files) {
        if (file.lastModified() < cutoff) {
            if (QFile::remove(file.absoluteFilePath())) {
                removedCount++;
            }
        }
    }
    
    if (removedCount > 0) {
        log("Cleaned up " + QString::number(removedCount) + " old autosave files");
    }
}

void AutoSaveManager::clearAllAutosaves() {
    QDir autosaveDirectory(autosaveDir);
    autosaveDirectory.removeRecursively();
    ensureDirectoryExists(autosaveDir);
    
    log("All autosaves cleared");
}

int AutoSaveManager::getDirtyFileCount() const {
    int count = 0;
    for (const AutoSaveEntry& entry : files) {
        if (entry.isDirty) {
            count++;
        }
    }
    return count;
}

qint64 AutoSaveManager::getTotalAutosaveSize() const {
    qint64 total = 0;
    
    QDir autosaveDirectory(autosaveDir);
    QFileInfoList files = autosaveDirectory.entryInfoList(QDir::Files);
    
    for (const QFileInfo& file : files) {
        total += file.size();
    }
    
    return total;
}

void AutoSaveManager::start() {
    if (running || !enabled) return;
    
    autosaveTimer->setInterval(intervalMinutes * 60 * 1000);
    autosaveTimer->start();
    
    cleanupTimer->setInterval(cleanupIntervalHours * 60 * 60 * 1000);
    cleanupTimer->start();
    
    running = true;
    ensureDirectoryExists(autosaveDir);
    
    log("AutoSaveManager started");
    emit statusChanged();
}

void AutoSaveManager::stop() {
    if (!running) return;
    
    // Save all dirty files before stopping
    saveAll();
    
    autosaveTimer->stop();
    cleanupTimer->stop();
    
    running = false;
    
    log("AutoSaveManager stopped");
    emit statusChanged();
}

void AutoSaveManager::onAutosaveTimer() {
    if (!enabled || !running) return;
    
    saveAll();
}

void AutoSaveManager::onCleanupTimer() {
    cleanupOldAutosaves(7); // Keep 7 days by default
}

bool AutoSaveManager::shouldSave(const AutoSaveEntry& entry) const {
    if (!entry.isDirty) return false;
    
    QDateTime now = QDateTime::currentDateTime();
    
    // If not autosaved yet and edited more than 5 minutes ago
    if (!entry.isAutosaved) {
        qint64 minutesSinceEdit = entry.lastEdit.secsTo(now) / 60;
        if (minutesSinceEdit >= intervalMinutes) {
            return true;
        }
    }
    
    // If already autosaved, don't save again until edited
    if (entry.isAutosaved) {
        return false;
    }
    
    return false;
}

bool AutoSaveManager::performAutosave(AutoSaveEntry& entry) {
    if (entry.content.isEmpty()) {
        return false;
    }
    
    ensureDirectoryExists(entry.autosavePath);
    
    if (writeAutosave(entry.autosavePath, entry.content)) {
        entry.lastAutosave = QDateTime::currentDateTime();
        entry.isAutosaved = true;
        entry.isDirty = false;
        
        emit fileAutosaved(entry.filePath, entry.autosavePath);
        
        log("Autosaved: " + entry.filePath + " -> " + entry.autosavePath);
        return true;
    } else {
        emit autosaveError(entry.filePath, "Failed to write autosave file");
        return false;
    }
}

QString AutoSaveManager::calculateAutosavePath(const QString& filePath) const {
    // Convert source path to autosave path
    // e.g., /project/src/module.prx -> /project/autosave/src/module.prx
    
    QString relativePath = filePath;
    
    if (!projectPath.isEmpty() && filePath.startsWith(projectPath)) {
        relativePath = filePath.mid(projectPath.length());
        if (relativePath.startsWith("/")) {
            relativePath = relativePath.mid(1);
        }
    }
    
    // Replace src with autosave
    relativePath.replace("src/", "autosave/");
    
    return autosaveDir + "/" + relativePath;
}

void AutoSaveManager::ensureDirectoryExists(const QString& path) {
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

bool AutoSaveManager::writeAutosave(const QString& path, const QString& content) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;
    file.close();
    
    return true;
}

QString AutoSaveManager::readAutosave(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    return content;
}

void AutoSaveManager::removeAutosave(const QString& path) {
    QFile::remove(path);
}

void AutoSaveManager::log(const QString& message) {
    LOG_INFO("[AutoSave] " + message);
}

} // namespace proxima
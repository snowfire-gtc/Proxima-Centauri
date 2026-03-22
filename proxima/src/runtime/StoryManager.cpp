#include "StoryManager.h"
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace proxima {

// ============================================================================
// StoryEntry Implementation
// ============================================================================

QColor StoryEntry::getAgeColor() const {
    qint64 minutes = ageInMinutes();
    
    if (minutes < 5) {
        // Менее 5 минут - ярко-зелёный (очень свежая правка)
        return QColor(100, 255, 100);
    } else if (minutes < 30) {
        // Менее 30 минут - зелёный (свежая правка)
        return QColor(50, 200, 50);
    } else if (minutes < 60) {
        // Менее часа - светло-зелёный
        return QColor(50, 180, 50);
    } else if (minutes < 120) {
        // Менее 2 часов - жёлто-зелёный
        return QColor(150, 200, 50);
    } else if (minutes < 240) {
        // Менее 4 часов - жёлтый
        return QColor(200, 200, 50);
    } else if (minutes < 480) {
        // Менее 8 часов - оранжево-жёлтый
        return QColor(220, 150, 50);
    } else if (minutes < 1440) {
        // Менее дня - оранжевый
        return QColor(255, 120, 50);
    } else if (minutes < 2880) {
        // Менее 2 дней - оранжево-красный
        return QColor(255, 100, 50);
    } else if (minutes < 4320) {
        // Менее 3 дней - красный
        return QColor(255, 80, 50);
    } else if (minutes < 10080) {
        // Менее недели - тёмно-красный
        return QColor(200, 50, 50);
    } else if (minutes < 43200) {
        // Менее месяца - бордовый
        return QColor(150, 50, 50);
    } else {
        // Старше месяца - тёмно-бордовый (очень старая правка)
        return QColor(100, 40, 40);
    }
}

QString StoryEntry::getAgeString() const {
    qint64 minutes = ageInMinutes();
    
    if (minutes < 1) {
        return "Только что";
    } else if (minutes < 60) {
        return QString("%1 мин.").arg(minutes);
    } else if (minutes < 1440) {
        int hours = minutes / 60;
        return QString("%1 ч.").arg(hours);
    } else if (minutes < 10080) {
        int days = minutes / 1440;
        return QString("%1 дн.").arg(days);
    } else if (minutes < 43200) {
        int weeks = minutes / 10080;
        return QString("%1 нед.").arg(weeks);
    } else {
        int months = minutes / 43200;
        return QString("%1 мес.").arg(months);
    }
}

// ============================================================================
// StoryManager Implementation
// ============================================================================

StoryManager& StoryManager::getInstance() {
    static StoryManager instance;
    return instance;
}

StoryManager::StoryManager(QObject *parent)
    : QObject(parent) {
}

StoryManager::~StoryManager() {
    // Сохранение всех историй перед закрытием
    for (auto it = stories.begin(); it != stories.end(); ++it) {
        saveStory(it.key());
    }
}

void StoryManager::initialize(const QString& projPath) {
    projectPath = projPath;
    storyDirectory = projectPath + "/.proxima/story";
    ensureStoryDirectory();
    
    LOG_INFO("StoryManager initialized: " + storyDirectory.toStdString());
}

void StoryManager::ensureStoryDirectory() {
    QDir dir(storyDirectory);
    if (!dir.exists()) {
        dir.mkpath(".");
        LOG_DEBUG("Created story directory: " + storyDirectory.toStdString());
    }
}

QString StoryManager::getRelativeFilePath(const QString& filePath) const {
    // Получение относительного пути от корня проекта
    QString relative = filePath;
    if (relative.startsWith(projectPath)) {
        relative = relative.mid(projectPath.length() + 1);
    }
    
    // Замена разделителей пути
    relative.replace("/", "_");
    relative.replace("\\", "_");
    
    return relative;
}

QString StoryManager::getStoryFilePath(const QString& filePath) const {
    QString relative = getRelativeFilePath(filePath);
    QFileInfo fileInfo(relative);
    return storyDirectory + "/" + fileInfo.baseName() + ".story";
}

bool StoryManager::loadStory(const QString& filePath) {
    QString storyPath = getStoryFilePath(filePath);
    
    if (!QFile::exists(storyPath)) {
        LOG_DEBUG("No story file found: " + storyPath.toStdString());
        stories[filePath].clear();
        currentEntryIndex[filePath] = 0;
        return false;
    }
    
    QFile file(storyPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOG_ERROR("Cannot open story file: " + storyPath.toStdString());
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString content = in.readAll();
    file.close();
    
    // Парсинг Collection формата
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(content);
    
    if (!result.success) {
        LOG_ERROR("Failed to parse story file: " + result.error.toStdString());
        return false;
    }
    
    // Извлечение записей истории
    QVector<StoryEntry>& entries = stories[filePath];
    entries.clear();
    
    Collection storyCollection = result.value;
    for (int i = 0; i < storyCollection.size(); i++) {
        Collection entryCollection = storyCollection.get(i);
        StoryEntry entry;
        entry.fromCollection(entryCollection);
        entries.append(entry);
    }
    
    currentEntryIndex[filePath] = entries.size();
    
    LOG_INFO("Loaded " + std::to_string(entries.size()) + 
             " story entries for: " + filePath.toStdString());
    
    emit storyLoaded(filePath);
    
    return true;
}

bool StoryManager::saveStory(const QString& filePath) {
    QString storyPath = getStoryFilePath(filePath);
    
    if (!stories.contains(filePath) || stories[filePath].isEmpty()) {
        // Если нет записей, удаляем файл истории
        if (QFile::exists(storyPath)) {
            QFile::remove(storyPath);
        }
        return true;
    }
    
    QFile file(storyPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        LOG_ERROR("Cannot write story file: " + storyPath.toStdString());
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    // Сериализация в Collection формат
    out << "[\n";
    
    const QVector<StoryEntry>& entries = stories[filePath];
    for (int i = 0; i < entries.size(); i++) {
        if (i > 0) {
            out << ",,\n";
        }
        
        Collection entryCollection = entries[i].toCollection();
        out << "    " << entryCollection.toCollectionString(1);
    }
    
    out << "\n]\n";
    file.close();
    
    LOG_INFO("Saved " + std::to_string(entries.size()) + 
             " story entries for: " + filePath.toStdString());
    
    emit storySaved(filePath);
    
    return true;
}

void StoryManager::addEditEntry(const QString& filePath, EditOperation operation,
                               const QString& text, int startLine, int endLine) {
    if (text.trimmed().isEmpty()) {
        return;  // Не записываем пустые правки
    }
    
    StoryEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.operation = operation;
    entry.text = text;
    entry.startLine = startLine;
    entry.endLine = endLine;
    entry.checksum = calculateChecksum(text);
    
    stories[filePath].append(entry);
    currentEntryIndex[filePath] = stories[filePath].size();
    
    // Автосохранение после добавления записи
    saveStory(filePath);
    
    LOG_DEBUG("Added story entry: " + operation == EditOperation::Add ? "add" : "remove" +
              " at lines " + std::to_string(startLine) + "-" + std::to_string(endLine));
    
    emit entryAdded(filePath, entry);
}

QVector<StoryEntry> StoryManager::getStoryEntries(const QString& filePath) const {
    if (stories.contains(filePath)) {
        return stories[filePath];
    }
    return QVector<StoryEntry>();
}

StoryEntry* StoryManager::getEntryForLine(const QString& filePath, int line) {
    if (!stories.contains(filePath)) {
        return nullptr;
    }
    
    QVector<StoryEntry>& entries = stories[filePath];
    
    // Поиск последней записи, которая затрагивает эту строку
    for (int i = entries.size() - 1; i >= 0; i--) {
        StoryEntry& entry = entries[i];
        if (line >= entry.startLine && line <= entry.endLine) {
            return &entry;
        }
    }
    
    return nullptr;
}

QMap<int, QDateTime> StoryManager::getAgeInfo(const QString& filePath) {
    QMap<int, QDateTime> ageInfo;
    
    if (!stories.contains(filePath)) {
        return ageInfo;
    }
    
    const QVector<StoryEntry>& entries = stories[filePath];
    
    // Для каждой записи истории определяем строки
    for (const StoryEntry& entry : entries) {
        for (int line = entry.startLine; line <= entry.endLine; line++) {
            // Если для этой строки ещё нет записи или эта запись новее
            if (!ageInfo.contains(line) || entry.timestamp > ageInfo[line]) {
                ageInfo[line] = entry.timestamp;
            }
        }
    }
    
    return ageInfo;
}

bool StoryManager::undoLastEdit(const QString& filePath) {
    if (!stories.contains(filePath) || currentEntryIndex[filePath] <= 0) {
        return false;
    }
    
    currentEntryIndex[filePath]--;
    
    // В полной реализации - применение обратной операции
    // Для add -> remove текст, для remove -> add текст
    
    emit editUndone(filePath);
    
    LOG_INFO("Undo last edit for: " + filePath.toStdString());
    
    return true;
}

bool StoryManager::redoLastEdit(const QString& filePath) {
    if (!stories.contains(filePath) || 
        currentEntryIndex[filePath] >= stories[filePath].size()) {
        return false;
    }
    
    currentEntryIndex[filePath]++;
    
    // В полной реализации - применение операции
    
    emit editRedone(filePath);
    
    LOG_INFO("Redo last edit for: " + filePath.toStdString());
    
    return true;
}

void StoryManager::clearStory(const QString& filePath) {
    stories.remove(filePath);
    currentEntryIndex.remove(filePath);
    
    QString storyPath = getStoryFilePath(filePath);
    if (QFile::exists(storyPath)) {
        QFile::remove(storyPath);
    }
    
    LOG_INFO("Cleared story for: " + filePath.toStdString());
}

bool StoryManager::hasStory(const QString& filePath) const {
    return stories.contains(filePath) && !stories[filePath].isEmpty();
}

QString StoryManager::calculateChecksum(const QString& text) {
    QByteArray hash = QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

} // namespace proxima
#include "StoryManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>

namespace proxima {

// ============================================================================
// StoryEntry Implementation
// ============================================================================

SimpleColor StoryEntry::getAgeColor() const {
    int64_t minutes = ageInMinutes();
    
    if (minutes < 5) {
        // Менее 5 минут - ярко-зелёный (очень свежая правка)
        return SimpleColor(100, 255, 100);
    } else if (minutes < 30) {
        // Менее 30 минут - зелёный (свежая правка)
        return SimpleColor(50, 200, 50);
    } else if (minutes < 60) {
        // Менее часа - светло-зелёный
        return SimpleColor(50, 180, 50);
    } else if (minutes < 120) {
        // Менее 2 часов - жёлто-зелёный
        return SimpleColor(150, 200, 50);
    } else if (minutes < 240) {
        // Менее 4 часов - жёлтый
        return SimpleColor(200, 200, 50);
    } else if (minutes < 480) {
        // Менее 8 часов - оранжево-жёлтый
        return SimpleColor(220, 150, 50);
    } else if (minutes < 1440) {
        // Менее дня - оранжевый
        return SimpleColor(255, 120, 50);
    } else if (minutes < 2880) {
        // Менее 2 дней - оранжево-красный
        return SimpleColor(255, 100, 50);
    } else if (minutes < 4320) {
        // Менее 3 дней - красный
        return SimpleColor(255, 80, 50);
    } else if (minutes < 10080) {
        // Менее недели - тёмно-красный
        return SimpleColor(200, 50, 50);
    } else if (minutes < 43200) {
        // Менее месяца - бордовый
        return SimpleColor(150, 50, 50);
    } else {
        // Старше месяца - тёмно-бордовый (очень старая правка)
        return SimpleColor(100, 40, 40);
    }
}

std::string StoryEntry::getAgeString() const {
    int64_t minutes = ageInMinutes();
    
    if (minutes < 1) {
        return "Только что";
    } else if (minutes < 60) {
        return std::to_string(minutes) + " мин.";
    } else if (minutes < 1440) {
        int hours = minutes / 60;
        return std::to_string(hours) + " ч.";
    } else if (minutes < 10080) {
        int days = minutes / 1440;
        return std::to_string(days) + " дн.";
    } else if (minutes < 43200) {
        int weeks = minutes / 10080;
        return std::to_string(weeks) + " нед.";
    } else {
        int months = minutes / 43200;
        return std::to_string(months) + " мес.";
    }
}

// ============================================================================
// StoryManager Implementation
// ============================================================================

StoryManager& StoryManager::getInstance() {
    static StoryManager instance;
    return instance;
}

StoryManager::StoryManager() {
}

StoryManager::~StoryManager() {
    // Сохранение всех историй перед закрытием
    for (auto& pair : stories) {
        saveStory(pair.first);
    }
}

void StoryManager::initialize(const std::string& projPath) {
    projectPath = projPath;
    storyDirectory = projectPath + "/.proxima/story";
    ensureStoryDirectory();
    
    LOG_INFO("StoryManager initialized: " + storyDirectory);
}

void StoryManager::ensureStoryDirectory() {
    std::filesystem::create_directories(storyDirectory);
    LOG_DEBUG("Created story directory: " + storyDirectory);
}

std::string StoryManager::getRelativeFilePath(const std::string& filePath) const {
    // Получение относительного пути от корня проекта
    std::string relative = filePath;
    if (relative.find(projectPath) == 0) {
        relative = relative.substr(projectPath.length() + 1);
    }
    
    // Замена разделителей пути
    for (char& c : relative) {
        if (c == '/' || c == '\\') {
            c = '_';
        }
    }
    
    return relative;
}

std::string StoryManager::getStoryFilePath(const std::string& filePath) const {
    std::string relative = getRelativeFilePath(filePath);
    
    // Получение имени файла без расширения
    size_t lastSlash = relative.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos) ? relative.substr(lastSlash + 1) : relative;
    size_t lastDot = filename.find_last_of('.');
    std::string baseName = (lastDot != std::string::npos) ? filename.substr(0, lastDot) : filename;
    
    return storyDirectory + "/" + baseName + ".story";
}

bool StoryManager::loadStory(const std::string& filePath) {
    std::string storyPath = getStoryFilePath(filePath);
    
    // Проверка существования файла
    struct stat buffer;
    bool fileExists = (stat(storyPath.c_str(), &buffer) == 0);
    
    if (!fileExists) {
        LOG_DEBUG("No story file found: " + storyPath);
        stories[filePath].clear();
        currentEntryIndex[filePath] = 0;
        return false;
    }
    
    // Чтение файла
    std::ifstream file(storyPath, std::ios::in);
    if (!file.is_open()) {
        LOG_ERROR("Cannot open story file: " + storyPath);
        return false;
    }
    
    std::stringstream buffer_ss;
    buffer_ss << file.rdbuf();
    std::string content = buffer_ss.str();
    file.close();
    
    // Парсинг Collection формата
    CollectionParser parser;
    CollectionParser::ParseResult result = parser.parse(content);
    
    if (!result.success) {
        LOG_ERROR("Failed to parse story file: " + result.error);
        return false;
    }
    
    // Извлечение записей истории
    std::vector<StoryEntry>& entries = stories[filePath];
    entries.clear();
    
    Collection storyCollection = result.value;
    for (int i = 0; i < storyCollection.size(); i++) {
        Collection entryCollection = storyCollection.get(i);
        StoryEntry entry;
        entry.fromCollection(entryCollection);
        entries.push_back(entry);
    }
    
    currentEntryIndex[filePath] = entries.size();
    
    LOG_INFO("Loaded " + std::to_string(entries.size()) + 
             " story entries for: " + filePath);
    
    return true;
}

bool StoryManager::saveStory(const std::string& filePath) {
    std::string storyPath = getStoryFilePath(filePath);
    
    auto it = stories.find(filePath);
    if (it == stories.end() || it->second.empty()) {
        // Если нет записей, удаляем файл истории
        struct stat buffer;
        if (stat(storyPath.c_str(), &buffer) == 0) {
            std::remove(storyPath.c_str());
        }
        return true;
    }
    
    // Запись в файл
    std::ofstream file(storyPath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("Cannot write story file: " + storyPath);
        return false;
    }
    
    // Сериализация в Collection формат
    file << "[\n";
    
    const std::vector<StoryEntry>& entries = it->second;
    for (size_t i = 0; i < entries.size(); i++) {
        if (i > 0) {
            file << ",,\n";
        }
        
        Collection entryCollection = entries[i].toCollection();
        file << "    " << entryCollection.toCollectionString(1);
    }
    
    file << "\n]\n";
    file.close();
    
    LOG_INFO("Saved " + std::to_string(entries.size()) + 
             " story entries for: " + filePath);
    
    return true;
}

void StoryManager::addEditEntry(const std::string& filePath, EditOperation operation,
                               const std::string& text, int startLine, int endLine) {
    // Проверка на пустой текст (trim)
    std::string trimmed = text;
    size_t first = trimmed.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return;  // Не записываем пустые правки
    }
    
    StoryEntry entry;
    auto now = std::chrono::system_clock::now();
    entry.timestamp = std::chrono::system_clock::to_time_t(now);
    entry.operation = operation;
    entry.text = text;
    entry.startLine = startLine;
    entry.endLine = endLine;
    entry.checksum = calculateChecksum(text);
    
    stories[filePath].push_back(entry);
    currentEntryIndex[filePath] = stories[filePath].size();
    
    // Автосохранение после добавления записи
    saveStory(filePath);
    
    LOG_DEBUG("Added story entry: " + std::string(operation == EditOperation::Add ? "add" : "remove") +
              " at lines " + std::to_string(startLine) + "-" + std::to_string(endLine));
}

std::vector<StoryEntry> StoryManager::getStoryEntries(const std::string& filePath) const {
    auto it = stories.find(filePath);
    if (it != stories.end()) {
        return it->second;
    }
    return std::vector<StoryEntry>();
}

StoryEntry* StoryManager::getEntryForLine(const std::string& filePath, int line) {
    auto it = stories.find(filePath);
    if (it == stories.end()) {
        return nullptr;
    }
    
    std::vector<StoryEntry>& entries = it->second;
    
    // Поиск последней записи, которая затрагивает эту строку
    for (int i = static_cast<int>(entries.size()) - 1; i >= 0; i--) {
        StoryEntry& entry = entries[i];
        if (line >= entry.startLine && line <= entry.endLine) {
            return &entry;
        }
    }
    
    return nullptr;
}

std::map<int, std::time_t> StoryManager::getAgeInfo(const std::string& filePath) {
    std::map<int, std::time_t> ageInfo;
    
    auto it = stories.find(filePath);
    if (it == stories.end()) {
        return ageInfo;
    }
    
    const std::vector<StoryEntry>& entries = it->second;
    
    // Для каждой записи истории определяем строки
    for (const StoryEntry& entry : entries) {
        for (int line = entry.startLine; line <= entry.endLine; line++) {
            // Если для этой строки ещё нет записи или эта запись новее
            auto mapIt = ageInfo.find(line);
            if (mapIt == ageInfo.end() || entry.timestamp > mapIt->second) {
                ageInfo[line] = entry.timestamp;
            }
        }
    }
    
    return ageInfo;
}

bool StoryManager::undoLastEdit(const std::string& filePath) {
    auto indexIt = currentEntryIndex.find(filePath);
    auto storyIt = stories.find(filePath);
    
    if (storyIt == stories.end() || indexIt == currentEntryIndex.end() || indexIt->second <= 0) {
        return false;
    }
    
    indexIt->second--;
    
    // В полной реализации - применение обратной операции
    // Для add -> remove текст, для remove -> add текст
    
    LOG_INFO("Undo last edit for: " + filePath);
    
    return true;
}

bool StoryManager::redoLastEdit(const std::string& filePath) {
    auto indexIt = currentEntryIndex.find(filePath);
    auto storyIt = stories.find(filePath);
    
    if (storyIt == stories.end() || indexIt == currentEntryIndex.end() || 
        indexIt->second >= storyIt->second.size()) {
        return false;
    }
    
    indexIt->second++;
    
    // В полной реализации - применение операции
    
    LOG_INFO("Redo last edit for: " + filePath);
    
    return true;
}

void StoryManager::clearStory(const std::string& filePath) {
    stories.erase(filePath);
    currentEntryIndex.erase(filePath);
    
    std::string storyPath = getStoryFilePath(filePath);
    struct stat buffer;
    if (stat(storyPath.c_str(), &buffer) == 0) {
        std::remove(storyPath.c_str());
    }
    
    LOG_INFO("Cleared story for: " + filePath);
}

bool StoryManager::hasStory(const std::string& filePath) const {
    auto it = stories.find(filePath);
    return it != stories.end() && !it->second.empty();
}

std::string StoryManager::calculateChecksum(const std::string& text) {
    // Простая реализация контрольной суммы (заменяет QCryptographicHash::Md5)
    // Используем алгоритм djb2 для простоты
    unsigned long hash = 5381;
    for (char c : text) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    
    // Конвертация в hex строку
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

} // namespace proxima

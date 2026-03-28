#include "StoryManager.h"
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>

namespace proxima {

// ============================================================================
// StoryManager Implementation
// ============================================================================

StoryManager::StoryManager() {}

StoryManager::~StoryManager() {}

bool StoryManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    entries.clear();
    currentFile = filename;
    
    std::string line;
    while (std::getline(file, line)) {
        // Парсинг JSON-подобного формата
        // Упрощённая реализация - полная версия требует CollectionParser
        if (line.empty() || line[0] == '#') continue;
        
        // TODO: реализовать полноценный парсинг
    }
    
    return true;
}

bool StoryManager::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Proxima Story File\n";
    file << "# Format: [timestamp, operation, text, startLine, endLine, checksum]\n\n";
    
    for (const auto& entry : entries) {
        char timeBuf[64];
        std::strftime(timeBuf, sizeof(timeBuf), "%d/%m/%Y %H:%M", std::localtime(&entry.timestamp));
        
        file << "[" << timeBuf << ", " 
             << (entry.operation == EditOperation::Add ? "add" : "remove") << ", "
             << "\"" << entry.text << "\", "
             << entry.startLine << ", "
             << entry.endLine << ", "
             << "\"" << entry.checksum << "\"]\n";
    }
    
    return true;
}

void StoryManager::addEntry(const std::string& text, EditOperation operation, int startLine, int endLine) {
    StoryEntry entry;
    entry.timestamp = std::time(nullptr);
    entry.operation = operation;
    entry.text = text;
    entry.startLine = startLine;
    entry.endLine = endLine;
    entry.checksum = computeChecksum(text);
    
    entries.push_back(entry);
}

const std::vector<StoryEntry>& StoryManager::getEntries() const {
    return entries;
}

bool StoryManager::revertTo(size_t index) {
    if (index >= entries.size()) {
        return false;
    }
    
    // Удаляем все записи после указанной
    entries.erase(entries.begin() + index + 1, entries.end());
    return true;
}

void StoryManager::clear() {
    entries.clear();
    currentFile.clear();
}

size_t StoryManager::size() const {
    return entries.size();
}

std::string StoryManager::computeChecksum(const std::string& text) const {
    // Простая контрольная сумма (можно заменить на CRC32 или MD5)
    unsigned long hash = 5381;
    for (char c : text) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    
    std::ostringstream oss;
    oss << std::hex << hash;
    return oss.str();
}

} // namespace proxima

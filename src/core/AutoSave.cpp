#include "AutoSave.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/stat.h>
#endif

namespace proxima {

AutoSave::AutoSave() 
    : intervalMinutes(5), autosaveDir("./autosave"), 
      sourceDir("./src"), running(false) {}

AutoSave::~AutoSave() {
    stop();
}

void AutoSave::setAutosaveInterval(int minutes) {
    intervalMinutes = minutes;
}

void AutoSave::setAutosaveDir(const std::string& dir) {
    autosaveDir = dir;
    ensureDirectoryExists(autosaveDir);
}

void AutoSave::setSourceDir(const std::string& dir) {
    sourceDir = dir;
}

void AutoSave::registerFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(filesMutex);
    
    FileState state;
    state.filePath = filePath;
    state.autosavePath = calculateAutosavePath(filePath);
    state.isDirty = false;
    state.isAutosaved = false;
    state.lastEdit = std::chrono::steady_clock::now();
    state.lastAutosave = state.lastEdit;
    
    files[filePath] = state;
}

void AutoSave::unregisterFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(filesMutex);
    files.erase(filePath);
}

void AutoSave::markEdited(const std::string& filePath, const std::string& content) {
    std::lock_guard<std::mutex> lock(filesMutex);
    
    auto it = files.find(filePath);
    if (it != files.end()) {
        it->second.content = content;
        it->second.lastEdit = std::chrono::steady_clock::now();
        it->second.isDirty = true;
        it->second.isAutosaved = false;
    }
}

void AutoSave::saveNow(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(filesMutex);
    
    auto it = files.find(filePath);
    if (it != files.end()) {
        performAutosave(it->second);
    }
}

void AutoSave::saveAll() {
    std::lock_guard<std::mutex> lock(filesMutex);
    
    for (auto& pair : files) {
        performAutosave(pair.second);
    }
}

std::string AutoSave::getAutosavePath(const std::string& filePath) const {
    return calculateAutosavePath(filePath);
}

std::string AutoSave::loadAutosave(const std::string& filePath) const {
    std::string autosavePath = calculateAutosavePath(filePath);
    
    std::ifstream file(autosavePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    return content;
}

bool AutoSave::hasAutosave(const std::string& filePath) const {
    std::string autosavePath = calculateAutosavePath(filePath);
    
    struct stat buffer;
    return (stat(autosavePath.c_str(), &buffer) == 0);
}

void AutoSave::start() {
    if (running) return;
    
    running = true;
    autosaveThread = std::thread(&AutoSave::autosaveLoop, this);
    
    std::cout << "[AutoSave] Started with interval: " << intervalMinutes << " minutes" << std::endl;
}

void AutoSave::stop() {
    if (!running) return;
    
    running = false;
    
    if (autosaveThread.joinable()) {
        autosaveThread.join();
    }
    
    // Final save
    saveAll();
    
    std::cout << "[AutoSave] Stopped" << std::endl;
}

std::vector<FileState> AutoSave::getFileStates() const {
    std::lock_guard<std::mutex> lock(filesMutex);
    
    std::vector<FileState> states;
    for (const auto& pair : files) {
        states.push_back(pair.second);
    }
    return states;
}

void AutoSave::autosaveLoop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
        
        std::lock_guard<std::mutex> lock(filesMutex);
        
        for (auto& pair : files) {
            if (shouldSave(pair.second)) {
                performAutosave(pair.second);
            }
        }
    }
}

bool AutoSave::shouldSave(const FileState& state) const {
    if (!state.isDirty) return false;
    
    auto now = std::chrono::steady_clock::now();
    auto timeSinceEdit = std::chrono::duration_cast<std::chrono::minutes>(
        now - state.lastEdit).count();
    auto timeSinceAutosave = std::chrono::duration_cast<std::chrono::minutes>(
        now - state.lastAutosave).count();
    
    // Save if edited more than 5 minutes ago
    if (timeSinceEdit >= intervalMinutes && !state.isAutosaved) {
        return true;
    }
    
    // One-time save if not edited for 5+ minutes
    if (timeSinceEdit >= intervalMinutes && state.isAutosaved) {
        return false;  // Don't save again until edited
    }
    
    return false;
}

bool AutoSave::performAutosave(FileState& state) {
    ensureDirectoryExists(state.autosavePath);
    
    std::ofstream file(state.autosavePath);
    if (!file.is_open()) {
        std::cerr << "[AutoSave] Failed to save: " << state.autosavePath << std::endl;
        return false;
    }
    
    file << state.content;
    file.close();
    
    state.lastAutosave = std::chrono::steady_clock::now();
    state.isAutosaved = true;
    state.isDirty = false;
    
    std::cout << "[AutoSave] Saved: " << state.filePath << std::endl;
    return true;
}

std::string AutoSave::calculateAutosavePath(const std::string& filePath) const {
    // Convert source path to autosave path
    // e.g., ./src/module.prx -> ./autosave/src/module.prx
    
    std::string relativePath = filePath;
    
    if (filePath.find(sourceDir) == 0) {
        relativePath = filePath.substr(sourceDir.length());
    }
    
    return autosaveDir + relativePath;
}

void AutoSave::ensureDirectoryExists(const std::string& path) {
    // Create directory if it doesn't exist
    size_t pos = 0;
    std::string dir;
    
    while ((pos = path.find('/', pos + 1)) != std::string::npos) {
        dir = path.substr(0, pos);
        mkdir(dir.c_str(), 0755);
    }
    
    mkdir(path.c_str(), 0755);
}

} // namespace proxima
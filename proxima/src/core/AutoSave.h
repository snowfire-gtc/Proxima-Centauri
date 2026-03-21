#ifndef PROXIMA_AUTOSAVE_H
#define PROXIMA_AUTOSAVE_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>

namespace proxima {

struct FileState {
    std::string filePath;
    std::string autosavePath;
    std::string content;
    std::chrono::steady_clock::time_point lastEdit;
    std::chrono::steady_clock::time_point lastAutosave;
    bool isDirty;
    bool isAutosaved;
    
    FileState() : isDirty(false), isAutosaved(false) {}
};

class AutoSave {
public:
    AutoSave();
    ~AutoSave();
    
    void setAutosaveInterval(int minutes);
    void setAutosaveDir(const std::string& dir);
    void setSourceDir(const std::string& dir);
    
    void registerFile(const std::string& filePath);
    void unregisterFile(const std::string& filePath);
    
    void markEdited(const std::string& filePath, const std::string& content);
    void saveNow(const std::string& filePath);
    void saveAll();
    
    std::string getAutosavePath(const std::string& filePath) const;
    std::string loadAutosave(const std::string& filePath) const;
    bool hasAutosave(const std::string& filePath) const;
    
    void start();
    void stop();
    bool isRunning() const { return running; }
    
    std::vector<FileState> getFileStates() const;
    int getAutosaveInterval() const { return intervalMinutes; }
    
private:
    int intervalMinutes;
    std::string autosaveDir;
    std::string sourceDir;
    bool running;
    
    std::map<std::string, FileState> files;
    std::mutex filesMutex;
    std::thread autosaveThread;
    
    void autosaveLoop();
    bool shouldSave(const FileState& state) const;
    bool performAutosave(FileState& state);
    std::string calculateAutosavePath(const std::string& filePath) const;
    void ensureDirectoryExists(const std::string& path);
};

} // namespace proxima

#endif // PROXIMA_AUTOSAVE_H
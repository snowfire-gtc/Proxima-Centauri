#ifndef PROXIMA_LOGGER_H
#define PROXIMA_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace proxima {

enum class LogLevel {
    ERROR = 0,
    WARNING = 1,
    INFO = 2,
    DEBUG = 3,
    TRACE = 4
};

class Logger {
public:
    static Logger& getInstance();
    
    void setLogLevel(LogLevel level);
    LogLevel getLogLevel() const { return currentLevel; }
    
    void setLogFile(const std::string& filename);
    void enableConsoleOutput(bool enable);
    
    void log(LogLevel level, const std::string& message, 
             const std::string& component = "");
    void log(LogLevel level, const std::string& format, ...);
    
    void error(const std::string& message, const std::string& component = "");
    void warning(const std::string& message, const std::string& component = "");
    void info(const std::string& message, const std::string& component = "");
    void debug(const std::string& message, const std::string& component = "");
    void trace(const std::string& message, const std::string& component = "");
    
    std::string getTimestamp() const;
    std::string levelToString(LogLevel level) const;
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    LogLevel currentLevel;
    std::unique_ptr<std::ofstream> logFile;
    bool consoleOutput;
    std::mutex logMutex;
    
    void writeLog(LogLevel level, const std::string& message, 
                  const std::string& component);
};

// Convenience macros
#define LOG_ERROR(msg) proxima::Logger::getInstance().error(msg)
#define LOG_WARNING(msg) proxima::Logger::getInstance().warning(msg)
#define LOG_INFO(msg) proxima::Logger::getInstance().info(msg)
#define LOG_DEBUG(msg) proxima::Logger::getInstance().debug(msg)
#define LOG_TRACE(msg) proxima::Logger::getInstance().trace(msg)

} // namespace proxima

#endif // PROXIMA_LOGGER_H
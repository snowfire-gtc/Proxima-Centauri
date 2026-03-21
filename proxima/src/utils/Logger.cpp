#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdarg>

namespace proxima {

Logger::Logger() 
    : currentLevel(LogLevel::INFO), consoleOutput(true) {}

Logger::~Logger() {
    if (logFile && logFile->is_open()) {
        logFile->close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::setLogFile(const std::string& filename) {
    logFile = std::make_unique<std::ofstream>(filename, std::ios::app);
}

void Logger::enableConsoleOutput(bool enable) {
    consoleOutput = enable;
}

void Logger::log(LogLevel level, const std::string& message, 
                 const std::string& component) {
    if (level > currentLevel) return;
    writeLog(level, message, component);
}

void Logger::log(LogLevel level, const std::string& format, ...) {
    if (level > currentLevel) return;
    
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
    va_end(args);
    
    writeLog(level, std::string(buffer), "");
}

void Logger::error(const std::string& message, const std::string& component) {
    log(LogLevel::ERROR, message, component);
}

void Logger::warning(const std::string& message, const std::string& component) {
    log(LogLevel::WARNING, message, component);
}

void Logger::info(const std::string& message, const std::string& component) {
    log(LogLevel::INFO, message, component);
}

void Logger::debug(const std::string& message, const std::string& component) {
    log(LogLevel::DEBUG, message, component);
}

void Logger::trace(const std::string& message, const std::string& component) {
    log(LogLevel::TRACE, message, component);
}

void Logger::writeLog(LogLevel level, const std::string& message, 
                      const std::string& component) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::ostringstream oss;
    oss << "[" << getTimestamp() << "] ";
    oss << "[" << std::setw(7) << levelToString(level) << "] ";
    
    if (!component.empty()) {
        oss << "[" << component << "] ";
    }
    
    oss << message;
    
    std::string logLine = oss.str();
    
    if (consoleOutput) {
        if (level <= LogLevel::WARNING) {
            std::cerr << logLine << std::endl;
        } else {
            std::cout << logLine << std::endl;
        }
    }
    
    if (logFile && logFile->is_open()) {
        *logFile << logLine << std::endl;
        logFile->flush();
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARNING: return "WARN";
        case LogLevel::INFO: return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

} // namespace proxima
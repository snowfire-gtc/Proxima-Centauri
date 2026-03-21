#ifndef PROXIMA_DEBUGGER_H
#define PROXIMA_DEBUGGER_H

#include "../parser/AST.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace proxima {

enum class BreakpointType {
    UNCONDITIONAL,
    CONDITIONAL,
    TEMPORARY
};

struct Breakpoint {
    int id;
    std::string filename;
    int line;
    BreakpointType type;
    std::string condition;
    int hitCount;
    int maxHits;
    bool enabled;
    
    Breakpoint() : id(0), line(0), type(BreakpointType::UNCONDITIONAL),
                   hitCount(0), maxHits(1), enabled(true) {}
};

struct StackFrame {
    std::string function;
    std::string filename;
    int line;
    std::unordered_map<std::string, std::string> variables;
    std::vector<std::string> parameters;
};

enum class DebugState {
    RUNNING,
    PAUSED,
    STOPPED,
    STEPPING
};

class Debugger {
public:
    Debugger();
    ~Debugger();
    
    // Breakpoint management
    int addBreakpoint(const std::string& filename, int line, 
                     BreakpointType type = BreakpointType::UNCONDITIONAL,
                     const std::string& condition = "",
                     int maxHits = 1);
    void removeBreakpoint(int id);
    void enableBreakpoint(int id, bool enable);
    void clearBreakpoints();
    std::vector<Breakpoint> getBreakpoints() const;
    
    // Execution control
    void start();
    void pause();
    void stop();
    void stepOver();
    void stepInto();
    void stepOut();
    void continueExecution();
    void runToLine(const std::string& filename, int line);
    void runToFunctionEnd();
    void runToNextFunction();
    
    // State queries
    DebugState getState() const { return state; }
    std::string getCurrentFile() const { return currentFile; }
    int getCurrentLine() const { return currentLine; }
    std::vector<StackFrame> getCallStack() const;
    std::unordered_map<std::string, std::string> getLocalVariables() const;
    std::string getVariableValue(const std::string& name) const;
    bool setVariableValue(const std::string& name, const std::string& value);
    
    // Visualization data
    std::vector<double> getVectorData(const std::string& name) const;
    std::vector<std::vector<double>> getMatrixData(const std::string& name) const;
    std::vector<std::vector<std::vector<double>>> getLayerData(const std::string& name) const;
    
    // Event callbacks
    using BreakpointCallback = std::function<void(int breakpointId)>;
    using PauseCallback = std::function<void()>;
    using StopCallback = std::function<void()>;
    
    void onBreakpoint(BreakpointCallback callback);
    void onPause(PauseCallback callback);
    void onStop(StopCallback callback);
    
    // Configuration
    void setVerboseLevel(int level) { verboseLevel = level; }
    int getVerboseLevel() const { return verboseLevel; }
    void setMaxMemory(size_t limit) { maxMemory = limit; }
    size_t getMaxMemory() const { return maxMemory; }
    
    // IDE communication
    std::string serializeState() const;
    void deserializeState(const std::string& data);
    
private:
    DebugState state;
    std::string currentFile;
    int currentLine;
    int verboseLevel;
    size_t maxMemory;
    size_t currentMemoryUsage;
    
    std::vector<Breakpoint> breakpoints;
    std::vector<StackFrame> callStack;
    std::unordered_map<std::string, std::string> variables;
    
    std::mutex debugMutex;
    std::condition_variable pauseCondition;
    bool pauseRequested;
    
    BreakpointCallback onBreakpointCallback;
    PauseCallback onPauseCallback;
    StopCallback onStopCallback;
    
    bool shouldBreak();
    bool evaluateCondition(const std::string& condition);
    void updateCallStack();
    void log(int level, const std::string& message);
};

} // namespace proxima

#endif // PROXIMA_DEBUGGER_H
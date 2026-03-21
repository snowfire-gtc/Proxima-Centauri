#include "Debugger.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace proxima {

Debugger::Debugger() 
    : state(DebugState::STOPPED), currentLine(0), verboseLevel(2),
      maxMemory(4 * 1024 * 1024 * 1024), currentMemoryUsage(0),
      pauseRequested(false) {}

Debugger::~Debugger() {
    stop();
}

int Debugger::addBreakpoint(const std::string& filename, int line,
                           BreakpointType type, const std::string& condition,
                           int maxHits) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    Breakpoint bp;
    bp.id = breakpoints.empty() ? 1 : breakpoints.back().id + 1;
    bp.filename = filename;
    bp.line = line;
    bp.type = type;
    bp.condition = condition;
    bp.maxHits = maxHits;
    bp.enabled = true;
    bp.hitCount = 0;
    
    breakpoints.push_back(bp);
    
    log(3, "Breakpoint added: " + filename + ":" + std::to_string(line));
    
    return bp.id;
}

void Debugger::removeBreakpoint(int id) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    breakpoints.erase(
        std::remove_if(breakpoints.begin(), breakpoints.end(),
            [id](const Breakpoint& bp) { return bp.id == id; }),
        breakpoints.end()
    );
    
    log(3, "Breakpoint removed: " + std::to_string(id));
}

void Debugger::enableBreakpoint(int id, bool enable) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    for (auto& bp : breakpoints) {
        if (bp.id == id) {
            bp.enabled = enable;
            break;
        }
    }
}

void Debugger::clearBreakpoints() {
    std::lock_guard<std::mutex> lock(debugMutex);
    breakpoints.clear();
}

std::vector<Breakpoint> Debugger::getBreakpoints() const {
    return breakpoints;
}

void Debugger::start() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::RUNNING;
    pauseRequested = false;
    log(1, "Debug session started");
}

void Debugger::pause() {
    std::lock_guard<std::mutex> lock(debugMutex);
    pauseRequested = true;
    log(2, "Pause requested");
}

void Debugger::stop() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::STOPPED;
    callStack.clear();
    variables.clear();
    
    if (onStopCallback) {
        onStopCallback();
    }
    
    log(1, "Debug session stopped");
}

void Debugger::stepOver() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::STEPPING;
    log(2, "Step over");
}

void Debugger::stepInto() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::STEPPING;
    log(2, "Step into");
}

void Debugger::stepOut() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::STEPPING;
    log(2, "Step out");
}

void Debugger::continueExecution() {
    std::lock_guard<std::mutex> lock(debugMutex);
    state = DebugState::RUNNING;
    pauseRequested = false;
    pauseCondition.notify_one();
    log(2, "Continue execution");
}

void Debugger::runToLine(const std::string& filename, int line) {
    // Add temporary breakpoint
    int bpId = addBreakpoint(filename, line, BreakpointType::TEMPORARY, "", 1);
    continueExecution();
}

void Debugger::runToFunctionEnd() {
    // Implementation would need function end line tracking
    stepOut();
}

void Debugger::runToNextFunction() {
    // Implementation would need function call tracking
    stepInto();
}

std::vector<StackFrame> Debugger::getCallStack() const {
    return callStack;
}

std::unordered_map<std::string, std::string> Debugger::getLocalVariables() const {
    if (!callStack.empty()) {
        return callStack.back().variables;
    }
    return variables;
}

std::string Debugger::getVariableValue(const std::string& name) const {
    auto vars = getLocalVariables();
    auto it = vars.find(name);
    if (it != vars.end()) {
        return it->second;
    }
    return "";
}

bool Debugger::setVariableValue(const std::string& name, const std::string& value) {
    std::lock_guard<std::mutex> lock(debugMutex);
    variables[name] = value;
    log(3, "Variable set: " + name + " = " + value);
    return true;
}

std::vector<double> Debugger::getVectorData(const std::string& name) const {
    // Would parse variable value as vector
    std::vector<double> data;
    // Implementation depends on variable storage format
    return data;
}

std::vector<std::vector<double>> Debugger::getMatrixData(const std::string& name) const {
    // Would parse variable value as matrix
    std::vector<std::vector<double>> data;
    // Implementation depends on variable storage format
    return data;
}

std::vector<std::vector<std::vector<double>>> Debugger::getLayerData(const std::string& name) const {
    // Would parse variable value as layer
    std::vector<std::vector<std::vector<double>>> data;
    // Implementation depends on variable storage format
    return data;
}

void Debugger::onBreakpoint(BreakpointCallback callback) {
    onBreakpointCallback = callback;
}

void Debugger::onPause(PauseCallback callback) {
    onPauseCallback = callback;
}

void Debugger::onStop(StopCallback callback) {
    onStopCallback = callback;
}

bool Debugger::shouldBreak() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (pauseRequested) {
        return true;
    }
    
    if (state != DebugState::RUNNING && state != DebugState::STEPPING) {
        return false;
    }
    
    // Check breakpoints
    for (auto& bp : breakpoints) {
        if (!bp.enabled) continue;
        if (bp.filename != currentFile) continue;
        if (bp.line != currentLine) continue;
        
        bp.hitCount++;
        
        if (bp.type == BreakpointType::CONDITIONAL) {
            if (!evaluateCondition(bp.condition)) {
                continue;
            }
        }
        
        if (bp.hitCount >= bp.maxHits) {
            if (bp.type == BreakpointType::TEMPORARY) {
                bp.enabled = false;
            }
        }
        
        if (onBreakpointCallback) {
            onBreakpointCallback(bp.id);
        }
        
        return true;
    }
    
    return false;
}

bool Debugger::evaluateCondition(const std::string& condition) {
    // Simple condition evaluator
    // Full implementation would parse and evaluate expressions
    return true;
}

void Debugger::updateCallStack() {
    // Would be called during execution to update call stack
}

void Debugger::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Debugger] " << message << std::endl;
    }
}

std::string Debugger::serializeState() const {
    std::ostringstream oss;
    oss << "{\"state\":\"";
    switch (state) {
        case DebugState::RUNNING: oss << "running"; break;
        case DebugState::PAUSED: oss << "paused"; break;
        case DebugState::STOPPED: oss << "stopped"; break;
        case DebugState::STEPPING: oss << "stepping"; break;
    }
    oss << "\",\"file\":\"" << currentFile << "\",\"line\":" << currentLine << "}";
    return oss.str();
}

void Debugger::deserializeState(const std::string& data) {
    // Parse JSON-like state data
    // Implementation would restore debugger state
}

} // namespace proxima
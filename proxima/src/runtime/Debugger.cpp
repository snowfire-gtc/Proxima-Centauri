#include "Debugger.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <chrono>
#include <thread>
#include "utils/Logger.h"
#include "stdlib/Collection.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

Debugger::Debugger() 
    : state(DebugState::STOPPED)
    , currentLine(0)
    , verboseLevel(2)
    , maxMemory(4 * 1024 * 1024 * 1024)
    , currentMemoryUsage(0)
    , pauseRequested(false)
    , nextBreakpointId(1) {
    
    LOG_INFO("Debugger initialized");
}

Debugger::~Debugger() {
    stop();
    LOG_INFO("Debugger destroyed");
}

// ============================================================================
// Управление точками останова
// ============================================================================

int Debugger::addBreakpoint(const std::string& filename, int line, 
                           BreakpointType type, const std::string& condition,
                           int maxHits) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    Breakpoint bp;
    bp.id = nextBreakpointId++;
    bp.filename = filename;
    bp.line = line;
    bp.type = type;
    bp.condition = condition;
    bp.maxHits = maxHits;
    bp.enabled = true;
    bp.hitCount = 0;
    bp.lastHitTime = "";
    
    breakpoints.push_back(bp);
    
    log(3, "Breakpoint added: " + filename + ":" + std::to_string(line) + 
            " (ID: " + std::to_string(bp.id) + ")");
    
    return bp.id;
}

void Debugger::removeBreakpoint(int id) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(),
        [id](const Breakpoint& bp) { return bp.id == id; });
    
    if (it != breakpoints.end()) {
        log(3, "Breakpoint removed: ID " + std::to_string(id));
        breakpoints.erase(it);
    }
}

void Debugger::enableBreakpoint(int id, bool enable) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    for (auto& bp : breakpoints) {
        if (bp.id == id) {
            bp.enabled = enable;
            log(3, "Breakpoint " + std::to_string(id) + 
                    (enable ? " enabled" : " disabled"));
            break;
        }
    }
}

void Debugger::clearBreakpoints() {
    std::lock_guard<std::mutex> lock(debugMutex);
    breakpoints.clear();
    log(2, "All breakpoints cleared");
}

void Debugger::clearFileBreakpoints(const std::string& filename) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    breakpoints.erase(
        std::remove_if(breakpoints.begin(), breakpoints.end(),
            [&filename](const Breakpoint& bp) { return bp.filename == filename; }),
        breakpoints.end()
    );
    
    log(2, "Breakpoints cleared for file: " + filename);
}

std::vector<Breakpoint> Debugger::getBreakpoints() const {
    return breakpoints;
}

bool Debugger::hasBreakpoint(const std::string& filename, int line) const {
    for (const auto& bp : breakpoints) {
        if (bp.filename == filename && bp.line == line && bp.enabled) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Контроль выполнения
// ============================================================================

void Debugger::start() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state == DebugState::RUNNING) {
        log(2, "Debugger already running");
        return;
    }
    
    state = DebugState::RUNNING;
    pauseRequested = false;
    currentMemoryUsage = 0;
    
    log(1, "Debug session started");
}

void Debugger::pause() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::RUNNING) {
        log(2, "Cannot pause - debugger not running");
        return;
    }
    
    pauseRequested = true;
    log(2, "Pause requested");
}

void Debugger::stop() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    state = DebugState::STOPPED;
    pauseRequested = false;
    currentLine = 0;
    currentFile = "";
    
    // Очистка стека вызовов
    callStack.clear();
    
    // Очистка переменных
    variables.clear();
    
    // Вызов callback
    if (onStopCallback) {
        onStopCallback();
    }
    
    log(1, "Debug session stopped");
}

void Debugger::stepOver() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::PAUSED) {
        log(2, "Cannot step over - debugger not paused");
        return;
    }
    
    stepMode = StepMode::Over;
    targetDepth = callStack.size();
    state = DebugState::STEPPING;
    
    log(2, "Step over initiated");
    pauseCondition.notify_one();
}

void Debugger::stepInto() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::PAUSED) {
        log(2, "Cannot step into - debugger not paused");
        return;
    }
    
    stepMode = StepMode::Into;
    targetDepth = callStack.size() + 1;
    state = DebugState::STEPPING;
    
    log(2, "Step into initiated");
    pauseCondition.notify_one();
}

void Debugger::stepOut() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::PAUSED) {
        log(2, "Cannot step out - debugger not paused");
        return;
    }
    
    stepMode = StepMode::Out;
    targetDepth = callStack.size() - 1;
    state = DebugState::STEPPING;
    
    log(2, "Step out initiated");
    pauseCondition.notify_one();
}

void Debugger::continueExecution() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::PAUSED) {
        log(2, "Cannot continue - debugger not paused");
        return;
    }
    
    state = DebugState::RUNNING;
    pauseRequested = false;
    
    log(2, "Execution continued");
    pauseCondition.notify_one();
}

void Debugger::runToLine(const std::string& filename, int line) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (state != DebugState::PAUSED) {
        log(2, "Cannot run to line - debugger not paused");
        return;
    }
    
    // Добавляем временную точку останова
    Breakpoint bp;
    bp.id = nextBreakpointId++;
    bp.filename = filename;
    bp.line = line;
    bp.type = BreakpointType::TEMPORARY;
    bp.enabled = true;
    bp.maxHits = 1;
    bp.hitCount = 0;
    
    breakpoints.push_back(bp);
    
    state = DebugState::RUNNING;
    
    log(2, "Run to line: " + filename + ":" + std::to_string(line));
    pauseCondition.notify_one();
}

void Debugger::runToFunctionEnd() {
    stepOut();
}

void Debugger::runToNextFunction() {
    stepInto();
}

// ============================================================================
// Запросы состояния
// ============================================================================

DebugState Debugger::getState() const {
    return state;
}

std::string Debugger::getCurrentFile() const {
    return currentFile;
}

int Debugger::getCurrentLine() const {
    return currentLine;
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
    
    if (!callStack.empty()) {
        callStack.back().variables[name] = value;
    }
    
    log(3, "Variable set: " + name + " = " + value);
    return true;
}

// ============================================================================
// Данные для визуализации
// ============================================================================

std::vector<double> Debugger::getVectorData(const std::string& name) const {
    std::vector<double> data;
    
    std::string value = getVariableValue(name);
    if (value.empty()) return data;
    
    // Парсинг формата вектора: [1.0, 2.0, 3.0, ...]
    std::regex vecRegex(R"(\[([^\]]+)\])");
    std::smatch match;
    
    if (std::regex_search(value, match, vecRegex)) {
        std::string content = match[1].str();
        std::stringstream ss(content);
        std::string item;
        
        while (std::getline(ss, item, ',')) {
            try {
                data.push_back(std::stod(item));
            } catch (...) {
                // Пропускаем нечисловые значения
            }
        }
    }
    
    return data;
}

std::vector<std::vector<double>> Debugger::getMatrixData(const std::string& name) const {
    std::vector<std::vector<double>> data;
    
    std::string value = getVariableValue(name);
    if (value.empty()) return data;
    
    // Парсинг формата матрицы: [[1,2],[3,4]] или [1,2,,3,4]
    // Упрощённая реализация
    std::vector<double> flatData = getVectorData(name);
    
    if (!flatData.empty()) {
        // Предполагаем квадратную матрицу для простоты
        size_t size = static_cast<size_t>(std::sqrt(flatData.size()));
        if (size * size == flatData.size()) {
            for (size_t i = 0; i < size; i++) {
                std::vector<double> row;
                for (size_t j = 0; j < size; j++) {
                    row.push_back(flatData[i * size + j]);
                }
                data.push_back(row);
            }
        }
    }
    
    return data;
}

std::vector<std::vector<std::vector<double>>> Debugger::getLayerData(const std::string& name) const {
    std::vector<std::vector<std::vector<double>>> data;
    
    // Упрощённая реализация для слоя
    std::string value = getVariableValue(name);
    if (value.empty()) return data;
    
    // Парсинг формата слоя: [[[1,2],[3,4]],[[5,6],[7,8]]]
    // Полная реализация требует парсера Proxima
    
    return data;
}

// ============================================================================
// Callbacks
// ============================================================================

void Debugger::onBreakpoint(BreakpointCallback callback) {
    onBreakpointCallback = callback;
}

void Debugger::onPause(PauseCallback callback) {
    onPauseCallback = callback;
}

void Debugger::onStop(StopCallback callback) {
    onStopCallback = callback;
}

// ============================================================================
// Конфигурация
// ============================================================================

void Debugger::setVerboseLevel(int level) {
    verboseLevel = level;
    log(3, "Verbose level set to: " + std::to_string(level));
}

int Debugger::getVerboseLevel() const {
    return verboseLevel;
}

void Debugger::setMaxMemory(size_t limit) {
    maxMemory = limit;
    log(2, "Max memory set to: " + std::to_string(limit) + " bytes");
}

size_t Debugger::getMaxMemory() const {
    return maxMemory;
}

// ============================================================================
// Сериализация состояния для IDE
// ============================================================================

std::string Debugger::serializeState() const {
    std::ostringstream oss;
    
    oss << "[\n";
    oss << "    \"state\", \"";
    
    switch (state) {
        case DebugState::RUNNING: oss << "running"; break;
        case DebugState::PAUSED: oss << "paused"; break;
        case DebugState::STOPPED: oss << "stopped"; break;
        case DebugState::STEPPING: oss << "stepping"; break;
    }
    
    oss << "\",,\n";
    oss << "    \"file\", \"" << currentFile << "\",,\n";
    oss << "    \"line\", " << currentLine << ",,\n";
    oss << "    \"breakpoints\", [\n";
    
    for (size_t i = 0; i < breakpoints.size(); i++) {
        const auto& bp = breakpoints[i];
        oss << "        [\n";
        oss << "            \"id\", " << bp.id << ",,\n";
        oss << "            \"file\", \"" << bp.filename << "\",,\n";
        oss << "            \"line\", " << bp.line << ",,\n";
        oss << "            \"enabled\", " << (bp.enabled ? "true" : "false") << ",,\n";
        oss << "            \"hitCount\", " << bp.hitCount << "\n";
        oss << "        ]";
        if (i < breakpoints.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "    ],,\n";
    oss << "    \"variables\", [\n";
    
    size_t varCount = 0;
    for (const auto& var : variables) {
        oss << "        [\n";
        oss << "            \"name\", \"" << var.first << "\",,\n";
        oss << "            \"value\", \"" << var.second << "\"\n";
        oss << "        ]";
        if (varCount < variables.size() - 1) oss << ",";
        oss << "\n";
        varCount++;
    }
    
    oss << "    ]\n";
    oss << "]";
    
    return oss.str();
}

void Debugger::deserializeState(const std::string& data) {
    // Парсинг состояния из формата collection
    // Упрощённая реализация
    
    std::regex stateRegex(R"("state",\s*"(\w+)")");
    std::regex fileRegex(R"("file",\s*"([^"]*)")");
    std::regex lineRegex(R"("line",\s*(\d+))");
    
    std::smatch match;
    
    if (std::regex_search(data, match, stateRegex)) {
        std::string stateStr = match[1].str();
        if (stateStr == "running") state = DebugState::RUNNING;
        else if (stateStr == "paused") state = DebugState::PAUSED;
        else if (stateStr == "stopped") state = DebugState::STOPPED;
        else if (stateStr == "stepping") state = DebugState::STEPPING;
    }
    
    if (std::regex_search(data, match, fileRegex)) {
        currentFile = match[1].str();
    }
    
    if (std::regex_search(data, match, lineRegex)) {
        currentLine = std::stoi(match[1].str());
    }
    
    log(3, "State deserialized");
}

// ============================================================================
// Внутренние методы
// ============================================================================

bool Debugger::shouldBreak() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    // Проверка запроса паузы
    if (pauseRequested) {
        return true;
    }
    
    // Проверка режима шага
    if (state == DebugState::STEPPING) {
        if (stepMode == StepMode::Over && callStack.size() <= targetDepth) {
            return true;
        } else if (stepMode == StepMode::Into && callStack.size() >= targetDepth) {
            return true;
        } else if (stepMode == StepMode::Out && callStack.size() <= targetDepth) {
            return true;
        }
    }
    
    // Проверка точек останова
    for (auto& bp : breakpoints) {
        if (!bp.enabled) continue;
        if (bp.filename != currentFile) continue;
        if (bp.line != currentLine) continue;
        
        bp.hitCount++;
        
        // Проверка условия
        if (bp.type == BreakpointType::CONDITIONAL) {
            if (!evaluateCondition(bp.condition)) {
                continue;
            }
        }
        
        // Проверка максимального количества срабатываний
        if (bp.hitCount >= bp.maxHits) {
            if (bp.type == BreakpointType::TEMPORARY) {
                bp.enabled = false;
            }
        }
        
        bp.lastHitTime = getCurrentTimestamp();
        
        // Вызов callback
        if (onBreakpointCallback) {
            onBreakpointCallback(bp.id);
        }
        
        log(2, "Breakpoint hit: " + bp.filename + ":" + std::to_string(bp.line) + 
                " (ID: " + std::to_string(bp.id) + ")");
        
        return true;
    }
    
    return false;
}

bool Debugger::evaluateCondition(const std::string& condition) {
    // Упрощённый_evaluator условий
    // В полной реализации нужен парсер выражений Proxima
    
    if (condition.empty()) return true;
    
    // Пример: "x > 10", "y == 5", "flag != 0"
    std::regex condRegex(R"((\w+)\s*(==|!=|>|<|>=|<=)\s*(\d+|\w+))");
    std::smatch match;
    
    if (std::regex_search(condition, match, condRegex)) {
        std::string varName = match[1].str();
        std::string op = match[2].str();
        std::string value = match[3].str();
        
        std::string varValue = getVariableValue(varName);
        
        try {
            int varInt = std::stoi(varValue);
            int condInt = std::stoi(value);
            
            if (op == "==") return varInt == condInt;
            if (op == "!=") return varInt != condInt;
            if (op == ">") return varInt > condInt;
            if (op == "<") return varInt < condInt;
            if (op == ">=") return varInt >= condInt;
            if (op == "<=") return varInt <= condInt;
        } catch (...) {
            // Строковое сравнение
            if (op == "==") return varValue == value;
            if (op == "!=") return varValue != value;
        }
    }
    
    return false;
}

void Debugger::updateCallStack() {
    // Обновление стека вызовов
    // В полной реализации интегрируется с runtime
    
    log(4, "Call stack updated");
}

void Debugger::pushStackFrame(const std::string& function, const std::string& filename, int line) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    StackFrame frame;
    frame.function = function;
    frame.filename = filename;
    frame.line = line;
    
    callStack.push_back(frame);
    
    log(4, "Stack frame pushed: " + function + " at " + filename + ":" + std::to_string(line));
}

void Debugger::popStackFrame() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (!callStack.empty()) {
        callStack.pop_back();
        log(4, "Stack frame popped");
    }
}

void Debugger::setCurrentLocation(const std::string& filename, int line) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    currentFile = filename;
    currentLine = line;
}

void Debugger::addVariable(const std::string& name, const std::string& value) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    variables[name] = value;
    
    if (!callStack.empty()) {
        callStack.back().variables[name] = value;
    }
}

void Debugger::removeVariable(const std::string& name) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    variables.erase(name);
    
    if (!callStack.empty()) {
        callStack.back().variables.erase(name);
    }
}

void Debugger::clearVariables() {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    variables.clear();
    
    for (auto& frame : callStack) {
        frame.variables.clear();
    }
}

void Debugger::recordMemoryAllocation(size_t size) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    currentMemoryUsage += size;
    
    if (currentMemoryUsage > maxMemory) {
        log(1, "WARNING: Memory limit exceeded!");
    }
}

void Debugger::recordMemoryDeallocation(size_t size) {
    std::lock_guard<std::mutex> lock(debugMutex);
    
    if (size <= currentMemoryUsage) {
        currentMemoryUsage -= size;
    }
}

size_t Debugger::getCurrentMemoryUsage() const {
    return currentMemoryUsage;
}

std::string Debugger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

void Debugger::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Debugger] " << message << std::endl;
        LOG_INFO("[Debugger] " + message);
    }
}

// ============================================================================
// Интеграция с Runtime
// ============================================================================

void Debugger::notifyFunctionEntry(const std::string& function, 
                                   const std::string& filename, 
                                   int line) {
    pushStackFrame(function, filename, line);
    setCurrentLocation(filename, line);
    
    if (state == DebugState::STEPPING && stepMode == StepMode::Into) {
        state = DebugState::PAUSED;
        if (onPauseCallback) onPauseCallback();
    }
}

void Debugger::notifyFunctionExit(const std::string& function) {
    popStackFrame();
    
    if (state == DebugState::STEPPING && stepMode == StepMode::Out) {
        if (callStack.size() <= static_cast<size_t>(targetDepth)) {
            state = DebugState::PAUSED;
            if (onPauseCallback) onPauseCallback();
        }
    }
}

void Debugger::notifyLineExecution(const std::string& filename, int line) {
    setCurrentLocation(filename, line);
    
    if (shouldBreak()) {
        state = DebugState::PAUSED;
        
        if (onPauseCallback) {
            onPauseCallback();
        }
        
        // Ожидание команды продолжения
        std::unique_lock<std::mutex> lock(debugMutex);
        pauseCondition.wait(lock, [this]() {
            return state != DebugState::PAUSED || pauseRequested;
        });
    }
}

void Debugger::notifyVariableChange(const std::string& name, const std::string& value) {
    addVariable(name, value);
}

// ============================================================================
// Экспорт/Импорт конфигурации точек останова
// ============================================================================

void Debugger::saveBreakpoints(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        log(1, "Failed to save breakpoints to: " + filename);
        return;
    }
    
    file << "# Proxima Debugger Breakpoints\n";
    file << "# Format: filename:line:type:condition:maxHits\n\n";
    
    for (const auto& bp : breakpoints) {
        file << bp.filename << ":" 
             << bp.line << ":"
             << bp.type << ":"
             << bp.condition << ":"
             << bp.maxHits << "\n";
    }
    
    file.close();
    log(2, "Breakpoints saved to: " + filename);
}

void Debugger::loadBreakpoints(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        log(1, "Failed to load breakpoints from: " + filename);
        return;
    }
    
    clearBreakpoints();
    
    std::string line;
    while (std::getline(file, line)) {
        // Пропуск комментариев и пустых строк
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        
        while (std::getline(iss, token, ':')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 2) {
            std::string bpFile = tokens[0];
            int bpLine = std::stoi(tokens[1]);
            BreakpointType bpType = BreakpointType::UNCONDITIONAL;
            std::string condition = "";
            int maxHits = 1;
            
            if (tokens.size() >= 3) {
                int typeVal = std::stoi(tokens[2]);
                bpType = static_cast<BreakpointType>(typeVal);
            }
            if (tokens.size() >= 4) {
                condition = tokens[3];
            }
            if (tokens.size() >= 5) {
                maxHits = std::stoi(tokens[4]);
            }
            
            addBreakpoint(bpFile, bpLine, bpType, condition, maxHits);
        }
    }
    
    file.close();
    log(2, "Breakpoints loaded from: " + filename);
}

} // namespace proxima
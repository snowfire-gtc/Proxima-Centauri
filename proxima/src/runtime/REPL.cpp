#include "REPL.h"
#include <ctime>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "utils/Logger.h"
#include "stdlib/Collection.h"
#include "stdlib/Math.h"
#include "stdlib/Time.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

REPL::REPL(Interpreter& interp)
    : interpreter(interp)
    , running(false) {
    
    running = false;
    
    LOG_INFO("REPL initialized (Proxima-native)");
}

REPL::~REPL() {
    stop();
    LOG_INFO("REPL destroyed");
}

// ============================================================================
// Запуск/Остановка REPL
// ============================================================================

void REPL::start() {
    running = true;
    std::cout << "Proxima REPL v1.0.0\n";
    std::cout << "Type 'help' for available commands.\n";
    std::cout << "Type 'exit' to quit.\n\n";
    
    while (running) {
        std::cout << ">> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        // Проверяем, является ли ввод командой
        REPLCommand cmd = parseCommand(input);
        if (cmd != REPLCommand::UNKNOWN && input.find('=') == std::string::npos && 
            input.find('(') == std::string::npos) {
            std::string result = handleCommand(input);
            if (!result.empty()) {
                std::cout << result << "\n";
            }
            
            if (cmd == REPLCommand::EXIT) {
                break;
            }
        } else {
            // Это код Proxima
            std::string output = executeCode(input);
            if (!output.empty()) {
                std::cout << output << "\n";
            }
        }
    }
}

void REPL::stop() {
    running = false;
}

// ============================================================================
// Обработка команд
// ============================================================================

std::string REPL::handleCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    
    // Получаем аргументы
    std::string args;
    std::getline(iss >> std::ws, args);
    
    REPLCommand command = parseCommand(cmd);
    
    switch (command) {
        case REPLCommand::HELP:
            return handleHelp();
        case REPLCommand::EXIT:
            return handleExit();
        case REPLCommand::CLEAR:
            return handleClear();
        case REPLCommand::HISTORY:
            return handleHistory();
        case REPLCommand::LOAD:
            return handleLoad(args);
        case REPLCommand::SAVE:
            return handleSave(args);
        case REPLCommand::WHOIS:
            return handleWhois(args);
        case REPLCommand::WHOS:
            return handleWhos();
        case REPLCommand::SHOW:
            return handleShow(args);
        case REPLCommand::TYPE:
            return handleType(args);
        case REPLCommand::SIZE:
            return handleSize(args);
        case REPLCommand::METHODS:
            return handleMethods(args);
        case REPLCommand::FIELDS:
            return handleFields(args);
        case REPLCommand::VERSION:
            return handleVersion();
        case REPLCommand::CONFIG:
            return handleConfig();
        case REPLCommand::RESET:
            return handleReset();
        default:
            return "Unknown command: " + cmd + ". Type 'help' for available commands.";
    }
}

std::string REPL::executeCode(const std::string& code) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Выполнение кода через интерпретатор
        // Здесь должна быть логика выполнения кода Proxima
        // Для простоты возвращаем заглушку
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        addToHistory(code, "", false, duration.count());
        
        return ""; // Пустой результат - код выполнен успешно
    } catch (const std::exception& e) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        addToHistory(code, e.what(), true, duration.count());
        
        return std::string("Error: ") + e.what();
    }
}

REPLCommand REPL::parseCommand(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;
    
    // Приводим к нижнему регистру
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "help" || cmd == "?") return REPLCommand::HELP;
    if (cmd == "exit" || cmd == "quit") return REPLCommand::EXIT;
    if (cmd == "clear") return REPLCommand::CLEAR;
    if (cmd == "history") return REPLCommand::HISTORY;
    if (cmd == "load") return REPLCommand::LOAD;
    if (cmd == "save") return REPLCommand::SAVE;
    if (cmd == "whois") return REPLCommand::WHOIS;
    if (cmd == "whos") return REPLCommand::WHOS;
    if (cmd == "show") return REPLCommand::SHOW;
    if (cmd == "type") return REPLCommand::TYPE;
    if (cmd == "size") return REPLCommand::SIZE;
    if (cmd == "methods") return REPLCommand::METHODS;
    if (cmd == "fields") return REPLCommand::FIELDS;
    if (cmd == "version") return REPLCommand::VERSION;
    if (cmd == "config") return REPLCommand::CONFIG;
    if (cmd == "reset") return REPLCommand::RESET;
    
    return REPLCommand::UNKNOWN;
}

void REPL::addToHistory(const std::string& code, const std::string& output, bool hasError, long long execTime) {
    REPLHistoryEntry entry;
    entry.code = code;
    entry.output = output;
    entry.timestamp = std::time(nullptr);
    entry.hasError = hasError;
    entry.executionTime = execTime;
    
    history.push_back(entry);
    
    // Ограничиваем историю 100 записями
    if (history.size() > 100) {
        history.erase(history.begin());
    }
}

const std::vector<REPLHistoryEntry>& REPL::getHistory() const {
    return history;
}

void REPL::clearHistory() {
    history.clear();
}

std::vector<REPLVariable> REPL::getVariables() const {
    // Возвращаем список переменных из интерпретатора
    return std::vector<REPLVariable>();
}

REPLVariable REPL::getVariableInfo(const std::string& name) const {
    // Возвращаем информацию о переменной
    REPLVariable var;
    var.name = name;
    var.type = "unknown";
    var.size = 0;
    var.bytes = 0;
    return var;
}

// ============================================================================
// Обработчики команд
// ============================================================================

std::string REPL::handleHelp() {
    std::ostringstream oss;
    oss << "Available commands:\n";
    oss << "  help, ?          - Show this help message\n";
    oss << "  exit, quit       - Exit REPL\n";
    oss << "  clear            - Clear screen\n";
    oss << "  history          - Show command history\n";
    oss << "  load <file>      - Load and execute a file\n";
    oss << "  save <file>      - Save history to file\n";
    oss << "  whois <var>      - Show variable info\n";
    oss << "  whos             - List all variables\n";
    oss << "  show <expr>      - Visualize expression\n";
    oss << "  type <var>       - Show variable type\n";
    oss << "  size <var>       - Show variable size\n";
    oss << "  methods <class>  - Show class methods\n";
    oss << "  fields <class>   - Show class fields\n";
    oss << "  version          - Show version\n";
    oss << "  config           - Show configuration\n";
    oss << "  reset            - Reset REPL state";
    return oss.str();
}

std::string REPL::handleExit() {
    running = false;
    return "Goodbye!";
}

std::string REPL::handleClear() {
    // Очистка экрана (platform-specific)
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    return "";
}

std::string REPL::handleHistory() {
    std::ostringstream oss;
    int i = 1;
    for (const auto& entry : history) {
        oss << i++ << ": " << entry.code;
        if (entry.hasError) {
            oss << " [ERROR]";
        }
        oss << " (" << entry.executionTime << "ms)\n";
    }
    return oss.str();
}

std::string REPL::handleLoad(const std::string& filename) {
    if (filename.empty()) {
        return "Usage: load <filename>";
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "Error: Cannot open file '" + filename + "'";
    }
    
    std::string line;
    std::ostringstream output;
    while (std::getline(file, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') continue;
        
        std::string result = executeCode(line);
        if (!result.empty()) {
            output << result << "\n";
        }
    }
    
    return output.str();
}

std::string REPL::handleSave(const std::string& filename) {
    if (filename.empty()) {
        return "Usage: save <filename>";
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return "Error: Cannot create file '" + filename + "'";
    }
    
    for (const auto& entry : history) {
        file << "# " << std::ctime(&entry.timestamp);
        file << entry.code << "\n";
        if (!entry.output.empty()) {
            file << "# Output: " << entry.output << "\n";
        }
    }
    
    return "History saved to '" + filename + "'";
}

std::string REPL::handleWhois(const std::string& varName) {
    if (varName.empty()) {
        return "Usage: whois <variable_name>";
    }
    
    REPLVariable var = getVariableInfo(varName);
    std::ostringstream oss;
    oss << "Variable: " << var.name << "\n";
    oss << "Type: " << var.type << "\n";
    oss << "Size: " << var.size << "\n";
    oss << "Bytes: " << var.bytes;
    return oss.str();
}

std::string REPL::handleWhos() {
    std::vector<REPLVariable> vars = getVariables();
    if (vars.empty()) {
        return "No variables defined.";
    }
    
    std::ostringstream oss;
    oss << "Variables:\n";
    for (const auto& var : vars) {
        oss << "  " << var.name << " (" << var.type << ")\n";
    }
    return oss.str();
}

std::string REPL::handleShow(const std::string& expr) {
    if (expr.empty()) {
        return "Usage: show <expression>";
    }
    
    // Визуализация выражения
    return "Showing: " + expr;
}

std::string REPL::handleType(const std::string& varName) {
    if (varName.empty()) {
        return "Usage: type <variable_name>";
    }
    
    REPLVariable var = getVariableInfo(varName);
    return "Type of " + varName + ": " + var.type;
}

std::string REPL::handleSize(const std::string& varName) {
    if (varName.empty()) {
        return "Usage: size <variable_name>";
    }
    
    REPLVariable var = getVariableInfo(varName);
    return "Size of " + varName + ": " + std::to_string(var.size);
}

std::string REPL::handleMethods(const std::string& className) {
    if (className.empty()) {
        return "Usage: methods <class_name>";
    }
    
    return "Methods of " + className + ": (not implemented)";
}

std::string REPL::handleFields(const std::string& className) {
    if (className.empty()) {
        return "Usage: fields <class_name>";
    }
    
    return "Fields of " + className + ": (not implemented)";
}

std::string REPL::handleVersion() {
    return "Proxima REPL v1.0.0";
}

std::string REPL::handleConfig() {
    std::ostringstream oss;
    oss << "Configuration:\n";
    oss << "  History size: " << history.size() << "/100\n";
    oss << "  Variables: " << getVariables().size();
    return oss.str();
}

std::string REPL::handleReset() {
    clearHistory();
    // Сброс переменных интерпретатора
    return "REPL state reset.";
}

} // namespace proxima

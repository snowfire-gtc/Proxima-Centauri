#include "REPL.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <sys/stat.h>
#include "utils/Logger.h"
#include "stdlib/Collection.h"
#include "stdlib/Math.h"
#include "stdlib/Time.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

REPL::REPL()
    : initialized(false)
    , running(false)
    , echoInput(true)
    , prompt(">> ")
    , runtime(nullptr)
    , debugger(nullptr)
    , typeChecker(nullptr)
    , ideInterface(nullptr)
    , startTime(0)
    , commandCount(0) {
    
    // Получение текущей директории
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        currentPath = std::string(cwd);
    }
    
    // Регистрация команд
    commands["help"] = REPLCommand::HELP;
    commands["?"] = REPLCommand::HELP;
    commands["exit"] = REPLCommand::EXIT;
    commands["quit"] = REPLCommand::EXIT;
    commands["clear"] = REPLCommand::CLEAR;
    commands["history"] = REPLCommand::HISTORY;
    commands["load"] = REPLCommand::LOAD;
    commands["save"] = REPLCommand::SAVE;
    commands["whois"] = REPLCommand::WHOIS;
    commands["whos"] = REPLCommand::WHOS;
    commands["show"] = REPLCommand::SHOW;
    commands["type"] = REPLCommand::TYPE;
    commands["size"] = REPLCommand::SIZE;
    commands["methods"] = REPLCommand::METHODS;
    commands["fields"] = REPLCommand::FIELDS;
    commands["version"] = REPLCommand::VERSION;
    commands["config"] = REPLCommand::CONFIG;
    commands["reset"] = REPLCommand::RESET;
    
    LOG_INFO("REPL initialized (Proxima-native)");
}

REPL::~REPL() {
    shutdown();
    LOG_INFO("REPL destroyed");
}

// ============================================================================
// Инициализация
// ============================================================================

bool REPL::initialize() {
    if (initialized) {
        return true;
    }
    
    // Инициализация runtime
    if (!runtime) {
        runtime = new Runtime();
        runtime->initialize();
    }
    
    // Инициализация type checker
    if (!typeChecker) {
        typeChecker = new TypeChecker();
    }
    
    // Инициализация workspace
    initializeWorkspace();
    
    // Загрузка истории из домашней директории
    std::string homeDir = getenv("HOME") ? getenv("HOME") : ".";
    std::string historyPath = homeDir + "/.proxima/repl_history.prx";
    loadHistory(historyPath);
    
    initialized = true;
    running = true;
    auto now = std::chrono::system_clock::now();
    startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    commandCount = 0;
    
    print("Proxima REPL v1.0.0\n");
    print("Type 'help' for available commands.\n");
    print("Type 'exit' to quit.\n\n");
    print("Примеры:\n");
    print("  >> x = 1:100          # создание вектора\n");
    print("  >> show x             # визуализация\n");
    print("  >> m = rand(10, 10)   # матрица\n");
    print("  >> whos               # список переменных\n\n");
    
    LOG_INFO("REPL started (Proxima-native mode)");
    return true;
}

void REPL::shutdown() {
    if (!initialized) return;
    
    // Сохранение истории в домашнюю директорию
    std::string homeDir = getenv("HOME") ? getenv("HOME") : ".";
    std::string historyPath = homeDir + "/.proxima/repl_history.prx";
    saveHistory(historyPath);
    
    // Сохранение workspace
    std::vector<std::string> args = {"workspace.prx"};
    saveCommand(args);
    
    clearVariables();
    
    running = false;
    initialized = false;
    
    LOG_INFO("REPL shutdown");
}

// ============================================================================
// Выполнение кода Proxima
// ============================================================================

std::string REPL::execute(const std::string& input) {
    if (!initialized) {
        return "Error: REPL not initialized.\n";
    }
    
    // Trim whitespace
    std::string trimmedInput = input;
    size_t start = trimmedInput.find_first_not_of(" \t\n\r");
    size_t end = trimmedInput.find_last_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return "";
    }
    trimmedInput = trimmedInput.substr(start, end - start + 1);
    
    // Пустой ввод
    if (trimmedInput.empty()) {
        return "";
    }
    
    // Комментарии
    if (trimmedInput.rfind("//", 0) == 0 || trimmedInput.rfind("#", 0) == 0) {
        return "";
    }
    
    // Проверка на команду REPL - разбиваем по пробелам
    std::vector<std::string> tokens;
    std::istringstream iss(trimmedInput);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    if (!tokens.empty()) {
        std::string firstToken = tokens[0];
        // Convert to lowercase
        for (auto& c : firstToken) {
            c = std::tolower(c);
        }
        
        // Проверка на команду
        auto it = commands.find(firstToken);
        if (it != commands.end()) {
            // Это команда REPL
            REPLCommand cmd = it->second;
            tokens.erase(tokens.begin());
            std::string result = executeCommand(cmd, tokens);
            
            addToHistory(trimmedInput, result, result.find("Error") != std::string::npos);
            commandCount++;
            
            return result;
        }
    }
    
    // Выполнение как код Proxima
    std::string result = executeProximaCode(trimmedInput);
    
    addToHistory(trimmedInput, result, result.find("Error") != std::string::npos);
    commandCount++;
    
    return result;
}

std::string REPL::executeProximaCode(const std::string& code) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string output;
    
    try {
        // Парсинг кода Proxima
        std::vector<Token> tokens = parser.tokenize(code);
        ProgramNodePtr ast = parser.parse(tokens);
        
        // Семантический анализ
        analyzer.analyze(ast);
        
        // Проверка на присваивание
        if (ast->statements.size() > 0) {
            auto stmt = ast->statements[0];
            if (stmt->nodeType == NodeType::ASSIGNMENT) {
                auto assign = std::static_pointer_cast<AssignmentNode>(stmt);
                if (assign->left->nodeType == NodeType::IDENTIFIER) {
                    auto ident = std::static_pointer_cast<IdentifierNode>(assign->left);
                    
                    // Выполнение выражения
                    RuntimeValue value = evaluateExpression(assign->right);
                    
                    // Обновление переменной
                    updateVariable(ident->name, value);
                    
                    // Вывод имени переменной
                    output = ident->name + " = " + formatOutput(value) + "\n";
                }
            } else {
                // Выполнение выражения без присваивания
                RuntimeValue value = evaluateExpression(stmt);
                output = formatOutput(value) + "\n";
            }
        }
        
        if (echoInput) {
            print(prompt + code + "\n");
        }
        
    } catch (const std::exception& e) {
        output = "Error: " + std::string(e.what()) + "\n";
        printError(output);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Execution time: " << (elapsed * 1000) << " ms\n";
    output += oss.str();
    
    return output;
}

RuntimeValue REPL::evaluateExpression(StatementNodePtr stmt) {
    // В полной реализации - выполнение через Interpreter
    return RuntimeValue();
}

void REPL::print(const std::string& output) {
    std::cout << output;
}

void REPL::printError(const std::string& error) {
    std::cerr << error;
}

// ============================================================================
// Команды REPL (минимальный набор)
// ============================================================================

std::string REPL::executeCommand(REPLCommand cmd, const std::vector<std::string>& args) {
    switch (cmd) {
        case REPLCommand::HELP:
            return helpCommand(args);
        case REPLCommand::WHOIS:
            return whoisCommand(args);
        case REPLCommand::WHOS:
            return whosCommand(args);
        case REPLCommand::SHOW:
            return showCommand(args);
        case REPLCommand::TYPE:
            return typeCommand(args);
        case REPLCommand::SIZE:
            return sizeCommand(args);
        case REPLCommand::METHODS:
            return methodsCommand(args);
        case REPLCommand::FIELDS:
            return fieldsCommand(args);
        case REPLCommand::EXIT:
            return exitCommand(args);
        case REPLCommand::VERSION:
            return versionCommand(args);
        case REPLCommand::CLEAR:
            return clearCommand(args);
        case REPLCommand::HISTORY:
            return historyCommand(args);
        case REPLCommand::LOAD:
            return loadCommand(args);
        case REPLCommand::SAVE:
            return saveCommand(args);
        case REPLCommand::CONFIG:
            return configCommand(args);
        case REPLCommand::RESET:
            return resetCommand(args);
        default:
            return "Unknown command. Type 'help' for available commands.\n";
    }
}

std::string REPL::helpCommand(const std::vector<std::string>& args) {
    std::string help;
    help += "Proxima REPL Commands\n";
    help += "=====================\n\n";
    help += "Все команды REPL выполняют код Proxima напрямую.\n";
    help += "Пример: x = 1:100; show x;\n\n";
    help += "Специальные команды:\n";
    help += "  help [cmd]     - справка\n";
    help += "  whois <var>    - информация о переменной\n";
    help += "  whos           - список переменных\n";
    help += "  show <var>     - визуализация (интеграция с IDE)\n";
    help += "  type <var>     - тип переменной\n";
    help += "  size <var>     - размер\n";
    help += "  methods <cls>  - методы класса\n";
    help += "  fields <cls>   - поля класса\n";
    help += "  load <file>    - загрузка файла .prx\n";
    help += "  save <file>    - сохранение workspace\n";
    help += "  history [n]    - история команд\n";
    help += "  clear          - очистка\n";
    help += "  reset          - сброс\n";
    help += "  version        - версия\n";
    help += "  exit           - выход\n\n";
    
    if (!args.empty()) {
        help += "Подробная справка по команде '" + args[0] + "'\n";
        // Детальная справка по команде
    }
    
    return help;
}

std::string REPL::whoisCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: whois <variable_name>\n"
               "Показывает детальную информацию о переменной.\n";
    }
    
    std::string varName = args[0];
    
    if (!variables.contains(varName)) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    const REPLVariable& var = variables.at(varName);
    
    std::string info;
    info += "Variable Information\n";
    info += "====================\n\n";
    info += "Name: " + var.name + "\n";
    info += "Type: " + var.type + "\n";
    info += "Size: " + std::to_string(var.size) + "\n";
    info += "Bytes: " + std::to_string(var.bytes) + "\n";
    
    // Использование Proxima show() для значения (language.txt #46)
    info += "Value: " + formatOutput(var.value) + "\n";
    
    return info;
}

std::string REPL::whosCommand(const std::vector<std::string>& args) {
    if (variables.empty()) {
        return "No variables defined.\n";
    }
    
    std::string output;
    output += "Variable Name    Type          Size      Bytes\n";
    output += "=============    ====          ====      =====\n";
    
    // Фильтрация по паттерну (Proxima-style)
    std::string pattern = args.empty() ? "*" : args[0];
    
    for (const auto& pair : variables) {
        const std::string& varName = pair.first;
        const REPLVariable& var = pair.second;
        
        bool matches = (pattern == "*");
        if (!matches) {
            // Простая проверка на вхождение подстроки
            matches = (varName.find(pattern) != std::string::npos);
        }
        
        if (matches) {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), "%-16s %-14s %-10d %-8d\n",
                     var.name.c_str(), var.type.c_str(), var.size, var.bytes);
            output += buffer;
        }
    }
    
    output += "\n";
    output += "Total: " + std::to_string(variables.size()) + " variables\n";
    
    return output;
}

std::string REPL::showCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: show <variable_name>\n"
               "Визуализация переменной через IDE (language.txt #46).\n"
               "  vector  -> график\n"
               "  matrix  -> изображение/таблица\n"
               "  layer   -> 3D визуализация\n"
               "  collection -> таблица\n"
               "  class   -> инспектор объекта\n";
    }
    
    std::string varName = args[0];
    
    auto it = variables.find(varName);
    if (it == variables.end()) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    const REPLVariable& var = it->second;
    
    // Запрос к IDE на визуализацию (интеграция с language.txt #46)
    std::string visType = "auto";
    if (var.type.find("vector") != std::string::npos) visType = "plot";
    else if (var.type.find("matrix") != std::string::npos) visType = "image";
    else if (var.type.find("layer") != std::string::npos) visType = "3d";
    else if (var.type.find("collection") != std::string::npos) visType = "table";
    else if (var.type.find("class") != std::string::npos) visType = "inspect";
    
    // Отправка запроса в IDE через callback
    emitVisualizationRequested(varName, visType);
    
    return "Opening visualizer for '" + varName + "' (" + visType + ")...\n";
}

std::string REPL::typeCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: type <variable_name>\n";
    }
    
    std::string varName = args[0];
    
    auto it = variables.find(varName);
    if (it == variables.end()) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    return "Type of " + varName + ": " + it->second.type + "\n";
}

std::string REPL::sizeCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: size <variable_name>\n";
    }
    
    std::string varName = args[0];
    
    auto it = variables.find(varName);
    if (it == variables.end()) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    return "Size of " + varName + ": " + std::to_string(it->second.size) + "\n";
}

std::string REPL::methodsCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: methods <class_name>\n";
    }
    
    std::string className = args[0];
    
    if (!typeChecker) {
        return "Error: TypeChecker not initialized.\n";
    }
    
    if (!typeChecker->typeExists(className)) {
        return "Class '" + className + "' not found.\n";
    }
    
    TypeInfo info = typeChecker->getType(className);
    
    if (info.methods.empty()) {
        return "No methods defined for " + className + ".\n";
    }
    
    std::string output;
    output += "Methods of " + className + ":\n";
    output += "========================\n\n";
    
    for (const auto& pair : info.methods) {
        const std::string& methodName = pair.first;
        const MethodInfo& method = pair.second;
        
        output += methodName + "(";
        std::vector<std::string> params;
        for (const auto& param : method.parameters) {
            params.push_back(param.first + ": " + param.second);
        }
        
        // Join params with ", "
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) output += ", ";
            output += params[i];
        }
        output += ") : " + method.returnType + "\n";
    }
    
    return output;
}

std::string REPL::fieldsCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: fields <class_name>\n";
    }
    
    std::string className = args[0];
    
    if (!typeChecker) {
        return "Error: TypeChecker not initialized.\n";
    }
    
    if (!typeChecker->typeExists(className)) {
        return "Class '" + className + "' not found.\n";
    }
    
    TypeInfo info = typeChecker->getType(className);
    
    if (info.fields.empty()) {
        return "No fields defined for " + className + ".\n";
    }
    
    std::string output;
    output += "Fields of " + className + ":\n";
    output += "========================\n\n";
    
    for (const auto& pair : info.fields) {
        output += pair.first + ": " + pair.second + "\n";
    }
    
    return output;
}

std::string REPL::exitCommand(const std::vector<std::string>& args) {
    print("Exiting REPL...\n");
    shutdown();
    return "";
}

std::string REPL::versionCommand(const std::vector<std::string>& args) {
    std::string output;
    output += "Proxima REPL v1.0.0\n";
    output += "===================\n\n";
    output += "Language: Proxima 1.0.0\n";
    output += "IDE: Centauri 1.0.0\n";
    output += "License: GPLv3\n\n";
    output += "Build: " + std::string(__DATE__) + " " + std::string(__TIME__) + "\n";
    output += "LLVM: " + std::string(LLVM_VERSION_STRING) + "\n";
    
    return output;
}

std::string REPL::clearCommand(const std::vector<std::string>& args) {
    // Проверка флагов
    bool clearVars = false;
    bool clearHist = false;
    
    for (const auto& arg : args) {
        if (arg == "-variables" || arg == "-vars") {
            clearVars = true;
        }
        if (arg == "-history" || arg == "-hist") {
            clearHist = true;
        }
    }
    
    if (clearVars) {
        clearVariables();
        return "Variables cleared.\n";
    }
    
    if (clearHist) {
        clearHistory();
        return "History cleared.\n";
    }
    
    // Очистка консоли (ANSI escape codes)
    print("\033[2J\033[H");
    return "";
}

std::string REPL::historyCommand(const std::vector<std::string>& args) {
    int count = 20;
    if (!args.empty()) {
        try {
            int parsed = std::stoi(args[0]);
            if (parsed > 0) {
                count = std::min(parsed, static_cast<int>(history.size()));
            }
        } catch (...) {
            // Игнорируем ошибки парсинга
        }
    }
    
    if (history.empty()) {
        return "History is empty.\n";
    }
    
    std::string output;
    output += "Command History\n";
    output += "===============\n\n";
    
    int start = std::max(0, static_cast<int>(history.size()) - count);
    for (size_t i = start; i < history.size(); ++i) {
        const REPLHistoryEntry& entry = history[i];
        std::string marker = entry.hasError ? "! " : "  ";
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "%s%4zu: %s\n", marker.c_str(), i + 1, entry.code.c_str());
        output += buffer;
    }
    
    return output;
}

std::string REPL::loadCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: load <filename.prx>\n"
               "Загружает и выполняет файл Proxima.\n";
    }
    
    std::string filename = args[0];
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return "Error: Cannot open file '" + filename + "'\n";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    print("Loading " + filename + "...\n");
    
    // Выполнение содержимого файла как кода Proxima
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    for (const std::string& l : lines) {
        // Trim whitespace
        size_t start = l.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = l.find_last_not_of(" \t\r\n");
        std::string trimmed = l.substr(start, end - start + 1);
        
        // Пропускаем пустые строки и комментарии
        if (trimmed.empty() || trimmed.rfind("//", 0) == 0 || trimmed.rfind("#", 0) == 0) {
            continue;
        }
        
        std::string result = execute(trimmed);
        if (!result.empty()) {
            print(result);
        }
    }
    
    return "File loaded successfully.\n";
}

std::string REPL::saveCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        return "Usage: save <filename.prx>\n"
               "Сохраняет переменные workspace в файл Proxima.\n";
    }
    
    std::string filename = args[0];
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        return "Error: Cannot create file '" + filename + "'\n";
    }
    
    file << "// Proxima Workspace\n";
    file << "// Saved: " << std::time(nullptr) << "\n\n";
    
    for (const auto& pair : variables) {
        file << pair.first << " = " << formatOutput(pair.second.value) << ";\n";
    }
    
    file.close();
    
    return "Workspace saved to " + filename + ".\n";
}

std::string REPL::configCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::string output;
        output += "REPL Configuration\n";
        output += "==================\n\n";
        output += "Prompt: " + prompt + "\n";
        output += "Echo Input: " + std::string(echoInput ? "on" : "off") + "\n";
        output += "Current Path: " + currentPath + "\n";
        return output;
    }
    
    if (args.size() >= 2) {
        std::string key = args[0];
        std::string value = args[1];
        
        if (key == "prompt") {
            prompt = value;
            return "Prompt set to: " + prompt + "\n";
        }
        if (key == "echo") {
            echoInput = (value == "on" || value == "true");
            return "Echo set to: " + (std::string)(echoInput ? "on" : "off") + "\n";
        }
    }
    
    return "Usage: config <key> <value>\n";
}

std::string REPL::resetCommand(const std::vector<std::string>& args) {
    clearVariables();
    clearHistory();
    commandCount = 0;
    startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    return "REPL reset.\n";
}

// ============================================================================
// Визуализация (интеграция с language.txt #46)
// ============================================================================

void REPL::showVariable(const std::string& varName) {
    showCommand(std::vector<std::string>{varName});
}

void REPL::plotVector(const std::string& varName) {
    auto it = variables.find(varName);
    if (it == variables.end()) return;
    
    const REPLVariable& var = it->second;
    if (var.type.find("vector") == std::string::npos) {
        printError("Variable '" + varName + "' is not a vector.\n");
        return;
    }
    
    emitVisualizationRequested(varName, "plot");
}

void REPL::showMatrix(const std::string& varName) {
    auto it = variables.find(varName);
    if (it == variables.end()) return;
    
    const REPLVariable& var = it->second;
    if (var.type.find("matrix") == std::string::npos) {
        printError("Variable '" + varName + "' is not a matrix.\n");
        return;
    }
    
    emitVisualizationRequested(varName, "image");
}

void REPL::showLayer3D(const std::string& varName) {
    auto it = variables.find(varName);
    if (it == variables.end()) return;
    
    const REPLVariable& var = it->second;
    if (var.type.find("layer") == std::string::npos) {
        printError("Variable '" + varName + "' is not a layer.\n");
        return;
    }
    
    emitVisualizationRequested(varName, "3d");
}

void REPL::showCollection(const std::string& varName) {
    auto it = variables.find(varName);
    if (it == variables.end()) return;
    
    const REPLVariable& var = it->second;
    if (var.type.find("collection") == std::string::npos) {
        printError("Variable '" + varName + "' is not a collection.\n");
        return;
    }
    
    emitVisualizationRequested(varName, "table");
}

void REPL::inspectObject(const std::string& varName) {
    auto it = variables.find(varName);
    if (it == variables.end()) return;
    
    emitVisualizationRequested(varName, "inspect");
}

// ============================================================================
// Управление переменными
// ============================================================================

bool REPL::hasVariable(const std::string& name) const {
    return variables.find(name) != variables.end();
}

void REPL::clearVariables() {
    variables.clear();
    
    // Предопределённые константы Proxima (language.txt #2)
    updateVariable("pi", "double", RuntimeValue(3.14159265358979));
    updateVariable("pi2", "double", RuntimeValue(6.28318530717959));
    updateVariable("exp", "double", RuntimeValue(2.71828182845905));
    updateVariable("nan", "double", RuntimeValue(std::numeric_limits<double>::quiet_NaN()));
    updateVariable("inf", "double", RuntimeValue(std::numeric_limits<double>::infinity()));
    updateVariable("ninf", "double", RuntimeValue(-std::numeric_limits<double>::infinity()));
}

void REPL::updateVariable(const std::string& name, const RuntimeValue& value) {
    REPLVariable var;
    var.name = name;
    var.type = getTypeName(value);
    var.value = value;
    var.size = getValueSize(value);
    var.bytes = getValueBytes(value);
    var.lastModified = std::time(nullptr);
    
    variables[name] = var;
    
    variableChanged(name);
}

std::string REPL::getTypeName(const RuntimeValue& value) const {
    // Определение типа из RuntimeValue
    if (std::holds_alternative<int64_t>(value)) return "int64";
    if (std::holds_alternative<double>(value)) return "double";
    if (std::holds_alternative<std::string>(value)) return "string";
    if (std::holds_alternative<bool>(value)) return "bool";
    if (std::holds_alternative<std::vector<RuntimeValue>>(value)) return "vector";
    if (std::holds_alternative<std::vector<std::vector<RuntimeValue>>>(value)) return "matrix";
    return "auto";
}

int REPL::getValueSize(const RuntimeValue& value) const {
    if (std::holds_alternative<std::vector<RuntimeValue>>(value)) {
        return std::get<std::vector<RuntimeValue>>(value).size();
    }
    return 1;
}

int REPL::getValueBytes(const RuntimeValue& value) const {
    // Приблизительный расчёт
    return sizeof(value) * getValueSize(value);
}

std::string REPL::formatOutput(const RuntimeValue& value) const {
    if (std::holds_alternative<int64_t>(value)) {
        return std::to_string(std::get<int64_t>(value));
    }
    if (std::holds_alternative<double>(value)) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.6f", std::get<double>(value));
        return std::string(buffer);
    }
    if (std::holds_alternative<std::string>(value)) {
        return "\"" + std::get<std::string>(value) + "\"";
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<std::vector<RuntimeValue>>(value)) {
        const auto& vec = std::get<std::vector<RuntimeValue>>(value);
        std::vector<std::string> items;
        for (const auto& item : vec) {
            items.push_back(formatOutput(item));
        }
        
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) result += ", ";
            result += items[i];
        }
        result += "]";
        return result;
    }
    if (std::holds_alternative<std::vector<std::vector<RuntimeValue>>>(value)) {
        return "[matrix]";
    }
    return "[unknown]";
}

// ============================================================================
// История
// ============================================================================

void REPL::addToHistory(const std::string& code, const std::string& output, bool hasError) {
    REPLHistoryEntry entry;
    entry.code = code;
    entry.output = output;
    entry.timestamp = std::time(nullptr);
    entry.hasError = hasError;
    entry.executionTime = 0;
    
    history.push_back(entry);
    
    // Ограничение размера
    while (history.size() > 1000) {
        history.erase(history.begin());
    }
    
    historyChanged();
}

void REPL::clearHistory() {
    history.clear();
}

void REPL::saveHistory(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    for (const REPLHistoryEntry& entry : history) {
        file << entry.code << "\n";
    }
    
    file.close();
}

void REPL::loadHistory(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            REPLHistoryEntry entry;
            entry.code = line;
            entry.timestamp = std::time(nullptr);
            history.push_back(entry);
        }
    }
    
    file.close();
}

// ============================================================================
// Интеграция
// ============================================================================

void REPL::setRuntime(Runtime* rt) {
    runtime = rt;
}

void REPL::setDebugger(Debugger* dbg) {
    debugger = dbg;
}

void REPL::setTypeChecker(TypeChecker* checker) {
    typeChecker = checker;
}

void REPL::setIDEInterface(void* ide) {
    ideInterface = ide;
}

void REPL::setPrompt(const std::string& newPrompt) {
    prompt = newPrompt;
}

void REPL::setEchoInput(bool enable) {
    echoInput = enable;
}

// ============================================================================
// Автодополнение
// ============================================================================

std::vector<std::string> REPL::getCompletions(const std::string& prefix) const {
    std::vector<std::string> completions;
    
    // Автодополнение команд
    for (const auto& pair : commands) {
        const std::string& cmdName = pair.first;
        if (cmdName.size() >= prefix.size() && 
            cmdName.compare(0, prefix.size(), prefix) == 0) {
            completions.push_back(cmdName);
        }
    }
    
    // Автодополнение переменных
    for (const auto& pair : variables) {
        const std::string& varName = pair.first;
        if (varName.size() >= prefix.size() && 
            varName.compare(0, prefix.size(), prefix) == 0) {
            completions.push_back(varName);
        }
    }
    
    // Автодополнение ключевых слов Proxima
    std::vector<std::string> keywords = {"if", "else", "elseif", "end", "for", "in", 
                           "while", "do", "switch", "case", "return", 
                           "break", "continue", "class", "interface", 
                           "function", "vector", "matrix", "layer"};
    
    for (const std::string& kw : keywords) {
        if (kw.size() >= prefix.size() && 
            kw.compare(0, prefix.size(), prefix) == 0) {
            completions.push_back(kw);
        }
    }
    
    return completions;
}

// ============================================================================
// Callback registration (Qt-free signals)
// ============================================================================

void REPL::onVisualizationRequested(VisualizationCallback callback) {
    visualizationCallback_ = callback;
}

void REPL::onVariableChanged(VariableChangedCallback callback) {
    variableChangedCallback_ = callback;
}

void REPL::onHistoryChanged(HistoryChangedCallback callback) {
    historyChangedCallback_ = callback;
}

void REPL::onOutputReceived(OutputCallback callback) {
    outputCallback_ = callback;
}

void REPL::onErrorReceived(ErrorCallback callback) {
    errorCallback_ = callback;
}

void REPL::onPromptDisplayed(PromptCallback callback) {
    promptCallback_ = callback;
}

// ============================================================================
// Signal emission helpers (Qt-free)
// ============================================================================

void REPL::emitVisualizationRequested(const std::string& varName, const std::string& type) {
    if (visualizationCallback_) {
        visualizationCallback_(varName, type);
    }
}

void REPL::emitVariableChanged(const std::string& varName) {
    if (variableChangedCallback_) {
        variableChangedCallback_(varName);
    }
}

void REPL::emitHistoryChanged() {
    if (historyChangedCallback_) {
        historyChangedCallback_();
    }
}

void REPL::emitOutputReceived(const std::string& output) {
    if (outputCallback_) {
        outputCallback_(output);
    } else {
        print(output);  // Fallback to default print
    }
}

void REPL::emitErrorReceived(const std::string& error) {
    if (errorCallback_) {
        errorCallback_(error);
    } else {
        printError(error);  // Fallback to default printError
    }
}

void REPL::emitPromptDisplayed() {
    if (promptCallback_) {
        promptCallback_();
    }
}

} // namespace proxima
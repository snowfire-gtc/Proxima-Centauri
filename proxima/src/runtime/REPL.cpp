#include "REPL.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include "utils/Logger.h"
#include "stdlib/Collection.h"
#include "stdlib/Math.h"
#include "stdlib/Time.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

REPL::REPL(QObject *parent)
    : QObject(parent)
    , initialized(false)
    , running(false)
    , echoInput(true)
    , prompt(">> ")
    , runtime(nullptr)
    , debugger(nullptr)
    , typeChecker(nullptr)
    , ideInterface(nullptr)
    , startTime(0)
    , commandCount(0) {
    
    currentPath = QDir::currentPath();
    
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
        runtime = new Runtime(this);
        runtime->initialize();
    }
    
    // Инициализация type checker
    if (!typeChecker) {
        typeChecker = new TypeChecker();
    }
    
    // Инициализация workspace
    initializeWorkspace();
    
    // Загрузка истории
    QString historyPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/repl_history.prx";
    loadHistory(historyPath);
    
    initialized = true;
    running = true;
    startTime = QDateTime::currentMSecsSinceEpoch();
    commandCount = 0;
    
    print("Proxima REPL v1.0.0\n");
    print("Type 'help' for available commands.\n");
    print("Type 'exit' to quit.\n\n");
    print("Примеры:\n");
    print("  >> x = 1:100          # создание вектора\n");
    print("  >> show x             # визуализация\n");
    print("  >> m = rand(10, 10)   # матрица\n");
    print("  >> whos               # список переменных\n\n");
    
    emit promptDisplayed();
    
    LOG_INFO("REPL started (Proxima-native mode)");
    return true;
}

void REPL::shutdown() {
    if (!initialized) return;
    
    // Сохранение истории
    QString historyPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/repl_history.prx";
    saveHistory(historyPath);
    
    // Сохранение workspace
    saveCommand(QStringList() << "workspace.prx");
    
    clearVariables();
    
    running = false;
    initialized = false;
    
    LOG_INFO("REPL shutdown");
}

// ============================================================================
// Выполнение кода Proxima
// ============================================================================

QString REPL::execute(const QString& input) {
    if (!initialized) {
        return "Error: REPL not initialized.\n";
    }
    
    QString trimmedInput = input.trimmed();
    
    // Пустой ввод
    if (trimmedInput.isEmpty()) {
        emit promptDisplayed();
        return "";
    }
    
    // Комментарии
    if (trimmedInput.startsWith("//") || trimmedInput.startsWith("#")) {
        emit promptDisplayed();
        return "";
    }
    
    // Проверка на команду REPL
    QStringList tokens = trimmedInput.split(QRegularExpression("\\s+"));
    if (!tokens.isEmpty()) {
        QString firstToken = tokens.first().toLower();
        
        // Проверка на команду (без "!" префикса - это Proxima-native)
        if (commands.contains(firstToken)) {
            // Это команда REPL, а не код Proxima
            REPLCommand cmd = commands[firstToken];
            tokens.removeFirst();
            QString result = executeCommand(cmd, tokens);
            
            addToHistory(trimmedInput, result, result.contains("Error"));
            commandCount++;
            
            emit promptDisplayed();
            return result;
        }
    }
    
    // Выполнение как код Proxima
    QString result = executeProximaCode(trimmedInput);
    
    addToHistory(trimmedInput, result, result.contains("Error"));
    commandCount++;
    
    emit promptDisplayed();
    
    return result;
}

QString REPL::executeProximaCode(const QString& code) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    QString output;
    
    try {
        // Парсинг кода Proxima
        std::vector<Token> tokens = parser.tokenize(code.toStdString());
        ProgramNodePtr ast = parser.parse(tokens);
        
        // Семантический анализ
        analyzer.analyze(ast);
        
        // Проверка на присваивание (для обновления переменных)
        if (ast->statements.size() > 0) {
            auto stmt = ast->statements[0];
            if (stmt->nodeType == NodeType::ASSIGNMENT) {
                auto assign = std::static_pointer_cast<AssignmentNode>(stmt);
                if (assign->left->nodeType == NodeType::IDENTIFIER) {
                    auto ident = std::static_pointer_cast<IdentifierNode>(assign->left);
                    
                    // Выполнение выражения
                    RuntimeValue value = evaluateExpression(assign->right);
                    
                    // Обновление переменной
                    updateVariable(QString::fromStdString(ident->name), value);
                    
                    // Вывод имени переменной (как в Matlab/Octave)
                    output = QString::fromStdString(ident->name) + " = " + 
                             formatOutput(value) + "\n";
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
        output = "Error: " + QString::fromUtf8(e.what()) + "\n";
        printError(output);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();
    
    output += QString("Execution time: %1 ms\n").arg(elapsed * 1000, 0, 'f', 2);
    
    return output;
}

RuntimeValue REPL::evaluateExpression(StatementNodePtr stmt) {
    // В полной реализации - выполнение через Interpreter
    // Для примера - заглушка
    return RuntimeValue();
}

void REPL::print(const QString& output) {
    emit outputReceived(output);
}

void REPL::printError(const QString& error) {
    emit errorReceived(error);
}

// ============================================================================
// Команды REPL (минимальный набор)
// ============================================================================

QString REPL::executeCommand(REPLCommand cmd, const QStringList& args) {
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

QString REPL::helpCommand(const QStringList& args) {
    QString help;
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
    
    if (!args.isEmpty()) {
        help += "Подробная справка по команде '" + args.first() + "'\n";
        // Детальная справка по команде
    }
    
    return help;
}

QString REPL::whoisCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: whois <variable_name>\n"
               "Показывает детальную информацию о переменной.\n";
    }
    
    QString varName = args.first();
    
    if (!variables.contains(varName)) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    const REPLVariable& var = variables[varName];
    
    QString info;
    info += "Variable Information\n";
    info += "====================\n\n";
    info += QString("Name: %1\n").arg(var.name);
    info += QString("Type: %1\n").arg(var.type);
    info += QString("Size: %1\n").arg(var.size);
    info += QString("Bytes: %1\n").arg(var.bytes);
    info += QString("Modified: %1\n").arg(var.lastModified.toString());
    
    // Использование Proxima show() для значения (language.txt #46)
    info += QString("Value: %1\n").arg(formatOutput(var.value));
    
    return info;
}

QString REPL::whosCommand(const QStringList& args) {
    if (variables.isEmpty()) {
        return "No variables defined.\n";
    }
    
    QString output;
    output += "Variable Name    Type          Size      Bytes\n";
    output += "=============    ====          ====      =====\n";
    
    // Фильтрация по паттерну (Proxima-style)
    QString pattern = args.isEmpty() ? "*" : args.first();
    
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        if (pattern == "*" || it.key().contains(pattern)) {
            const REPLVariable& var = it.value();
            output += QString("%1%2%3%4\n")
                .arg(var.name, -16)
                .arg(var.type, -14)
                .arg(QString::number(var.size), -10)
                .arg(QString::number(var.bytes), -8);
        }
    }
    
    output += "\n";
    output += QString("Total: %1 variables\n").arg(variables.size());
    
    return output;
}

QString REPL::showCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: show <variable_name>\n"
               "Визуализация переменной через IDE (language.txt #46).\n"
               "  vector  -> график\n"
               "  matrix  -> изображение/таблица\n"
               "  layer   -> 3D визуализация\n"
               "  collection -> таблица\n"
               "  class   -> инспектор объекта\n";
    }
    
    QString varName = args.first();
    
    if (!variables.contains(varName)) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    const REPLVariable& var = variables[varName];
    
    // Запрос к IDE на визуализацию (интеграция с language.txt #46)
    QString visType = "auto";
    if (var.type.contains("vector")) visType = "plot";
    else if (var.type.contains("matrix")) visType = "image";
    else if (var.type.contains("layer")) visType = "3d";
    else if (var.type.contains("collection")) visType = "table";
    else if (var.type.contains("class")) visType = "inspect";
    
    // Отправка запроса в IDE
    emit visualizationRequested(varName, visType);
    
    return "Opening visualizer for '" + varName + "' (" + visType + ")...\n";
}

QString REPL::typeCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: type <variable_name>\n";
    }
    
    QString varName = args.first();
    
    if (!variables.contains(varName)) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    return "Type of " + varName + ": " + variables[varName].type + "\n";
}

QString REPL::sizeCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: size <variable_name>\n";
    }
    
    QString varName = args.first();
    
    if (!variables.contains(varName)) {
        return "Variable '" + varName + "' not found.\n";
    }
    
    return "Size of " + varName + ": " + QString::number(variables[varName].size) + "\n";
}

QString REPL::methodsCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: methods <class_name>\n";
    }
    
    QString className = args.first();
    
    if (!typeChecker) {
        return "Error: TypeChecker not initialized.\n";
    }
    
    if (!typeChecker->typeExists(className)) {
        return "Class '" + className + "' not found.\n";
    }
    
    TypeInfo info = typeChecker->getType(className);
    
    if (info.methods.isEmpty()) {
        return "No methods defined for " + className + ".\n";
    }
    
    QString output;
    output += "Methods of " + className + ":\n";
    output += "========================\n\n";
    
    for (auto it = info.methods.begin(); it != info.methods.end(); ++it) {
        output += it.key() + "(";
        QStringList params;
        for (const auto& param : it->parameters) {
            params.append(param.first + ": " + param.second);
        }
        output += params.join(", ") + ") : " + it->returnType + "\n";
    }
    
    return output;
}

QString REPL::fieldsCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: fields <class_name>\n";
    }
    
    QString className = args.first();
    
    if (!typeChecker) {
        return "Error: TypeChecker not initialized.\n";
    }
    
    if (!typeChecker->typeExists(className)) {
        return "Class '" + className + "' not found.\n";
    }
    
    TypeInfo info = typeChecker->getType(className);
    
    if (info.fields.isEmpty()) {
        return "No fields defined for " + className + ".\n";
    }
    
    QString output;
    output += "Fields of " + className + ":\n";
    output += "========================\n\n";
    
    for (auto it = info.fields.begin(); it != info.fields.end(); ++it) {
        output += it.key() + ": " + it.value() + "\n";
    }
    
    return output;
}

QString REPL::exitCommand(const QStringList& args) {
    print("Exiting REPL...\n");
    shutdown();
    return "";
}

QString REPL::versionCommand(const QStringList& args) {
    QString output;
    output += "Proxima REPL v1.0.0\n";
    output += "===================\n\n";
    output += "Language: Proxima 1.0.0\n";
    output += "IDE: Centauri 1.0.0\n";
    output += "License: GPLv3\n\n";
    output += "Build: " + QString(__DATE__) + " " + QString(__TIME__) + "\n";
    output += "Qt: " + QString(QT_VERSION_STR) + "\n";
    output += "LLVM: " + QString(LLVM_VERSION_STRING) + "\n";
    
    return output;
}

QString REPL::clearCommand(const QStringList& args) {
    if (args.contains("-variables") || args.contains("-vars")) {
        clearVariables();
        return "Variables cleared.\n";
    }
    
    if (args.contains("-history") || args.contains("-hist")) {
        clearHistory();
        return "History cleared.\n";
    }
    
    // Очистка консоли (ANSI escape codes)
    emit outputReceived("\033[2J\033[H");
    return "";
}

QString REPL::historyCommand(const QStringList& args) {
    int count = 20;
    if (!args.isEmpty()) {
        bool ok;
        int parsed = args.first().toInt(&ok);
        if (ok && parsed > 0) {
            count = qMin(parsed, history.size());
        }
    }
    
    if (history.isEmpty()) {
        return "History is empty.\n";
    }
    
    QString output;
    output += "Command History\n";
    output += "===============\n\n";
    
    int start = qMax(0, history.size() - count);
    for (int i = start; i < history.size(); i++) {
        const REPLHistoryEntry& entry = history[i];
        QString marker = entry.hasError ? "! " : "  ";
        output += QString("%1%2: %3\n")
            .arg(marker)
            .arg(i + 1, 4)
            .arg(entry.code);
    }
    
    return output;
}

QString REPL::loadCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: load <filename.prx>\n"
               "Загружает и выполняет файл Proxima.\n";
    }
    
    QString filename = args.first();
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "Error: Cannot open file '" + filename + "'\n";
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    print("Loading " + filename + "...\n");
    
    // Выполнение содержимого файла как кода Proxima
    QStringList lines = content.split("\n");
    for (const QString& line : lines) {
        if (!line.trimmed().isEmpty() && !line.trimmed().startsWith("//")) {
            QString result = execute(line);
            if (!result.isEmpty()) {
                print(result);
            }
        }
    }
    
    return "File loaded successfully.\n";
}

QString REPL::saveCommand(const QStringList& args) {
    if (args.isEmpty()) {
        return "Usage: save <filename.prx>\n"
               "Сохраняет переменные workspace в файл Proxima.\n";
    }
    
    QString filename = args.first();
    QFile file(filename);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return "Error: Cannot create file '" + filename + "'\n";
    }
    
    QTextStream out(&file);
    out << "// Proxima Workspace\n";
    out << "// Saved: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        out << it->name << " = " << formatOutput(it->value) << ";\n";
    }
    
    file.close();
    
    return "Workspace saved to " + filename + ".\n";
}

QString REPL::configCommand(const QStringList& args) {
    if (args.isEmpty()) {
        QString output;
        output += "REPL Configuration\n";
        output += "==================\n\n";
        output += QString("Prompt: %1\n").arg(prompt);
        output += QString("Echo Input: %1\n").arg(echoInput ? "on" : "off");
        output += QString("Current Path: %1\n").arg(currentPath);
        return output;
    }
    
    if (args.size() >= 2) {
        QString key = args.first();
        QString value = args[1];
        
        if (key == "prompt") {
            prompt = value;
            return "Prompt set to: " + prompt + "\n";
        }
        if (key == "echo") {
            echoInput = (value == "on" || value == "true");
            return "Echo set to: " + (echoInput ? "on" : "off") + "\n";
        }
    }
    
    return "Usage: config <key> <value>\n";
}

QString REPL::resetCommand(const QStringList& args) {
    clearVariables();
    clearHistory();
    commandCount = 0;
    startTime = QDateTime::currentMSecsSinceEpoch();
    
    return "REPL reset.\n";
}

// ============================================================================
// Визуализация (интеграция с language.txt #46)
// ============================================================================

void REPL::showVariable(const QString& varName) {
    showCommand(QStringList() << varName);
}

void REPL::plotVector(const QString& varName) {
    if (!variables.contains(varName)) return;
    
    const REPLVariable& var = variables[varName];
    if (!var.type.contains("vector")) {
        printError("Variable '" + varName + "' is not a vector.\n");
        return;
    }
    
    emit visualizationRequested(varName, "plot");
}

void REPL::showMatrix(const QString& varName) {
    if (!variables.contains(varName)) return;
    
    const REPLVariable& var = variables[varName];
    if (!var.type.contains("matrix")) {
        printError("Variable '" + varName + "' is not a matrix.\n");
        return;
    }
    
    emit visualizationRequested(varName, "image");
}

void REPL::showLayer3D(const QString& varName) {
    if (!variables.contains(varName)) return;
    
    const REPLVariable& var = variables[varName];
    if (!var.type.contains("layer")) {
        printError("Variable '" + varName + "' is not a layer.\n");
        return;
    }
    
    emit visualizationRequested(varName, "3d");
}

void REPL::showCollection(const QString& varName) {
    if (!variables.contains(varName)) return;
    
    const REPLVariable& var = variables[varName];
    if (!var.type.contains("collection")) {
        printError("Variable '" + varName + "' is not a collection.\n");
        return;
    }
    
    emit visualizationRequested(varName, "table");
}

void REPL::inspectObject(const QString& varName) {
    if (!variables.contains(varName)) return;
    
    emit visualizationRequested(varName, "inspect");
}

// ============================================================================
// Управление переменными
// ============================================================================

bool REPL::hasVariable(const QString& name) const {
    return variables.contains(name);
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

void REPL::updateVariable(const QString& name, const RuntimeValue& value) {
    REPLVariable var;
    var.name = name;
    var.type = getTypeName(value);
    var.value = value;
    var.size = getValueSize(value);
    var.bytes = getValueBytes(value);
    var.lastModified = QDateTime::currentDateTime();
    
    variables[name] = var;
    
    emit variableChanged(name);
}

QString REPL::getTypeName(const RuntimeValue& value) const {
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

QString REPL::formatOutput(const RuntimeValue& value) const {
    if (std::holds_alternative<int64_t>(value)) {
        return QString::number(std::get<int64_t>(value));
    }
    if (std::holds_alternative<double>(value)) {
        return QString::number(std::get<double>(value), 'f', 6);
    }
    if (std::holds_alternative<std::string>(value)) {
        return "\"" + QString::fromStdString(std::get<std::string>(value)) + "\"";
    }
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<std::vector<RuntimeValue>>(value)) {
        const auto& vec = std::get<std::vector<RuntimeValue>>(value);
        QStringList items;
        for (const auto& item : vec) {
            items.append(formatOutput(item));
        }
        return "[" + items.join(", ") + "]";
    }
    if (std::holds_alternative<std::vector<std::vector<RuntimeValue>>>(value)) {
        return "[matrix]";
    }
    return "[unknown]";
}

// ============================================================================
// История
// ============================================================================

void REPL::addToHistory(const QString& code, const QString& output, bool hasError) {
    REPLHistoryEntry entry;
    entry.code = code;
    entry.output = output;
    entry.timestamp = QDateTime::currentDateTime();
    entry.hasError = hasError;
    entry.executionTime = 0;
    
    history.append(entry);
    
    // Ограничение размера
    while (history.size() > 1000) {
        history.removeFirst();
    }
    
    emit historyChanged();
}

void REPL::clearHistory() {
    history.clear();
}

void REPL::saveHistory(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    
    QTextStream out(&file);
    for (const REPLHistoryEntry& entry : history) {
        out << entry.code << "\n";
    }
    
    file.close();
}

void REPL::loadHistory(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty()) {
            REPLHistoryEntry entry;
            entry.code = line;
            entry.timestamp = QDateTime::currentDateTime();
            history.append(entry);
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

void REPL::setIDEInterface(QObject* ide) {
    ideInterface = ide;
}

void REPL::setPrompt(const QString& newPrompt) {
    prompt = newPrompt;
}

void REPL::setEchoInput(bool enable) {
    echoInput = enable;
}

// ============================================================================
// Автодополнение
// ============================================================================

QStringList REPL::getCompletions(const QString& prefix) const {
    QStringList completions;
    
    // Автодополнение команд
    for (auto it = commands.begin(); it != commands.end(); ++it) {
        if (it.key().startsWith(prefix)) {
            completions.append(it.key());
        }
    }
    
    // Автодополнение переменных
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        if (it.key().startsWith(prefix)) {
            completions.append(it.key());
        }
    }
    
    // Автодополнение ключевых слов Proxima
    QStringList keywords = {"if", "else", "elseif", "end", "for", "in", 
                           "while", "do", "switch", "case", "return", 
                           "break", "continue", "class", "interface", 
                           "function", "vector", "matrix", "layer"};
    
    for (const QString& kw : keywords) {
        if (kw.startsWith(prefix)) {
            completions.append(kw);
        }
    }
    
    return completions;
}

} // namespace proxima
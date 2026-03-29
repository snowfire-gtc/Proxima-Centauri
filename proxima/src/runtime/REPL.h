#ifndef PROXIMA_REPL_H
#define PROXIMA_REPL_H

#include "../parser/Parser.h"
#include "../semantic/SemanticAnalyzer.h"
#include "../semantic/TypeChecker.h"
#include "../codegen/LLVMCodeGen.h"
#include "../runtime/Runtime.h"
#include "../runtime/Debugger.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <ctime>
#include <functional>
#include <memory>

namespace proxima {

// Минимальный набор REPL-специфичных команд
enum class REPLCommand {
    UNKNOWN,
    HELP,           // help - справка
    EXIT,           // exit/quit - выход
    CLEAR,          // clear - очистка
    HISTORY,        // history - история
    LOAD,           // load - загрузка файла
    SAVE,           // save - сохранение
    WHOIS,          // whois - информация о переменной (Proxima-native)
    WHOS,           // whos - список переменных (Proxima-native)
    SHOW,           // show - визуализация (интеграция с language.txt #46)
    TYPE,           // type - тип переменной
    SIZE,           // size - размер
    METHODS,        // methods - методы класса
    FIELDS,         // fields - поля класса
    VERSION,        // version - версия
    CONFIG,         // config - конфигурация
    RESET           // reset - сброс
};

struct REPLHistoryEntry {
    std::string code;           // Код Proxima
    std::string output;         // Результат выполнения
    std::time_t timestamp;
    bool hasError;
    int64_t executionTime;
};

struct REPLVariable {
    std::string name;
    std::string type;
    RuntimeValue value;     // RuntimeValue из Interpreter
    int size;
    int bytes;
    std::time_t lastModified;
};

// Callback types for REPL events (Qt-free signal replacement)
using VisualizationCallback = std::function<void(const std::string& varName, const std::string& type)>;
using VariableChangedCallback = std::function<void(const std::string& varName)>;
using HistoryChangedCallback = std::function<void()>;
using OutputCallback = std::function<void(const std::string& output)>;
using ErrorCallback = std::function<void(const std::string& error)>;
using PromptCallback = std::function<void()>;

class REPL {
    
public:
    REPL();
    ~REPL();
    
    // Инициализация
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    // Выполнение кода Proxima
    std::string execute(const std::string& code);
    std::string evaluate(const std::string& expression);
    
    // REPL команды (минимальный набор)
    REPLCommand parseCommand(const std::string& input);
    std::string executeCommand(REPLCommand cmd, const std::vector<std::string>& args);
    
    // Визуализация (интеграция с language.txt #46)
    void showVariable(const std::string& varName);
    void plotVector(const std::string& varName);
    void showMatrix(const std::string& varName);
    void showLayer3D(const std::string& varName);
    void showCollection(const std::string& varName);
    void inspectObject(const std::string& varName);
    
    // Управление переменными
    std::map<std::string, REPLVariable> getVariables() const { return variables; }
    bool hasVariable(const std::string& name) const;
    void clearVariables();
    
    // История
    void addToHistory(const std::string& code, const std::string& output, bool hasError);
    void clearHistory();
    void saveHistory(const std::string& path);
    void loadHistory(const std::string& path);
    
    // Конфигурация
    void setPrompt(const std::string& prompt);
    std::string getPrompt() const { return prompt; }
    void setEchoInput(bool enable);
    
    // Интеграция
    void setRuntime(Runtime* runtime);
    void setDebugger(Debugger* debugger);
    void setTypeChecker(TypeChecker* checker);
    void setIDEInterface(void* ide);  // Для связи с визуализаторами IDE
    
    // Callback registration (Qt-free signals)
    void onVisualizationRequested(VisualizationCallback callback);
    void onVariableChanged(VariableChangedCallback callback);
    void onHistoryChanged(HistoryChangedCallback callback);
    void onOutputReceived(OutputCallback callback);
    void onErrorReceived(ErrorCallback callback);
    void onPromptDisplayed(PromptCallback callback);
    
    // Автодополнение
    std::vector<std::string> getCompletions(const std::string& prefix) const;
    
private:
    void registerBuiltinFunctions();
    void initializeWorkspace();
    std::string executeProximaCode(const std::string& code);
    std::string formatOutput(const RuntimeValue& value) const;
    void updateVariable(const std::string& name, const RuntimeValue& value);
    std::string getTypeName(const RuntimeValue& value) const;
    
    // Вспомогательные функции
    void print(const std::string& output);
    void printError(const std::string& error);
    RuntimeValue evaluateExpression(StatementNodePtr stmt);
    
    // Signal emission helpers (Qt-free)
    void emitVisualizationRequested(const std::string& varName, const std::string& type);
    void emitVariableChanged(const std::string& varName);
    void emitHistoryChanged();
    void emitOutputReceived(const std::string& output);
    void emitErrorReceived(const std::string& error);
    void emitPromptDisplayed();
    
    // Команды
    std::string helpCommand(const std::vector<std::string>& args);
    std::string whoisCommand(const std::vector<std::string>& args);
    std::string whosCommand(const std::vector<std::string>& args);
    std::string showCommand(const std::vector<std::string>& args);
    std::string typeCommand(const std::vector<std::string>& args);
    std::string sizeCommand(const std::vector<std::string>& args);
    std::string methodsCommand(const std::vector<std::string>& args);
    std::string fieldsCommand(const std::vector<std::string>& args);
    std::string exitCommand(const std::vector<std::string>& args);
    std::string versionCommand(const std::vector<std::string>& args);
    std::string clearCommand(const std::vector<std::string>& args);
    std::string historyCommand(const std::vector<std::string>& args);
    std::string loadCommand(const std::vector<std::string>& args);
    std::string saveCommand(const std::vector<std::string>& args);
    std::string configCommand(const std::vector<std::string>& args);
    std::string resetCommand(const std::vector<std::string>& args);
    
    bool initialized;
    bool running;
    bool echoInput;
    
    std::string prompt;
    std::string currentPath;
    
    Runtime* runtime;
    Debugger* debugger;
    TypeChecker* typeChecker;
    void* ideInterface;  // Связь с IDE для визуализации
    
    Parser parser;
    SemanticAnalyzer analyzer;
    LLVMCodeGen codeGen;
    
    std::vector<REPLHistoryEntry> history;
    std::map<std::string, REPLVariable> variables;
    std::map<std::string, REPLCommand> commands;
    
    // Callback storage (Qt-free signals)
    VisualizationCallback visualizationCallback_;
    VariableChangedCallback variableChangedCallback_;
    HistoryChangedCallback historyChangedCallback_;
    OutputCallback outputCallback_;
    ErrorCallback errorCallback_;
    PromptCallback promptCallback_;
    
    int64_t startTime;
    int commandCount;
};

} // namespace proxima

#endif // PROXIMA_REPL_H
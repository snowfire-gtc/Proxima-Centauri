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
    long long executionTime;
};

struct REPLVariable {
    std::string name;
    std::string type;
    RuntimeValue value;     // RuntimeValue из Interpreter
    int size;
    int bytes;
};

class REPL {
public:
    REPL(Interpreter& interpreter);
    ~REPL();
    
    // Запуск REPL
    void start();
    
    // Остановка REPL
    void stop();
    
    // Обработка команды
    std::string handleCommand(const std::string& input);
    
    // Выполнение кода
    std::string executeCode(const std::string& code);
    
    // Парсинг команды
    REPLCommand parseCommand(const std::string& input);
    
    // Добавление в историю
    void addToHistory(const std::string& code, const std::string& output, bool hasError, long long execTime);
    
    // Получение истории
    const std::vector<REPLHistoryEntry>& getHistory() const;
    
    // Очистка истории
    void clearHistory();
    
    // Список переменных
    std::vector<REPLVariable> getVariables() const;
    
    // Информация о переменной
    REPLVariable getVariableInfo(const std::string& name) const;
    
private:
    Interpreter& interpreter;
    Debugger debugger;
    bool running;
    std::vector<REPLHistoryEntry> history;
    std::map<std::string, REPLVariable> variables;
    
    // Вспомогательные методы
    std::string handleHelp();
    std::string handleExit();
    std::string handleClear();
    std::string handleHistory();
    std::string handleLoad(const std::string& filename);
    std::string handleSave(const std::string& filename);
    std::string handleWhois(const std::string& varName);
    std::string handleWhos();
    std::string handleShow(const std::string& expr);
    std::string handleType(const std::string& varName);
    std::string handleSize(const std::string& varName);
    std::string handleMethods(const std::string& className);
    std::string handleFields(const std::string& className);
    std::string handleVersion();
    std::string handleConfig();
    std::string handleReset();
};

} // namespace proxima

#endif // PROXIMA_REPL_H

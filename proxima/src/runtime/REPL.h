#ifndef PROXIMA_REPL_H
#define PROXIMA_REPL_H

#include "../parser/Parser.h"
#include "../semantic/SemanticAnalyzer.h"
#include "../semantic/TypeChecker.h"
#include "../codegen/LLVMCodeGen.h"
#include "../runtime/Runtime.h"
#include "../runtime/Debugger.h"
#include <QString>
#include <QVector>
#include <QMap>

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
    QString code;           // Код Proxima
    QString output;         // Результат выполнения
    QDateTime timestamp;
    bool hasError;
    qint64 executionTime;
};

struct REPLVariable {
    QString name;
    QString type;
    RuntimeValue value;     // RuntimeValue из Interpreter
    int size;
    int bytes;
    QDateTime lastModified;
};

class REPL : public QObject {
    Q_OBJECT
    
public:
    explicit REPL(QObject *parent = nullptr);
    ~REPL();
    
    // Инициализация
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    // Выполнение кода Proxima
    QString execute(const QString& code);
    QString evaluate(const QString& expression);
    
    // REPL команды (минимальный набор)
    REPLCommand parseCommand(const QString& input);
    QString executeCommand(REPLCommand cmd, const QStringList& args);
    
    // Визуализация (интеграция с language.txt #46)
    void showVariable(const QString& varName);
    void plotVector(const QString& varName);
    void showMatrix(const QString& varName);
    void showLayer3D(const QString& varName);
    void showCollection(const QString& varName);
    void inspectObject(const QString& varName);
    
    // Управление переменными
    QMap<QString, REPLVariable> getVariables() const { return variables; }
    bool hasVariable(const QString& name) const;
    void clearVariables();
    
    // История
    void addToHistory(const QString& code, const QString& output, bool hasError);
    void clearHistory();
    void saveHistory(const QString& path);
    void loadHistory(const QString& path);
    
    // Конфигурация
    void setPrompt(const QString& prompt);
    QString getPrompt() const { return prompt; }
    void setEchoInput(bool enable);
    
    // Интеграция
    void setRuntime(Runtime* runtime);
    void setDebugger(Debugger* debugger);
    void setTypeChecker(TypeChecker* checker);
    void setIDEInterface(QObject* ide);  // Для связи с визуализаторами IDE
    
    // Автодополнение
    QStringList getCompletions(const QString& prefix) const;
    
signals:
    void outputReceived(const QString& output);
    void errorReceived(const QString& error);
    void promptDisplayed();
    void visualizationRequested(const QString& varName, const QString& visType);
    void variableChanged(const QString& name);
    
private:
    void registerBuiltinFunctions();
    void initializeWorkspace();
    QString executeProximaCode(const QString& code);
    QString formatOutput(const RuntimeValue& value) const;
    void updateVariable(const QString& name, const RuntimeValue& value);
    
    // Команды
    QString helpCommand(const QStringList& args);
    QString whoisCommand(const QStringList& args);
    QString whosCommand(const QStringList& args);
    QString showCommand(const QStringList& args);
    QString typeCommand(const QStringList& args);
    QString sizeCommand(const QStringList& args);
    QString methodsCommand(const QStringList& args);
    QString fieldsCommand(const QStringList& args);
    QString exitCommand(const QStringList& args);
    QString versionCommand(const QStringList& args);
    QString clearCommand(const QStringList& args);
    QString historyCommand(const QStringList& args);
    QString loadCommand(const QStringList& args);
    QString saveCommand(const QStringList& args);
    QString configCommand(const QStringList& args);
    QString resetCommand(const QStringList& args);
    
    bool initialized;
    bool running;
    bool echoInput;
    
    QString prompt;
    QString currentPath;
    
    Runtime* runtime;
    Debugger* debugger;
    TypeChecker* typeChecker;
    QObject* ideInterface;  // Связь с IDE для визуализации
    
    Parser parser;
    SemanticAnalyzer analyzer;
    LLVMCodeGen codeGen;
    
    QVector<REPLHistoryEntry> history;
    QMap<QString, REPLVariable> variables;
    QMap<QString, REPLCommand> commands;
    
    qint64 startTime;
    int commandCount;
};

} // namespace proxima

#endif // PROXIMA_REPL_H
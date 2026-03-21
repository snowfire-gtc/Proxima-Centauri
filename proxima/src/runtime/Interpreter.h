#ifndef PROXIMA_INTERPRETER_H
#define PROXIMA_INTERPRETER_H

#include "../parser/AST.h"
#include "../semantic/SymbolTable.h"
#include "../semantic/TypeChecker.h"
#include <unordered_map>
#include <variant>
#include <stack>

namespace proxima {

using RuntimeValue = std::variant<
    int64_t,
    double,
    std::string,
    bool,
    std::vector<RuntimeValue>,
    std::vector<std::vector<RuntimeValue>>,
    void*
>;

struct ExecutionContext {
    SymbolTable symbolTable;
    std::unordered_map<std::string, RuntimeValue> variables;
    std::stack<std::string> callStack;
    int currentLine;
    std::string currentFile;
    bool shouldStop;
    bool shouldPause;
};

class Interpreter {
public:
    Interpreter();
    ~Interpreter();
    
    // Execution modes
    void setInterpretMode(bool enable);
    bool isInterpretMode() const { return interpretMode; }
    
    // Execution control
    bool execute(ProgramNodePtr program);
    bool executeStatement(StatementNodePtr stmt, ExecutionContext& context);
    RuntimeValue executeExpression(ExpressionNodePtr expr, ExecutionContext& context);
    
    // Debug control
    void pause();
    void resume();
    void step();
    void stop();
    
    // State
    ExecutionContext& getCurrentContext() { return currentContext; }
    bool isPaused() const { return paused; }
    bool isRunning() const { return running; }
    
private:
    bool interpretMode;
    bool running;
    bool paused;
    ExecutionContext currentContext;
    TypeChecker typeChecker;
    
    // Statement handlers
    bool executeAssignment(AssignmentNodePtr node, ExecutionContext& context);
    bool executeIf(IfNodePtr node, ExecutionContext& context);
    bool executeFor(ForNodePtr node, ExecutionContext& context);
    bool executeWhile(WhileNodePtr node, ExecutionContext& context);
    bool executeReturn(ReturnNodePtr node, ExecutionContext& context);
    
    // Expression handlers
    RuntimeValue evaluateBinaryOp(BinaryOpNodePtr node, ExecutionContext& context);
    RuntimeValue evaluateUnaryOp(UnaryOpNodePtr node, ExecutionContext& context);
    RuntimeValue evaluateCall(CallNodePtr node, ExecutionContext& context);
    RuntimeValue evaluateIdentifier(IdentifierNodePtr node, ExecutionContext& context);
    RuntimeValue evaluateLiteral(LiteralNodePtr node, ExecutionContext& context);
    
    // Debug support
    void checkBreakpoint(int line, const std::string& file);
    void notifyLineExecution(int line, const std::string& file);
};

} // namespace proxima

#endif // PROXIMA_INTERPRETER_H
#ifndef PROXIMA_SEMANTIC_ANALYZER_H
#define PROXIMA_SEMANTIC_ANALYZER_H

#include "parser/AST.h"
#include "semantic/TypeChecker.h"
#include "semantic/SymbolTable.h"
#include "semantic/ArgumentValidator.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace proxima {

struct SemanticError {
    std::string message;
    Token token;
    int severity;  // 0=warning, 1=error, 2=fatal
};

struct SemanticWarning {
    std::string message;
    Token token;
};

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    
    void analyze(ProgramNodePtr program);
    
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    
    const std::vector<SemanticError>& getErrors() const { return errors; }
    const std::vector<SemanticWarning>& getWarnings() const { return warnings; }
    
    SymbolTable& getSymbolTable() { return symbolTable; }
    TypeChecker& getTypeChecker() { return typeChecker; }
    ArgumentValidator& getArgumentValidator() { return argumentValidator; }
    
    void setVerboseLevel(int level) { verboseLevel = level; }
    int getVerboseLevel() const { return verboseLevel; }
    
    // Валидация конструкции arguments
    void validateArgumentsBlock(ArgumentsNodePtr argumentsNode, 
                               FunctionDeclNodePtr functionNode);
    
    // Проверка соответствия аргументов при вызове
    void validateFunctionCall(const std::vector<ValidationRule>& rules,
                             const std::vector<ExpressionNodePtr>& args,
                             const Token& token);
    
private:
    void analyzeDeclaration(DeclarationNodePtr decl);
    void analyzeStatement(StatementNodePtr stmt);
    void analyzeExpression(ExpressionNodePtr expr);
    
    void analyzeFunction(FunctionDeclNodePtr func);
    void analyzeClass(ClassDeclNodePtr cls);
    void analyzeInterface(InterfaceDeclNodePtr iface);
    void analyzeVariable(VariableDeclNodePtr var);
    
    void analyzeIf(IfNodePtr ifStmt);
    void analyzeFor(ForNodePtr forStmt);
    void analyzeWhile(WhileNodePtr whileStmt);
    void analyzeDoWhile(DoWhileNodePtr doStmt);
    void analyzeSwitch(SwitchNodePtr switchStmt);
    void analyzeReturn(ReturnNodePtr returnStmt);
    void analyzeBreak(BreakNodePtr breakStmt);
    void analyzeContinue(ContinueNodePtr continueStmt);
    
    void analyzeAssignment(AssignmentNodePtr assign);
    void analyzeBinaryOp(BinaryOpNodePtr binop);
    void analyzeUnaryOp(UnaryOpNodePtr unary);
    void analyzeCall(CallNodePtr call);
    void analyzeIndex(IndexNodePtr index);
    void analyzeMemberAccess(MemberAccessNodePtr member);
    
    void checkFunctionCall(const std::string& funcName, 
                          const std::vector<ExpressionNodePtr>& args,
                          const Token& token);
    void checkArrayIndexing(ExpressionNodePtr expr, const std::string& arrayType);
    void check1BasedIndexing(ExpressionNodePtr expr);
    
    void addError(const std::string& message, const Token& token, int severity = 1);
    void addWarning(const std::string& message, const Token& token);
    void log(int level, const std::string& message);
    
    std::vector<SemanticError> errors;
    std::vector<SemanticWarning> warnings;
    
    SymbolTable symbolTable;
    TypeChecker typeChecker;
    ArgumentValidator argumentValidator;
    
    int verboseLevel;
    std::string currentFunction;
    std::string currentClass;
    std::string currentNamespace;
    
    std::unordered_map<std::string, std::vector<ValidationRule>> functionValidationRules;
};

} // namespace proxima

#endif // PROXIMA_SEMANTIC_ANALYZER_H
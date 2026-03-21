#ifndef PROXIMA_SEMANTIC_ANALYZER_H
#define PROXIMA_SEMANTIC_ANALYZER_H

#include "../parser/AST.h"
#include "SymbolTable.h"
#include "TypeChecker.h"
#include <vector>
#include <string>
#include <memory>

namespace proxima {

struct SemanticError {
    std::string message;
    std::string filename;
    int line;
    int column;
    
    SemanticError(const std::string& msg, const std::string& file, int l, int c)
        : message(msg), filename(file), line(l), column(c) {}
};

struct SemanticWarning {
    std::string message;
    std::string filename;
    int line;
    int column;
    
    SemanticWarning(const std::string& msg, const std::string& file, int l, int c)
        : message(msg), filename(file), line(l), column(c) {}
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
    
    void setVerboseLevel(int level) { verboseLevel = level; }
    
private:
    SymbolTable symbolTable;
    TypeChecker typeChecker;
    std::vector<SemanticError> errors;
    std::vector<SemanticWarning> warnings;
    int verboseLevel;
    std::string currentFilename;
    std::string currentNamespace;
    std::string currentClass;
    std::string currentFunction;
    
    void analyzeDeclaration(DeclarationNodePtr decl);
    void analyzeStatement(StatementNodePtr stmt);
    void analyzeExpression(ExpressionNodePtr expr);
    
    void analyzeVariableDecl(VariableDeclNodePtr var);
    void analyzeFunctionDecl(FunctionDeclNodePtr func);
    void analyzeClassDecl(ClassDeclNodePtr cls);
    void analyzeInterfaceDecl(InterfaceDeclNodePtr iface);
    
    void analyzeIf(IfNodePtr ifStmt);
    void analyzeFor(ForNodePtr forStmt);
    void analyzeWhile(WhileNodePtr whileStmt);
    void analyzeReturn(ReturnNodePtr retStmt);
    void analyzeBlock(BlockNodePtr block);
    
    void checkTypeCompatibility(const std::string& expected, const std::string& actual, 
                                const Token& token);
    void checkVariableDeclaration(const std::string& name, const std::string& type,
                                  const Token& token);
    void checkFunctionCall(const std::string& name, const std::vector<ExpressionNodePtr>& args,
                          const Token& token);
    
    void addError(const std::string& message, const Token& token);
    void addWarning(const std::string& message, const Token& token);
    void log(int level, const std::string& message);
};

} // namespace proxima

#endif // PROXIMA_SEMANTIC_ANALYZER_H
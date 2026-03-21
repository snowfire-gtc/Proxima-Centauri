#include "SemanticAnalyzer.h"
#include <iostream>
#include <algorithm>

namespace proxima {

SemanticAnalyzer::SemanticAnalyzer() : verboseLevel(2) {}

void SemanticAnalyzer::analyze(ProgramNodePtr program) {
    errors.clear();
    warnings.clear();
    symbolTable.clear();
    typeChecker.clear();
    
    currentFilename = program->filename;
    currentNamespace = "";
    currentClass = "";
    currentFunction = "";
    
    log(1, "Starting semantic analysis for: " + currentFilename);
    
    // First pass: collect all declarations
    for (const auto& decl : program->declarations) {
        analyzeDeclaration(decl);
    }
    
    // Second pass: analyze statements and expressions
    for (const auto& stmt : program->statements) {
        analyzeStatement(stmt);
    }
    
    log(1, "Semantic analysis completed. Errors: " + 
           std::to_string(errors.size()) + 
           ", Warnings: " + std::to_string(warnings.size()));
}

void SemanticAnalyzer::analyzeDeclaration(DeclarationNodePtr decl) {
    if (!decl) return;
    
    switch (decl->nodeType) {
        case NodeType::VARIABLE_DECL:
            analyzeVariableDecl(std::static_pointer_cast<VariableDeclNode>(decl));
            break;
        case NodeType::FUNCTION_DECL:
            analyzeFunctionDecl(std::static_pointer_cast<FunctionDeclNode>(decl));
            break;
        case NodeType::CLASS_DECL:
            analyzeClassDecl(std::static_pointer_cast<ClassDeclNode>(decl));
            break;
        case NodeType::INTERFACE_DECL:
            analyzeInterfaceDecl(std::static_pointer_cast<InterfaceDeclNode>(decl));
            break;
        case NodeType::NAMESPACE_DECL:
            // Handle namespace
            currentNamespace = decl->name;
            for (const auto& innerDecl : decl->getDeclarations()) {
                analyzeDeclaration(innerDecl);
            }
            currentNamespace = "";
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::analyzeVariableDecl(VariableDeclNodePtr var) {
    if (!var) return;
    
    std::string fullName = var->name;
    if (!currentNamespace.empty()) {
        fullName = currentNamespace + "::" + var->name;
    }
    
    // Check for redeclaration
    if (symbolTable.exists(fullName)) {
        addError("Variable '" + var->name + "' already declared", var->token);
        return;
    }
    
    // Infer type if not specified
    std::string type = var->typeName;
    if (type == "auto" && var->initializer) {
        type = typeChecker.inferType(var->initializer);
    }
    
    if (type.empty()) {
        type = "double"; // Default type
        addWarning("Type not specified, defaulting to 'double'", var->token);
    }
    
    // Add to symbol table
    Symbol symbol;
    symbol.name = var->name;
    symbol.fullName = fullName;
    symbol.type = type;
    symbol.kind = SymbolKind::VARIABLE;
    symbol.line = var->token.line;
    symbol.column = var->token.column;
    symbol.filename = currentFilename;
    
    symbolTable.add(symbol);
    
    // Analyze initializer
    if (var->initializer) {
        analyzeExpression(var->initializer);
        
        std::string initType = typeChecker.inferType(var->initializer);
        checkTypeCompatibility(type, initType, var->token);
    }
    
    log(3, "Variable declared: " + fullName + " : " + type);
}

void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclNodePtr func) {
    if (!func) return;
    
    std::string fullName = func->name;
    if (!currentNamespace.empty()) {
        fullName = currentNamespace + "::" + func->name;
    }
    
    // Check for redeclaration (allow overloading)
    if (symbolTable.exists(fullName)) {
        // Check if it's an overload
        auto existing = symbolTable.get(fullName);
        if (existing.kind == SymbolKind::FUNCTION) {
            // Allow overloading with different parameters
            log(3, "Function overload: " + fullName);
        } else {
            addError("Function '" + func->name + "' conflicts with existing symbol", func->token);
            return;
        }
    }
    
    // Add function to symbol table
    Symbol symbol;
    symbol.name = func->name;
    symbol.fullName = fullName;
    symbol.type = func->returnType;
    symbol.kind = SymbolKind::FUNCTION;
    symbol.line = func->token.line;
    symbol.column = func->token.column;
    symbol.filename = currentFilename;
    
    // Add parameters to symbol
    for (const auto& param : func->parameters) {
        symbol.parameters.push_back({param.first, param.second});
    }
    
    symbolTable.add(symbol);
    
    // Analyze function body
    std::string prevFunction = currentFunction;
    currentFunction = fullName;
    
    // Add parameters to local scope
    symbolTable.pushScope();
    for (const auto& param : func->parameters) {
        Symbol paramSymbol;
        paramSymbol.name = param.first;
        paramSymbol.fullName = currentFunction + "::" + param.first;
        paramSymbol.type = param.second;
        paramSymbol.kind = SymbolKind::PARAMETER;
        symbolTable.add(paramSymbol);
    }
    
    analyzeStatement(func->body);
    symbolTable.popScope();
    
    currentFunction = prevFunction;
    
    log(3, "Function declared: " + fullName + " : " + func->returnType);
}

void SemanticAnalyzer::analyzeStatement(StatementNodePtr stmt) {
    if (!stmt) return;
    
    switch (stmt->nodeType) {
        case NodeType::IF_STATEMENT:
            analyzeIf(std::static_pointer_cast<IfNode>(stmt));
            break;
        case NodeType::FOR_LOOP:
            analyzeFor(std::static_pointer_cast<ForNode>(stmt));
            break;
        case NodeType::WHILE_LOOP:
            analyzeWhile(std::static_pointer_cast<WhileNode>(stmt));
            break;
        case NodeType::RETURN_STATEMENT:
            analyzeReturn(std::static_pointer_cast<ReturnNode>(stmt));
            break;
        case NodeType::BLOCK:
            analyzeBlock(std::static_pointer_cast<BlockNode>(stmt));
            break;
        case NodeType::ASSIGNMENT:
            analyzeExpression(std::static_pointer_cast<ExpressionNode>(stmt));
            break;
        case NodeType::EXPRESSION_STATEMENT:
            analyzeExpression(std::static_pointer_cast<ExpressionNode>(stmt));
            break;
        default:
            break;
    }
}

void SemanticAnalyzer::analyzeIf(IfNodePtr ifStmt) {
    if (!ifStmt) return;
    
    // Check condition type
    analyzeExpression(ifStmt->condition);
    std::string condType = typeChecker.inferType(ifStmt->condition);
    
    if (condType != "bool" && condType != "auto") {
        addWarning("Condition expression type is '" + condType + "', expected 'bool'", 
                  ifStmt->condition->token);
    }
    
    // Analyze branches
    symbolTable.pushScope();
    analyzeStatement(ifStmt->thenBranch);
    symbolTable.popScope();
    
    if (ifStmt->elseBranch) {
        symbolTable.pushScope();
        analyzeStatement(ifStmt->elseBranch);
        symbolTable.popScope();
    }
}

void SemanticAnalyzer::analyzeFor(ForNodePtr forStmt) {
    if (!forStmt) return;
    
    // Add loop variable to scope
    symbolTable.pushScope();
    
    Symbol loopVar;
    loopVar.name = forStmt->variable;
    loopVar.fullName = currentFunction + "::" + forStmt->variable;
    loopVar.type = "auto"; // Will be inferred from iterable
    loopVar.kind = SymbolKind::VARIABLE;
    symbolTable.add(loopVar);
    
    // Analyze iterable
    analyzeExpression(forStmt->iterable);
    
    // Analyze body
    analyzeStatement(forStmt->body);
    
    symbolTable.popScope();
}

void SemanticAnalyzer::analyzeReturn(ReturnNodePtr retStmt) {
    if (!retStmt) return;
    
    if (currentFunction.empty()) {
        addError("Return statement outside of function", retStmt->token);
        return;
    }
    
    if (retStmt->value) {
        analyzeExpression(retStmt->value);
        
        // Get function return type
        auto funcSymbol = symbolTable.get(currentFunction);
        if (funcSymbol.type != "void" && funcSymbol.type != "[]") {
            std::string returnType = typeChecker.inferType(retStmt->value);
            checkTypeCompatibility(funcSymbol.type, returnType, retStmt->token);
        } else {
            addWarning("Return value in void function", retStmt->token);
        }
    }
}

void SemanticAnalyzer::analyzeBlock(BlockNodePtr block) {
    if (!block) return;
    
    symbolTable.pushScope();
    
    for (const auto& stmt : block->statements) {
        analyzeStatement(stmt);
    }
    
    symbolTable.popScope();
}

// Добавлена поддержка вызова статических методов класса
void SemanticAnalyzer::analyzeExpression(ExpressionNodePtr expr) {
    if (!expr) return;
    
    switch (expr->nodeType) {
    	case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            analyzeExpression(call->callee);
            
            // Проверяем, является ли вызов статическим методом класса
            if (call->callee->nodeType == NodeType::MEMBER_ACCESS) {
                auto memberAccess = std::static_pointer_cast<MemberAccessNode>(call->callee);
                
                if (memberAccess->object->nodeType == NodeType::TYPE_REF) {
                    // Это вызов статического метода класса: ClassName.method()
                    auto typeRef = std::static_pointer_cast<TypeRefNode>(memberAccess->object);
                    std::string className = typeRef->typeName;
                    std::string methodName = memberAccess->memberName;
                    
                    // Проверяем существование класса
                    if (!typeChecker.typeExists(className)) {
                        addError("Unknown class: '" + className + "'", call->token);
                        break;
                    }
                    
                    // Проверяем существование статического метода
                    auto staticMethods = stdlib::StdLibRegistry::getInstance()
                        .getStaticMethods(className);
                    
                    if (std::find(staticMethods.begin(), staticMethods.end(), 
                                 methodName) == staticMethods.end()) {
                        addWarning("Static method '" + methodName + "' not found in class '" + 
                                  className + "'", call->token);
                        
                        // Предоставляем подсказку
                        std::string hint = "Available static methods: ";
                        for (size_t i = 0; i < staticMethods.size() && i < 5; i++) {
                            if (i > 0) hint += ", ";
                            hint += staticMethods[i];
                        }
                        if (staticMethods.size() > 5) {
                            hint += ", ...";
                        }
                        addWarning(hint, call->token);
                    }
                    
                    // Проверяем аргументы
                    std::vector<ExpressionNodePtr> args;
                    for (const auto& arg : call->arguments) {
                        analyzeExpression(arg);
                        args.push_back(arg);
                    }
                    
                    checkFunctionCall(className + "::" + methodName, args, call->token);
                    break;
                }
            }
            
            // Обычный вызов функции
            std::vector<ExpressionNodePtr> args;
            for (const auto& arg : call->arguments) {
                analyzeExpression(arg);
                args.push_back(arg);
            }
            
            if (call->callee->nodeType == NodeType::IDENTIFIER) {
                auto ident = std::static_pointer_cast<IdentifierNode>(call->callee);
                checkFunctionCall(ident->name, args, call->token);
            }
            break;
        }
        
        case NodeType::MEMBER_ACCESS: {
            auto memberAccess = std::static_pointer_cast<MemberAccessNode>(expr);
            analyzeExpression(memberAccess->object);
            
            // Проверяем тип объекта
            std::string objectType = typeChecker.inferType(memberAccess->object);
            
            if (typeChecker.typeExists(objectType)) {
                // Проверяем существование поля/метода
                auto instanceMethods = stdlib::StdLibRegistry::getInstance()
                    .getInstanceMethods(objectType);
                auto properties = stdlib::StdLibRegistry::getInstance()
                    .getProperties(objectType);
                
                bool found = std::find(instanceMethods.begin(), instanceMethods.end(),
                              memberAccess->memberName) != instanceMethods.end() ||
                            std::find(properties.begin(), properties.end(),
                              memberAccess->memberName) != properties.end();
                
                if (!found) {
                    addWarning("Member '" + memberAccess->memberName + "' not found in type '" + 
                              objectType + "'", memberAccess->token);
                    
                    // Предоставляем подсказку с доступными методами
                    std::string hint = "Available members: ";
                    size_t count = 0;
                    for (const auto& method : instanceMethods) {
                        if (count > 0) hint += ", ";
                        hint += method;
                        count++;
                        if (count >= 10) {
                            hint += ", ...";
                            break;
                        }
                    }
                    addWarning(hint, memberAccess->token);
                }
            }
            break;
        }
        
        case NodeType::BINARY_OP: {
            auto binop = std::static_pointer_cast<BinaryOpNode>(expr);
            analyzeExpression(binop->left);
            analyzeExpression(binop->right);
            
            std::string leftType = typeChecker.inferType(binop->left);
            std::string rightType = typeChecker.inferType(binop->right);
            
            typeChecker.checkBinaryOperator(binop->op, leftType, rightType, expr->token);
            break;
        }
        
        case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            analyzeExpression(call->callee);
            
            std::vector<ExpressionNodePtr> args;
            for (const auto& arg : call->arguments) {
                analyzeExpression(arg);
                args.push_back(arg);
            }
            
            if (call->callee->nodeType == NodeType::IDENTIFIER) {
                auto ident = std::static_pointer_cast<IdentifierNode>(call->callee);
                checkFunctionCall(ident->name, args, call->token);
            }
            break;
        }
        
        case NodeType::IDENTIFIER: {
            auto ident = std::static_pointer_cast<IdentifierNode>(expr);
            if (!symbolTable.exists(ident->name) && 
                !symbolTable.existsInGlobal(ident->name)) {
                addWarning("Undefined variable: '" + ident->name + "'", ident->token);
            }
            break;
        }
        
        default:
            break;
    }
}

void SemanticAnalyzer::checkTypeCompatibility(const std::string& expected, 
                                              const std::string& actual,
                                              const Token& token) {
    if (expected == "auto" || actual == "auto") {
        return; // Skip check for auto
    }
    
    if (expected == actual) {
        return; // Types match
    }
    
    // Check implicit conversions
    if (typeChecker.canConvert(actual, expected)) {
        log(4, "Implicit conversion: " + actual + " -> " + expected);
        return;
    }
    
    addWarning("Type mismatch: expected '" + expected + "', got '" + actual + "'", token);
}

void SemanticAnalyzer::addError(const std::string& message, const Token& token) {
    errors.emplace_back(message, currentFilename, token.line, token.column);
    std::cerr << "ERROR: " << currentFilename << ":" << token.line << ":" 
              << token.column << " - " << message << std::endl;
}

void SemanticAnalyzer::addWarning(const std::string& message, const Token& token) {
    warnings.emplace_back(message, currentFilename, token.line, token.column);
    if (verboseLevel >= 2) {
        std::cout << "WARNING: " << currentFilename << ":" << token.line << ":" 
                  << token.column << " - " << message << std::endl;
    }
}

void SemanticAnalyzer::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Semantic] " << message << std::endl;
    }
}

} // namespace proxima

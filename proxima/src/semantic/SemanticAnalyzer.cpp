#include "SemanticAnalyzer.h"
#include "utils/Logger.h"
#include "runtime/Runtime.h"

#include <algorithm>
#include <cmath>
#include <stack>
#include <set>
#include <memory>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

SemanticAnalyzer::SemanticAnalyzer()
    : verboseLevel(2)
    , currentFunction("")
    , currentClass("")
    , currentNamespace("") {

    LOG_INFO("SemanticAnalyzer initialized");
    LOG_DEBUG("Verbose level: " + std::to_string(verboseLevel));
}

SemanticAnalyzer::~SemanticAnalyzer() {
    LOG_DEBUG("SemanticAnalyzer destroyed");
}

// ============================================================================
// Основной метод анализа
// ============================================================================

void SemanticAnalyzer::analyze(ProgramNodePtr program) {
    if (!program) {
        LOG_ERROR("Null program node");
        addError("Null program node", Token());
        return;
    }

    LOG_INFO("========================================");
    LOG_INFO("Starting semantic analysis...");
    LOG_INFO("========================================");
    LOG_DEBUG("Program has " + std::to_string(program->declarations.size()) +
              " declarations and " +
              std::to_string(program->statements.size()) + " statements");

    errors.clear();
    warnings.clear();

    // Анализ пространств имён
    if (!program->namespace_.empty()) {
        currentNamespace = program->namespace_;
        LOG_DEBUG("Entering namespace: " + currentNamespace);

        symbolTable.pushScope();
        symbolTable.addNamespace(currentNamespace);
    }

    // Первый проход - объявление символов
    LOG_DEBUG("First pass: declaring symbols...");
    for (const auto& decl : program->declarations) {
        analyzeDeclaration(decl);
    }

    // Второй проход - анализ тел
    LOG_DEBUG("Second pass: analyzing bodies...");
    for (const auto& decl : program->declarations) {
        analyzeDeclarationBody(decl);
    }

    // Анализ statement'ов глобальной области
    LOG_DEBUG("Analyzing global statements...");
    for (const auto& stmt : program->statements) {
        analyzeStatement(stmt);
    }

    // Закрытие пространства имён
    if (!currentNamespace.empty()) {
        symbolTable.popScope();
        LOG_DEBUG("Exiting namespace: " + currentNamespace);
        currentNamespace.clear();
    }

    // Проверка 1-based индексации во всём дереве
    LOG_DEBUG("Checking 1-based indexing...");
    for (const auto& decl : program->declarations) {
        check1BasedIndexing(decl);
    }
    for (const auto& stmt : program->statements) {
        check1BasedIndexing(stmt);
    }

    LOG_INFO("========================================");
    LOG_INFO("Semantic analysis completed");
    LOG_INFO("Errors: " + std::to_string(errors.size()));
    LOG_INFO("Warnings: " + std::to_string(warnings.size()));
    LOG_INFO("========================================");

    if (!errors.empty()) {
        LOG_ERROR("Semantic analysis failed with " +
                 std::to_string(errors.size()) + " errors");
    }
}

// ============================================================================
// Анализ деклараций
// ============================================================================

void SemanticAnalyzer::analyzeDeclaration(DeclarationNodePtr decl) {
    if (!decl) return;

    LOG_DEBUG("Analyzing declaration at line " +
              std::to_string(decl->token.line));

    switch (decl->nodeType) {
        case NodeType::FUNCTION_DECL:
            analyzeFunction(std::static_pointer_cast<FunctionDeclNode>(decl));
            break;

        case NodeType::CLASS_DECL:
            analyzeClass(std::static_pointer_cast<ClassDeclNode>(decl));
            break;

        case NodeType::INTERFACE_DECL:
            analyzeInterface(std::static_pointer_cast<InterfaceDeclNode>(decl));
            break;

        case NodeType::VARIABLE_DECL:
            analyzeVariable(std::static_pointer_cast<VariableDeclNode>(decl));
            break;

        case NodeType::NAMESPACE_DECL:
            analyzeNamespace(std::static_pointer_cast<NamespaceDeclNode>(decl));
            break;

        case NodeType::TEMPLATE_DECL:
            analyzeTemplate(std::static_pointer_cast<TemplateDeclNode>(decl));
            break;

        default:
            LOG_WARNING("Unknown declaration type: " +
                       std::to_string(static_cast<int>(decl->nodeType)));
            break;
    }
}

void SemanticAnalyzer::analyzeDeclarationBody(DeclarationNodePtr decl) {
    if (!decl) return;

    switch (decl->nodeType) {
        case NodeType::FUNCTION_DECL: {
            auto func = std::static_pointer_cast<FunctionDeclNode>(decl);
            // Анализ тела функции уже выполнен в analyzeFunction
            break;
        }

        case NodeType::CLASS_DECL: {
            auto cls = std::static_pointer_cast<ClassDeclNode>(decl);
            // Анализ членов класса
            for (const auto& member : cls->members) {
                analyzeDeclaration(member);
            }
            break;
        }

        default:
            break;
    }
}

// ============================================================================
// Анализ функций
// ============================================================================

void SemanticAnalyzer::analyzeFunction(FunctionDeclNodePtr func) {
    if (!func) return;

    LOG_DEBUG("Analyzing function: " + func->name);

    std::string prevFunction = currentFunction;
    currentFunction = func->name;

    symbolTable.pushScope();

    // Добавление параметров в таблицу символов
    LOG_DEBUG("Adding " + std::to_string(func->parameters.size()) + " parameters");
    for (const auto& param : func->parameters) {
        Symbol symbol;
        symbol.name = param.first;
        symbol.type = param.second;
        symbol.kind = SymbolKind::PARAMETER;
        symbol.line = func->token.line;
        symbol.column = func->token.column;
        symbol.filename = func->filename;
        symbol.scope = symbolTable.getCurrentScope();
        symbol.access = SymbolAccess::Public;  // Параметры всегда публичные

        if (!symbolTable.add(symbol)) {
            addWarning("Parameter '" + param.first + "' shadows outer scope variable",
                      func->token);
        }

        LOG_DEBUG("  Parameter: " + param.first + " : " + param.second);
    }

    // Валидация конструкции arguments
    if (func->arguments) {
        LOG_DEBUG("Validating arguments block");
        validateArgumentsBlock(func->arguments, func);
    }

    // Проверка типа возврата
    if (!func->returnType.empty() && func->returnType != "void") {
        if (!typeChecker.typeExists(func->returnType)) {
            addWarning("Unknown return type: " + func->returnType, func->token);
        }
    }

    // Анализ тела функции
    LOG_DEBUG("Analyzing function body");
    for (const auto& stmt : func->body) {
        analyzeStatement(stmt);
    }

    // Проверка наличия return в функциях с типом возврата
    if (!func->returnType.empty() && func->returnType != "void") {
        if (!hasReturnStatement(func->body)) {
            addWarning("Function '" + func->name +
                      "' should return a value of type " + func->returnType,
                      func->token);
        }
    }

    symbolTable.popScope();
    currentFunction = prevFunction;

    LOG_DEBUG("Function analysis completed: " + func->name);
}

bool SemanticAnalyzer::hasReturnStatement(const std::vector<StatementNodePtr>& body) {
    for (const auto& stmt : body) {
        if (stmt->nodeType == NodeType::RETURN_STATEMENT) {
            return true;
        }

        // Рекурсивная проверка вложенных структур
        if (stmt->nodeType == NodeType::BLOCK) {
            auto block = std::static_pointer_cast<BlockNode>(stmt);
            if (hasReturnStatement(block->statements)) {
                return true;
            }
        }

        if (stmt->nodeType == NodeType::IF_STATEMENT) {
            auto ifStmt = std::static_pointer_cast<IfNode>(stmt);
            if (hasReturnStatement(ifStmt->thenBranch)) {
                return true;
            }
            if (ifStmt->elseBranch && hasReturnStatement(ifStmt->elseBranch)) {
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// Анализ классов
// ============================================================================

void SemanticAnalyzer::analyzeClass(ClassDeclNodePtr cls) {
    if (!cls) return;

    LOG_DEBUG("Analyzing class: " + cls->name);

    std::string prevClass = currentClass;
    currentClass = cls->name;

    // Проверка наследования
    if (!cls->parentClass.empty()) {
        Symbol parentSymbol = symbolTable.get(cls->parentClass);
        if (parentSymbol.kind != SymbolKind::CLASS) {
            addError("Parent class '" + cls->parentClass + "' not found",
                    cls->token);
        }
    }

    // Проверка интерфейсов
    for (const auto& iface : cls->interfaces) {
        Symbol ifaceSymbol = symbolTable.get(iface);
        if (ifaceSymbol.kind != SymbolKind::INTERFACE) {
            addError("Interface '" + iface + "' not found", cls->token);
        }
    }

    // Добавление класса в таблицу символов
    Symbol classSymbol;
    classSymbol.name = cls->name;
    classSymbol.type = "class";
    classSymbol.kind = SymbolKind::CLASS;
    classSymbol.line = cls->token.line;
    classSymbol.column = cls->token.column;
    classSymbol.filename = cls->filename;
    classSymbol.scope = symbolTable.getCurrentScope();

    if (!symbolTable.add(classSymbol)) {
        addError("Class '" + cls->name + "' already defined", cls->token);
    }

    symbolTable.pushScope();

    // Добавление implicit 'this' parameter
    Symbol thisSymbol;
    thisSymbol.name = "this";
    thisSymbol.type = cls->name;
    thisSymbol.kind = SymbolKind::VARIABLE;
    thisSymbol.access = SymbolAccess::Private;
    symbolTable.add(thisSymbol);

    // Анализ членов класса
    for (const auto& member : cls->members) {
        analyzeDeclaration(member);
    }

    symbolTable.popScope();
    currentClass = prevClass;

    LOG_DEBUG("Class analysis completed: " + cls->name);
}

// ============================================================================
// Анализ интерфейсов
// ============================================================================

void SemanticAnalyzer::analyzeInterface(InterfaceDeclNodePtr iface) {
    if (!iface) return;

    LOG_DEBUG("Analyzing interface: " + iface->name);

    // Добавление интерфейса в таблицу символов
    Symbol ifaceSymbol;
    ifaceSymbol.name = iface->name;
    ifaceSymbol.type = "interface";
    ifaceSymbol.kind = SymbolKind::INTERFACE;
    ifaceSymbol.line = iface->token.line;
    ifaceSymbol.column = iface->token.column;
    ifaceSymbol.filename = iface->filename;
    ifaceSymbol.scope = symbolTable.getCurrentScope();

    if (!symbolTable.add(ifaceSymbol)) {
        addError("Interface '" + iface->name + "' already defined", iface->token);
    }

    // Анализ методов интерфейса
    for (const auto& method : iface->methods) {
        Symbol methodSymbol;
        methodSymbol.name = method->name;
        methodSymbol.type = method->returnType;
        methodSymbol.kind = SymbolKind::METHOD;
        methodSymbol.access = SymbolAccess::Public;  // Все методы интерфейса публичные
        methodSymbol.line = method->token.line;

        symbolTable.add(methodSymbol);
    }

    LOG_DEBUG("Interface analysis completed: " + iface->name);
}

// ============================================================================
// Анализ переменных
// ============================================================================

void SemanticAnalyzer::analyzeVariable(VariableDeclNodePtr var) {
    if (!var) return;

    LOG_DEBUG("Analyzing variable: " + var->name);

    // Проверка типа
    if (!var->typeName.empty() && !typeChecker.typeExists(var->typeName)) {
        // Проверка на шаблонные типы
        if (var->typeName.find("<") == std::string::npos) {
            addWarning("Unknown type: " + var->typeName, var->token);
        }
    }

    // Добавление переменной в таблицу символов
    Symbol symbol;
    symbol.name = var->name;
    symbol.type = var->typeName.empty() ? "auto" : var->typeName;
    symbol.kind = SymbolKind::VARIABLE;
    symbol.line = var->token.line;
    symbol.column = var->token.column;
    symbol.filename = var->filename;
    symbol.scope = symbolTable.getCurrentScope();
    symbol.access = var->access;

    if (!symbolTable.add(symbol)) {
        addError("Variable '" + var->name + "' already defined in this scope",
                var->token);
    }

    // Анализ инициализатора
    if (var->initializer) {
        analyzeExpression(var->initializer);

        // Проверка совместимости типов
        if (!var->typeName.empty()) {
            std::string initType = typeChecker.inferType(var->initializer);
            if (!typeChecker.isCompatible(initType, var->typeName)) {
                addWarning("Type mismatch: cannot initialize '" + var->typeName +
                          "' with '" + initType + "'", var->token);
            }
        }
    }

    LOG_DEBUG("Variable analysis completed: " + var->name);
}

// ============================================================================
// Анализ пространств имён
// ============================================================================

void SemanticAnalyzer::analyzeNamespace(NamespaceDeclNodePtr ns) {
    if (!ns) return;

    LOG_DEBUG("Analyzing namespace: " + ns->name);

    std::string prevNamespace = currentNamespace;

    if (!currentNamespace.empty()) {
        currentNamespace = currentNamespace + "::" + ns->name;
    } else {
        currentNamespace = ns->name;
    }

    symbolTable.pushScope();
    symbolTable.addNamespace(currentNamespace);

    // Анализ содержимого пространства имён
    for (const auto& decl : ns->declarations) {
        analyzeDeclaration(decl);
    }

    symbolTable.popScope();
    currentNamespace = prevNamespace;

    LOG_DEBUG("Namespace analysis completed: " + ns->name);
}

// ============================================================================
// Анализ шаблонов
// ============================================================================

void SemanticAnalyzer::analyzeTemplate(TemplateDeclNodePtr tmpl) {
    if (!tmpl) return;

    LOG_DEBUG("Analyzing template with " +
              std::to_string(tmpl->parameters.size()) + " parameters");

    symbolTable.pushScope();

    // Добавление параметров шаблона
    for (const auto& param : tmpl->parameters) {
        Symbol symbol;
        symbol.name = param;
        symbol.type = "type";
        symbol.kind = SymbolKind::TYPE;
        symbol.access = SymbolAccess::Public;

        symbolTable.add(symbol);
    }

    // Анализ объявлений внутри шаблона
    for (const auto& decl : tmpl->declarations) {
        analyzeDeclaration(decl);
    }

    symbolTable.popScope();

    LOG_DEBUG("Template analysis completed");
}

// ============================================================================
// Анализ statement'ов
// ============================================================================

void SemanticAnalyzer::analyzeStatement(StatementNodePtr stmt) {
    if (!stmt) return;

    LOG_DEBUG("Analyzing statement at line " +
              std::to_string(stmt->token.line));

    switch (stmt->nodeType) {
        case NodeType::BLOCK:
            analyzeBlock(std::static_pointer_cast<BlockNode>(stmt));
            break;

        case NodeType::IF_STATEMENT:
            analyzeIf(std::static_pointer_cast<IfNode>(stmt));
            break;

        case NodeType::FOR_LOOP:
            analyzeFor(std::static_pointer_cast<ForNode>(stmt));
            break;

        case NodeType::WHILE_LOOP:
            analyzeWhile(std::static_pointer_cast<WhileNode>(stmt));
            break;

        case NodeType::DO_WHILE:
            analyzeDoWhile(std::static_pointer_cast<DoWhileNode>(stmt));
            break;

        case NodeType::SWITCH_STATEMENT:
            analyzeSwitch(std::static_pointer_cast<SwitchNode>(stmt));
            break;

        case NodeType::RETURN_STATEMENT:
            analyzeReturn(std::static_pointer_cast<ReturnNode>(stmt));
            break;

        case NodeType::BREAK_STATEMENT:
            analyzeBreak(std::static_pointer_cast<BreakNode>(stmt));
            break;

        case NodeType::CONTINUE_STATEMENT:
            analyzeContinue(std::static_pointer_cast<ContinueNode>(stmt));
            break;

        case NodeType::EXPRESSION_STATEMENT:
            analyzeExpressionStatement(std::static_pointer_cast<ExpressionStatementNode>(stmt));
            break;

        case NodeType::VARIABLE_DECL:
            analyzeVariable(std::static_pointer_cast<VariableDeclNode>(stmt));
            break;

        case NodeType::ASSIGNMENT:
            analyzeAssignment(std::static_pointer_cast<AssignmentNode>(stmt));
            break;

        default:
            LOG_WARNING("Unknown statement type: " +
                       std::to_string(static_cast<int>(stmt->nodeType)));
            break;
    }
}

void SemanticAnalyzer::analyzeBlock(BlockNodePtr block) {
    if (!block) return;

    LOG_DEBUG("Analyzing block");

    symbolTable.pushScope();

    for (const auto& stmt : block->statements) {
        analyzeStatement(stmt);
    }

    symbolTable.popScope();
}

void SemanticAnalyzer::analyzeIf(IfNodePtr ifStmt) {
    if (!ifStmt) return;

    LOG_DEBUG("Analyzing if statement");

    // Анализ условия
    analyzeExpression(ifStmt->condition);

    // Проверка типа условия (должен быть bool)
    std::string condType = typeChecker.inferType(ifStmt->condition);
    if (condType != "bool" && !typeChecker.isCompatible(condType, "bool")) {
        addWarning("If condition should be of type 'bool', got '" + condType + "'",
                  ifStmt->token);
    }

    symbolTable.pushScope();

    // Анализ then-ветви
    for (const auto& stmt : ifStmt->thenBranch) {
        analyzeStatement(stmt);
    }

    symbolTable.popScope();

    // Анализ else-ветви
    if (ifStmt->elseBranch) {
        symbolTable.pushScope();

        for (const auto& stmt : ifStmt->elseBranch) {
            analyzeStatement(stmt);
        }

        symbolTable.popScope();
    }

    // Анализ elseif-ветвей
    for (const auto& elseif : ifStmt->elseIfBranches) {
        analyzeIf(elseif);
    }
}

void SemanticAnalyzer::analyzeFor(ForNodePtr forStmt) {
    if (!forStmt) return;

    LOG_DEBUG("Analyzing for loop");

    symbolTable.pushScope();

    // Анализ переменной цикла
    if (!forStmt->variable.empty()) {
        Symbol symbol;
        symbol.name = forStmt->variable;
        symbol.type = "auto";
        symbol.kind = SymbolKind::VARIABLE;
        symbol.access = SymbolAccess::Public;
        symbolTable.add(symbol);
    }

    // Анализ итерируемого выражения
    analyzeExpression(forStmt->iterable);

    // Анализ тела цикла
    for (const auto& stmt : forStmt->body) {
        analyzeStatement(stmt);
    }

    symbolTable.popScope();
}

void SemanticAnalyzer::analyzeWhile(WhileNodePtr whileStmt) {
    if (!whileStmt) return;

    LOG_DEBUG("Analyzing while loop");

    // Анализ условия
    analyzeExpression(whileStmt->condition);

    // Проверка типа условия
    std::string condType = typeChecker.inferType(whileStmt->condition);
    if (condType != "bool" && !typeChecker.isCompatible(condType, "bool")) {
        addWarning("While condition should be of type 'bool', got '" + condType + "'",
                  whileStmt->token);
    }

    symbolTable.pushScope();

    // Анализ тела цикла
    for (const auto& stmt : whileStmt->body) {
        analyzeStatement(stmt);
    }

    symbolTable.popScope();
}

void SemanticAnalyzer::analyzeDoWhile(DoWhileNodePtr doStmt) {
    if (!doStmt) return;

    LOG_DEBUG("Analyzing do-while loop");

    symbolTable.pushScope();

    // Анализ тела цикла
    for (const auto& stmt : doStmt->body) {
        analyzeStatement(stmt);
    }

    symbolTable.popScope();

    // Анализ условия
    analyzeExpression(doStmt->condition);

    // Проверка типа условия
    std::string condType = typeChecker.inferType(doStmt->condition);
    if (condType != "bool" && !typeChecker.isCompatible(condType, "bool")) {
        addWarning("Do-while condition should be of type 'bool', got '" + condType + "'",
                  doStmt->token);
    }
}

void SemanticAnalyzer::analyzeSwitch(SwitchNodePtr switchStmt) {
    if (!switchStmt) return;

    LOG_DEBUG("Analyzing switch statement");

    // Анализ выражения
    analyzeExpression(switchStmt->expression);

    symbolTable.pushScope();

    // Анализ case'ов
    for (const auto& case_ : switchStmt->cases) {
        if (case_->value) {
            analyzeExpression(case_->value);
        }

        for (const auto& stmt : case_->statements) {
            analyzeStatement(stmt);
        }
    }

    // Анализ default
    if (switchStmt->defaultCase) {
        for (const auto& stmt : switchStmt->defaultCase->statements) {
            analyzeStatement(stmt);
        }
    }

    symbolTable.popScope();
}

void SemanticAnalyzer::analyzeReturn(ReturnNodePtr returnStmt) {
    if (!returnStmt) return;

    LOG_DEBUG("Analyzing return statement");

    // Проверка нахождения внутри функции
    if (currentFunction.empty()) {
        addError("Return statement outside of function", returnStmt->token);
        return;
    }

    // Анализ возвращаемого значения
    if (returnStmt->value) {
        analyzeExpression(returnStmt->value);

        // Проверка типа возврата (если возможно)
        // В полной реализации - сравнение с типом функции
    }
}

void SemanticAnalyzer::analyzeBreak(BreakNodePtr breakStmt) {
    if (!breakStmt) return;

    LOG_DEBUG("Analyzing break statement");

    // Проверка нахождения внутри цикла или switch
    // В полной реализации - проверка контекста
}

void SemanticAnalyzer::analyzeContinue(ContinueNodePtr continueStmt) {
    if (!continueStmt) return;

    LOG_DEBUG("Analyzing continue statement");

    // Проверка нахождения внутри цикла
    // В полной реализации - проверка контекста
}

void SemanticAnalyzer::analyzeExpressionStatement(ExpressionStatementNodePtr stmt) {
    if (!stmt) return;

    LOG_DEBUG("Analyzing expression statement");

    analyzeExpression(stmt->expression);
}

// ============================================================================
// Анализ выражений
// ============================================================================

void SemanticAnalyzer::analyzeExpression(ExpressionNodePtr expr) {
    if (!expr) return;

    switch (expr->nodeType) {
        case NodeType::LITERAL:
            analyzeLiteral(std::static_pointer_cast<LiteralNode>(expr));
            break;

        case NodeType::IDENTIFIER:
            analyzeIdentifier(std::static_pointer_cast<IdentifierNode>(expr));
            break;

        case NodeType::BINARY_OP:
            analyzeBinaryOp(std::static_pointer_cast<BinaryOpNode>(expr));
            break;

        case NodeType::UNARY_OP:
            analyzeUnaryOp(std::static_pointer_cast<UnaryOpNode>(expr));
            break;

        case NodeType::CALL_EXPR:
            analyzeCall(std::static_pointer_cast<CallNode>(expr));
            break;

        case NodeType::INDEX_EXPR:
            analyzeIndex(std::static_pointer_cast<IndexNode>(expr));
            break;

        case NodeType::MEMBER_ACCESS:
            analyzeMemberAccess(std::static_pointer_cast<MemberAccessNode>(expr));
            break;

        case NodeType::ASSIGNMENT:
            analyzeAssignment(std::static_pointer_cast<AssignmentNode>(expr));
            break;

        case NodeType::ARRAY_LITERAL:
            analyzeArrayLiteral(std::static_pointer_cast<ArrayLiteralNode>(expr));
            break;

        case NodeType::MATRIX_LITERAL:
            analyzeMatrixLiteral(std::static_pointer_cast<MatrixLiteralNode>(expr));
            break;

        case NodeType::TERNARY:
            analyzeTernary(std::static_pointer_cast<TernaryNode>(expr));
            break;

        default:
            LOG_WARNING("Unknown expression type: " +
                       std::to_string(static_cast<int>(expr->nodeType)));
            break;
    }
}

void SemanticAnalyzer::analyzeLiteral(LiteralNodePtr literal) {
    if (!literal) return;

    LOG_DEBUG("Analyzing literal: " + literal->token.value);

    // Проверка специальных значений
    if (literal->token.type == TokenType::KEYWORD_NAN ||
        literal->token.type == TokenType::KEYWORD_INF ||
        literal->token.type == TokenType::KEYWORD_PI ||
        literal->token.type == TokenType::KEYWORD_PI2 ||
        literal->token.type == TokenType::KEYWORD_EXP) {
        // Специальные константы - проверка в typeChecker
    }
}

void SemanticAnalyzer::analyzeIdentifier(IdentifierNodePtr ident) {
    if (!ident) return;

    LOG_DEBUG("Analyzing identifier: " + ident->name);

    // Поиск символа в таблице символов
    Symbol symbol = symbolTable.get(ident->name);

    if (symbol.kind == SymbolKind::UNKNOWN) {
        // Проверка на встроенные функции/константы
        if (!isBuiltin(ident->name)) {
            addWarning("Undefined identifier: '" + ident->name + "'",
                      ident->token);
        }
    }
}

bool SemanticAnalyzer::isBuiltin(const std::string& name) {
    static const std::set<std::string> builtins = {
        // Функции
        "print", "write", "read", "open", "close",
        "zeros", "ones", "eye", "rand", "size", "length",
        "sum", "mean", "max", "min", "abs", "sqrt", "exp", "log", "log10",
        "sin", "cos", "tan", "asin", "acos", "atan",
        "time_now", "sleep",
        "type", "rtti",
        "dbgstop", "dbgprint", "dbgcontext", "dbgstack",
        "assert",

        // Константы
        "pi", "pi2", "exp", "nan", "inf", "ninf",
        "true", "false", "null"
    };

    return builtins.count(name) > 0;
}

void SemanticAnalyzer::analyzeBinaryOp(BinaryOpNodePtr binop) {
    if (!binop) return;

    LOG_DEBUG("Analyzing binary operator: " + binop->op);

    // Анализ операндов
    analyzeExpression(binop->left);
    analyzeExpression(binop->right);

    // Проверка типов операндов
    std::string leftType = typeChecker.inferType(binop->left);
    std::string rightType = typeChecker.inferType(binop->right);

   // Специальная обработка для операторов проверки типов (OP_TYPE_EQ, OP_TYPE_NEQ)
    if (binop->isTypeCheck) {
        // Для === и !== проверяем только типы, значения не важны
        LOG_DEBUG("Type check operator: " + binop->op +
                 " leftType=" + leftType + " rightType=" + rightType);

        // Проверка что оба операнда имеют известные типы
        if (leftType == "auto" || rightType == "auto") {
            addWarning("Type check operator with 'auto' type may produce unexpected results",
                      binop->token);
        }

        return;
    }

    // Проверка допустимости оператора
    if (!typeChecker.checkBinaryOperator(binop->op, leftType, rightType, binop->token)) {
        addWarning("Invalid operator '" + binop->op + "' for types '" +
                  leftType + "' and '" + rightType + "'", binop->token);
    }
}

void SemanticAnalyzer::analyzeUnaryOp(UnaryOpNodePtr unary) {
    if (!unary) return;

    LOG_DEBUG("Analyzing unary operator: " + unary->op);

    // Анализ операнда
    analyzeExpression(unary->operand);

    // Проверка типа операнда
    std::string operandType = typeChecker.inferType(unary->operand);

    if (!typeChecker.checkUnaryOperator(unary->op, operandType, unary->token)) {
        addWarning("Invalid operator '" + unary->op + "' for type '" +
                  operandType + "'", unary->token);
    }
}

void SemanticAnalyzer::analyzeCall(CallNodePtr call) {
    if (!call) return;

    LOG_DEBUG("Analyzing function call");

    // Анализ callee
    analyzeExpression(call->callee);

    // Анализ аргументов
    for (const auto& arg : call->arguments) {
        analyzeExpression(arg);
    }

    // Проверка функции
    if (call->callee->nodeType == NodeType::IDENTIFIER) {
        auto ident = std::static_pointer_cast<IdentifierNode>(call->callee);
        checkFunctionCall(ident->name, call->arguments, call->token);

        // Проверка правил валидации аргументов
        auto it = functionValidationRules.find(ident->name);
        if (it != functionValidationRules.end()) {
            validateFunctionCall(it->second, call->arguments, call->token);
        }
    }
}

void SemanticAnalyzer::analyzeIndex(IndexNodePtr index) {
    if (!index) return;

    LOG_DEBUG("Analyzing index expression");

    // Анализ объекта
    analyzeExpression(index->object);

    // Получение типа массива
    std::string arrayType = typeChecker.inferType(index->object);

    // Проверка индексации
    checkArrayIndexing(index, arrayType);

    // Анализ индексов
    for (const auto& idx : index->indices) {
        analyzeExpression(idx);

        // Проверка типа индекса
        std::string indexType = typeChecker.inferType(idx);
        if (!typeChecker.isNumericType(indexType)) {
            addError("Array index must be numeric type, got: " + indexType,
                    idx->token);
        }
    }
}

void SemanticAnalyzer::analyzeMemberAccess(MemberAccessNodePtr member) {
    if (!member) return;

    LOG_DEBUG("Analyzing member access: " + member->memberName);

    // Анализ объекта
    analyzeExpression(member->object);

    // Получение типа объекта
    std::string objectType = typeChecker.inferType(member->object);

    // Проверка существования члена
    if (!objectType.empty() && objectType != "auto") {
        Symbol typeSymbol = symbolTable.get(objectType);
        if (typeSymbol.kind == SymbolKind::CLASS ||
            typeSymbol.kind == SymbolKind::INTERFACE) {
            // Проверка наличия члена в классе/интерфейсе
            // В полной реализации
        }
    }
}

void SemanticAnalyzer::analyzeAssignment(AssignmentNodePtr assign) {
    if (!assign) return;

    LOG_DEBUG("Analyzing assignment");

    // Анализ правой части
    analyzeExpression(assign->right);

    // Анализ левой части
    analyzeExpression(assign->left);

    // Проверка совместимости типов
    std::string leftType = typeChecker.inferType(assign->left);
    std::string rightType = typeChecker.inferType(assign->right);

    if (!typeChecker.isCompatible(rightType, leftType)) {
        addWarning("Type mismatch in assignment: cannot assign '" +
                  rightType + "' to '" + leftType + "'", assign->token);
    }
}

void SemanticAnalyzer::analyzeArrayLiteral(ArrayLiteralNodePtr array) {
    if (!array) return;

    LOG_DEBUG("Analyzing array literal with " +
              std::to_string(array->elements.size()) + " elements");

    // Анализ элементов
    for (const auto& elem : array->elements) {
        analyzeExpression(elem);
    }

    // Проверка однородности типов
    if (array->elements.size() > 1) {
        std::string firstType = typeChecker.inferType(array->elements[0]);
        for (size_t i = 1; i < array->elements.size(); i++) {
            std::string elemType = typeChecker.inferType(array->elements[i]);
            if (!typeChecker.isCompatible(firstType, elemType)) {
                addWarning("Array elements should have compatible types",
                          array->token);
                break;
            }
        }
    }
}

void SemanticAnalyzer::analyzeMatrixLiteral(MatrixLiteralNodePtr matrix) {
    if (!matrix) return;

    LOG_DEBUG("Analyzing matrix literal with " +
              std::to_string(matrix->rows.size()) + " rows");

    // Проверка одинакового количества столбцов
    if (!matrix->rows.empty()) {
        size_t firstRowSize = matrix->rows[0].size();
        for (size_t i = 1; i < matrix->rows.size(); i++) {
            if (matrix->rows[i].size() != firstRowSize) {
                addError("Matrix rows must have the same number of columns",
                        matrix->token);
            }
        }
    }

    // Анализ элементов
    for (const auto& row : matrix->rows) {
        for (const auto& elem : row) {
            analyzeExpression(elem);
        }
    }
}

void SemanticAnalyzer::analyzeTernary(TernaryNodePtr ternary) {
    if (!ternary) return;

    LOG_DEBUG("Analyzing ternary expression");

    // Анализ условия
    analyzeExpression(ternary->condition);

    // Проверка типа условия
    std::string condType = typeChecker.inferType(ternary->condition);
    if (condType != "bool" && !typeChecker.isCompatible(condType, "bool")) {
        addWarning("Ternary condition should be of type 'bool', got '" + condType + "'",
                  ternary->token);
    }

    // Анализ ветвей
    analyzeExpression(ternary->trueExpr);
    analyzeExpression(ternary->falseExpr);

    // Проверка совместимости типов ветвей
    std::string trueType = typeChecker.inferType(ternary->trueExpr);
    std::string falseType = typeChecker.inferType(ternary->falseExpr);

    if (!typeChecker.isCompatible(trueType, falseType)) {
        addWarning("Ternary branches should have compatible types",
                  ternary->token);
    }
}

// ============================================================================
// Валидация аргументов
// ============================================================================

void SemanticAnalyzer::validateArgumentsBlock(
    ArgumentsNodePtr argumentsNode,
    FunctionDeclNodePtr functionNode) {

    if (!argumentsNode || !functionNode) {
        return;
    }

    LOG_DEBUG("Validating arguments block for function: " + functionNode->name);

    // Парсинг правил валидации
    QVector<ValidationRule> rules = argumentValidator.parseArgumentsBlock(
        argumentsNode, functionNode);

    // Сохранение правил для функции
    functionValidationRules[functionNode->name] = rules;

    // Валидация каждого правила
    for (const auto& rule : rules) {
        // Проверка существования аргумента
        bool argFound = false;
        for (const auto& param : functionNode->parameters) {
            if (param.first == rule.argumentName) {
                argFound = true;
                break;
            }
        }

        if (!argFound) {
            addWarning("Argument '" + rule.argumentName +
                      "' not found in function parameters",
                      argumentsNode->token);
        }

        // Проверка корректности валидатора
        if (rule.validator == ValidatorType::Custom && !rule.validatorName.empty()) {
            if (!argumentValidator.hasCustomValidator(rule.validatorName)) {
                addWarning("Custom validator '" + rule.validatorName +
                          "' not registered", argumentsNode->token);
            }
        }
    }

    LOG_DEBUG("Found " + std::to_string(rules.size()) + " validation rules");
}

void SemanticAnalyzer::validateFunctionCall(
    const QVector<ValidationRule>& rules,
    const std::vector<ExpressionNodePtr>& args,
    const Token& token) {

    if (rules.isEmpty()) {
        return;
    }

    LOG_DEBUG("Validating function call with " +
              std::to_string(rules.size()) + " rules");

    // Валидация будет выполнена в runtime
    // Здесь только статическая проверка

    for (const auto& rule : rules) {
        // Проверка на обязательность аргумента
        if (!rule.isOptional && rule.argumentName.empty()) {
            // Позиционный аргумент
            int argIndex = rules.indexOf(rule);
            if (argIndex >= static_cast<int>(args.size())) {
                addError("Missing required argument at position " +
                        std::to_string(argIndex + 1), token);
            }
        }
    }
}

// ============================================================================
// Проверка 1-based индексации
// ============================================================================

void SemanticAnalyzer::checkArrayIndexing(
    ExpressionNodePtr expr,
    const std::string& arrayType) {

    if (!expr) return;

    if (expr->nodeType == NodeType::INDEX_EXPR) {
        auto indexExpr = std::static_pointer_cast<IndexNode>(expr);

        for (const auto& index : indexExpr->indices) {
            if (index->nodeType == NodeType::LITERAL) {
                auto literal = std::static_pointer_cast<LiteralNode>(index);

                if (literal->token.type == TokenType::INTEGER) {
                    try {
                        int64_t indexValue = std::stoll(literal->token.value);

                        // Проверка: индекс должен быть >= 1 (language.txt пункт 11.1)
                        if (indexValue < 1) {
                            addError("Array indexing starts from 1, not 0. "
                                    "Invalid index: " + std::to_string(indexValue),
                                    literal->token);
                        }
                    } catch (...) {
                        LOG_WARNING("Failed to parse index value");
                    }
                }
            }
        }
    }
}

void SemanticAnalyzer::check1BasedIndexing(ExpressionNodePtr expr) {
    if (!expr) return;

    switch (expr->nodeType) {
        case NodeType::INDEX_EXPR: {
            auto indexExpr = std::static_pointer_cast<IndexNode>(expr);
            analyzeExpression(indexExpr->object);

            std::string arrayType = typeChecker.inferType(indexExpr->object);
            checkArrayIndexing(expr, arrayType);

            for (const auto& index : indexExpr->indices) {
                analyzeExpression(index);
            }
            break;
        }

        case NodeType::BINARY_OP: {
            auto binop = std::static_pointer_cast<BinaryOpNode>(expr);
            check1BasedIndexing(binop->left);
            check1BasedIndexing(binop->right);
            break;
        }

        case NodeType::UNARY_OP: {
            auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
            check1BasedIndexing(unary->operand);
            break;
        }

        case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            for (const auto& arg : call->arguments) {
                check1BasedIndexing(arg);
            }
            break;
        }

        case NodeType::TERNARY: {
            auto ternary = std::static_pointer_cast<TernaryNode>(expr);
            check1BasedIndexing(ternary->condition);
            check1BasedIndexing(ternary->trueExpr);
            check1BasedIndexing(ternary->falseExpr);
            break;
        }

        default:
            break;
    }
}

// ============================================================================
// Проверка вызовов функций
// ============================================================================

void SemanticAnalyzer::checkFunctionCall(
    const std::string& funcName,
    const std::vector<ExpressionNodePtr>& args,
    const Token& token) {

    Symbol symbol = symbolTable.get(funcName);

    if (symbol.kind != SymbolKind::FUNCTION &&
        symbol.kind != SymbolKind::METHOD) {
        // Проверка на встроенные функции
        if (!isBuiltin(funcName)) {
            addError("'" + funcName + "' is not a function", token);
        }
        return;
    }

    // Проверка количества аргументов
    if (args.size() < symbol.parameters.size()) {
        // Проверка на наличие значений по умолчанию
        // В полной реализации
        LOG_DEBUG("Function " + funcName + " has " +
                 std::to_string(symbol.parameters.size()) +
                 " parameters but " + std::to_string(args.size()) +
                 " arguments provided");
    }

    // Проверка типов аргументов
    for (size_t i = 0; i < args.size() && i < symbol.parameters.size(); i++) {
        std::string argType = typeChecker.inferType(args[i]);
        std::string paramType = symbol.parameters[i].second;

        if (!typeChecker.isCompatible(argType, paramType)) {
            addWarning("Type mismatch for argument " + std::to_string(i + 1) +
                      ": expected " + paramType + ", got " + argType, token);
        }
    }
}

// ============================================================================
// Обработка ошибок
// ============================================================================

void SemanticAnalyzer::addError(const std::string& message, const Token& token, int severity) {
    SemanticError error;
    error.message = message;
    error.token = token;
    error.severity = severity;

    errors.push_back(error);

    if (severity >= verboseLevel) {
        LOG_ERROR("Semantic error: " + message +
                 " at line " + std::to_string(token.line) +
                 ", column " + std::to_string(token.column));
    }
}

void SemanticAnalyzer::addWarning(const std::string& message, const Token& token) {
    SemanticWarning warning;
    warning.message = message;
    warning.token = token;

    warnings.push_back(warning);

    if (verboseLevel >= 2) {
        LOG_WARNING("Semantic warning: " + message +
                   " at line " + std::to_string(token.line) +
                   ", column " + std::to_string(token.column));
    }
}

// ============================================================================
// Геттеры
// ============================================================================

bool SemanticAnalyzer::hasErrors() const {
    return !errors.empty();
}

bool SemanticAnalyzer::hasWarnings() const {
    return !warnings.empty();
}

const std::vector<SemanticError>& SemanticAnalyzer::getErrors() const {
    return errors;
}

const std::vector<SemanticWarning>& SemanticAnalyzer::getWarnings() const {
    return warnings;
}

SymbolTable& SemanticAnalyzer::getSymbolTable() {
    return symbolTable;
}

TypeChecker& SemanticAnalyzer::getTypeChecker() {
    return typeChecker;
}

ArgumentValidator& SemanticAnalyzer::getArgumentValidator() {
    return argumentValidator;
}

void SemanticAnalyzer::setVerboseLevel(int level) {
    verboseLevel = level;
    LOG_INFO("Verbose level set to: " + std::to_string(level));
}

int SemanticAnalyzer::getVerboseLevel() const {
    return verboseLevel;
}

} // namespace proxima

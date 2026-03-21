#include "TypeChecker.h"
#include <algorithm>
#include <cmath>
#include <regex>
#include <sstream>
#include "utils/Logger.h"

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

TypeChecker::TypeChecker() {
    registerBuiltinTypes();
    LOG_DEBUG("TypeChecker initialized");
}

TypeChecker::~TypeChecker() {
    clear();
}

void TypeChecker::clear() {
    types.clear();
    typeAliases.clear();
    registerBuiltinTypes();
    LOG_DEBUG("TypeChecker cleared");
}

// ============================================================================
// Регистрация встроенных типов
// ============================================================================

void TypeChecker::registerBuiltinTypes() {
    // Скалярные целочисленные типы
    registerType("int4", createTypeInfo("int4", TypeCategory::SCALAR, "", {}, 1));
    registerType("int8", createTypeInfo("int8", TypeCategory::SCALAR, "", {}, 1));
    registerType("int16", createTypeInfo("int16", TypeCategory::SCALAR, "", {}, 2));
    registerType("int32", createTypeInfo("int32", TypeCategory::SCALAR, "", {}, 4));
    registerType("int64", createTypeInfo("int64", TypeCategory::SCALAR, "", {}, 8));
    registerType("int", createTypeInfo("int", TypeCategory::SCALAR, "", {}, 4)); // alias для int32
    
    // Числа с плавающей точкой
    registerType("single", createTypeInfo("single", TypeCategory::SCALAR, "", {}, 4));
    registerType("double", createTypeInfo("double", TypeCategory::SCALAR, "", {}, 8));
    registerType("float", createTypeInfo("float", TypeCategory::SCALAR, "", {}, 4)); // alias для single
    
    // Логический и символьный типы
    registerType("bool", createTypeInfo("bool", TypeCategory::SCALAR, "", {}, 1));
    registerType("char", createTypeInfo("char", TypeCategory::SCALAR, "", {}, 1));
    
    // Строковый тип
    registerType("string", createTypeInfo("string", TypeCategory::SCALAR, "", {}, 32));
    
    // Специальные значения
    registerType("nan", createTypeInfo("nan", TypeCategory::SCALAR, "double", {}, 8));
    registerType("inf", createTypeInfo("inf", TypeCategory::SCALAR, "double", {}, 8));
    registerType("ninf", createTypeInfo("ninf", TypeCategory::SCALAR, "double", {}, 8));
    
    // Встроенные константы
    registerType("pi", createTypeInfo("pi", TypeCategory::SCALAR, "double", {}, 8));
    registerType("pi2", createTypeInfo("pi2", TypeCategory::SCALAR, "double", {}, 8));
    registerType("exp", createTypeInfo("exp", TypeCategory::SCALAR, "double", {}, 8));
    
    // Тип времени
    registerType("time", createTypeInfo("time", TypeCategory::CLASS, "", {}, 8));
    
    // Тип файла
    registerType("file", createTypeInfo("file", TypeCategory::CLASS, "", {}, 64));
    
    // Вектора
    TypeInfo vectorInfo = createTypeInfo("vector", TypeCategory::VECTOR, "", {}, 0);
    vectorInfo.isTemplate = true;
    registerType("vector", vectorInfo);
    
    // Матрицы
    TypeInfo matrixInfo = createTypeInfo("matrix", TypeCategory::MATRIX, "", {}, 0);
    matrixInfo.isTemplate = true;
    registerType("matrix", matrixInfo);
    
    // Слои
    TypeInfo layerInfo = createTypeInfo("layer", TypeCategory::LAYER, "", {}, 0);
    layerInfo.isTemplate = true;
    registerType("layer", layerInfo);
    
    // Коллекции
    registerType("collection", createTypeInfo("collection", TypeCategory::COLLECTION, "", {}, 64));
    
    // Матрицы фиксированного размера
    registerType("matrix22", createTypeInfo("matrix22", TypeCategory::MATRIX, "double", {2, 2}, 32));
    registerType("matrix33", createTypeInfo("matrix33", TypeCategory::MATRIX, "double", {3, 3}, 72));
    registerType("matrix44", createTypeInfo("matrix44", TypeCategory::MATRIX, "double", {4, 4}, 128));
    registerType("matrix34", createTypeInfo("matrix34", TypeCategory::MATRIX, "double", {3, 4}, 96));
    
    // Точки
    registerType("point2", createTypeInfo("point2", TypeCategory::CLASS, "double", {2}, 16));
    registerType("point3", createTypeInfo("point3", TypeCategory::CLASS, "double", {3}, 24));
    registerType("point4", createTypeInfo("point4", TypeCategory::CLASS, "double", {4}, 32));
    
    // Специальные типы
    registerType("auto", createTypeInfo("auto", TypeCategory::AUTO, "", {}, 0));
    registerType("void", createTypeInfo("void", TypeCategory::VOID, "", {}, 0));
    registerType("type", createTypeInfo("type", TypeCategory::CLASS, "", {}, 64));
    registerType("method", createTypeInfo("method", TypeCategory::FUNCTION, "", {}, 8));
    registerType("rtti", createTypeInfo("rtti", TypeCategory::CLASS, "", {}, 128));
    registerType("parallel", createTypeInfo("parallel", TypeCategory::CLASS, "", {}, 64));
    registerType("document", createTypeInfo("document", TypeCategory::CLASS, "", {}, 256));
    
    // Типы указателей
    TypeInfo ptrInfo = createTypeInfo("ptr", TypeCategory::POINTER, "", {}, 8);
    ptrInfo.isTemplate = true;
    registerType("ptr", ptrInfo);
    
    // Алиасы типов
    typeAliases["i32"] = "int32";
    typeAliases["i64"] = "int64";
    typeAliases["f32"] = "single";
    typeAliases["f64"] = "double";
    typeAliases["real"] = "double";
    typeAliases["logical"] = "bool";
    
    LOG_INFO("Built-in types registered: " + std::to_string(types.size()));
}

TypeInfo TypeChecker::createTypeInfo(const std::string& name, TypeCategory category,
                                     const std::string& elementType,
                                     const std::vector<size_t>& dimensions,
                                     int size) {
    TypeInfo info;
    info.name = name;
    info.category = category;
    info.elementType = elementType;
    info.dimensions = dimensions;
    info.size = size;
    info.isMutable = true;
    info.isNullable = (category == TypeCategory::POINTER);
    info.isTemplate = false;
    info.baseType = "";
    info.templateArgs.clear();
    info.methods.clear();
    info.fields.clear();
    info.operators.clear();
    info.parentClasses.clear();
    info.interfaces.clear();
    info.isInterface = false;
    info.isAbstract = false;
    info.isFinal = false;
    return info;
}

void TypeChecker::registerType(const std::string& name, TypeInfo info) {
    std::string normalizedName = normalizeType(name);
    types[normalizedName] = info;
    LOG_DEBUG("Type registered: " + normalizedName);
}

// ============================================================================
// Нормализация и получение типов
// ============================================================================

std::string TypeChecker::normalizeType(const std::string& type) {
    std::string normalized = type;
    
    // Удаление пробелов
    normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());
    
    // Проверка алиасов
    auto it = typeAliases.find(normalized);
    if (it != typeAliases.end()) {
        return it->second;
    }
    
    return normalized;
}

TypeInfo TypeChecker::getType(const std::string& name) {
    std::string normalized = normalizeType(name);
    
    auto it = types.find(normalized);
    if (it != types.end()) {
        return it->second;
    }
    
    // Проверка параметризированных типов (vector<T>, matrix<T>, etc.)
    if (normalized.find("<") != std::string::npos && normalized.find(">") != std::string::npos) {
        std::string baseType = normalized.substr(0, normalized.find("<"));
        std::string elemType = normalized.substr(normalized.find("<") + 1, 
                          normalized.find(">") - normalized.find("<") - 1);
        
        auto baseIt = types.find(baseType);
        if (baseIt != types.end()) {
            TypeInfo info = baseIt->second;
            info.name = normalized;
            info.elementType = elemType;
            return info;
        }
    }
    
    // Тип не найден
    TypeInfo unknown;
    unknown.name = name;
    unknown.category = TypeCategory::UNKNOWN;
    return unknown;
}

bool TypeChecker::typeExists(const std::string& name) {
    return getType(name).category != TypeCategory::UNKNOWN;
}

// ============================================================================
// Вывод типов
// ============================================================================

std::string TypeChecker::inferType(ExpressionNodePtr expr) {
    if (!expr) {
        return "auto";
    }
    
    switch (expr->nodeType) {
        case NodeType::LITERAL: {
            auto literal = std::static_pointer_cast<LiteralNode>(expr);
            return inferLiteralType(literal);
        }
        
        case NodeType::IDENTIFIER: {
            auto ident = std::static_pointer_cast<IdentifierNode>(expr);
            return inferIdentifierType(ident);
        }
        
        case NodeType::BINARY_OP: {
            auto binop = std::static_pointer_cast<BinaryOpNode>(expr);
            return inferBinaryOpType(binop);
        }
        
        case NodeType::UNARY_OP: {
            auto unary = std::static_pointer_cast<UnaryOpNode>(expr);
            return inferUnaryOpType(unary);
        }
        
        case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            return inferCallType(call);
        }
        
        case NodeType::INDEX_EXPR: {
            auto index = std::static_pointer_cast<IndexNode>(expr);
            return inferIndexType(index);
        }
        
        case NodeType::MEMBER_ACCESS: {
            auto member = std::static_pointer_cast<MemberAccessNode>(expr);
            return inferMemberType(member);
        }
        
        case NodeType::ARRAY_LITERAL: {
            auto array = std::static_pointer_cast<ArrayLiteralNode>(expr);
            return inferArrayType(array);
        }
        
        case NodeType::MATRIX_LITERAL: {
            auto matrix = std::static_pointer_cast<MatrixLiteralNode>(expr);
            return inferMatrixType(matrix);
        }
        
        case NodeType::TERNARY: {
            auto ternary = std::static_pointer_cast<TernaryNode>(expr);
            return inferTernaryType(ternary);
        }
        
        case NodeType::TYPE_REF: {
            auto typeRef = std::static_pointer_cast<TypeRefNode>(expr);
            return typeRef->typeName;
        }
        
        default:
            return "auto";
    }
}

std::string TypeChecker::inferLiteralType(LiteralNodePtr literal) {
    if (!literal) return "auto";
    
    switch (literal->token.type) {
        case TokenType::INTEGER:
            return "int32";
        
        case TokenType::FLOAT:
            return "double";
        
        case TokenType::STRING:
            return "string";
        
        case TokenType::CHAR:
            return "char";
        
        case TokenType::KEYWORD_TRUE:
        case TokenType::KEYWORD_FALSE:
            return "bool";
        
        case TokenType::KEYWORD_NULL:
            return "auto*";
        
        case TokenType::KEYWORD_NAN:
        case TokenType::KEYWORD_INF:
            return "double";
        
        case TokenType::KEYWORD_PI:
        case TokenType::KEYWORD_PI2:
        case TokenType::KEYWORD_EXP:
            return "double";
        
        default:
            return "auto";
    }
}

std::string TypeChecker::inferIdentifierType(IdentifierNodePtr ident) {
    if (!ident) return "auto";
    
    // Поиск в таблице символов
    Symbol symbol = symbolTable.get(ident->name);
    if (symbol.kind != SymbolKind::UNKNOWN) {
        return symbol.type;
    }
    
    // Проверка встроенных констант
    if (ident->name == "pi" || ident->name == "pi2" || ident->name == "exp") {
        return "double";
    }
    
    if (ident->name == "nan" || ident->name == "inf" || ident->name == "ninf") {
        return "double";
    }
    
    return "auto";
}

std::string TypeChecker::inferBinaryOpType(BinaryOpNodePtr binop) {
    if (!binop || !binop->left || !binop->right) {
        return "auto";
    }
    
    std::string leftType = inferType(binop->left);
    std::string rightType = inferType(binop->right);
    std::string op = binop->op;
    
    // Проверка оператора
    if (!checkBinaryOperator(op, leftType, rightType, binop->token)) {
        return "auto";
    }
    
    // Арифметические операторы
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op == "\\") {
        return getCommonType(leftType, rightType);
    }
    
    // Операторы сравнения
    if (op == "==" || op == "!=" || op == "<" || op == ">" || 
        op == "<=" || op == ">=" || op == ".=" || op == ".==" ||
        op == ".<" || op == ".>" || op == ".<=" || op == ".>=") {
        return "bool";
    }
    
    // Логические операторы
    if (op == "&" || op == "|" || op == "%" || op == "&&" || op == "||" || op == "%%") {
        return "bool";
    }
    
    // Поэлементные операторы
    if (op.substr(0, 1) == ".") {
        TypeInfo leftInfo = getType(leftType);
        if (leftInfo.category == TypeCategory::VECTOR ||
            leftInfo.category == TypeCategory::MATRIX ||
            leftInfo.category == TypeCategory::LAYER) {
            return leftType;
        }
    }
    
    // Конкатенация
    if (op == ",") {
        TypeInfo leftInfo = getType(leftType);
        if (leftInfo.category == TypeCategory::VECTOR) {
            return leftType;
        }
    }
    
    return getCommonType(leftType, rightType);
}

std::string TypeChecker::inferUnaryOpType(UnaryOpNodePtr unary) {
    if (!unary || !unary->operand) {
        return "auto";
    }
    
    std::string operandType = inferType(unary->operand);
    std::string op = unary->op;
    
    if (!checkUnaryOperator(op, operandType, unary->token)) {
        return "auto";
    }
    
    if (op == "!" || op == "!!") {
        return "bool";
    }
    
    if (op == "-" || op == "+") {
        return operandType;
    }
    
    if (op == "'") {  // Транспонирование
        TypeInfo info = getType(operandType);
        if (info.category == TypeCategory::VECTOR) {
            return "vector<" + info.elementType + ">";
        }
        if (info.category == TypeCategory::MATRIX) {
            // Поменять размерности местами
            return operandType;  // Упрощённо
        }
    }
    
    if (op == "&") {  // Адрес
        return operandType + "*";
    }
    
    if (op == "*") {  // Разыменование
        if (operandType.find("*") != std::string::npos) {
            return operandType.substr(0, operandType.length() - 1);
        }
    }
    
    return operandType;
}

std::string TypeChecker::inferCallType(CallNodePtr call) {
    if (!call || !call->callee) {
        return "auto";
    }
    
    // Получение типа функции
    std::string calleeType = inferType(call->callee);
    
    // Поиск функции в таблице символов
    if (call->callee->nodeType == NodeType::IDENTIFIER) {
        auto ident = std::static_pointer_cast<IdentifierNode>(call->callee);
        Symbol symbol = symbolTable.get(ident->name);
        
        if (symbol.kind == SymbolKind::FUNCTION) {
            return symbol.returnType;
        }
        
        // Проверка встроенных функций
        if (isBuiltinFunction(ident->name)) {
            return getBuiltinFunctionReturnType(ident->name, call->arguments);
        }
    }
    
    // Если это метод класса
    if (call->callee->nodeType == NodeType::MEMBER_ACCESS) {
        auto member = std::static_pointer_cast<MemberAccessNode>(call->callee);
        return inferMemberType(member);
    }
    
    return "auto";
}

std::string TypeChecker::inferIndexType(IndexNodePtr index) {
    if (!index || !index->object) {
        return "auto";
    }
    
    std::string objectType = inferType(index->object);
    TypeInfo info = getType(objectType);
    
    switch (info.category) {
        case TypeCategory::VECTOR:
            return info.elementType;
        
        case TypeCategory::MATRIX:
            return info.elementType;
        
        case TypeCategory::LAYER:
            return info.elementType;
        
        case TypeCategory::COLLECTION:
            return "auto";  // Collection может содержать любые типы
        
        case TypeCategory::STRING:
            return "char";
        
        default:
            return "auto";
    }
}

std::string TypeChecker::inferMemberType(MemberAccessNodePtr member) {
    if (!member || !member->object) {
        return "auto";
    }
    
    std::string objectType = inferType(member->object);
    TypeInfo info = getType(objectType);
    
    // Поиск поля в классе
    auto fieldIt = info.fields.find(member->memberName);
    if (fieldIt != info.fields.end()) {
        return fieldIt->second;
    }
    
    // Поиск метода в классе
    auto methodIt = info.methods.find(member->memberName);
    if (methodIt != info.methods.end()) {
        return methodIt->second.returnType;
    }
    
    // Специальные поля GEM интерфейса
    if (member->memberName == "name") {
        return "string";
    }
    if (member->memberName == "params") {
        return "collection";
    }
    if (member->memberName == "time_last_call") {
        return "time";
    }
    
    return "auto";
}

std::string TypeChecker::inferArrayType(ArrayLiteralNodePtr array) {
    if (!array || array->elements.empty()) {
        return "vector<auto>";
    }
    
    // Вывод типа элементов
    std::string elemType = inferType(array->elements[0]);
    
    // Проверка согласованности типов
    for (size_t i = 1; i < array->elements.size(); i++) {
        std::string type = inferType(array->elements[i]);
        if (!isCompatible(elemType, type)) {
            elemType = getCommonType(elemType, type);
        }
    }
    
    return "vector<" + elemType + ">";
}

std::string TypeChecker::inferMatrixType(MatrixLiteralNodePtr matrix) {
    if (!matrix || matrix->rows.empty()) {
        return "matrix<auto>";
    }
    
    // Вывод типа элементов
    std::string elemType = "double";  // По умолчанию
    
    if (!matrix->rows[0].empty()) {
        elemType = inferType(matrix->rows[0][0]);
    }
    
    return "matrix<" + elemType + ">";
}

std::string TypeChecker::inferTernaryType(TernaryNodePtr ternary) {
    if (!ternary || !ternary->trueExpr || !ternary->falseExpr) {
        return "auto";
    }
    
    std::string trueType = inferType(ternary->trueExpr);
    std::string falseType = inferType(ternary->falseExpr);
    
    return getCommonType(trueType, falseType);
}

TypeInfo TypeChecker::getFullTypeInfo(ExpressionNodePtr expr) {
    std::string typeName = inferType(expr);
    return getType(typeName);
}

// ============================================================================
// Проверка совместимости типов
// ============================================================================

bool TypeChecker::isCompatible(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return true;
    if (type1 == "auto" || type2 == "auto") return true;
    if (type1 == "void" || type2 == "void") return false;
    
    std::string norm1 = normalizeType(type1);
    std::string norm2 = normalizeType(type2);
    
    // Числовые типы
    if (isNumericType(norm1) && isNumericType(norm2)) {
        return true;
    }
    
    // Проверка иерархии классов
    TypeInfo info1 = getType(norm1);
    TypeInfo info2 = getType(norm2);
    
    if (info1.category == TypeCategory::CLASS && 
        info2.category == TypeCategory::CLASS) {
        // Проверка наследования
        if (isSubclassOf(norm1, norm2)) {
            return true;
        }
    }
    
    // Проверка интерфейсов
    if (implementsInterface(norm1, norm2)) {
        return true;
    }
    
    return canConvert(norm1, norm2);
}

bool TypeChecker::canConvert(const std::string& from, const std::string& to) {
    if (from == to) return true;
    if (to == "auto") return true;
    if (from == "void") return false;
    
    std::string normFrom = normalizeType(from);
    std::string normTo = normalizeType(to);
    
    // Расширяющие преобразования числовых типов
    if (isNumericType(normFrom) && isNumericType(normTo)) {
        int fromRank = getNumericTypeRank(normFrom);
        int toRank = getNumericTypeRank(normTo);
        return toRank >= fromRank;  // Только расширяющие
    }
    
    // Преобразование vector<char> -> string
    if (normFrom.find("vector<char>") != std::string::npos && normTo == "string") {
        return true;
    }
    
    // Преобразование matrix<char> -> vector<string>
    if (normFrom.find("matrix<char>") != std::string::npos && 
        normTo.find("vector<string>") != std::string::npos) {
        return true;
    }
    
    // Преобразование указателей
    if (normFrom.find("*") != std::string::npos && normTo == "auto*") {
        return true;
    }
    
    // Null pointer
    if (normFrom == "null" && normTo.find("*") != std::string::npos) {
        return true;
    }
    
    return false;
}

std::string TypeChecker::getCommonType(const std::string& type1, const std::string& type2) {
    if (type1 == "auto") return type2;
    if (type2 == "auto") return type1;
    if (type1 == type2) return type1;
    
    std::string norm1 = normalizeType(type1);
    std::string norm2 = normalizeType(type2);
    
    // Числовая иерархия
    if (isNumericType(norm1) && isNumericType(norm2)) {
        int rank1 = getNumericTypeRank(norm1);
        int rank2 = getNumericTypeRank(norm2);
        return (rank1 > rank2) ? norm1 : norm2;
    }
    
    // Вектора/матрицы с разными типами элементов
    TypeInfo info1 = getType(norm1);
    TypeInfo info2 = getType(norm2);
    
    if (info1.category == info2.category) {
        std::string commonElem = getCommonType(info1.elementType, info2.elementType);
        
        if (info1.category == TypeCategory::VECTOR) {
            return "vector<" + commonElem + ">";
        }
        if (info1.category == TypeCategory::MATRIX) {
            return "matrix<" + commonElem + ">";
        }
        if (info1.category == TypeCategory::LAYER) {
            return "layer<" + commonElem + ">";
        }
    }
    
    // По умолчанию
    return "double";
}

bool TypeChecker::isNumericType(const std::string& type) {
    std::string norm = normalizeType(type);
    return (norm == "int4" || norm == "int8" || norm == "int16" || 
            norm == "int32" || norm == "int64" || norm == "int" ||
            norm == "single" || norm == "double" || norm == "float");
}

int TypeChecker::getNumericTypeRank(const std::string& type) {
    std::string norm = normalizeType(type);
    
    if (norm == "int4") return 1;
    if (norm == "int8") return 2;
    if (norm == "int16") return 3;
    if (norm == "int32" || norm == "int") return 4;
    if (norm == "int64") return 5;
    if (norm == "single" || norm == "float") return 6;
    if (norm == "double") return 7;
    
    return 0;
}

bool TypeChecker::isSubclassOf(const std::string& subclass, const std::string& superclass) {
    TypeInfo info = getType(subclass);
    
    // Проверка прямых родителей
    for (const auto& parent : info.parentClasses) {
        if (parent == superclass) {
            return true;
        }
        // Рекурсивная проверка
        if (isSubclassOf(parent, superclass)) {
            return true;
        }
    }
    
    return false;
}

bool TypeChecker::implementsInterface(const std::string& className, const std::string& interfaceName) {
    TypeInfo info = getType(className);
    
    // Проверка реализованных интерфейсов
    for (const auto& iface : info.interfaces) {
        if (iface == interfaceName) {
            return true;
        }
    }
    
    // Проверка GEM интерфейса
    if (interfaceName == "gem") {
        // Все классы могут реализовывать GEM
        return true;
    }
    
    return false;
}

// ============================================================================
// Проверка операторов
// ============================================================================

bool TypeChecker::checkBinaryOperator(const std::string& op, const std::string& leftType,
                                      const std::string& rightType, const Token& token) {
    std::string normLeft = normalizeType(leftType);
    std::string normRight = normalizeType(rightType);
    
    // Арифметические операторы
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^") {
        if (!isNumericType(normLeft) || !isNumericType(normRight)) {
            LOG_WARNING("Arithmetic operator requires numeric types: " + op);
            return false;
        }
        return true;
    }
    
    // Матричное деление
    if (op == "\\") {
        TypeInfo leftInfo = getType(normLeft);
        TypeInfo rightInfo = getType(normRight);
        
        if (leftInfo.category != TypeCategory::MATRIX) {
            LOG_WARNING("Matrix division requires matrix on left side");
            return false;
        }
        return true;
    }
    
    // Операторы сравнения
    if (op == "==" || op == "!=" || op == "<" || op == ">" || 
        op == "<=" || op == ">=") {
        return isCompatible(normLeft, normRight);
    }
    
    // Логические операторы
    if (op == "&" || op == "|" || op == "%" || 
        op == "&&" || op == "||" || op == "%%") {
        if (normLeft != "bool" || normRight != "bool") {
            LOG_WARNING("Logical operator requires bool types: " + op);
            return false;
        }
        return true;
    }
    
    // Поэлементные операторы
    if (op.substr(0, 1) == ".") {
        TypeInfo leftInfo = getType(normLeft);
        TypeInfo rightInfo = getType(normRight);
        
        if (leftInfo.category != TypeCategory::VECTOR &&
            leftInfo.category != TypeCategory::MATRIX &&
            leftInfo.category != TypeCategory::LAYER) {
            LOG_WARNING("Element-wise operator requires vector/matrix/layer type");
            return false;
        }
        
        // Проверка broadcasting
        return checkBroadcasting(leftInfo, rightInfo);
    }
    
    // Операторы присваивания
    if (op == "=") {
        return checkAssignment(normLeft, normRight, token);
    }
    
    // Конкатенация
    if (op == ",") {
        // Разрешена для векторов и коллекций
        TypeInfo leftInfo = getType(normLeft);
        if (leftInfo.category == TypeCategory::VECTOR ||
            leftInfo.category == TypeCategory::COLLECTION) {
            return true;
        }
    }
    
    // Неизвестный оператор
    LOG_WARNING("Unknown binary operator: " + op);
    return true;  // Разрешить по умолчанию
}

bool TypeChecker::checkUnaryOperator(const std::string& op, const std::string& operandType,
                                     const Token& token) {
    std::string norm = normalizeType(operandType);
    
    if (op == "!" || op == "!!") {
        if (norm != "bool") {
            LOG_WARNING("Logical NOT requires bool type");
            return false;
        }
        return true;
    }
    
    if (op == "-" || op == "+") {
        if (!isNumericType(norm)) {
            LOG_WARNING("Unary +/- requires numeric type");
            return false;
        }
        return true;
    }
    
    if (op == "'") {  // Транспонирование
        TypeInfo info = getType(norm);
        if (info.category != TypeCategory::VECTOR &&
            info.category != TypeCategory::MATRIX) {
            LOG_WARNING("Transpose requires vector or matrix type");
            return false;
        }
        return true;
    }
    
    if (op == "&") {  // Адрес
        return true;  // Можно взять адрес любой переменной
    }
    
    if (op == "*") {  // Разыменование
        if (norm.find("*") == std::string::npos) {
            LOG_WARNING("Dereference requires pointer type");
            return false;
        }
        return true;
    }
    
    return true;
}

bool TypeChecker::checkAssignment(const std::string& targetType, const std::string& valueType,
                                  const Token& token) {
    if (targetType == "auto") return true;
    if (valueType == "auto") return true;
    
    std::string normTarget = normalizeType(targetType);
    std::string normValue = normalizeType(valueType);
    
    if (isCompatible(normValue, normTarget)) {
        return true;
    }
    
    // Проверка явных преобразований
    if (canConvert(normValue, normTarget)) {
        LOG_DEBUG("Implicit conversion: " + normValue + " -> " + normTarget);
        return true;
    }
    
    LOG_WARNING("Type mismatch in assignment: " + normValue + " -> " + normTarget);
    return false;
}

bool TypeChecker::checkBroadcasting(TypeInfo& left, TypeInfo& right) {
    // MATLAB-style broadcasting rules
    
    // Скаляр может распространяться на любую размерность
    if (right.category == TypeCategory::SCALAR) {
        return true;
    }
    
    // Вектор может распространяться на матрицу
    if (left.category == TypeCategory::MATRIX && 
        right.category == TypeCategory::VECTOR) {
        // Проверка совместимости размеров
        return true;  // Упрощённо
    }
    
    // Одинаковые категории
    if (left.category == right.category) {
        return isCompatible(left.elementType, right.elementType);
    }
    
    return false;
}

// ============================================================================
// Проверка матричных операций
// ============================================================================

bool TypeChecker::checkMatrixOperation(const std::string& op, TypeInfo& left, TypeInfo& right) {
    if (left.category != TypeCategory::MATRIX || 
        right.category != TypeCategory::MATRIX) {
        return false;
    }
    
    // Умножение матриц
    if (op == "*") {
        // Число столбцов левой == числу строк правой
        if (left.dimensions.size() >= 2 && right.dimensions.size() >= 2) {
            return left.dimensions[1] == right.dimensions[0];
        }
        return true;  // Для динамических матриц
    }
    
    // Поэлементные операции
    if (op.substr(0, 1) == ".") {
        // Размеры должны совпадать или применяться broadcasting
        return checkBroadcasting(left, right);
    }
    
    return true;
}

// ============================================================================
// Проверка вызовов функций
// ============================================================================

bool TypeChecker::checkFunctionCall(const std::string& name, 
                                    const std::vector<ExpressionNodePtr>& args,
                                    const Token& token) {
    Symbol symbol = symbolTable.get(name);
    
    if (symbol.kind != SymbolKind::FUNCTION) {
        // Проверка встроенных функций
        if (isBuiltinFunction(name)) {
            return checkBuiltinFunctionCall(name, args, token);
        }
        
        LOG_WARNING("Function not found: " + name);
        return false;
    }
    
    // Проверка количества аргументов
    if (args.size() < symbol.parameters.size()) {
        // Проверка опциональных параметров
        // Упрощённо
    }
    
    // Проверка типов аргументов
    for (size_t i = 0; i < args.size() && i < symbol.parameters.size(); i++) {
        std::string argType = inferType(args[i]);
        std::string paramType = symbol.parameters[i].second;
        
        if (!isCompatible(argType, paramType) && !canConvert(argType, paramType)) {
            LOG_WARNING("Argument type mismatch for " + name + 
                       ": expected " + paramType + ", got " + argType);
        }
    }
    
    return true;
}

bool TypeChecker::checkBuiltinFunctionCall(const std::string& name,
                                           const std::vector<ExpressionNodePtr>& args,
                                           const Token& token) {
    // Проверка встроенных функций
    
    if (name == "print" || name == "write") {
        return true;  // Vararg функция
    }
    
    if (name == "size") {
        if (args.empty()) {
            LOG_WARNING("size() requires at least one argument");
            return false;
        }
        return true;
    }
    
    if (name == "zeros" || name == "ones" || name == "eye" || name == "rand") {
        if (args.size() < 1 || args.size() > 2) {
            LOG_WARNING(name + "() requires 1 or 2 arguments");
            return false;
        }
        return true;
    }
    
    if (name == "time") {
        // time() может иметь 0, 1, или 2 аргумента
        if (args.size() > 2) {
            LOG_WARNING("time() requires at most 2 arguments");
            return false;
        }
        return true;
    }
    
    if (name == "type" || name == "rtti") {
        if (args.empty()) {
            LOG_WARNING(name + "() requires one argument");
            return false;
        }
        return true;
    }
    
    if (name == "parallel") {
        // parallel() для GPU конфигурации
        return true;
    }
    
    if (name == "dbgstop" || name == "dbgprint" || 
        name == "dbgcontext" || name == "dbgstack") {
        // Отладочные функции
        return true;
    }
    
    return true;  // По умолчанию разрешить
}

std::string TypeChecker::getBuiltinFunctionReturnType(const std::string& name,
                                                      const std::vector<ExpressionNodePtr>& args) {
    if (name == "print" || name == "write" || 
        name == "dbgstop" || name == "dbgprint" || 
        name == "dbgcontext" || name == "dbgstack") {
        return "void";
    }
    
    if (name == "size") {
        return "vector<int32>";
    }
    
    if (name == "zeros" || name == "ones" || name == "rand") {
        return "matrix<double>";
    }
    
    if (name == "eye") {
        return "matrix<double>";
    }
    
    if (name == "time" || name == "time_now") {
        return "time";
    }
    
    if (name == "type" || name == "rtti") {
        return "rtti";
    }
    
    if (name == "sum" || name == "mean" || name == "max" || name == "min") {
        return "double";
    }
    
    if (name == "abs" || name == "sqrt" || name == "exp" || 
        name == "log" || name == "sin" || name == "cos" || name == "tan") {
        return "double";
    }
    
    if (name == "length") {
        return "int32";
    }
    
    return "auto";
}

bool TypeChecker::isBuiltinFunction(const std::string& name) {
    static const std::vector<std::string> builtins = {
        "print", "write", "read", "open", "close",
        "zeros", "ones", "eye", "rand", "size", "length",
        "sum", "mean", "max", "min", "abs", "sqrt", "exp", "log", "log10",
        "sin", "cos", "tan", "asin", "acos", "atan",
        "time", "time_now", "sleep",
        "type", "rtti",
        "dbgstop", "dbgprint", "dbgcontext", "dbgstack",
        "parallel", "assert",
        "transpose", "inv", "det"
    };
    
    return std::find(builtins.begin(), builtins.end(), name) != builtins.end();
}

// ============================================================================
// Регистрация пользовательских типов
// ============================================================================

void TypeChecker::registerClass(const std::string& name, TypeInfo info) {
    info.category = TypeCategory::CLASS;
    registerType(name, info);
    LOG_INFO("Class registered: " + name);
}

void TypeChecker::registerInterface(const std::string& name, TypeInfo info) {
    info.category = TypeCategory::INTERFACE;
    info.isInterface = true;
    registerType(name, info);
    LOG_INFO("Interface registered: " + name);
}

void TypeChecker::addMethod(const std::string& className, const std::string& methodName,
                            const std::string& returnType,
                            const std::vector<std::pair<std::string, std::string>>& params) {
    TypeInfo& info = types[className];
    
    MethodInfo method;
    method.name = methodName;
    method.returnType = returnType;
    method.parameters = params;
    method.isStatic = false;
    method.isVirtual = false;
    method.isAbstract = false;
    
    info.methods[methodName] = method;
}

void TypeChecker::addField(const std::string& className, const std::string& fieldName,
                           const std::string& fieldType) {
    TypeInfo& info = types[className];
    info.fields[fieldName] = fieldType;
}

void TypeChecker::addParentClass(const std::string& className, const std::string& parentName) {
    TypeInfo& info = types[className];
    info.parentClasses.push_back(parentName);
}

void TypeChecker::addInterface(const std::string& className, const std::string& interfaceName) {
    TypeInfo& info = types[className];
    info.interfaces.push_back(interfaceName);
}

void TypeChecker::addOperator(const std::string& className, const std::string& op,
                              const std::string& returnType,
                              const std::vector<std::string>& paramTypes) {
    TypeInfo& info = types[className];
    
    OperatorInfo opInfo;
    opInfo.op = op;
    opInfo.returnType = returnType;
    opInfo.paramTypes = paramTypes;
    
    info.operators.push_back(opInfo);
}

// ============================================================================
// Валидация
// ============================================================================

bool TypeChecker::validateType(const std::string& typeName) {
    TypeInfo info = getType(typeName);
    
    if (info.category == TypeCategory::UNKNOWN) {
        LOG_ERROR("Unknown type: " + typeName);
        return false;
    }
    
    // Проверка шаблонных типов
    if (info.isTemplate && info.elementType.empty()) {
        LOG_ERROR("Template type without parameter: " + typeName);
        return false;
    }
    
    // Проверка элементов шаблона
    if (!info.elementType.empty()) {
        if (!validateType(info.elementType)) {
            return false;
        }
    }
    
    return true;
}

bool TypeChecker::validateClass(const std::string& className) {
    TypeInfo info = getType(className);
    
    if (info.category != TypeCategory::CLASS) {
        LOG_ERROR("Not a class: " + className);
        return false;
    }
    
    // Проверка родителей
    for (const auto& parent : info.parentClasses) {
        TypeInfo parentInfo = getType(parent);
        if (parentInfo.category == TypeCategory::UNKNOWN) {
            LOG_ERROR("Unknown parent class: " + parent);
            return false;
        }
    }
    
    // Проверка интерфейсов
    for (const auto& iface : info.interfaces) {
        TypeInfo ifaceInfo = getType(iface);
        if (ifaceInfo.category != TypeCategory::INTERFACE) {
            LOG_ERROR("Not an interface: " + iface);
            return false;
        }
        
        // Проверка реализации методов интерфейса
        // Упрощённо
    }
    
    return true;
}

bool TypeChecker::validateFunction(const std::string& funcName) {
    Symbol symbol = symbolTable.get(funcName);
    
    if (symbol.kind != SymbolKind::FUNCTION) {
        LOG_ERROR("Not a function: " + funcName);
        return false;
    }
    
    // Проверка типа возврата
    if (!validateType(symbol.returnType)) {
        return false;
    }
    
    // Проверка типов параметров
    for (const auto& param : symbol.parameters) {
        if (!validateType(param.second)) {
            return false;
        }
    }
    
    return true;
}

// ============================================================================
// Логирование
// ============================================================================

void TypeChecker::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[TypeChecker] " << message << std::endl;
        LOG_INFO("[TypeChecker] " + message);
    }
}

void TypeChecker::setVerboseLevel(int level) {
    verboseLevel = level;
}

int TypeChecker::getVerboseLevel() const {
    return verboseLevel;
}

// ============================================================================
// Сериализация
// ============================================================================

std::string TypeChecker::typeToString(const TypeInfo& info) const {
    std::ostringstream oss;
    
    oss << "Type: " << info.name << "\n";
    oss << "  Category: ";
    
    switch (info.category) {
        case TypeCategory::SCALAR: oss << "SCALAR"; break;
        case TypeCategory::VECTOR: oss << "VECTOR"; break;
        case TypeCategory::MATRIX: oss << "MATRIX"; break;
        case TypeCategory::LAYER: oss << "LAYER"; break;
        case TypeCategory::COLLECTION: oss << "COLLECTION"; break;
        case TypeCategory::CLASS: oss << "CLASS"; break;
        case TypeCategory::INTERFACE: oss << "INTERFACE"; break;
        case TypeCategory::FUNCTION: oss << "FUNCTION"; break;
        case TypeCategory::POINTER: oss << "POINTER"; break;
        case TypeCategory::AUTO: oss << "AUTO"; break;
        case TypeCategory::VOID: oss << "VOID"; break;
        default: oss << "UNKNOWN"; break;
    }
    
    oss << "\n";
    
    if (!info.elementType.empty()) {
        oss << "  Element Type: " << info.elementType << "\n";
    }
    
    if (!info.dimensions.empty()) {
        oss << "  Dimensions: [";
        for (size_t i = 0; i < info.dimensions.size(); i++) {
            if (i > 0) oss << ", ";
            oss << info.dimensions[i];
        }
        oss << "]\n";
    }
    
    oss << "  Size: " << info.size << " bytes\n";
    oss << "  Mutable: " << (info.isMutable ? "yes" : "no") << "\n";
    oss << "  Nullable: " << (info.isNullable ? "yes" : "no") << "\n";
    
    return oss.str();
}

std::string TypeChecker::getReport() const {
    std::ostringstream oss;
    
    oss << "=== Type Checker Report ===\n\n";
    oss << "Total types registered: " << types.size() << "\n\n";
    
    oss << "Types by category:\n";
    
    std::map<TypeCategory, int> categoryCount;
    for (const auto& pair : types) {
        categoryCount[pair.second.category]++;
    }
    
    for (const auto& pair : categoryCount) {
        oss << "  ";
        switch (pair.first) {
            case TypeCategory::SCALAR: oss << "SCALAR"; break;
            case TypeCategory::VECTOR: oss << "VECTOR"; break;
            case TypeCategory::MATRIX: oss << "MATRIX"; break;
            case TypeCategory::CLASS: oss << "CLASS"; break;
            case TypeCategory::INTERFACE: oss << "INTERFACE"; break;
            default: oss << "OTHER"; break;
        }
        oss << ": " << pair.second << "\n";
    }
    
    return oss.str();
}

} // namespace proxima
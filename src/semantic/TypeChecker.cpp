#include "TypeChecker.h"
#include <algorithm>
#include <regex>

namespace proxima {

TypeChecker::TypeChecker() {
    registerBuiltinTypes();
}

void TypeChecker::clear() {
    types.clear();
    typeAliases.clear();
    registerBuiltinTypes();
}

void TypeChecker::registerBuiltinTypes() {
    // Scalar types
    TypeInfo int4; int4.name = "int4"; int4.category = TypeCategory::SCALAR; int4.size = 1;
    registerType("int4", int4);
    
    TypeInfo int8; int8.name = "int8"; int8.category = TypeCategory::SCALAR; int8.size = 1;
    registerType("int8", int8);
    
    TypeInfo int16; int16.name = "int16"; int16.category = TypeCategory::SCALAR; int16.size = 2;
    registerType("int16", int16);
    
    TypeInfo int32; int32.name = "int32"; int32.category = TypeCategory::SCALAR; int32.size = 4;
    registerType("int32", int32);
    registerType("int", int32);
    
    TypeInfo int64; int64.name = "int64"; int64.category = TypeCategory::SCALAR; int64.size = 8;
    registerType("int64", int64);
    
    TypeInfo single; single.name = "single"; single.category = TypeCategory::SCALAR; single.size = 4;
    registerType("single", single);
    registerType("float", single);
    
    TypeInfo double_; double_.name = "double"; double_.category = TypeCategory::SCALAR; double_.size = 8;
    registerType("double", double_);
    
    TypeInfo bool_; bool_.name = "bool"; bool_.category = TypeCategory::SCALAR; bool_.size = 1;
    registerType("bool", bool_);
    
    TypeInfo char_; char_.name = "char"; char_.category = TypeCategory::SCALAR; char_.size = 1;
    registerType("char", char_);
    
    TypeInfo string; string.name = "string"; string.category = TypeCategory::SCALAR; string.size = 32;
    registerType("string", string);
    
    TypeInfo time; time.name = "time"; time.category = TypeCategory::CLASS; time.size = 8;
    registerType("time", time);
    
    TypeInfo file; file.name = "file"; file.category = TypeCategory::CLASS; file.size = 64;
    registerType("file", file);
    
    // Generic types
    TypeInfo vector; vector.name = "vector"; vector.category = TypeCategory::VECTOR;
    registerType("vector", vector);
    
    TypeInfo matrix; matrix.name = "matrix"; matrix.category = TypeCategory::MATRIX;
    registerType("matrix", matrix);
    
    TypeInfo layer; layer.name = "layer"; layer.category = TypeCategory::LAYER;
    registerType("layer", layer);
    
    TypeInfo collection; collection.name = "collection"; collection.category = TypeCategory::COLLECTION;
    registerType("collection", collection);
    
    // Fixed-size matrices
    TypeInfo matrix22; matrix22.name = "matrix22"; matrix22.category = TypeCategory::MATRIX;
    matrix22.dimensions = {2, 2}; matrix22.size = 32;
    registerType("matrix22", matrix22);
    
    TypeInfo matrix33; matrix33.name = "matrix33"; matrix33.category = TypeCategory::MATRIX;
    matrix33.dimensions = {3, 3}; matrix33.size = 72;
    registerType("matrix33", matrix33);
    
    TypeInfo matrix44; matrix44.name = "matrix44"; matrix44.category = TypeCategory::MATRIX;
    matrix44.dimensions = {4, 4}; matrix44.size = 128;
    registerType("matrix44", matrix44);
    
    TypeInfo matrix34; matrix34.name = "matrix34"; matrix34.category = TypeCategory::MATRIX;
    matrix34.dimensions = {3, 4}; matrix34.size = 96;
    registerType("matrix34", matrix34);
    
    // Point types
    TypeInfo point2; point2.name = "point2"; point2.category = TypeCategory::CLASS; point2.size = 8;
    registerType("point2", point2);
    
    TypeInfo point3; point3.name = "point3"; point3.category = TypeCategory::CLASS; point3.size = 12;
    registerType("point3", point3);
    
    TypeInfo point4; point4.name = "point4"; point4.category = TypeCategory::CLASS; point4.size = 16;
    registerType("point4", point4);
    
    // Special types
    TypeInfo auto_; auto_.name = "auto"; auto_.category = TypeCategory::AUTO;
    registerType("auto", auto_);
    
    TypeInfo void_; void_.name = "void"; void_.category = TypeCategory::VOID;
    registerType("void", void_);
    
    TypeInfo type; type.name = "type"; type.category = TypeCategory::CLASS;
    registerType("type", type);
    
    TypeInfo method; method.name = "method"; method.category = TypeCategory::FUNCTION;
    registerType("method", method);
    
    TypeInfo rtti; rtti.name = "rtti"; rtti.category = TypeCategory::CLASS;
    registerType("rtti", rtti);
    
    TypeInfo parallel; parallel.name = "parallel"; parallel.category = TypeCategory::CLASS;
    registerType("parallel", parallel);
    
    // Type aliases
    typeAliases["i32"] = "int32";
    typeAliases["i64"] = "int64";
    typeAliases["f32"] = "single";
    typeAliases["f64"] = "double";
}

void TypeChecker::registerType(const std::string& name, TypeInfo info) {
    types[name] = info;
}

TypeInfo TypeChecker::getType(const std::string& name) {
    std::string normalized = normalizeType(name);
    
    auto it = types.find(normalized);
    if (it != types.end()) {
        return it->second;
    }
    
    // Check for generic types like vector<T>
    if (normalized.find("<") != std::string::npos) {
        std::string baseType = normalized.substr(0, normalized.find("<"));
        auto baseIt = types.find(baseType);
        if (baseIt != types.end()) {
            TypeInfo info = baseIt->second;
            info.name = normalized;
            info.elementType = extractElementType(normalized);
            return info;
        }
    }
    
    return TypeInfo();
}

bool TypeChecker::typeExists(const std::string& name) {
    return getType(name).category != TypeCategory::UNKNOWN;
}

std::string TypeChecker::normalizeType(const std::string& type) {
    std::string normalized = type;
    
    // Remove whitespace
    normalized.erase(std::remove(normalized.begin(), normalized.end(), ' '), normalized.end());
    
    // Check aliases
    auto it = typeAliases.find(normalized);
    if (it != typeAliases.end()) {
        return it->second;
    }
    
    return normalized;
}

std::string TypeChecker::inferType(ExpressionNodePtr expr) {
    if (!expr) return "auto";
    
    switch (expr->nodeType) {
        case NodeType::LITERAL: {
            auto literal = std::static_pointer_cast<LiteralNode>(expr);
            if (literal->token.type == TokenType::INTEGER) {
                return "int32";
            }
            if (literal->token.type == TokenType::FLOAT) {
                return "double";
            }
            if (literal->token.type == TokenType::STRING) {
                return "string";
            }
            if (literal->token.type == TokenType::CHAR) {
                return "char";
            }
            if (literal->token.type == TokenType::KEYWORD_TRUE || 
                literal->token.type == TokenType::KEYWORD_FALSE) {
                return "bool";
            }
            break;
        }
        
        case NodeType::IDENTIFIER: {
            // Would need symbol table lookup
            return "auto";
        }
        
        case NodeType::BINARY_OP: {
            auto binop = std::static_pointer_cast<BinaryOpNode>(expr);
            std::string leftType = inferType(binop->left);
            std::string rightType = inferType(binop->right);
            return getCommonType(leftType, rightType);
        }
        
        case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            // Would need function return type lookup
            return "auto";
        }
        
        case NodeType::ARRAY_LITERAL: {
            auto array = std::static_pointer_cast<ArrayLiteralNode>(expr);
            if (!array->elements.empty()) {
                std::string elemType = inferType(array->elements[0]);
                return "vector<" + elemType + ">";
            }
            return "vector<auto>";
        }
        
        default:
            break;
    }
    
    return "auto";
}

TypeInfo TypeChecker::getFullTypeInfo(ExpressionNodePtr expr) {
    std::string typeName = inferType(expr);
    return getType(typeName);
}

bool TypeChecker::isCompatible(const std::string& type1, const std::string& type2) {
    if (type1 == type2) return true;
    if (type1 == "auto" || type2 == "auto") return true;
    if (type1 == "void" || type2 == "void") return false;
    
    // Check numeric type compatibility
    std::vector<std::string> intTypes = {"int4", "int8", "int16", "int32", "int64"};
    std::vector<std::string> floatTypes = {"single", "double"};
    
    bool bothInt = std::find(intTypes.begin(), intTypes.end(), type1) != intTypes.end() &&
                   std::find(intTypes.begin(), intTypes.end(), type2) != intTypes.end();
    bool bothFloat = std::find(floatTypes.begin(), floatTypes.end(), type1) != floatTypes.end() &&
                     std::find(floatTypes.begin(), floatTypes.end(), type2) != floatTypes.end();
    
    if (bothInt || bothFloat) return true;
    
    // Int can convert to float
    if (std::find(intTypes.begin(), intTypes.end(), type1) != intTypes.end() &&
        std::find(floatTypes.begin(), floatTypes.end(), type2) != floatTypes.end()) {
        return true;
    }
    
    return canConvert(type1, type2);
}

bool TypeChecker::canConvert(const std::string& from, const std::string& to) {
    if (from == to) return true;
    if (to == "auto") return true;
    if (from == "void") return false;
    
    // Widening conversions are safe
    std::unordered_map<std::string, int> typeRank = {
        {"int4", 1}, {"int8", 2}, {"int16", 3}, {"int32", 4}, {"int64", 5},
        {"single", 6}, {"double", 7}
    };
    
    auto fromIt = typeRank.find(from);
    auto toIt = typeRank.find(to);
    
    if (fromIt != typeRank.end() && toIt != typeRank.end()) {
        return toIt->second >= fromIt->second;  // Widening only
    }
    
    return false;
}

std::string TypeChecker::getCommonType(const std::string& type1, const std::string& type2) {
    if (type1 == "auto") return type2;
    if (type2 == "auto") return type1;
    if (type1 == type2) return type1;
    
    // Numeric promotion
    std::unordered_map<std::string, int> typeRank = {
        {"int4", 1}, {"int8", 2}, {"int16", 3}, {"int32", 4}, {"int64", 5},
        {"single", 6}, {"double", 7}
    };
    
    auto t1 = typeRank.find(type1);
    auto t2 = typeRank.find(type2);
    
    if (t1 != typeRank.end() && t2 != typeRank.end()) {
        return t1->second > t2->second ? type1 : type2;
    }
    
    return "double";  // Default
}

bool TypeChecker::checkBinaryOperator(const std::string& op, const std::string& leftType,
                                      const std::string& rightType, const Token& token) {
    // Arithmetic operators
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        return isCompatible(leftType, rightType);
    }
    
    // Comparison operators
    if (op == "==" || op == "!=" || op == "<" || op == ">" || 
        op == "<=" || op == ">=") {
        return isCompatible(leftType, rightType);
    }
    
    // Logical operators
    if (op == "&" || op == "|" || op == "%") {
        return leftType == "bool" && rightType == "bool";
    }
    
    // Element-wise operators (start with .)
    if (op[0] == '.') {
        TypeInfo leftInfo = getType(leftType);
        TypeInfo rightInfo = getType(rightType);
        return checkMatrixOperation(op, leftInfo, rightInfo);
    }
    
    return true;  // Assume compatible for unknown operators
}

bool TypeChecker::checkUnaryOperator(const std::string& op, const std::string& operandType,
                                     const Token& token) {
    if (op == "!" || op == "!!") {
        return operandType == "bool";
    }
    if (op == "-" || op == "+") {
        TypeInfo info = getType(operandType);
        return info.category == TypeCategory::SCALAR;
    }
    if (op == "'") {  // Transpose
        TypeInfo info = getType(operandType);
        return info.category == TypeCategory::VECTOR || 
               info.category == TypeCategory::MATRIX;
    }
    return true;
}

bool TypeChecker::checkAssignment(const std::string& targetType, const std::string& valueType,
                                  const Token& token) {
    if (targetType == "auto") return true;
    if (valueType == "auto") return true;
    return isCompatible(valueType, targetType);
}

bool TypeChecker::checkMatrixOperation(const std::string& op, TypeInfo& left, TypeInfo& right) {
    // Check if both are matrix/vector types
    if (left.category != right.category) {
        // Check for broadcasting
        return checkBroadcasting(left, right);
    }
    
    // Same category - check element type compatibility
    return isCompatible(left.elementType, right.elementType);
}

bool TypeChecker::checkBroadcasting(TypeInfo& left, TypeInfo& right) {
    // Implement MATLAB-style broadcasting rules
    // Scalar can broadcast to any size
    if (left.category == TypeCategory::SCALAR || 
        right.category == TypeCategory::SCALAR) {
        return true;
    }
    
    // Vector can broadcast to matrix if dimensions match
    // This is simplified - full implementation would check all dimensions
    return true;
}

bool TypeChecker::isGPUType(const std::string& type) {
    return type.find("*") != std::string::npos;  // Pointer types
}

bool TypeChecker::isPointerType(const std::string& type) {
    return type.find("*") != std::string::npos;
}

size_t TypeChecker::getTypeSize(const std::string& type) {
    TypeInfo info = getType(type);
    return info.size > 0 ? info.size : 8;  // Default 8 bytes
}

TypeCategory TypeChecker::categorizeType(const std::string& type) {
    TypeInfo info = getType(type);
    return info.category;
}

std::string TypeChecker::extractElementType(const std::string& type) {
    size_t start = type.find("<");
    size_t end = type.find(">");
    if (start != std::string::npos && end != std::string::npos) {
        return type.substr(start + 1, end - start - 1);
    }
    return "";
}

std::vector<size_t> TypeChecker::extractDimensions(const std::string& type) {
    TypeInfo info = getType(type);
    return info.dimensions;
}

} // namespace proxima
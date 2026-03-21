#ifndef PROXIMA_TYPE_CHECKER_H
#define PROXIMA_TYPE_CHECKER_H

#include "../parser/AST.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace proxima {

enum class TypeCategory {
    SCALAR,
    VECTOR,
    MATRIX,
    LAYER,
    COLLECTION,
    CLASS,
    INTERFACE,
    FUNCTION,
    POINTER,
    AUTO,
    VOID,
    UNKNOWN
};

struct TypeInfo {
    std::string name;
    TypeCategory category;
    std::string elementType;  // For vector<T>, matrix<T>, etc.
    std::vector<size_t> dimensions;  // For fixed-size matrices
    bool isMutable;
    bool isNullable;
    int size;  // In bytes
    
    TypeInfo() : category(TypeCategory::UNKNOWN), isMutable(true), 
                 isNullable(false), size(0) {}
};

class TypeChecker {
public:
    TypeChecker();
    
    void clear();
    
    // Type inference
    std::string inferType(ExpressionNodePtr expr);
    TypeInfo getFullTypeInfo(ExpressionNodePtr expr);
    
    // Type compatibility
    bool isCompatible(const std::string& type1, const std::string& type2);
    bool canConvert(const std::string& from, const std::string& to);
    std::string getCommonType(const std::string& type1, const std::string& type2);
    
    // Operator type checking
    bool checkBinaryOperator(const std::string& op, const std::string& leftType,
                            const std::string& rightType, const Token& token);
    bool checkUnaryOperator(const std::string& op, const std::string& operandType,
                           const Token& token);
    bool checkAssignment(const std::string& targetType, const std::string& valueType,
                        const Token& token);
    
    // Type registration
    void registerType(const std::string& name, TypeInfo info);
    TypeInfo getType(const std::string& name);
    bool typeExists(const std::string& name);
    
    // Built-in types
    void registerBuiltinTypes();
    
    // Matrix/Vector operations
    bool checkMatrixOperation(const std::string& op, TypeInfo& left, TypeInfo& right);
    bool checkBroadcasting(TypeInfo& left, TypeInfo& right);
    
    // GPU types
    bool isGPUType(const std::string& type);
    bool isPointerType(const std::string& type);
    
private:
    std::unordered_map<std::string, TypeInfo> types;
    std::unordered_map<std::string, std::string> typeAliases;
    
    std::string normalizeType(const std::string& type);
    size_t getTypeSize(const std::string& type);
    TypeCategory categorizeType(const std::string& type);
    std::string extractElementType(const std::string& type);
    std::vector<size_t> extractDimensions(const std::string& type);
};

} // namespace proxima

#endif // PROXIMA_TYPE_CHECKER_H
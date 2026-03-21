#ifndef PROXIMA_TYPECHECKER_H
#define PROXIMA_TYPECHECKER_H

#include "../parser/AST.h"
#include "SymbolTable.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

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

struct MethodInfo {
    std::string name;
    std::string returnType;
    std::vector<std::pair<std::string, std::string>> parameters;
    bool isStatic;
    bool isVirtual;
    bool isAbstract;
};

struct OperatorInfo {
    std::string op;
    std::string returnType;
    std::vector<std::string> paramTypes;
};

struct TypeInfo {
    std::string name;
    TypeCategory category;
    std::string elementType;
    std::vector<size_t> dimensions;
    bool isMutable;
    bool isNullable;
    bool isTemplate;
    int size;
    std::string baseType;
    std::vector<std::string> templateArgs;
    std::unordered_map<std::string, MethodInfo> methods;
    std::unordered_map<std::string, std::string> fields;
    std::vector<OperatorInfo> operators;
    std::vector<std::string> parentClasses;
    std::vector<std::string> interfaces;
    bool isInterface;
    bool isAbstract;
    bool isFinal;
    
    TypeInfo() : category(TypeCategory::UNKNOWN), isMutable(true), 
                 isNullable(false), isTemplate(false), size(0),
                 isInterface(false), isAbstract(false), isFinal(false) {}
};

class TypeChecker {
public:
    TypeChecker();
    ~TypeChecker();
    
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
    
    // Matrix operations
    bool checkMatrixOperation(const std::string& op, TypeInfo& left, TypeInfo& right);
    bool checkBroadcasting(TypeInfo& left, TypeInfo& right);
    
    // Function calls
    bool checkFunctionCall(const std::string& name, 
                          const std::vector<ExpressionNodePtr>& args,
                          const Token& token);
    bool checkBuiltinFunctionCall(const std::string& name,
                                 const std::vector<ExpressionNodePtr>& args,
                                 const Token& token);
    std::string getBuiltinFunctionReturnType(const std::string& name,
                                            const std::vector<ExpressionNodePtr>& args);
    bool isBuiltinFunction(const std::string& name);
    
    // Type registration
    void registerType(const std::string& name, TypeInfo info);
    void registerClass(const std::string& name, TypeInfo info);
    void registerInterface(const std::string& name, TypeInfo info);
    void addMethod(const std::string& className, const std::string& methodName,
                  const std::string& returnType,
                  const std::vector<std::pair<std::string, std::string>>& params);
    void addField(const std::string& className, const std::string& fieldName,
                 const std::string& fieldType);
    void addParentClass(const std::string& className, const std::string& parentName);
    void addInterface(const std::string& className, const std::string& interfaceName);
    void addOperator(const std::string& className, const std::string& op,
                    const std::string& returnType,
                    const std::vector<std::string>& paramTypes);
    
    // Type queries
    TypeInfo getType(const std::string& name);
    bool typeExists(const std::string& name);
    std::string normalizeType(const std::string& type);
    
    // Validation
    bool validateType(const std::string& typeName);
    bool validateClass(const std::string& className);
    bool validateFunction(const std::string& funcName);
    
    // Configuration
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }
    
    // Reporting
    std::string typeToString(const TypeInfo& info) const;
    std::string getReport() const;
    
    // Symbol table integration
    void setSymbolTable(SymbolTable& table) { symbolTable = table; }
    
private:
    void registerBuiltinTypes();
    TypeInfo createTypeInfo(const std::string& name, TypeCategory category,
                           const std::string& elementType,
                           const std::vector<size_t>& dimensions,
                           int size);
    
    std::string inferLiteralType(LiteralNodePtr literal);
    std::string inferIdentifierType(IdentifierNodePtr ident);
    std::string inferBinaryOpType(BinaryOpNodePtr binop);
    std::string inferUnaryOpType(UnaryOpNodePtr unary);
    std::string inferCallType(CallNodePtr call);
    std::string inferIndexType(IndexNodePtr index);
    std::string inferMemberType(MemberAccessNodePtr member);
    std::string inferArrayType(ArrayLiteralNodePtr array);
    std::string inferMatrixType(MatrixLiteralNodePtr matrix);
    std::string inferTernaryType(TernaryNodePtr ternary);
    
    bool isNumericType(const std::string& type);
    int getNumericTypeRank(const std::string& type);
    bool isSubclassOf(const std::string& subclass, const std::string& superclass);
    bool implementsInterface(const std::string& className, const std::string& interfaceName);
    
    void log(int level, const std::string& message);
    
    std::unordered_map<std::string, TypeInfo> types;
    std::unordered_map<std::string, std::string> typeAliases;
    SymbolTable symbolTable;
    int verboseLevel;
};

} // namespace proxima

#endif // PROXIMA_TYPECHECKER_H
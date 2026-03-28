#ifndef PROXIMA_AST_H
#define PROXIMA_AST_H

#include "../lexer/Token.h"
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace proxima {

// Forward declarations
class ASTNode;
class ExpressionNode;
class StatementNode;
class DeclarationNode;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ExpressionNodePtr = std::shared_ptr<ExpressionNode>;
using StatementNodePtr = std::shared_ptr<StatementNode>;
using DeclarationNodePtr = std::shared_ptr<DeclarationNode>;

enum class NodeType {
    // Declarations
    VARIABLE_DECL,
    FUNCTION_DECL,
    CLASS_DECL,
    INTERFACE_DECL,
    NAMESPACE_DECL,
    
    // LLM directives
    INTENT_BLOCK,
    GENERATED_BLOCK,
    FIXED_BLOCK,
    
    // Statements
    ASSIGNMENT,
    IF_STATEMENT,
    FOR_LOOP,
    WHILE_LOOP,
    DO_WHILE_LOOP,
    SWITCH_STATEMENT,
    RETURN_STATEMENT,
    BREAK_STATEMENT,
    CONTINUE_STATEMENT,
    EXPRESSION_STATEMENT,
    BLOCK,
    
    // Expressions
    BINARY_OP,
    UNARY_OP,
    CALL_EXPR,
    INDEX_EXPR,
    MEMBER_ACCESS,
    LITERAL,
    IDENTIFIER,
    ARRAY_LITERAL,
    MATRIX_LITERAL,
    TERNARY,
    
    // Types
    TYPE_REF,
    TEMPLATE_TYPE
};

class ASTNode {
public:
    NodeType nodeType;
    Token token;
    std::string filename;
    
    ASTNode(NodeType type, const Token& tok, const std::string& file = "")
        : nodeType(type), token(tok), filename(file) {}
    
    virtual ~ASTNode() = default;
    virtual std::string toString(int indent = 0) const = 0;
};

class ExpressionNode : public ASTNode {
public:
    ExpressionNode(NodeType type, const Token& tok, const std::string& file = "")
        : ASTNode(type, tok, file) {}
};

class StatementNode : public ASTNode {
public:
    StatementNode(NodeType type, const Token& tok, const std::string& file = "")
        : ASTNode(type, tok, file) {}
};

class DeclarationNode : public ASTNode {
public:
    std::string name;
    std::string typeName;
    
    DeclarationNode(NodeType type, const Token& tok, const std::string& n, const std::string& t, const std::string& file = "")
        : ASTNode(type, tok, file), name(n), typeName(t) {}
};

// Expression types
class LiteralNode : public ExpressionNode {
public:
    std::variant<int64_t, double, std::string, char, bool> value;
    
    LiteralNode(const Token& tok, const std::string& file = "")
        : ExpressionNode(NodeType::LITERAL, tok, file) {}
    
    std::string toString(int indent = 0) const override;
};

class IdentifierNode : public ExpressionNode {
public:
    std::string name;
    
    IdentifierNode(const Token& tok, const std::string& file = "")
        : ExpressionNode(NodeType::IDENTIFIER, tok, file), name(tok.value) {}
    
    std::string toString(int indent = 0) const override;
};

class BinaryOpNode : public ExpressionNode {
public:
    ExpressionNodePtr left;
    ExpressionNodePtr right;
    std::string op;
    
    BinaryOpNode(const Token& tok, ExpressionNodePtr l, ExpressionNodePtr r, const std::string& file = "")
        : ExpressionNode(NodeType::BINARY_OP, tok, file), left(l), right(r), op(tok.value) {}
    
    std::string toString(int indent = 0) const override;
};

class CallNode : public ExpressionNode {
public:
    ExpressionNodePtr callee;
    std::vector<ExpressionNodePtr> arguments;
    
    CallNode(const Token& tok, ExpressionNodePtr c, const std::vector<ExpressionNodePtr>& args, const std::string& file = "")
        : ExpressionNode(NodeType::CALL_EXPR, tok, file), callee(c), arguments(args) {}
    
    std::string toString(int indent = 0) const override;
};

// Statement types
class IfNode : public StatementNode {
public:
    ExpressionNodePtr condition;
    StatementNodePtr thenBranch;
    StatementNodePtr elseBranch;
    
    IfNode(const Token& tok, ExpressionNodePtr cond, StatementNodePtr thenB, StatementNodePtr elseB, const std::string& file = "")
        : StatementNode(NodeType::IF_STATEMENT, tok, file), condition(cond), thenBranch(thenB), elseBranch(elseB) {}
    
    std::string toString(int indent = 0) const override;
};

class ForNode : public StatementNode {
public:
    std::string variable;
    ExpressionNodePtr iterable;
    StatementNodePtr body;
    
    ForNode(const Token& tok, const std::string& var, ExpressionNodePtr iter, StatementNodePtr b, const std::string& file = "")
        : StatementNode(NodeType::FOR_LOOP, tok, file), variable(var), iterable(iter), body(b) {}
    
    std::string toString(int indent = 0) const override;
};

class ReturnNode : public StatementNode {
public:
    ExpressionNodePtr value;
    
    ReturnNode(const Token& tok, ExpressionNodePtr v, const std::string& file = "")
        : StatementNode(NodeType::RETURN_STATEMENT, tok, file), value(v) {}
    
    std::string toString(int indent = 0) const override;
};

class BlockNode : public StatementNode {
public:
    std::vector<StatementNodePtr> statements;
    
    BlockNode(const Token& tok, const std::string& file = "")
        : StatementNode(NodeType::BLOCK, tok, file) {}
    
    std::string toString(int indent = 0) const override;
};

// Declaration types
class VariableDeclNode : public DeclarationNode {
public:
    ExpressionNodePtr initializer;
    
    VariableDeclNode(const Token& tok, const std::string& n, const std::string& t, ExpressionNodePtr init, const std::string& file = "")
        : DeclarationNode(NodeType::VARIABLE_DECL, tok, n, t, file), initializer(init) {}
    
    std::string toString(int indent = 0) const override;
};

class FunctionDeclNode : public DeclarationNode {
public:
    std::vector<std::pair<std::string, std::string>> parameters;
    std::string returnType;
    StatementNodePtr body;
    
    FunctionDeclNode(const Token& tok, const std::string& n, const std::string& t, 
                     const std::vector<std::pair<std::string, std::string>>& params,
                     StatementNodePtr b, const std::string& file = "")
        : DeclarationNode(NodeType::FUNCTION_DECL, tok, n, t, file), parameters(params), returnType(t), body(b) {}
    
    std::string toString(int indent = 0) const override;
};

class ProgramNode : public ASTNode {
public:
    std::vector<DeclarationNodePtr> declarations;
    std::vector<StatementNodePtr> statements;
    
    ProgramNode(const std::string& file = "")
        : ASTNode(NodeType::BLOCK, Token(), file) {}
    
    std::string toString(int indent = 0) const override;
};

// LLM directive nodes
class IntentBlockNode : public StatementNode {
public:
    std::string intent;  // Текст интента (описание задачи для LLM)
    StatementNodePtr body;  // Сгенерированный блок кода
    
    IntentBlockNode(const Token& tok, const std::string& intentText, StatementNodePtr b, const std::string& file = "")
        : StatementNode(NodeType::INTENT_BLOCK, tok, file), intent(intentText), body(b) {}
    
    std::string toString(int indent = 0) const override;
};

class GeneratedBlockNode : public StatementNode {
public:
    StatementNodePtr body;  // Сгенерированный код
    
    GeneratedBlockNode(const Token& tok, StatementNodePtr b, const std::string& file = "")
        : StatementNode(NodeType::GENERATED_BLOCK, tok, file), body(b) {}
    
    std::string toString(int indent = 0) const override;
};

class FixedBlockNode : public StatementNode {
public:
    StatementNodePtr body;  // Зафиксированный код (не может быть изменён LLM)
    
    FixedBlockNode(const Token& tok, StatementNodePtr b, const std::string& file = "")
        : StatementNode(NodeType::FIXED_BLOCK, tok, file), body(b) {}
    
    std::string toString(int indent = 0) const override;
};

} // namespace proxima

#endif // PROXIMA_AST_H
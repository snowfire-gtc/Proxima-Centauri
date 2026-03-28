#include "AST.h"
#include <sstream>
#include <iomanip>

namespace proxima {

std::string ASTNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "ASTNode[" << nodeType << "]";
    return oss.str();
}

std::string ExpressionNode::toString(int indent) const {
    return ASTNode::toString(indent);
}

std::string StatementNode::toString(int indent) const {
    return ASTNode::toString(indent);
}

std::string DeclarationNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "DeclarationNode[" << name << ": " << typeName << "]";
    return oss.str();
}

// LiteralNode
LiteralNode::LiteralNode(const Token& tok, const std::string& file)
    : ExpressionNode(NodeType::LITERAL, tok, file) {
    switch (tok.type) {
        case TokenType::INTEGER:
            value = std::stoll(tok.value);
            break;
        case TokenType::FLOAT:
            value = std::stod(tok.value);
            break;
        case TokenType::STRING:
            value = tok.value;
            break;
        case TokenType::CHAR:
            value = tok.value.empty() ? '\0' : tok.value[0];
            break;
        case TokenType::KEYWORD_TRUE:
        case TokenType::KEYWORD_FALSE:
            value = (tok.type == TokenType::KEYWORD_TRUE);
            break;
        default:
            value = tok.value;
    }
}

std::string LiteralNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Literal[";
    if (std::holds_alternative<int64_t>(value)) {
        oss << "int: " << std::get<int64_t>(value);
    } else if (std::holds_alternative<double>(value)) {
        oss << "double: " << std::get<double>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        oss << "string: \"" << std::get<std::string>(value) << "\"";
    } else if (std::holds_alternative<char>(value)) {
        oss << "char: '" << std::get<char>(value) << "'";
    } else if (std::holds_alternative<bool>(value)) {
        oss << "bool: " << (std::get<bool>(value) ? "true" : "false");
    }
    oss << "]";
    return oss.str();
}

// IdentifierNode
IdentifierNode::IdentifierNode(const Token& tok, const std::string& file)
    : ExpressionNode(NodeType::IDENTIFIER, tok, file), name(tok.value) {}

std::string IdentifierNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Identifier[" << name << "]";
    return oss.str();
}

// BinaryOpNode
BinaryOpNode::BinaryOpNode(const Token& op, ExpressionNodePtr l, ExpressionNodePtr r, const std::string& file)
    : ExpressionNode(NodeType::BINARY_OP, op, file), left(l), right(r), opType(op.type) {}

std::string BinaryOpNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "BinaryOp[" << Token::typeToString(opType) << "]\n";
    if (left) oss << left->toString(indent + 1) << "\n";
    if (right) oss << right->toString(indent + 1);
    return oss.str();
}

// CallNode
CallNode::CallNode(ExpressionNodePtr c, const std::vector<ExpressionNodePtr>& args, const std::string& file)
    : ExpressionNode(NodeType::CALL_EXPR, c->token, file), callee(c), arguments(args) {}

std::string CallNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Call\n";
    if (callee) oss << callee->toString(indent + 1) << "\n";
    oss << std::string(indent * 2 + 2, ' ') << "Arguments:\n";
    for (const auto& arg : arguments) {
        if (arg) oss << arg->toString(indent + 2) << "\n";
    }
    return oss.str();
}

// IfNode
IfNode::IfNode(const Token& tok, ExpressionNodePtr cond, StatementNodePtr thenBranch, 
               const std::vector<std::pair<ExpressionNodePtr, StatementNodePtr>>& elifs,
               StatementNodePtr elseBranch, const std::string& file)
    : StatementNode(NodeType::IF_STATEMENT, tok, file), condition(cond), thenBlock(thenBranch),
      elseifBlocks(elifs), elseBlock(elseBranch) {}

std::string IfNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "If\n";
    if (condition) oss << condition->toString(indent + 1) << "\n";
    if (thenBlock) oss << thenBlock->toString(indent + 1) << "\n";
    for (const auto& elif : elseifBlocks) {
        oss << std::string(indent * 2, ' ') << "ElseIf\n";
        if (elif.first) oss << elif.first->toString(indent + 1) << "\n";
        if (elif.second) oss << elif.second->toString(indent + 1) << "\n";
    }
    if (elseBlock) oss << std::string(indent * 2, ' ') << "Else\n" << elseBlock->toString(indent + 1);
    return oss.str();
}

// ForNode
ForNode::ForNode(const Token& tok, const std::string& var, ExpressionNodePtr iter, 
                 StatementNodePtr body, const std::string& file)
    : StatementNode(NodeType::FOR_LOOP, tok, file), variable(var), iterable(iter), body(body) {}

std::string ForNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "For[" << variable << "]\n";
    if (iterable) oss << iterable->toString(indent + 1) << "\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// ReturnNode
ReturnNode::ReturnNode(const Token& tok, ExpressionNodePtr val, const std::string& file)
    : StatementNode(NodeType::RETURN_STATEMENT, tok, file), value(val) {}

std::string ReturnNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Return";
    if (value) oss << "\n" << value->toString(indent + 1);
    return oss.str();
}

// BlockNode
BlockNode::BlockNode(const Token& tok, const std::vector<StatementNodePtr>& stmts, const std::string& file)
    : StatementNode(NodeType::BLOCK, tok, file), statements(stmts) {}

std::string BlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Block\n";
    for (const auto& stmt : statements) {
        if (stmt) oss << stmt->toString(indent + 1) << "\n";
    }
    return oss.str();
}

// VariableDeclNode
VariableDeclNode::VariableDeclNode(const Token& tok, const std::string& n, const std::string& t, 
                                   ExpressionNodePtr init, const std::string& file)
    : DeclarationNode(NodeType::VARIABLE_DECL, tok, n, t, file), initializer(init) {}

std::string VariableDeclNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "VarDecl[" << name << ": " << typeName << "]";
    if (initializer) oss << "\n" << initializer->toString(indent + 1);
    return oss.str();
}

// FunctionDeclNode
FunctionDeclNode::FunctionDeclNode(const Token& tok, const std::string& n, const std::string& t, 
                                   const std::vector<std::pair<std::string, std::string>>& params,
                                   StatementNodePtr b, const std::string& file)
    : DeclarationNode(NodeType::FUNCTION_DECL, tok, n, t, file), parameters(params), returnType(t), body(b) {}

std::string FunctionDeclNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "FunctionDecl[" << name << " -> " << returnType << "]\n";
    oss << std::string(indent * 2 + 2, ' ') << "Parameters:\n";
    for (const auto& param : parameters) {
        oss << std::string(indent * 2 + 4, ' ') << param.first << ": " << param.second << "\n";
    }
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// ProgramNode
std::string ProgramNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Program\n";
    for (const auto& decl : declarations) {
        if (decl) oss << decl->toString(indent + 1) << "\n";
    }
    for (const auto& stmt : statements) {
        if (stmt) oss << stmt->toString(indent + 1) << "\n";
    }
    return oss.str();
}

// IntentBlockNode
IntentBlockNode::IntentBlockNode(const Token& tok, const std::string& intentText, StatementNodePtr b, const std::string& file)
    : StatementNode(NodeType::INTENT_BLOCK, tok, file), intent(intentText), body(b) {}

std::string IntentBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "IntentBlock[\"" << intent << "\"]\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// GeneratedBlockNode
GeneratedBlockNode::GeneratedBlockNode(const Token& tok, StatementNodePtr b, const std::string& file)
    : StatementNode(NodeType::GENERATED_BLOCK, tok, file), body(b) {}

std::string GeneratedBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "GeneratedBlock\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// FixedBlockNode
FixedBlockNode::FixedBlockNode(const Token& tok, StatementNodePtr b, const std::string& file)
    : StatementNode(NodeType::FIXED_BLOCK, tok, file), body(b) {}

std::string FixedBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "FixedBlock\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

} // namespace proxima

#include "AST.h"
#include <sstream>
#include <iomanip>

namespace proxima {

// LiteralNode
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
std::string IdentifierNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Identifier[" << name << "]";
    return oss.str();
}

// BinaryOpNode
std::string BinaryOpNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "BinaryOp[" << Token::typeToString(opType) << "]\n";
    if (left) oss << left->toString(indent + 1) << "\n";
    if (right) oss << right->toString(indent + 1);
    return oss.str();
}

// CallNode
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
std::string IfNode::toString(int indent) const {
    std::ostringstream oss;
    if (isElseIf) {
        oss << std::string(indent * 2, ' ') << "ElseIf\n";
    } else {
        oss << std::string(indent * 2, ' ') << "If\n";
    }
    if (condition) oss << condition->toString(indent + 1) << "\n";
    if (thenBranch) oss << thenBranch->toString(indent + 1) << "\n";
    if (elseBranch) {
        // Check if elseBranch is an ElseIf node
        auto elseifNode = std::dynamic_pointer_cast<IfNode>(elseBranch);
        if (elseifNode && elseifNode->isElseIf) {
            oss << elseifNode->toString(indent);  // Same indent for elseif chain
        } else {
            oss << std::string(indent * 2, ' ') << "Else\n" << elseBranch->toString(indent + 1);
        }
    }
    return oss.str();
}

// ForNode
std::string ForNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "For[" << variable << "]\n";
    if (iterable) oss << iterable->toString(indent + 1) << "\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// ReturnNode
std::string ReturnNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Return";
    if (value) oss << "\n" << value->toString(indent + 1);
    return oss.str();
}

// BlockNode
std::string BlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Block\n";
    for (const auto& stmt : statements) {
        if (stmt) oss << stmt->toString(indent + 1) << "\n";
    }
    return oss.str();
}

// VariableDeclNode
std::string VariableDeclNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "VarDecl[" << name << ": " << typeName << "]";
    if (initializer) oss << "\n" << initializer->toString(indent + 1);
    return oss.str();
}

// FunctionDeclNode
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
std::string IntentBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "IntentBlock[\"" << intent << "\"]\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// GeneratedBlockNode
std::string GeneratedBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "GeneratedBlock\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// FixedBlockNode
std::string FixedBlockNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "FixedBlock\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// ClassDeclNode
std::string ClassDeclNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "ClassDecl[" << name << "]\n";
    oss << std::string(indent * 2 + 2, ' ') << "Parents:\n";
    for (const auto& parent : parentClasses) {
        oss << std::string(indent * 2 + 4, ' ') << parent << "\n";
    }
    oss << std::string(indent * 2 + 2, ' ') << "Members:\n";
    for (const auto& member : members) {
        oss << std::string(indent * 2 + 4, ' ') << member.first << ": " << member.second << "\n";
    }
    return oss.str();
}

// InterfaceDeclNode
std::string InterfaceDeclNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "InterfaceDecl[" << name << "]\n";
    oss << std::string(indent * 2 + 2, ' ') << "Parents:\n";
    for (const auto& parent : parentInterfaces) {
        oss << std::string(indent * 2 + 4, ' ') << parent << "\n";
    }
    oss << std::string(indent * 2 + 2, ' ') << "Methods:\n";
    for (const auto& method : methods) {
        oss << std::string(indent * 2 + 4, ' ') << method.first << ": " << method.second << "\n";
    }
    return oss.str();
}

// ExpressionStatementNode
std::string ExpressionStatementNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "ExpressionStatement\n";
    if (expression) oss << expression->toString(indent + 1);
    return oss.str();
}

// WhileNode
std::string WhileNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "While\n";
    if (condition) oss << condition->toString(indent + 1) << "\n";
    if (body) oss << body->toString(indent + 1);
    return oss.str();
}

// TernaryNode
std::string TernaryNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Ternary\n";
    if (condition) oss << condition->toString(indent + 1) << "\n";
    if (trueExpr) oss << trueExpr->toString(indent + 1) << "\n";
    if (falseExpr) oss << falseExpr->toString(indent + 1);
    return oss.str();
}

// UnaryOpNode
std::string UnaryOpNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "UnaryOp[" << op << "]\n";
    if (operand) oss << operand->toString(indent + 1);
    return oss.str();
}

// IndexNode
std::string IndexNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "Index\n";
    if (object) oss << object->toString(indent + 1) << "\n";
    oss << std::string(indent * 2 + 2, ' ') << "Indices:\n";
    for (size_t i = 0; i < indices.size(); ++i) {
        if (indices[i]) {
            bool isSlice = (i < isSliceIndex.size() && isSliceIndex[i]);
            oss << std::string(indent * 2 + 4, ' ');
            if (isSlice) oss << "[SLICE]";
            else oss << indices[i]->toString(0);
            oss << "\n";
        }
    }
    return oss.str();
}

// MemberAccessNode
std::string MemberAccessNode::toString(int indent) const {
    std::ostringstream oss;
    oss << std::string(indent * 2, ' ') << "MemberAccess[" << member << "]\n";
    if (object) oss << object->toString(indent + 1);
    return oss.str();
}

} // namespace proxima

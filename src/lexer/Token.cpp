#include "Token.h"
#include <sstream>

namespace proxima {

std::string Token::typeToString(TokenType type) {
    switch (type) {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::KEYWORD_IF: return "KEYWORD_IF";
        case TokenType::KEYWORD_END: return "KEYWORD_END";
        case TokenType::OP_PLUS: return "OP_PLUS";
        case TokenType::OP_ASSIGN: return "OP_ASSIGN";
        case TokenType::EOF_TOKEN: return "EOF";
        default: return "UNKNOWN";
    }
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(" << typeToString(type) 
        << ", \"" << value << "\""
        << ", line:" << line 
        << ", col:" << column << ")";
    return oss.str();
}

} // namespace proxima
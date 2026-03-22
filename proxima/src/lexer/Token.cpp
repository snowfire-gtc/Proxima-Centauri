#include "Token.h"
#include <sstream>
#include <unordered_map>

namespace proxima {

// ============================================================================
// Таблица соответствия типов токенов и строк
// ============================================================================

static const std::unordered_map<TokenType, std::string> typeToStringMap = {
    // Literals
    {TokenType::INTEGER, "INTEGER"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::STRING, "STRING"},
    {TokenType::CHAR, "CHAR"},
    {TokenType::BOOL, "BOOL"},

    // Identifiers
    {TokenType::IDENTIFIER, "IDENTIFIER"},

    // Keywords
    {TokenType::KEYWORD_IF, "KEYWORD_IF"},
    {TokenType::KEYWORD_ELSEIF, "KEYWORD_ELSEIF"},
    {TokenType::KEYWORD_ELSE, "KEYWORD_ELSE"},
    {TokenType::KEYWORD_END, "KEYWORD_END"},
    {TokenType::KEYWORD_FOR, "KEYWORD_FOR"},
    {TokenType::KEYWORD_IN, "KEYWORD_IN"},
    {TokenType::KEYWORD_WHILE, "KEYWORD_WHILE"},
    {TokenType::KEYWORD_DO, "KEYWORD_DO"},
    {TokenType::KEYWORD_SWITCH, "KEYWORD_SWITCH"},
    {TokenType::KEYWORD_CASE, "KEYWORD_CASE"},
    {TokenType::KEYWORD_DEFAULT, "KEYWORD_DEFAULT"},
    {TokenType::KEYWORD_RETURN, "KEYWORD_RETURN"},
    {TokenType::KEYWORD_CONTINUE, "KEYWORD_CONTINUE"},
    {TokenType::KEYWORD_BREAK, "KEYWORD_BREAK"},
    {TokenType::KEYWORD_CLASS, "KEYWORD_CLASS"},
    {TokenType::KEYWORD_INTERFACE, "KEYWORD_INTERFACE"},
    {TokenType::KEYWORD_TEMPLATE, "KEYWORD_TEMPLATE"},
    {TokenType::KEYWORD_PUBLIC, "KEYWORD_PUBLIC"},
    {TokenType::KEYWORD_PROTECTED, "KEYWORD_PROTECTED"},
    {TokenType::KEYWORD_PRIVATE, "KEYWORD_PRIVATE"},
    {TokenType::KEYWORD_CONSTRUCTOR, "KEYWORD_CONSTRUCTOR"},
    {TokenType::KEYWORD_DESTRUCTOR, "KEYWORD_DESTRUCTOR"},
    {TokenType::KEYWORD_NAMESPACE, "KEYWORD_NAMESPACE"},
    {TokenType::KEYWORD_USING, "KEYWORD_USING"},
    {TokenType::KEYWORD_INCLUDE, "KEYWORD_INCLUDE"},
    {TokenType::KEYWORD_EXCLUDE, "KEYWORD_EXCLUDE"},
    {TokenType::KEYWORD_DEFINE, "KEYWORD_DEFINE"},
    {TokenType::KEYWORD_IFDEF, "KEYWORD_IFDEF"},
    {TokenType::KEYWORD_ENDIF, "KEYWORD_ENDIF"},
    {TokenType::KEYWORD_AUTO, "KEYWORD_AUTO"},
    {TokenType::KEYWORD_VOID, "KEYWORD_VOID"},
    {TokenType::KEYWORD_TYPE, "KEYWORD_TYPE"},
    {TokenType::KEYWORD_ARGUMENTS, "KEYWORD_ARGUMENTS"},
    {TokenType::KEYWORD_PARALLEL, "KEYWORD_PARALLEL"},
    {TokenType::KEYWORD_TRUE, "KEYWORD_TRUE"},
    {TokenType::KEYWORD_FALSE, "KEYWORD_FALSE"},
    {TokenType::KEYWORD_NULL, "KEYWORD_NULL"},
    {TokenType::KEYWORD_NAN, "KEYWORD_NAN"},
    {TokenType::KEYWORD_INF, "KEYWORD_INF"},
    {TokenType::KEYWORD_PI, "KEYWORD_PI"},
    {TokenType::KEYWORD_REGION, "KEYWORD_REGION"},
    {TokenType::KEYWORD_ENDREGION, "KEYWORD_ENDREGION"},
    {TokenType::KEYWORD_SUITE, "KEYWORD_SUITE"},
    {TokenType::KEYWORD_TEST, "KEYWORD_TEST"},
    {TokenType::KEYWORD_ASSERT, "KEYWORD_ASSERT"},

    // Types
    {TokenType::TYPE_INT4, "TYPE_INT4"},
    {TokenType::TYPE_INT8, "TYPE_INT8"},
    {TokenType::TYPE_INT16, "TYPE_INT16"},
    {TokenType::TYPE_INT32, "TYPE_INT32"},
    {TokenType::TYPE_INT64, "TYPE_INT64"},
    {TokenType::TYPE_SINGLE, "TYPE_SINGLE"},
    {TokenType::TYPE_DOUBLE, "TYPE_DOUBLE"},
    {TokenType::TYPE_BOOL, "TYPE_BOOL"},
    {TokenType::TYPE_CHAR, "TYPE_CHAR"},
    {TokenType::TYPE_STRING, "TYPE_STRING"},
    {TokenType::TYPE_VECTOR, "TYPE_VECTOR"},
    {TokenType::TYPE_MATRIX, "TYPE_MATRIX"},
    {TokenType::TYPE_LAYER, "TYPE_LAYER"},
    {TokenType::TYPE_COLLECTION, "TYPE_COLLECTION"},
    {TokenType::TYPE_POINT2, "TYPE_POINT2"},
    {TokenType::TYPE_POINT3, "TYPE_POINT3"},
    {TokenType::TYPE_POINT4, "TYPE_POINT4"},
    {TokenType::TYPE_TIME, "TYPE_TIME"},
    {TokenType::TYPE_FILE, "TYPE_FILE"},
    {TokenType::TYPE_RTTI, "TYPE_RTTI"},
    {TokenType::TYPE_METHOD, "TYPE_METHOD"},

    // Operators
    {TokenType::OP_PLUS, "OP_PLUS"},
    {TokenType::OP_MINUS, "OP_MINUS"},
    {TokenType::OP_STAR, "OP_STAR"},
    {TokenType::OP_SLASH, "OP_SLASH"},
    {TokenType::OP_BACKSLASH, "OP_BACKSLASH"},
    {TokenType::OP_PERCENT, "OP_PERCENT"},
    {TokenType::OP_CARET, "OP_CARET"},
    {TokenType::OP_DOT, "OP_DOT"},
    {TokenType::OP_COMMA, "OP_COMMA"},
    {TokenType::OP_SEMICOLON, "OP_SEMICOLON"},
    {TokenType::OP_COLON, "OP_COLON"},
    {TokenType::OP_ASSIGN, "OP_ASSIGN"},
    {TokenType::OP_EQ, "OP_EQ"},
    {TokenType::OP_NEQ, "OP_NEQ"},
    {TokenType::OP_TYPE_EQ, "OP_TYPE_EQ"},
    {TokenType::OP_TYPE_NEQ, "OP_TYPE_NEQ"},
    {TokenType::OP_LT, "OP_LT"},
    {TokenType::OP_GT, "OP_GT"},
    {TokenType::OP_LTE, "OP_LTE"},
    {TokenType::OP_GTE, "OP_GTE"},
    {TokenType::OP_AND, "OP_AND"},
    {TokenType::OP_OR, "OP_OR"},
    {TokenType::OP_XOR, "OP_XOR"},
    {TokenType::OP_NOT, "OP_NOT"},
    {TokenType::OP_BIN_AND, "OP_BIN_AND"},
    {TokenType::OP_BIN_OR, "OP_BIN_OR"},
    {TokenType::OP_BIN_XOR, "OP_BIN_XOR"},
    {TokenType::OP_BIN_NOT, "OP_BIN_NOT"},
    {TokenType::OP_SHIFT_LEFT, "OP_SHIFT_LEFT"},
    {TokenType::OP_SHIFT_RIGHT, "OP_SHIFT_RIGHT"},
    {TokenType::OP_DOT_PLUS, "OP_DOT_PLUS"},
    {TokenType::OP_DOT_MINUS, "OP_DOT_MINUS"},
    {TokenType::OP_DOT_STAR, "OP_DOT_STAR"},
    {TokenType::OP_DOT_SLASH, "OP_DOT_SLASH"},
    {TokenType::OP_DOT_CARET, "OP_DOT_CARET"},
    {TokenType::OP_DOT_EQ, "OP_DOT_EQ"},
    {TokenType::OP_DOT_LT, "OP_DOT_LT"},
    {TokenType::OP_DOT_GT, "OP_DOT_GT"},
    {TokenType::OP_DOT_LTE, "OP_DOT_LTE"},
    {TokenType::OP_DOT_GTE, "OP_DOT_GTE"},
    {TokenType::OP_TRANSPOSE, "OP_TRANSPOSE"},
    {TokenType::OP_TERNARY, "OP_TERNARY"},
    {TokenType::OP_AMPERSAND, "OP_AMPERSAND"},

    // Delimiters
    {TokenType::DELIM_LPAREN, "DELIM_LPAREN"},
    {TokenType::DELIM_RPAREN, "DELIM_RPAREN"},
    {TokenType::DELIM_LBRACKET, "DELIM_LBRACKET"},
    {TokenType::DELIM_RBRACKET, "DELIM_RBRACKET"},
    {TokenType::DELIM_LBRACE, "DELIM_LBRACE"},
    {TokenType::DELIM_RBRACE, "DELIM_RBRACE"},

    // Special
    {TokenType::NEWLINE, "NEWLINE"},
    {TokenType::COMMENT_SINGLE, "COMMENT_SINGLE"},
    {TokenType::COMMENT_MULTI, "COMMENT_MULTI"},
    {TokenType::EOF_TOKEN, "EOF_TOKEN"},
    {TokenType::UNKNOWN, "UNKNOWN"},
};

// ============================================================================
// Методы класса Token
// ============================================================================

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token(" << typeToString(type) << ", \"" << value << "\", "
        << line << ":" << column << ", " << length << ")";
    return oss.str();
}

std::string Token::typeToString(TokenType type) {
    auto it = typeToStringMap.find(type);
    if (it != typeToStringMap.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

} // namespace proxima

#ifndef PROXIMA_TOKEN_H
#define PROXIMA_TOKEN_H

#include <string>
#include <variant>
#include <memory>

namespace proxima {

enum class TokenType {
    // Literals
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    BOOL,
    
    // Identifiers
    IDENTIFIER,
    
    // Keywords
    KEYWORD_IF,
    KEYWORD_ELSEIF,
    KEYWORD_ELSE,
    KEYWORD_END,
    KEYWORD_FOR,
    KEYWORD_IN,
    KEYWORD_WHILE,
    KEYWORD_DO,
    KEYWORD_SWITCH,
    KEYWORD_CASE,
    KEYWORD_DEFAULT,
    KEYWORD_RETURN,
    KEYWORD_CONTINUE,
    KEYWORD_BREAK,
    KEYWORD_CLASS,
    KEYWORD_INTERFACE,
    KEYWORD_TEMPLATE,
    KEYWORD_PUBLIC,
    KEYWORD_PROTECTED,
    KEYWORD_PRIVATE,
    KEYWORD_CONSTRUCTOR,
    KEYWORD_DESTRUCTOR,
    KEYWORD_NAMESPACE,
    KEYWORD_USING,
    KEYWORD_INCLUDE,
    KEYWORD_EXCLUDE,
    KEYWORD_DEFINE,
    KEYWORD_IFDEF,
    KEYWORD_ENDIF,
    KEYWORD_AUTO,
    KEYWORD_VOID,
    KEYWORD_TYPE,
    KEYWORD_ARGUMENTS,
    KEYWORD_PARALLEL,
    KEYWORD_TRUE,
    KEYWORD_FALSE,
    KEYWORD_NULL,
    KEYWORD_NAN,
    KEYWORD_INF,
    KEYWORD_PI,
    KEYWORD_REGION,
    KEYWORD_ENDREGION,
    KEYWORD_SUITE,
    KEYWORD_TEST,
    KEYWORD_ASSERT,
    
    // Types
    TYPE_INT4,
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_SINGLE,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_VECTOR,
    TYPE_MATRIX,
    TYPE_LAYER,
    TYPE_COLLECTION,
    TYPE_POINT2,
    TYPE_POINT3,
    TYPE_POINT4,
    TYPE_TIME,
    TYPE_FILE,
    TYPE_RTTI,
    TYPE_METHOD,
    
    // Operators
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,
    OP_BACKSLASH,
    OP_PERCENT,
    OP_CARET,
    OP_DOT,
    OP_COMMA,
    OP_SEMICOLON,
    OP_COLON,
    OP_ASSIGN,
    OP_EQ,
    OP_NEQ,
    OP_STRICT_EQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOT,
    OP_BIN_AND,
    OP_BIN_OR,
    OP_BIN_XOR,
    OP_BIN_NOT,
    OP_SHIFT_LEFT,
    OP_SHIFT_RIGHT,
    OP_DOT_PLUS,
    OP_DOT_MINUS,
    OP_DOT_STAR,
    OP_DOT_SLASH,
    OP_DOT_CARET,
    OP_DOT_EQ,
    OP_DOT_LT,
    OP_DOT_GT,
    OP_DOT_LTE,
    OP_DOT_GTE,
    OP_TRANSPOSE,
    OP_TERNARY,
    OP_AMPERSAND,
    
    // Delimiters
    DELIM_LPAREN,
    DELIM_RPAREN,
    DELIM_LBRACKET,
    DELIM_RBRACKET,
    DELIM_LBRACE,
    DELIM_RBRACE,
    
    // Special
    NEWLINE,
    COMMENT_SINGLE,
    COMMENT_MULTI,
    EOF_TOKEN,
    UNKNOWN
};

class Token {
public:
    TokenType type;
    std::string value;
    int line;
    int column;
    int length;
    
    Token(TokenType t = TokenType::UNKNOWN, 
          const std::string& v = "", 
          int l = 0, 
          int c = 0,
          int len = 0)
        : type(t), value(v), line(l), column(c), length(len) {}
    
    std::string toString() const;
    static std::string typeToString(TokenType type);
};

} // namespace proxima

#endif // PROXIMA_TOKEN_H
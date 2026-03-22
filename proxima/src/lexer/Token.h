#ifndef PROXIMA_TOKEN_H
#define PROXIMA_TOKEN_H

#include <string>
#include <variant>
#include <memory>

namespace proxima {

/**
 * @brief Типы токенов языка Proxima
 * 
 * Согласно спецификации language.txt:
 * - Пункт 34: Логические операции (!, !=, ==, ===, &, |, %, etc.)
 * - Пункт 35: Бинарные операции (!!, &&, ||, %%, <<, >>)
 * - Пункт 36: Дополнительные операции (%, ^, .^, \, ?:)
 */
enum class TokenType {
    // Literals
    INTEGER,            // Целочисленный литерал
    FLOAT,              // Число с плавающей точкой
    STRING,             // Строковый литерал
    CHAR,               // Символьный литерал
    BOOL,               // Булевый литерал
    
    // Identifiers
    IDENTIFIER,         // Идентификатор
    
    // Keywords
    KEYWORD_IF,         // if
    KEYWORD_ELSEIF,     // elseif
    KEYWORD_ELSE,       // else
    KEYWORD_END,        // end
    KEYWORD_FOR,        // for
    KEYWORD_IN,         // in
    KEYWORD_WHILE,      // while
    KEYWORD_DO,         // do
    KEYWORD_SWITCH,     // switch
    KEYWORD_CASE,       // case
    KEYWORD_DEFAULT,    // default
    KEYWORD_RETURN,     // return
    KEYWORD_CONTINUE,   // continue
    KEYWORD_BREAK,      // break
    KEYWORD_CLASS,      // class
    KEYWORD_INTERFACE,  // interface
    KEYWORD_TEMPLATE,   // template
    KEYWORD_PUBLIC,     // public
    KEYWORD_PROTECTED,  // protected
    KEYWORD_PRIVATE,    // private
    KEYWORD_CONSTRUCTOR, // constructor
    KEYWORD_DESTRUCTOR, // destructor
    KEYWORD_NAMESPACE,  // namespace
    KEYWORD_USING,      // using
    KEYWORD_INCLUDE,    // include
    KEYWORD_EXCLUDE,    // exclude
    KEYWORD_DEFINE,     // define
    KEYWORD_IFDEF,      // ifdef
    KEYWORD_ENDIF,      // endif
    KEYWORD_AUTO,       // auto
    KEYWORD_VOID,       // void
    KEYWORD_TYPE,       // type
    KEYWORD_ARGUMENTS,  // arguments
    KEYWORD_PARALLEL,   // parallel
    KEYWORD_TRUE,       // true
    KEYWORD_FALSE,      // false
    KEYWORD_NULL,       // null
    KEYWORD_NAN,        // nan
    KEYWORD_INF,        // inf
    KEYWORD_PI,         // pi
    KEYWORD_REGION,     // region
    KEYWORD_ENDREGION,  // endregion
    KEYWORD_SUITE,      // suite
    KEYWORD_TEST,       // test
    KEYWORD_ASSERT,     // assert
    
    // Types
    TYPE_INT4,          // int4
    TYPE_INT8,          // int8
    TYPE_INT16,         // int16
    TYPE_INT32,         // int32
    TYPE_INT64,         // int64
    TYPE_SINGLE,        // single
    TYPE_DOUBLE,        // double
    TYPE_BOOL,          // bool
    TYPE_CHAR,          // char
    TYPE_STRING,        // string
    TYPE_VECTOR,        // vector
    TYPE_MATRIX,        // matrix
    TYPE_LAYER,         // layer
    TYPE_COLLECTION,    // collection
    TYPE_POINT2,        // point2
    TYPE_POINT3,        // point3
    TYPE_POINT4,        // point4
    TYPE_TIME,          // time
    TYPE_FILE,          // file
    TYPE_RTTI,          // rtti
    TYPE_METHOD,        // method
    
    // Operators (все операторы с префиксом OP_)
    OP_PLUS,            // +
    OP_MINUS,           // -
    OP_STAR,            // *
    OP_SLASH,           // /
    OP_BACKSLASH,       // \ (решение линейных уравнений)
    OP_PERCENT,         // %
    OP_CARET,           // ^
    OP_DOT,             // . (доступ к полям)
    OP_COMMA,           // ,
    OP_SEMICOLON,       // ;
    OP_COLON,           // :
    OP_ASSIGN,          // =
    OP_EQ,              // == (равенство значений)
    OP_NEQ,             // != (неравенство значений)
    OP_TYPE_EQ,         // === (проверка равенства ТОЛЬКО типов)
    OP_TYPE_NEQ,        // !== (проверка неравенства ТОЛЬКО типов)
    OP_LT,              // <
    OP_GT,              // >
    OP_LTE,             // <=
    OP_GTE,             // >=
    OP_AND,             // &
    OP_OR,              // |
    OP_XOR,             // %
    OP_NOT,             // !
    OP_BIN_AND,         // &&
    OP_BIN_OR,          // ||
    OP_BIN_XOR,         // %%
    OP_BIN_NOT,         // !!
    OP_SHIFT_LEFT,      // <<
    OP_SHIFT_RIGHT,     // >>
    OP_DOT_PLUS,        // .+
    OP_DOT_MINUS,       // .-
    OP_DOT_STAR,        // .*
    OP_DOT_SLASH,       // ./
    OP_DOT_CARET,       // .^
    OP_DOT_EQ,          // .=
    OP_DOT_LT,          // .<
    OP_DOT_GT,          // .>
    OP_DOT_LTE,         // .<=
    OP_DOT_GTE,         // .>=
    OP_TRANSPOSE,       // ' (апостроф)
    OP_TERNARY,         // ?:
    OP_AMPERSAND,       // @
    
    // Delimiters
    DELIM_LPAREN,       // (
    DELIM_RPAREN,       // )
    DELIM_LBRACKET,     // [
    DELIM_RBRACKET,     // ]
    DELIM_LBRACE,       // {
    DELIM_RBRACE,       // }
    
    // Special
    NEWLINE,            // Перевод строки
    COMMENT_SINGLE,     // //
    COMMENT_MULTI,      // /* */
    EOF_TOKEN,          // Конец файла
    UNKNOWN             // Неизвестный токен
};

/**
 * @brief Класс токена
 */
class Token {
public:
    TokenType type;
    std::string value;
    int line;
    int column;
    int length;
    
    Token() : type(TokenType::UNKNOWN), line(0), column(0), length(0) {}
    
    Token(TokenType type, const std::string& value, int line, int column, int length = 0)
        : type(type), value(value), line(line), column(column), length(length) {}
    
    std::string toString() const;
    static std::string typeToString(TokenType type);
};

} // namespace proxima

#endif // PROXIMA_TOKEN_H

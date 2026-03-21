#include "Lexer.h"
#include <cctype>
#include <stdexcept>
#include <iostream>

namespace proxima {

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source(source), filename(filename), pos(0), line(1), column(1) {
    initKeywords();
}

void Lexer::initKeywords() {
    keywords["if"] = TokenType::KEYWORD_IF;
    keywords["elseif"] = TokenType::KEYWORD_ELSEIF;
    keywords["else"] = TokenType::KEYWORD_ELSE;
    keywords["end"] = TokenType::KEYWORD_END;
    keywords["for"] = TokenType::KEYWORD_FOR;
    keywords["in"] = TokenType::KEYWORD_IN;
    keywords["while"] = TokenType::KEYWORD_WHILE;
    keywords["do"] = TokenType::KEYWORD_DO;
    keywords["switch"] = TokenType::KEYWORD_SWITCH;
    keywords["case"] = TokenType::KEYWORD_CASE;
    keywords["default"] = TokenType::KEYWORD_DEFAULT;
    keywords["return"] = TokenType::KEYWORD_RETURN;
    keywords["continue"] = TokenType::KEYWORD_CONTINUE;
    keywords["break"] = TokenType::KEYWORD_BREAK;
    keywords["class"] = TokenType::KEYWORD_CLASS;
    keywords["interface"] = TokenType::KEYWORD_INTERFACE;
    keywords["template"] = TokenType::KEYWORD_TEMPLATE;
    keywords["public"] = TokenType::KEYWORD_PUBLIC;
    keywords["protected"] = TokenType::KEYWORD_PROTECTED;
    keywords["private"] = TokenType::KEYWORD_PRIVATE;
    keywords["constructor"] = TokenType::KEYWORD_CONSTRUCTOR;
    keywords["destructor"] = TokenType::KEYWORD_DESTRUCTOR;
    keywords["namespace"] = TokenType::KEYWORD_NAMESPACE;
    keywords["using"] = TokenType::KEYWORD_USING;
    keywords["include"] = TokenType::KEYWORD_INCLUDE;
    keywords["exclude"] = TokenType::KEYWORD_EXCLUDE;
    keywords["define"] = TokenType::KEYWORD_DEFINE;
    keywords["ifdef"] = TokenType::KEYWORD_IFDEF;
    keywords["endif"] = TokenType::KEYWORD_ENDIF;
    keywords["auto"] = TokenType::KEYWORD_AUTO;
    keywords["void"] = TokenType::KEYWORD_VOID;
    keywords["type"] = TokenType::KEYWORD_TYPE;
    keywords["arguments"] = TokenType::KEYWORD_ARGUMENTS;
    keywords["parallel"] = TokenType::KEYWORD_PARALLEL;
    keywords["true"] = TokenType::KEYWORD_TRUE;
    keywords["false"] = TokenType::KEYWORD_FALSE;
    keywords["null"] = TokenType::KEYWORD_NULL;
    keywords["nan"] = TokenType::KEYWORD_NAN;
    keywords["inf"] = TokenType::KEYWORD_INF;
    keywords["pi"] = TokenType::KEYWORD_PI;
    keywords["region"] = TokenType::KEYWORD_REGION;
    keywords["endregion"] = TokenType::KEYWORD_ENDREGION;
    keywords["suite"] = TokenType::KEYWORD_SUITE;
    keywords["test"] = TokenType::KEYWORD_TEST;
    keywords["assert"] = TokenType::KEYWORD_ASSERT;
    
    keywords["int4"] = TokenType::TYPE_INT4;
    keywords["int8"] = TokenType::TYPE_INT8;
    keywords["int16"] = TokenType::TYPE_INT16;
    keywords["int32"] = TokenType::TYPE_INT32;
    keywords["int64"] = TokenType::TYPE_INT64;
    keywords["single"] = TokenType::TYPE_SINGLE;
    keywords["double"] = TokenType::TYPE_DOUBLE;
    keywords["bool"] = TokenType::TYPE_BOOL;
    keywords["char"] = TokenType::TYPE_CHAR;
    keywords["string"] = TokenType::TYPE_STRING;
    keywords["vector"] = TokenType::TYPE_VECTOR;
    keywords["matrix"] = TokenType::TYPE_MATRIX;
    keywords["layer"] = TokenType::TYPE_LAYER;
    keywords["collection"] = TokenType::TYPE_COLLECTION;
    keywords["point2"] = TokenType::TYPE_POINT2;
    keywords["point3"] = TokenType::TYPE_POINT3;
    keywords["point4"] = TokenType::TYPE_POINT4;
    keywords["time"] = TokenType::TYPE_TIME;
    keywords["file"] = TokenType::TYPE_FILE;
    keywords["rtti"] = TokenType::TYPE_RTTI;
    keywords["method"] = TokenType::TYPE_METHOD;
}

char Lexer::currentChar() const {
    if (pos >= source.length()) return '\0';
    return source[pos];
}

char Lexer::peekChar() const {
    if (pos + 1 >= source.length()) return '\0';
    return source[pos + 1];
}

void Lexer::advance() {
    if (currentChar() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    pos++;
}

void Lexer::skipWhitespace() {
    while (std::isspace(currentChar())) {
        advance();
    }
}

void Lexer::skipComment() {
    if (currentChar() == '/' && peekChar() == '/') {
        while (currentChar() != '\n' && currentChar() != '\0') {
            advance();
        }
    } else if (currentChar() == '/' && peekChar() == '*') {
        advance(); advance();
        while (currentChar() != '\0') {
            if (currentChar() == '*' && peekChar() == '/') {
                advance(); advance();
                break;
            }
            advance();
        }
    }
}

Token Lexer::readNumber() {
    startLine = line;
    startColumn = column;
    std::string value;
    bool isFloat = false;
    
    while (std::isdigit(currentChar())) {
        value += currentChar();
        advance();
    }
    
    if (currentChar() == '.' && std::isdigit(peekChar())) {
        isFloat = true;
        value += currentChar();
        advance();
        while (std::isdigit(currentChar())) {
            value += currentChar();
            advance();
        }
    }
    
    return makeToken(isFloat ? TokenType::FLOAT : TokenType::INTEGER, value);
}

Token Lexer::readIdentifier() {
    startLine = line;
    startColumn = column;
    std::string value;
    
    while (std::isalnum(currentChar()) || currentChar() == '_') {
        value += currentChar();
        advance();
    }
    
    auto it = keywords.find(value);
    if (it != keywords.end()) {
        return makeToken(it->second, value);
    }
    
    return makeToken(TokenType::IDENTIFIER, value);
}

Token Lexer::readString() {
    startLine = line;
    startColumn = column;
    advance();
    std::string value;
    
    while (currentChar() != '"' && currentChar() != '\0') {
        if (currentChar() == '\\' && peekChar() != '\0') {
            advance();
            switch (currentChar()) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                default: value += currentChar();
            }
        } else {
            value += currentChar();
        }
        advance();
    }
    
    if (currentChar() == '"') advance();
    
    return makeToken(TokenType::STRING, value);
}

Token Lexer::readChar() {
    startLine = line;
    startColumn = column;
    advance();
    std::string value;
    
    if (currentChar() == '\\' && peekChar() != '\0') {
        advance();
        switch (currentChar()) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            default: value = currentChar();
        }
        advance();
    } else if (currentChar() != '\'') {
        value = currentChar();
        advance();
    }
    
    if (currentChar() == '\'') advance();
    
    return makeToken(TokenType::CHAR, value);
}

Token Lexer::readOperator() {
    startLine = line;
    startColumn = column;
    char c = currentChar();
    char next = peekChar();
    std::string value(1, c);
    
    advance();
    
    switch (c) {
        case '+':
            if (next == '.') { advance(); value += '.'; return makeToken(TokenType::OP_DOT_PLUS, value); }
            return makeToken(TokenType::OP_PLUS, value);
        case '-':
            if (next == '>') { advance(); value += '>'; return makeToken(TokenType::OP_ASSIGN, value); }
            return makeToken(TokenType::OP_MINUS, value);
        case '*':
            if (next == '.') { advance(); value += '.'; return makeToken(TokenType::OP_DOT_STAR, value); }
            return makeToken(TokenType::OP_STAR, value);
        case '/':
            if (next == '.') { advance(); value += '.'; return makeToken(TokenType::OP_DOT_SLASH, value); }
            return makeToken(TokenType::OP_SLASH, value);
        case '\\':
            return makeToken(TokenType::OP_BACKSLASH, value);
        case '%':
            if (next == '%') { advance(); value += '%'; return makeToken(TokenType::OP_BIN_XOR, value); }
            if (next == '.') { advance(); value += '.'; return makeToken(TokenType::OP_DOT_EQ, value); }
            return makeToken(TokenType::OP_PERCENT, value);
        case '^':
            if (next == '.') { advance(); value += '.'; return makeToken(TokenType::OP_DOT_CARET, value); }
            return makeToken(TokenType::OP_CARET, value);
        case '.':
            if (next == '=') { advance(); value += '='; return makeToken(TokenType::OP_DOT_EQ, value); }
            if (next == '<') { advance(); value += '<'; return makeToken(TokenType::OP_DOT_LT, value); }
            if (next == '>') { advance(); value += '>'; return makeToken(TokenType::OP_DOT_GT, value); }
            if (next == '&') { advance(); value += '&'; return makeToken(TokenType::OP_DOT_AND, value); }
            if (next == '|') { advance(); value += '|'; return makeToken(TokenType::OP_DOT_OR, value); }
            return makeToken(TokenType::OP_DOT, value);
        case ',':
            return makeToken(TokenType::OP_COMMA, value);
        case ';':
            return makeToken(TokenType::OP_SEMICOLON, value);
        case ':':
            return makeToken(TokenType::OP_COLON, value);
        case '=':
            if (next == '=') {
                advance();
                if (peekChar() == '=') { advance(); value += '='; return makeToken(TokenType::OP_STRICT_EQ, value); }
                value += '='; return makeToken(TokenType::OP_EQ, value);
            }
            return makeToken(TokenType::OP_ASSIGN, value);
        case '<':
            if (next == '=') { advance(); value += '='; return makeToken(TokenType::OP_LTE, value); }
            if (next == '<') { advance(); value += '<'; return makeToken(TokenType::OP_SHIFT_LEFT, value); }
            return makeToken(TokenType::OP_LT, value);
        case '>':
            if (next == '=') { advance(); value += '='; return makeToken(TokenType::OP_GTE, value); }
            if (next == '>') { advance(); value += '>'; return makeToken(TokenType::OP_SHIFT_RIGHT, value); }
            return makeToken(TokenType::OP_GT, value);
        case '!':
            if (next == '=') { advance(); value += '='; return makeToken(TokenType::OP_NEQ, value); }
            if (next == '!') { advance(); value += '!'; return makeToken(TokenType::OP_BIN_NOT, value); }
            return makeToken(TokenType::OP_NOT, value);
        case '&':
            if (next == '&') { advance(); value += '&'; return makeToken(TokenType::OP_BIN_AND, value); }
            return makeToken(TokenType::OP_AND, value);
        case '|':
            if (next == '|') { advance(); value += '|'; return makeToken(TokenType::OP_BIN_OR, value); }
            return makeToken(TokenType::OP_OR, value);
        case '?':
            return makeToken(TokenType::OP_TERNARY, value);
        case '\'':
            return makeToken(TokenType::OP_TRANSPOSE, value);
        case '(':
            return makeToken(TokenType::DELIM_LPAREN, value);
        case ')':
            return makeToken(TokenType::DELIM_RPAREN, value);
        case '[':
            return makeToken(TokenType::DELIM_LBRACKET, value);
        case ']':
            return makeToken(TokenType::DELIM_RBRACKET, value);
        case '{':
            return makeToken(TokenType::DELIM_LBRACE, value);
        case '}':
            return makeToken(TokenType::DELIM_RBRACE, value);
        default:
            return makeToken(TokenType::UNKNOWN, value);
    }
}

Token Lexer::makeToken(TokenType type, const std::string& value) {
    return Token(type, value, startLine, startColumn, static_cast<int>(value.length()));
}

Token Lexer::nextToken() {
    skipWhitespace();
    skipComment();
    skipWhitespace();
    
    startLine = line;
    startColumn = column;
    
    if (currentChar() == '\0') {
        return makeToken(TokenType::EOF_TOKEN, "");
    }
    
    if (std::isdigit(currentChar())) {
        return readNumber();
    }
    
    if (std::isalpha(currentChar()) || currentChar() == '_') {
        return readIdentifier();
    }
    
    if (currentChar() == '"') {
        return readString();
    }
    
    if (currentChar() == '\'') {
        return readChar();
    }
    
    return readOperator();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    
    return tokens;
}

} // namespace proxima
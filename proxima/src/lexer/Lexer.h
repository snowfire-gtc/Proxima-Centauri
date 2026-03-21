#ifndef PROXIMA_LEXER_H
#define PROXIMA_LEXER_H

#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace proxima {

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename = "<input>");
    
    std::vector<Token> tokenize();
    Token nextToken();
    
    int getCurrentLine() const { return line; }
    int getCurrentColumn() const { return column; }
    std::string getFilename() const { return filename; }
    
private:
    std::string source;
    std::string filename;
    size_t pos;
    int line;
    int column;
    int startLine;
    int startColumn;
    
    std::unordered_map<std::string, TokenType> keywords;
    
    void initKeywords();
    char currentChar() const;
    char peekChar() const;
    void advance();
    void skipWhitespace();
    void skipComment();
    
    Token readNumber();
    Token readIdentifier();
    Token readString();
    Token readChar();
    Token readOperator();
    
    Token makeToken(TokenType type, const std::string& value);
};

} // namespace proxima

#endif // PROXIMA_LEXER_H
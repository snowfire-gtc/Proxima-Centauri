#ifndef PROXIMA_PARSER_H
#define PROXIMA_PARSER_H

#include "../lexer/Lexer.h"
#include "AST.h"
#include <vector>
#include <memory>

namespace proxima {

class Parser {
public:
    Parser(const std::vector<Token>& tokens, const std::string& filename = "<input>");
    
    ProgramNodePtr parse();
    
private:
    std::vector<Token> tokens;
    size_t pos;
    std::string filename;
    
    Token currentToken() const;
    Token peekToken() const;
    void advance();
    bool match(TokenType type);
    bool check(TokenType type) const;
    Token expect(TokenType type, const std::string& message);
    
    ProgramNodePtr parseProgram();
    DeclarationNodePtr parseDeclaration();
    StatementNodePtr parseStatement();
    ExpressionNodePtr parseExpression();
    ExpressionNodePtr parseAssignment();
    ExpressionNodePtr parseTernary();
    ExpressionNodePtr parseOr();
    ExpressionNodePtr parseAnd();
    ExpressionNodePtr parseEquality();
    ExpressionNodePtr parseComparison();
    ExpressionNodePtr parseTerm();
    ExpressionNodePtr parseFactor();
    ExpressionNodePtr parseUnary();
    ExpressionNodePtr parseCall();
    ExpressionNodePtr parsePrimary();
    
    StatementNodePtr parseIf();
    StatementNodePtr parseFor();
    StatementNodePtr parseWhile();
    StatementNodePtr parseReturn();
    StatementNodePtr parseBlock();
    
    std::vector<ExpressionNodePtr> parseArguments();
};

} // namespace proxima

#endif // PROXIMA_PARSER_H
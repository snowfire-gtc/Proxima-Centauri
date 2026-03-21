#include "Parser.h"
#include <stdexcept>
#include <iostream>

namespace proxima {

Parser::Parser(const std::vector<Token>& tokens, const std::string& filename)
    : tokens(tokens), pos(0), filename(filename) {}

Token Parser::currentToken() const {
    if (pos >= tokens.size()) return Token(TokenType::EOF_TOKEN);
    return tokens[pos];
}

Token Parser::peekToken() const {
    if (pos + 1 >= tokens.size()) return Token(TokenType::EOF_TOKEN);
    return tokens[pos + 1];
}

void Parser::advance() {
    if (pos < tokens.size()) pos++;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return currentToken().type == type;
}

Token Parser::expect(TokenType type, const std::string& message) {
    if (!check(type)) {
        throw std::runtime_error(message + " at line " + std::to_string(currentToken().line));
    }
    Token tok = currentToken();
    advance();
    return tok;
}

ProgramNodePtr Parser::parseProgram() {
    auto program = std::make_shared<ProgramNode>(filename);
    
    while (!check(TokenType::EOF_TOKEN)) {
        if (check(TokenType::KEYWORD_CLASS) || 
            check(TokenType::KEYWORD_INTERFACE) ||
            check(TokenType::KEYWORD_TEMPLATE) ||
            check(TokenType::KEYWORD_NAMESPACE)) {
            program->declarations.push_back(parseDeclaration());
        } else if (check(TokenType::KEYWORD_FUNCTION) || 
                   check(TokenType::IDENTIFIER)) {
            auto decl = parseDeclaration();
            if (decl) program->declarations.push_back(decl);
        } else {
            auto stmt = parseStatement();
            if (stmt) program->statements.push_back(stmt);
        }
    }
    
    return program;
}

DeclarationNodePtr Parser::parseDeclaration() {
    Token tok = currentToken();
    
    if (match(TokenType::KEYWORD_CLASS)) {
        // Parse class declaration
        return nullptr; // TODO: Implement
    }
    
    if (match(TokenType::KEYWORD_INTERFACE)) {
        // Parse interface declaration
        return nullptr; // TODO: Implement
    }
    
    // Variable or function declaration
    if (check(TokenType::IDENTIFIER)) {
        std::string name = currentToken().value;
        advance();
        
        std::string typeName = "auto";
        if (match(TokenType::OP_COLON)) {
            typeName = currentToken().value;
            advance();
        }
        
        if (match(TokenType::DELIM_LPAREN)) {
            // Function declaration
            std::vector<std::pair<std::string, std::string>> params;
            while (!check(TokenType::DELIM_RPAREN) && !check(TokenType::EOF_TOKEN)) {
                std::string paramName = currentToken().value;
                advance();
                std::string paramType = "auto";
                if (match(TokenType::OP_COLON)) {
                    paramType = currentToken().value;
                    advance();
                }
                params.push_back({paramName, paramType});
                if (!match(TokenType::OP_COMMA)) break;
            }
            expect(TokenType::DELIM_RPAREN, "Expected ')'");
            
            std::string returnType = "void";
            if (match(TokenType::OP_COLON)) {
                returnType = currentToken().value;
                advance();
            }
            
            auto body = parseBlock();
            return std::make_shared<FunctionDeclNode>(tok, name, returnType, params, body, filename);
        } else {
            // Variable declaration
            ExpressionNodePtr initializer = nullptr;
            if (match(TokenType::OP_ASSIGN)) {
                initializer = parseExpression();
            }
            return std::make_shared<VariableDeclNode>(tok, name, typeName, initializer, filename);
        }
    }
    
    return nullptr;
}

StatementNodePtr Parser::parseStatement() {
    Token tok = currentToken();
    
    if (match(TokenType::KEYWORD_IF)) {
        return parseIf();
    }
    
    if (match(TokenType::KEYWORD_FOR)) {
        return parseFor();
    }
    
    if (match(TokenType::KEYWORD_WHILE)) {
        return parseWhile();
    }
    
    if (match(TokenType::KEYWORD_RETURN)) {
        return parseReturn();
    }
    
    if (match(TokenType::DELIM_LBRACE)) {
        return parseBlock();
    }
    
    // Expression statement or assignment
    auto expr = parseExpression();
    if (match(TokenType::OP_SEMICOLON) || match(TokenType::NEWLINE)) {
        return std::make_shared<ExpressionStatementNode>(tok, expr, filename);
    }
    
    return nullptr;
}

StatementNodePtr Parser::parseIf() {
    Token tok = currentToken();
    auto condition = parseExpression();
    auto thenBranch = parseBlock();
    StatementNodePtr elseBranch = nullptr;
    
    if (match(TokenType::KEYWORD_ELSE)) {
        elseBranch = parseBlock();
    }
    
    expect(TokenType::KEYWORD_END, "Expected 'end' for if statement");
    
    return std::make_shared<IfNode>(tok, condition, thenBranch, elseBranch, filename);
}

StatementNodePtr Parser::parseFor() {
    Token tok = currentToken();
    std::string varName = currentToken().value;
    expect(TokenType::IDENTIFIER, "Expected variable name in for loop");
    expect(TokenType::KEYWORD_IN, "Expected 'in' in for loop");
    auto iterable = parseExpression();
    auto body = parseBlock();
    expect(TokenType::KEYWORD_END, "Expected 'end' for for loop");
    
    return std::make_shared<ForNode>(tok, varName, iterable, body, filename);
}

StatementNodePtr Parser::parseReturn() {
    Token tok = currentToken();
    ExpressionNodePtr value = nullptr;
    
    if (!check(TokenType::KEYWORD_END) && 
        !check(TokenType::OP_SEMICOLON) && 
        !check(TokenType::NEWLINE)) {
        value = parseExpression();
    }
    
    if (match(TokenType::OP_SEMICOLON) || match(TokenType::NEWLINE)) {
        // consume
    }
    expect(TokenType::KEYWORD_END, "Expected 'end' for return statement");
    
    return std::make_shared<ReturnNode>(tok, value, filename);
}

StatementNodePtr Parser::parseBlock() {
    Token tok = currentToken();
    auto block = std::make_shared<BlockNode>(tok, filename);
    
    while (!check(TokenType::KEYWORD_END) && 
           !check(TokenType::KEYWORD_ELSE) &&
           !check(TokenType::EOF_TOKEN)) {
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(stmt);
        } else {
            advance();
        }
    }
    
    return block;
}

ExpressionNodePtr Parser::parseExpression() {
    return parseAssignment();
}

ExpressionNodePtr Parser::parseAssignment() {
    auto expr = parseTernary();
    
    if (match(TokenType::OP_ASSIGN)) {
        Token opToken = currentToken();
        auto right = parseAssignment();
        return std::make_shared<BinaryOpNode>(opToken, expr, right, filename);
    }
    
    return expr;
}

ExpressionNodePtr Parser::parseTernary() {
    auto expr = parseOr();
    
    if (match(TokenType::OP_TERNARY)) {
        auto trueExpr = parseExpression();
        expect(TokenType::OP_COLON, "Expected ':' in ternary expression");
        auto falseExpr = parseExpression();
        return std::make_shared<TernaryNode>(currentToken(), expr, trueExpr, falseExpr, filename);
    }
    
    return expr;
}

ExpressionNodePtr Parser::parseOr() {
    auto left = parseAnd();
    
    while (match(TokenType::OP_OR) || match(TokenType::OP_BIN_OR)) {
        Token opToken = currentToken();
        auto right = parseAnd();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseAnd() {
    auto left = parseEquality();
    
    while (match(TokenType::OP_AND) || match(TokenType::OP_BIN_AND)) {
        Token opToken = currentToken();
        auto right = parseEquality();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseEquality() {
    auto left = parseComparison();
    
    while (match(TokenType::OP_EQ) || match(TokenType::OP_NEQ)) {
        Token opToken = currentToken();
        auto right = parseComparison();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseComparison() {
    auto left = parseTerm();
    
    while (match(TokenType::OP_LT) || match(TokenType::OP_GT) ||
           match(TokenType::OP_LTE) || match(TokenType::OP_GTE)) {
        Token opToken = currentToken();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseTerm() {
    auto left = parseFactor();
    
    while (match(TokenType::OP_PLUS) || match(TokenType::OP_MINUS)) {
        Token opToken = currentToken();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseFactor() {
    auto left = parseUnary();
    
    while (match(TokenType::OP_STAR) || match(TokenType::OP_SLASH) ||
           match(TokenType::OP_PERCENT)) {
        Token opToken = currentToken();
        auto right = parseUnary();
        left = std::make_shared<BinaryOpNode>(opToken, left, right, filename);
    }
    
    return left;
}

ExpressionNodePtr Parser::parseUnary() {
    if (match(TokenType::OP_NOT) || match(TokenType::OP_MINUS) ||
        match(TokenType::OP_PLUS)) {
        Token opToken = currentToken();
        auto operand = parseUnary();
        return std::make_shared<UnaryOpNode>(opToken, operand, filename);
    }
    
    return parseCall();
}

ExpressionNodePtr Parser::parseCall() {
    auto expr = parsePrimary();
    
    while (true) {
        if (match(TokenType::DELIM_LPAREN)) {
            auto args = parseArguments();
            expect(TokenType::DELIM_RPAREN, "Expected ')'");
            expr = std::make_shared<CallNode>(currentToken(), expr, args, filename);
        } else if (match(TokenType::DELIM_LBRACKET)) {
            auto indices = parseArguments();
            expect(TokenType::DELIM_RBRACKET, "Expected ']'");
            expr = std::make_shared<IndexNode>(currentToken(), expr, indices, filename);
        } else if (match(TokenType::OP_DOT)) {
            std::string member = currentToken().value;
            advance();
            expr = std::make_shared<MemberAccessNode>(currentToken(), expr, member, filename);
        } else {
            break;
        }
    }
    
    return expr;
}

ExpressionNodePtr Parser::parsePrimary() {
    Token tok = currentToken();
    
    if (match(TokenType::INTEGER)) {
        return std::make_shared<LiteralNode>(tok, filename);
    }
    
    if (match(TokenType::FLOAT)) {
        return std::make_shared<LiteralNode>(tok, filename);
    }
    
    if (match(TokenType::STRING)) {
        return std::make_shared<LiteralNode>(tok, filename);
    }
    
    if (match(TokenType::KEYWORD_TRUE) || match(TokenType::KEYWORD_FALSE)) {
        return std::make_shared<LiteralNode>(tok, filename);
    }
    
    if (match(TokenType::IDENTIFIER)) {
        return std::make_shared<IdentifierNode>(tok, filename);
    }
    
    if (match(TokenType::DELIM_LPAREN)) {
        auto expr = parseExpression();
        expect(TokenType::DELIM_RPAREN, "Expected ')'");
        return expr;
    }
    
    if (match(TokenType::DELIM_LBRACKET)) {
        auto elements = parseArguments();
        expect(TokenType::DELIM_RBRACKET, "Expected ']'");
        return std::make_shared<ArrayLiteralNode>(tok, elements, filename);
    }
    
    throw std::runtime_error("Unexpected token: " + tok.toString());
}

std::vector<ExpressionNodePtr> Parser::parseArguments() {
    std::vector<ExpressionNodePtr> args;
    
    if (check(TokenType::DELIM_RPAREN) || check(TokenType::DELIM_RBRACKET)) {
        return args;
    }
    
    args.push_back(parseExpression());
    
    while (match(TokenType::OP_COMMA)) {
        args.push_back(parseExpression());
    }
    
    return args;
}

ProgramNodePtr Parser::parse() {
    return parseProgram();
}

} // namespace proxima
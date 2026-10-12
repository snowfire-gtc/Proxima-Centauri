#include "../../src/lexer/Lexer.h"
#include <iostream>
#include <cassert>

using namespace proxima;

void testNumbers() {
    Lexer lexer("123 45.67 0x1A");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::INTEGER);
    assert(tokens[0].value == "123");
    assert(tokens[1].type == TokenType::FLOAT);
    assert(tokens[1].value == "45.67");
    
    std::cout << "✓ testNumbers passed" << std::endl;
}

void testStrings() {
    Lexer lexer("\"Hello\" \"World\"");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::STRING);
    assert(tokens[0].value == "Hello");
    assert(tokens[1].type == TokenType::STRING);
    assert(tokens[1].value == "World");
    
    std::cout << "✓ testStrings passed" << std::endl;
}

void testKeywords() {
    Lexer lexer("if else end for while");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::KEYWORD_IF);
    assert(tokens[1].type == TokenType::KEYWORD_ELSE);
    assert(tokens[2].type == TokenType::KEYWORD_END);
    assert(tokens[3].type == TokenType::KEYWORD_FOR);
    assert(tokens[4].type == TokenType::KEYWORD_WHILE);
    
    std::cout << "✓ testKeywords passed" << std::endl;
}

void testOperators() {
    Lexer lexer("+ - * / = == != < >");
    auto tokens = lexer.tokenize();
    
    assert(tokens[0].type == TokenType::OP_PLUS);
    assert(tokens[1].type == TokenType::OP_MINUS);
    assert(tokens[2].type == TokenType::OP_STAR);
    assert(tokens[3].type == TokenType::OP_SLASH);
    assert(tokens[4].type == TokenType::OP_ASSIGN);
    assert(tokens[5].type == TokenType::OP_EQ);
    assert(tokens[6].type == TokenType::OP_NEQ);
    assert(tokens[7].type == TokenType::OP_LT);
    assert(tokens[8].type == TokenType::OP_GT);
    
    std::cout << "✓ testOperators passed" << std::endl;
}

void testXorOperators() {
    // "%" - деление нацело, "#" - логическое XOR, "##" - побитовое XOR, ".#" - поэлементное XOR
    Lexer lexer("% # ## .#");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::OP_INT_DIV);   // %
    assert(tokens[1].type == TokenType::OP_LOGIC_XOR); // #
    assert(tokens[1].value == "#");
    assert(tokens[2].type == TokenType::OP_BIN_XOR);   // ##
    assert(tokens[2].value == "##");
    assert(tokens[3].type == TokenType::OP_DOT_XOR);   // .#
    assert(tokens[3].value == ".#");

    // Директивы не конфликтуют с операторами: "#intent" — директива, "a#b" — оператор
    Lexer lexer2("#intent a#b c##d");
    auto t2 = lexer2.tokenize();
    assert(t2[0].value == "#intent");
    assert(t2[1].type == TokenType::IDENTIFIER && t2[1].value == "a");
    assert(t2[2].type == TokenType::OP_LOGIC_XOR);
    assert(t2[3].type == TokenType::IDENTIFIER && t2[3].value == "b");
    assert(t2[4].type == TokenType::IDENTIFIER && t2[4].value == "c");
    assert(t2[5].type == TokenType::OP_BIN_XOR);
    assert(t2[6].type == TokenType::IDENTIFIER && t2[6].value == "d");

    std::cout << "✓ testXorOperators passed" << std::endl;
}

int main() {
    std::cout << "Running lexer tests..." << std::endl;
    
    testNumbers();
    testStrings();
    testKeywords();
    testOperators();
    testXorOperators();
    
    std::cout << "All lexer tests passed!" << std::endl;
    return 0;
}
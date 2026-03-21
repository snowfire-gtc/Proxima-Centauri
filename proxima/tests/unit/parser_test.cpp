#include "../../src/lexer/Lexer.h"
#include "../../src/parser/Parser.h"
#include <iostream>
#include <cassert>

using namespace proxima;

void testVariableDeclaration() {
    std::string source = "x: int32 = 100;";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->declarations.size() == 1);
    assert(program->declarations[0]->nodeType == NodeType::VARIABLE_DECL);
    
    std::cout << "✓ testVariableDeclaration passed" << std::endl;
}

void testFunctionDeclaration() {
    std::string source = R"(
        add(a, b): int32
            return a + b;
        end
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->declarations.size() == 1);
    assert(program->declarations[0]->nodeType == NodeType::FUNCTION_DECL);
    
    std::cout << "✓ testFunctionDeclaration passed" << std::endl;
}

void testIfStatement() {
    std::string source = R"(
        if x > 10
            print("Large");
        else
            print("Small");
        end
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->statements.size() == 1);
    assert(program->statements[0]->nodeType == NodeType::IF_STATEMENT);
    
    std::cout << "✓ testIfStatement passed" << std::endl;
}

void testForLoop() {
    std::string source = R"(
        for i in 1:10
            print(i);
        end
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto program = parser.parse();
    
    assert(program->statements.size() == 1);
    assert(program->statements[0]->nodeType == NodeType::FOR_LOOP);
    
    std::cout << "✓ testForLoop passed" << std::endl;
}

int main() {
    std::cout << "Running parser tests..." << std::endl;
    
    testVariableDeclaration();
    testFunctionDeclaration();
    testIfStatement();
    testForLoop();
    
    std::cout << "All parser tests passed!" << std::endl;
    return 0;
}
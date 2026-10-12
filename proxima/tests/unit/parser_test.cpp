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

// Извлечение корневого бинарного оператора из разобранной программы.
// "x = <expr>;" парсится как VariableDeclNode (initializer — BinaryOp "=")
// либо ExpressionStatementNode (expression — BinaryOp "="); в обоих случаях
// правое поддерево присваивания — само выражение <expr>.
static BinaryOpNodePtr rootBinaryOf(ProgramNodePtr prog) {
    if (!prog->declarations.empty()) {
        auto var = std::dynamic_pointer_cast<VariableDeclNode>(prog->declarations[0]);
        if (var && var->initializer) {
            auto assign = std::dynamic_pointer_cast<BinaryOpNode>(var->initializer);
            if (assign && assign->op == "=") return assign;
        }
    }
    if (!prog->statements.empty()) {
        auto es = std::dynamic_pointer_cast<ExpressionStatementNode>(prog->statements[0]);
        if (es && es->expression) {
            auto assign = std::dynamic_pointer_cast<BinaryOpNode>(es->expression);
            if (assign && assign->op == "=") return assign;
        }
    }
    return nullptr;
}

void testXorOperatorPrecedence() {
    // "##" (побитовое XOR) имеет приоритет мультипликативного уровня (наравне с * / %),
    // "#" (логическое XOR) — приоритет уровня дизъюнкции (наравне с |).
    // x = a ## b * c  ==>  x = (a ## (b * c))
    Lexer lexer1("x = a ## b * c;");
    Parser p1(lexer1.tokenize());
    auto prog1 = p1.parse();
    auto assign1 = rootBinaryOf(prog1);
    assert(assign1 && assign1->op == "=");
    auto bin1 = std::dynamic_pointer_cast<BinaryOpNode>(assign1->right);
    assert(bin1 && bin1->op == "##");
    auto right1 = std::dynamic_pointer_cast<BinaryOpNode>(bin1->right);
    assert(right1 && right1->op == "*"); // b*c группируется раньше, чем ##

    // y = a # b | c  ==>  y = ((a # b) | c)  (оба на одном уровне, левая ассоциативность)
    Lexer lexer2("y = a # b | c;");
    Parser p2(lexer2.tokenize());
    auto prog2 = p2.parse();
    auto assign2 = rootBinaryOf(prog2);
    assert(assign2 && assign2->op == "=");
    auto bin2 = std::dynamic_pointer_cast<BinaryOpNode>(assign2->right);
    assert(bin2 && bin2->op == "|");
    auto left2 = std::dynamic_pointer_cast<BinaryOpNode>(bin2->left);
    assert(left2 && left2->op == "#");

    std::cout << "✓ testXorOperatorPrecedence passed" << std::endl;
}

int main() {
    std::cout << "Running parser tests..." << std::endl;
    
    testVariableDeclaration();
    testFunctionDeclaration();
    testIfStatement();
    testForLoop();
    testXorOperatorPrecedence();
    
    std::cout << "All parser tests passed!" << std::endl;
    return 0;
}
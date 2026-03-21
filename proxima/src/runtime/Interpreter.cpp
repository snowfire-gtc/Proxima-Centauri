#include "Interpreter.h"
#include "../runtime/Debugger.h"
#include "utils/Logger.h"
#include <chrono>
#include <thread>

namespace proxima {

Interpreter::Interpreter()
    : interpretMode(true)
    , running(false)
    , paused(false) {
    
    LOG_INFO("Interpreter initialized");
}

Interpreter::~Interpreter() {
    stop();
    LOG_INFO("Interpreter destroyed");
}

void Interpreter::setInterpretMode(bool enable) {
    interpretMode = enable;
    LOG_INFO("Interpret mode: " + std::string(enable ? "enabled" : "disabled"));
}

bool Interpreter::execute(ProgramNodePtr program) {
    if (!program) {
        LOG_ERROR("Null program provided to interpreter");
        return false;
    }
    
    running = true;
    paused = false;
    currentContext.shouldStop = false;
    currentContext.shouldPause = false;
    
    LOG_INFO("Starting interpretation...");
    
    // Execute declarations first
    for (const auto& decl : program->declarations) {
        if (currentContext.shouldStop) break;
        // Process declarations
    }
    
    // Execute statements
    for (const auto& stmt : program->statements) {
        if (currentContext.shouldStop) break;
        
        currentContext.currentLine = stmt->token.line;
        currentContext.currentFile = stmt->filename;
        
        // Check for breakpoints
        checkBreakpoint(stmt->token.line, stmt->filename);
        
        // Wait if paused
        while (paused && !currentContext.shouldStop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (!executeStatement(stmt, currentContext)) {
            LOG_ERROR("Statement execution failed at line " + 
                     std::to_string(stmt->token.line));
            running = false;
            return false;
        }
    }
    
    running = false;
    LOG_INFO("Interpretation completed");
    return true;
}

bool Interpreter::executeStatement(StatementNodePtr stmt, ExecutionContext& context) {
    if (!stmt || context.shouldStop) return false;
    
    notifyLineExecution(stmt->token.line, stmt->filename);
    
    switch (stmt->nodeType) {
        case NodeType::ASSIGNMENT:
            return executeAssignment(std::static_pointer_cast<AssignmentNode>(stmt), context);
        
        case NodeType::IF_STATEMENT:
            return executeIf(std::static_pointer_cast<IfNode>(stmt), context);
        
        case NodeType::FOR_LOOP:
            return executeFor(std::static_pointer_cast<ForNode>(stmt), context);
        
        case NodeType::WHILE_LOOP:
            return executeWhile(std::static_pointer_cast<WhileNode>(stmt), context);
        
        case NodeType::RETURN_STATEMENT:
            return executeReturn(std::static_pointer_cast<ReturnNode>(stmt), context);
        
        case NodeType::BLOCK: {
            auto block = std::static_pointer_cast<BlockNode>(stmt);
            context.symbolTable.pushScope();
            for (const auto& child : block->statements) {
                if (!executeStatement(child, context)) {
                    context.symbolTable.popScope();
                    return false;
                }
            }
            context.symbolTable.popScope();
            return true;
        }
        
        default:
            LOG_WARNING("Unknown statement type: " + std::to_string(static_cast<int>(stmt->nodeType)));
            return true;
    }
}

RuntimeValue Interpreter::executeExpression(ExpressionNodePtr expr, ExecutionContext& context) {
    if (!expr) return RuntimeValue();
    
    switch (expr->nodeType) {
        case NodeType::LITERAL:
            return evaluateLiteral(std::static_pointer_cast<LiteralNode>(expr), context);
        
        case NodeType::IDENTIFIER:
            return evaluateIdentifier(std::static_pointer_cast<IdentifierNode>(expr), context);
        
        case NodeType::BINARY_OP:
            return evaluateBinaryOp(std::static_pointer_cast<BinaryOpNode>(expr), context);
        
        case NodeType::UNARY_OP:
            return evaluateUnaryOp(std::static_pointer_cast<UnaryOpNode>(expr), context);
        
        case NodeType::CALL_EXPR:
            return evaluateCall(std::static_pointer_cast<CallNode>(expr), context);
        
        default:
            LOG_WARNING("Unknown expression type");
            return RuntimeValue();
    }
}

void Interpreter::pause() {
    paused = true;
    LOG_INFO("Interpreter paused");
}

void Interpreter::resume() {
    paused = false;
    LOG_INFO("Interpreter resumed");
}

void Interpreter::step() {
    paused = true;
    currentContext.shouldPause = true;
    LOG_INFO("Interpreter step");
}

void Interpreter::stop() {
    currentContext.shouldStop = true;
    running = false;
    paused = false;
    LOG_INFO("Interpreter stopped");
}

void Interpreter::checkBreakpoint(int line, const std::string& file) {
    // Integration with Debugger
    // Would check if breakpoint exists at this location
}

void Interpreter::notifyLineExecution(int line, const std::string& file) {
    // Notify IDE about current execution position
    // For visualization and debugging
}

// ... остальные методы реализации ...

} // namespace proxima
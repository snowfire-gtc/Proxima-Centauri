#include "LLVMCodeGen.h"
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>
#include <fstream>
#include <iostream>

namespace proxima {

LLVMCodeGen::LLVMCodeGen() 
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("proxima_module", *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context)) {
    
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
}

llvm::Type* LLVMCodeGen::getType(const std::string& typeName) {
    if (typeName == "int32" || typeName == "int") {
        return llvm::Type::getInt32Ty(*context);
    }
    if (typeName == "int64") {
        return llvm::Type::getInt64Ty(*context);
    }
    if (typeName == "single" || typeName == "float") {
        return llvm::Type::getFloatTy(*context);
    }
    if (typeName == "double") {
        return llvm::Type::getDoubleTy(*context);
    }
    if (typeName == "bool") {
        return llvm::Type::getInt1Ty(*context);
    }
    if (typeName == "void") {
        return llvm::Type::getVoidTy(*context);
    }
    // Default to double
    return llvm::Type::getDoubleTy(*context);
}

llvm::Value* LLVMCodeGen::generateExpression(ExpressionNodePtr expr) {
    if (!expr) return nullptr;
    
    switch (expr->nodeType) {
        case NodeType::LITERAL: {
            auto literal = std::static_pointer_cast<LiteralNode>(expr);
            if (literal->token.type == TokenType::INTEGER) {
                int64_t val = std::stoll(literal->token.value);
                return llvm::ConstantInt::get(*context, llvm::APInt(64, val));
            }
            if (literal->token.type == TokenType::FLOAT) {
                double val = std::stod(literal->token.value);
                return llvm::ConstantFP::get(*context, llvm::APFloat(val));
            }
            if (literal->token.type == TokenType::KEYWORD_TRUE) {
                return llvm::ConstantInt::get(*context, llvm::APInt(1, 1));
            }
            if (literal->token.type == TokenType::KEYWORD_FALSE) {
                return llvm::ConstantInt::get(*context, llvm::APInt(1, 0));
            }
            break;
        }
        
        case NodeType::IDENTIFIER: {
            auto ident = std::static_pointer_cast<IdentifierNode>(expr);
            if (namedValues.count(ident->name)) {
                return namedValues[ident->name];
            }
            // Could be a function call without parentheses
            break;
        }
        
        case NodeType::BINARY_OP: {
            auto binop = std::static_pointer_cast<BinaryOpNode>(expr);
            llvm::Value* left = generateExpression(binop->left);
            llvm::Value* right = generateExpression(binop->right);
            return createBinaryOp(binop->op, left, right);
        }
        
        case NodeType::CALL_EXPR: {
            auto call = std::static_pointer_cast<CallNode>(expr);
            llvm::Function* func = nullptr;
            
            if (call->callee->nodeType == NodeType::IDENTIFIER) {
                auto ident = std::static_pointer_cast<IdentifierNode>(call->callee);
                if (functionMap.count(ident->name)) {
                    func = functionMap[ident->name];
                }
            }
            
            if (!func) {
                // Create implicit declaration
                std::vector<llvm::Type*> argTypes;
                for (const auto& arg : call->arguments) {
                    argTypes.push_back(getType("double"));
                }
                llvm::FunctionType* funcType = llvm::FunctionType::get(
                    getType("double"), argTypes, false);
                func = llvm::Function::Create(funcType, 
                    llvm::Function::ExternalLinkage, 
                    call->callee->token.value, 
                    module.get());
                functionMap[call->callee->token.value] = func;
            }
            
            std::vector<llvm::Value*> args;
            for (const auto& arg : call->arguments) {
                args.push_back(generateExpression(arg));
            }
            
            return builder->CreateCall(func, args);
        }
        
        default:
            break;
    }
    
    return nullptr;
}

llvm::Value* LLVMCodeGen::createBinaryOp(const std::string& op, llvm::Value* left, llvm::Value* right) {
    if (!left || !right) return nullptr;
    
    if (op == "+") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFAdd(left, right);
        }
        return builder->CreateAdd(left, right);
    }
    if (op == "-") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFSub(left, right);
        }
        return builder->CreateSub(left, right);
    }
    if (op == "*") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFMul(left, right);
        }
        return builder->CreateMul(left, right);
    }
    if (op == "/") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFDiv(left, right);
        }
        return builder->CreateSDiv(left, right);
    }
    if (op == "==") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOEQ(left, right);
        }
        return builder->CreateICmpEQ(left, right);
    }
    if (op == "!=") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpONE(left, right);
        }
        return builder->CreateICmpNE(left, right);
    }
    if (op == "<") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOLT(left, right);
        }
        return builder->CreateICmpSLT(left, right);
    }
    if (op == ">") {
        if (left->getType()->isFloatingPointTy()) {
            return builder->CreateFCmpOGT(left, right);
        }
        return builder->CreateICmpSGT(left, right);
    }
    
    return nullptr;
}

void LLVMCodeGen::generateStatement(StatementNodePtr stmt) {
    if (!stmt) return;
    
    switch (stmt->nodeType) {
        case NodeType::VARIABLE_DECL: {
            auto var = std::static_pointer_cast<VariableDeclNode>(stmt);
            generateVariable(var);
            break;
        }
        
        case NodeType::ASSIGNMENT: {
            auto assign = std::static_pointer_cast<BinaryOpNode>(stmt);
            llvm::Value* value = generateExpression(assign->right);
            if (assign->left->nodeType == NodeType::IDENTIFIER) {
                auto ident = std::static_pointer_cast<IdentifierNode>(assign->left);
                namedValues[ident->name] = value;
            }
            break;
        }
        
        case NodeType::IF_STATEMENT: {
            auto ifStmt = std::static_pointer_cast<IfNode>(stmt);
            llvm::Value* condition = generateExpression(ifStmt->condition);
            
            llvm::Function* func = builder->GetInsertBlock()->getParent();
            llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "then", func);
            llvm::BasicBlock* elseBB = ifStmt->elseBranch ? 
                llvm::BasicBlock::Create(*context, "else") : nullptr;
            llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "ifcont");
            
            builder->CreateCondBr(condition, thenBB, elseBB ? elseBB : mergeBB);
            
            builder->SetInsertPoint(thenBB);
            generateStatement(ifStmt->thenBranch);
            builder->CreateBr(mergeBB);
            
            if (elseBB) {
                func->getBasicBlockList().push_back(elseBB);
                builder->SetInsertPoint(elseBB);
                generateStatement(ifStmt->elseBranch);
                builder->CreateBr(mergeBB);
            }
            
            func->getBasicBlockList().push_back(mergeBB);
            builder->SetInsertPoint(mergeBB);
            break;
        }
        
        case NodeType::RETURN_STATEMENT: {
            auto ret = std::static_pointer_cast<ReturnNode>(stmt);
            if (ret->value) {
                llvm::Value* retVal = generateExpression(ret->value);
                builder->CreateRet(retVal);
            } else {
                builder->CreateRetVoid();
            }
            break;
        }
        
        default:
            break;
    }
}

llvm::Value* LLVMCodeGen::generateVariable(VariableDeclNodePtr var) {
    llvm::Type* type = getType(var->typeName);
    llvm::Value* init = nullptr;
    
    if (var->initializer) {
        init = generateExpression(var->initializer);
    } else {
        init = llvm::Constant::getNullValue(type);
    }
    
    namedValues[var->name] = init;
    return init;
}

llvm::Function* LLVMCodeGen::generateFunction(FunctionDeclNodePtr func) {
    std::vector<llvm::Type*> argTypes;
    for (const auto& param : func->parameters) {
        argTypes.push_back(getType(param.second));
    }
    
    llvm::Type* returnType = getType(func->returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, argTypes, false);
    llvm::Function* F = llvm::Function::Create(funcType, 
        llvm::Function::ExternalLinkage, 
        func->name, 
        module.get());
    
    functionMap[func->name] = F;
    
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*context, "entry", F);
    builder->SetInsertPoint(BB);
    
    namedValues.clear();
    for (auto& arg : F->args()) {
        namedValues[arg.getName().str()] = &arg;
    }
    
    generateStatement(func->body);
    
    // Add implicit return if needed
    if (func->returnType == "void") {
        if (!BB->getTerminator()) {
            builder->CreateRetVoid();
        }
    } else {
        if (!BB->getTerminator()) {
            builder->CreateRet(llvm::Constant::getNullValue(returnType));
        }
    }
    
    llvm::verifyFunction(*F);
    return F;
}

std::unique_ptr<llvm::Module> LLVMCodeGen::generate(ProgramNodePtr program) {
    namedValues.clear();
    functionMap.clear();
    
    // Generate declarations first
    for (const auto& decl : program->declarations) {
        if (decl->nodeType == NodeType::FUNCTION_DECL) {
            generateFunction(std::static_pointer_cast<FunctionDeclNode>(decl));
        }
    }
    
    // Generate statements
    for (const auto& stmt : program->statements) {
        generateStatement(stmt);
    }
    
    return std::move(module);
}

std::string LLVMCodeGen::generateIR(ProgramNodePtr program) {
    generate(program);
    std::string ir;
    llvm::raw_string_ostream os(ir);
    module->print(os, nullptr);
    return ir;
}

bool LLVMCodeGen::compileToExecutable(ProgramNodePtr program, const std::string& outputPath) {
    generate(program);
    
    // Verify module
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Module verification failed!" << std::endl;
        return false;
    }
    
    // Write bitcode
    std::error_code EC;
    llvm::ToolOutputFile out(outputPath, EC, llvm::sys::fs::OF_None);
    if (EC) {
        std::cerr << "Could not open output file: " << EC.message() << std::endl;
        return false;
    }
    
    llvm::WriteBitcodeToFile(*module, out.os());
    out.keep();
    
    return true;
}

} // namespace proxima
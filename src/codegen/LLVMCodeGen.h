#ifndef PROXIMA_LLVM_CODEGEN_H
#define PROXIMA_LLVM_CODEGEN_H

#include "../parser/AST.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <memory>

namespace proxima {

class LLVMCodeGen {
public:
    LLVMCodeGen();
    
    std::unique_ptr<llvm::Module> generate(ProgramNodePtr program);
    std::string generateIR(ProgramNodePtr program);
    bool compileToExecutable(ProgramNodePtr program, const std::string& outputPath);
    
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unordered_map<std::string, llvm::Value*> namedValues;
    std::unordered_map<std::string, llvm::Function*> functionMap;
    
    llvm::Value* generateExpression(ExpressionNodePtr expr);
    void generateStatement(StatementNodePtr stmt);
    void generateDeclaration(DeclarationNodePtr decl);
    llvm::Function* generateFunction(FunctionDeclNodePtr func);
    llvm::Value* generateVariable(VariableDeclNodePtr var);
    
    llvm::Type* getType(const std::string& typeName);
    llvm::Value* createBinaryOp(const std::string& op, llvm::Value* left, llvm::Value* right);
    llvm::Value* createUnaryOp(const std::string& op, llvm::Value* operand);
};

} // namespace proxima

#endif // PROXIMA_LLVM_CODEGEN_H
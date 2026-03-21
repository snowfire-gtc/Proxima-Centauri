#include "Compiler.h"
#include <fstream>
#include <chrono>
#include <iostream>

namespace proxima {

Compiler::Compiler() : verboseLevel(2) {
    lexer = std::make_unique<Lexer>("");
    parser = std::make_unique<Parser>(std::vector<Token>());
    semanticAnalyzer = std::make_unique<SemanticAnalyzer>();
    codeGen = std::make_unique<LLVMCodeGen>();
    runtime = std::make_unique<Runtime>();
}

void Compiler::setVerboseLevel(int level) {
    verboseLevel = level;
}

void Compiler::log(int level, const std::string& message) {
    if (level <= verboseLevel) {
        std::cout << "[Proxima] " << message << std::endl;
    }
}

CompileResult Compiler::compile(const std::string& sourceFile, const Config& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    CompileResult result;
    result.success = false;
    
    try {
        // Read source file
        std::ifstream file(sourceFile);
        if (!file.is_open()) {
            result.errorMessage = "Could not open file: " + sourceFile;
            return result;
        }
        
        std::string source((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        file.close();
        
        log(1, "Compiling: " + sourceFile);
        
        // Lexing
        log(2, "Stage: Lexing");
        auto tokens = tokenize(source);
        
        // Parsing
        log(2, "Stage: Parsing");
        auto program = parse(tokens);
        
        // Semantic Analysis
        log(2, "Stage: Semantic Analysis");
        analyze(program);
        
        // Code Generation
        log(2, "Stage: Code Generation");
        std::string outputPath = config.outputPath;
        if (outputPath.empty()) {
            outputPath = sourceFile.substr(0, sourceFile.find_last_of('.')) + ".bc";
        }
        
        result.success = generateExecutable(program, outputPath);
        result.outputPath = outputPath;
        
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
        result.success = false;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.compileTime = std::chrono::duration<double>(endTime - startTime).count();
    
    log(1, "Compilation completed in " + std::to_string(result.compileTime) + "s");
    
    return result;
}

CompileResult Compiler::compileString(const std::string& source, const std::string& filename, const Config& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    CompileResult result;
    result.success = false;
    
    try {
        log(1, "Compiling string: " + filename);
        
        auto tokens = tokenize(source);
        auto program = parse(tokens);
        analyze(program);
        
        std::string outputPath = config.outputPath;
        if (outputPath.empty()) {
            outputPath = filename + ".bc";
        }
        
        result.success = generateExecutable(program, outputPath);
        result.outputPath = outputPath;
        
    } catch (const std::exception& e) {
        result.errorMessage = e.what();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.compileTime = std::chrono::duration<double>(endTime - startTime).count();
    
    return result;
}

std::vector<Token> Compiler::tokenize(const std::string& source) {
    lexer = std::make_unique<Lexer>(source, "<input>");
    return lexer->tokenize();
}

ProgramNodePtr Compiler::parse(const std::vector<Token>& tokens) {
    parser = std::make_unique<Parser>(tokens, "<input>");
    return parser->parse();
}

void Compiler::analyze(ProgramNodePtr program) {
    semanticAnalyzer->analyze(program);
}

std::string Compiler::generateIR(ProgramNodePtr program) {
    return codeGen->generateIR(program);
}

bool Compiler::generateExecutable(ProgramNodePtr program, const std::string& outputPath) {
    return codeGen->compileToExecutable(program, outputPath);
}

} // namespace proxima
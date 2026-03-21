#ifndef PROXIMA_COMPILER_H
#define PROXIMA_COMPILER_H

#include "../lexer/Lexer.h"
#include "../parser/Parser.h"
#include "../semantic/SemanticAnalyzer.h"
#include "../codegen/LLVMCodeGen.h"
#include "../runtime/Runtime.h"
#include "Config.h"
#include <string>
#include <memory>

namespace proxima {

enum class CompileStage {
    LEXING,
    PARSING,
    SEMANTIC_ANALYSIS,
    CODE_GENERATION,
    OPTIMIZATION,
    LINKING
};

struct CompileResult {
    bool success;
    std::string errorMessage;
    std::string outputPath;
    double compileTime;
    std::vector<std::string> warnings;
};

class Compiler {
public:
    Compiler();
    
    CompileResult compile(const std::string& sourceFile, const Config& config);
    CompileResult compileString(const std::string& source, const std::string& filename, const Config& config);
    
    std::vector<Token> tokenize(const std::string& source);
    ProgramNodePtr parse(const std::vector<Token>& tokens);
    void analyze(ProgramNodePtr program);
    std::string generateIR(ProgramNodePtr program);
    bool generateExecutable(ProgramNodePtr program, const std::string& outputPath);
    
    void setVerboseLevel(int level);
    int getVerboseLevel() const { return verboseLevel; }
    
private:
    int verboseLevel;
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Parser> parser;
    std::unique_ptr<SemanticAnalyzer> semanticAnalyzer;
    std::unique_ptr<LLVMCodeGen> codeGen;
    std::unique_ptr<Runtime> runtime;
    
    void log(int level, const std::string& message);
};

} // namespace proxima

#endif // PROXIMA_COMPILER_H
#ifndef PROXIMA_OPTIMIZER_H
#define PROXIMA_OPTIMIZER_H

#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <memory>

namespace proxima {

enum class OptimizationLevel {
    O0,  // No optimization
    O1,  // Basic optimization
    O2,  // Standard optimization
    O3   // Aggressive optimization
};

struct OptimizationConfig {
    OptimizationLevel level;
    bool enableInlining;
    bool enableLoopUnrolling;
    bool enableVectorization;
    bool enableDeadCodeElimination;
    bool enableConstantPropagation;
    bool enableFunctionSpecialization;
    bool enableInterProceduralOptimization;
    size_t maxInlineSize;
    size_t maxUnrollCount;
};

class Optimizer {
public:
    Optimizer();
    ~Optimizer();
    
    void setConfig(const OptimizationConfig& config);
    OptimizationConfig getConfig() const { return config; }
    
    bool optimize(llvm::Module& module);
    bool optimizeFunction(llvm::Function& func);
    
    // Specific optimization passes
    void runInliningPass(llvm::Module& module);
    void runLoopUnrollPass(llvm::Function& func);
    void runVectorizationPass(llvm::Function& func);
    void runDeadCodeElimination(llvm::Module& module);
    void runConstantPropagation(llvm::Module& module);
    void runMemoryOptimization(llvm::Module& module);
    void runGPUSpecificOptimization(llvm::Module& module);
    void runAVXOptimization(llvm::Module& module);
    
    // Statistics
    struct OptimizationStats {
        size_t functionsOptimized;
        size_t instructionsRemoved;
        size_t instructionsAdded;
        double optimizationTime;
        size_t memorySaved;
    };
    
    OptimizationStats getStats() const { return stats; }
    void resetStats();
    
    // Validation
    bool validateModule(llvm::Module& module);
    std::string getValidationErrors() const { return validationErrors; }
    
private:
    void setupPassManager();
    void addBasicOptimizations(llvm::ModulePassManager& pm);
    void addStandardOptimizations(llvm::ModulePassManager& pm);
    void addAggressiveOptimizations(llvm::ModulePassManager& pm);
    void addGPUPass(llvm::ModulePassManager& pm);
    void addAVXPass(llvm::ModulePassManager& pm);
    
    OptimizationConfig config;
    OptimizationStats stats;
    std::string validationErrors;
    
    std::unique_ptr<llvm::PassManager<llvm::Module>> modulePM;
    std::unique_ptr<llvm::PassManager<llvm::Function>> functionPM;
    std::unique_ptr<llvm::PassBuilder> passBuilder;
};

} // namespace proxima

#endif // PROXIMA_OPTIMIZER_H
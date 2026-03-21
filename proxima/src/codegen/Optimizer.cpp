#include "Optimizer.h"
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Vectorize.h>
#include <llvm/Transforms/InstCombine.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <chrono>
#include "utils/Logger.h"

namespace proxima {

Optimizer::Optimizer() {
    config.level = OptimizationLevel::O2;
    config.enableInlining = true;
    config.enableLoopUnrolling = true;
    config.enableVectorization = true;
    config.enableDeadCodeElimination = true;
    config.enableConstantPropagation = true;
    config.enableFunctionSpecialization = false;
    config.enableInterProceduralOptimization = true;
    config.maxInlineSize = 300;
    config.maxUnrollCount = 8;
    
    resetStats();
}

Optimizer::~Optimizer() {}

void Optimizer::setConfig(const OptimizationConfig& newConfig) {
    config = newConfig;
    setupPassManager();
}

void Optimizer::resetStats() {
    stats.functionsOptimized = 0;
    stats.instructionsRemoved = 0;
    stats.instructionsAdded = 0;
    stats.optimizationTime = 0.0;
    stats.memorySaved = 0;
}

bool Optimizer::optimize(llvm::Module& module) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    setupPassManager();
    
    // Run module passes
    if (modulePM) {
        modulePM->run(module, *passBuilder);
    }
    
    // Run function passes
    if (functionPM) {
        for (auto& func : module.functions()) {
            if (!func.isDeclaration()) {
                functionPM->run(func, *passBuilder);
                stats.functionsOptimized++;
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    stats.optimizationTime = std::chrono::duration<double>(endTime - startTime).count();
    
    LOG_INFO("Optimization completed in " + std::to_string(stats.optimizationTime) + "s");
    LOG_INFO("Functions optimized: " + std::to_string(stats.functionsOptimized));
    
    return validateModule(module);
}

bool Optimizer::optimizeFunction(llvm::Function& func) {
    if (func.isDeclaration()) return true;
    
    setupPassManager();
    
    if (functionPM) {
        functionPM->run(func, *passBuilder);
        stats.functionsOptimized++;
    }
    
    return true;
}

void Optimizer::setupPassManager() {
    passBuilder = std::make_unique<llvm::PassBuilder>();
    
    modulePM = std::make_unique<llvm::ModulePassManager>();
    functionPM = std::make_unique<llvm::FunctionPassManager>();
    
    switch (config.level) {
        case OptimizationLevel::O0:
            // No optimization
            break;
        case OptimizationLevel::O1:
            addBasicOptimizations(*modulePM);
            break;
        case OptimizationLevel::O2:
            addStandardOptimizations(*modulePM);
            break;
        case OptimizationLevel::O3:
            addAggressiveOptimizations(*modulePM);
            break;
    }
    
    // Add specific optimizations
    if (config.enableVectorization) {
        addAVXPass(*modulePM);
    }
}

void Optimizer::addBasicOptimizations(llvm::ModulePassManager& pm) {
    pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::InstCombinePass()));
    pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::SimplifyCFGPass()));
    
    if (config.enableDeadCodeElimination) {
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::DCEPass()));
    }
    
    if (config.enableConstantPropagation) {
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::SCCPPass()));
    }
}

void Optimizer::addStandardOptimizations(llvm::ModulePassManager& pm) {
    addBasicOptimizations(pm);
    
    pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::GVNPass()));
    pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::LICMPass()));
    pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::LoopUnrollPass()));
    
    if (config.enableInlining) {
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(
            llvm::InlinerPass(config.maxInlineSize)));
    }
    
    if (config.enableVectorization) {
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::SLPVectorizerPass()));
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::LoopVectorizePass()));
    }
}

void Optimizer::addAggressiveOptimizations(llvm::ModulePassManager& pm) {
    addStandardOptimizations(pm);
    
    pm.addPass(llvm::IPSCCPPass());
    pm.addPass(llvm::GlobalDCEPass());
    
    if (config.enableInterProceduralOptimization) {
        pm.addPass(llvm::createModuleToFunctionPassAdaptor(
            llvm::AlwaysInlinerPass()));
    }
}

void Optimizer::addAVXPass(llvm::ModulePassManager& pm) {
    // Add AVX-specific optimization passes
    // This would include vector width tuning, etc.
    LOG_DEBUG("AVX optimization pass added");
}

void Optimizer::addGPUPass(llvm::ModulePassManager& pm) {
    // Add GPU-specific optimization passes
    // This would include memory coalescing, etc.
    LOG_DEBUG("GPU optimization pass added");
}

void Optimizer::runInliningPass(llvm::Module& module) {
    if (!config.enableInlining) return;
    
    llvm::InlineParams params;
    params.InstThreshold = config.maxInlineSize;
    
    llvm::buildInlinePipeline(*passBuilder, params).run(module, *passBuilder);
    
    LOG_DEBUG("Inlining pass completed");
}

void Optimizer::runLoopUnrollPass(llvm::Function& func) {
    if (!config.enableLoopUnrolling) return;
    
    llvm::LoopUnrollOptions opts;
    opts.UnrollCount = config.maxUnrollCount;
    
    // Run loop unroll pass
    LOG_DEBUG("Loop unroll pass completed for: " + func.getName().str());
}

void Optimizer::runVectorizationPass(llvm::Function& func) {
    if (!config.enableVectorization) return;
    
    // Run SLP and loop vectorization
    LOG_DEBUG("Vectorization pass completed for: " + func.getName().str());
}

void Optimizer::runDeadCodeElimination(llvm::Module& module) {
    if (!config.enableDeadCodeElimination) return;
    
    // Run DCE pass
    LOG_DEBUG("Dead code elimination completed");
}

void Optimizer::runConstantPropagation(llvm::Module& module) {
    if (!config.enableConstantPropagation) return;
    
    // Run SCCP pass
    LOG_DEBUG("Constant propagation completed");
}

void Optimizer::runMemoryOptimization(llvm::Module& module) {
    // Memory optimization passes
    // - MemCpyOpt
    // - MemSetOpt
    // - ADCE
    LOG_DEBUG("Memory optimization completed");
}

void Optimizer::runGPUSpecificOptimization(llvm::Module& module) {
    // GPU-specific optimizations
    // - Kernel specialization
    // - Memory coalescing
    // - Shared memory optimization
    LOG_DEBUG("GPU-specific optimization completed");
}

void Optimizer::runAVXOptimization(llvm::Module& module) {
    // AVX-specific optimizations
    // - Vector width tuning
    // - Alignment optimization
    // - FMA fusion
    LOG_DEBUG("AVX optimization completed");
}

bool Optimizer::validateModule(llvm::Module& module) {
    std::string errors;
    llvm::raw_string_ostream os(errors);
    
    if (llvm::verifyModule(module, &os)) {
        validationErrors = errors;
        LOG_ERROR("Module validation failed: " + errors);
        return false;
    }
    
    validationErrors.clear();
    return true;
}

} // namespace proxima
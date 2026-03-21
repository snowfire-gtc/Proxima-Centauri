#include "compiler/Compiler.h"
#include "compiler/Config.h"
#include "utils/Logger.h"
#include <iostream>
#include <string>
#include <cstring>

void printHelp() {
    std::cout << "Proxima Compiler v1.0.0 (GPLv3)\n";
    std::cout << "Usage: proxima <command> [options] [files...]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  build     Compile source files\n";
    std::cout << "  run       Compile and run\n";
    std::cout << "  clean     Clean build artifacts\n";
    std::cout << "  test      Run tests\n";
    std::cout << "  time      Benchmark execution\n";
    std::cout << "  deps      List dependencies\n";
    std::cout << "  doc       Generate documentation\n";
    std::cout << "  help      Show this help\n\n";
    std::cout << "Options:\n";
    std::cout << "  -o, --output <file>    Output file\n";
    std::cout << "  -O0, -O1, -O2, -O3     Optimization level\n";
    std::cout << "  -v, --verbose <level>  Verbose level (0-5)\n";
    std::cout << "  --debug                Enable debug symbols\n";
    std::cout << "  --release              Release mode\n";
    std::cout << "  --cuda                 Enable CUDA support\n";
    std::cout << "  --avx2                 Enable AVX2 optimizations\n";
    std::cout << "  --help                 Show this help\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }
    
    std::string command = argv[1];
    proxima::Config config;
    proxima::Compiler compiler;
    
    // Parse options
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                config.outputPath = argv[++i];
            }
        } else if (arg == "-v" || arg == "--verbose") {
            if (i + 1 < argc) {
                compiler.setVerboseLevel(std::stoi(argv[++i]));
            }
        } else if (arg == "-O0") {
            config.optimizationLevel = 0;
        } else if (arg == "-O1") {
            config.optimizationLevel = 1;
        } else if (arg == "-O2") {
            config.optimizationLevel = 2;
        } else if (arg == "-O3") {
            config.optimizationLevel = 3;
        } else if (arg == "--debug") {
            config.debugSymbols = true;
        } else if (arg == "--release") {
            config.releaseMode = true;
        } else if (arg == "--cuda") {
            config.enableCUDA = true;
        } else if (arg == "--avx2") {
            config.enableAVX2 = true;
        } else if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg[0] != '-') {
            config.sourceFiles.push_back(arg);
        }
    }
    
    if (command == "build") {
        if (config.sourceFiles.empty()) {
            std::cerr << "Error: No source files specified\n";
            return 1;
        }
        
        for (const auto& file : config.sourceFiles) {
            auto result = compiler.compile(file, config);
            if (result.success) {
                std::cout << "✓ Compiled: " << file << " -> " << result.outputPath << std::endl;
            } else {
                std::cerr << "✗ Failed: " << file << " - " << result.errorMessage << std::endl;
                return 1;
            }
        }
        
        std::cout << "Build completed successfully!\n";
        return 0;
    }
    
    if (command == "run") {
        if (config.sourceFiles.empty()) {
            std::cerr << "Error: No source files specified\n";
            return 1;
        }
        
        auto result = compiler.compile(config.sourceFiles[0], config);
        if (!result.success) {
            std::cerr << "Compilation failed: " << result.errorMessage << std::endl;
            return 1;
        }
        
        // Run the compiled program
        // TODO: Implement runtime execution
        std::cout << "Running: " << result.outputPath << std::endl;
        return 0;
    }
    
    if (command == "clean") {
        std::cout << "Cleaning build artifacts...\n";
        // TODO: Implement clean
        return 0;
    }
    
    if (command == "test") {
        std::cout << "Running tests...\n";
        // TODO: Implement test framework
        return 0;
    }
    
    if (command == "time") {
        std::cout << "Benchmarking...\n";
        // TODO: Implement benchmarking
        return 0;
    }
    
    if (command == "deps") {
        std::cout << "Analyzing dependencies...\n";
        // TODO: Implement dependency analysis
        return 0;
    }
    
    if (command == "doc") {
        std::cout << "Generating documentation...\n";
        // TODO: Implement documentation generation
        return 0;
    }
    
    if (command == "help") {
        printHelp();
        return 0;
    }
    
    std::cerr << "Unknown command: " << command << std::endl;
    printHelp();
    return 1;
}
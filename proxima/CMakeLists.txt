cmake_minimum_required(VERSION 3.15)
project(Proxima VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_CURL "Enable CURL for LLM" OFF)
option(BUILD_IDE "Build Centauri IDE" OFF)

# Найти LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs Support Core Passes IR TransformUtils OrcJIT ExecutionEngine)

# Найти CURL (опционально)
if(ENABLE_CURL)
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIRS})
endif()

# Найти Qt (для IDE)
if(BUILD_IDE)
    find_package(Qt5 COMPONENTS Widgets Core Network REQUIRED)
endif()

# Исходные файлы компилятора
set(COMPILER_SOURCES
    src/main.cpp
    src/compiler/Compiler.cpp
    src/compiler/Config.cpp
    src/compiler/BuildRulesParser.cpp
    src/lexer/Lexer.cpp
    src/lexer/Token.cpp
    src/parser/Parser.cpp
    src/semantic/SemanticAnalyzer.cpp
    src/semantic/TypeChecker.cpp
    src/semantic/SymbolTable.cpp
    src/semantic/ArgumentValidator.cpp
    src/codegen/LLVMCodeGen.cpp
    src/codegen/Optimizer.cpp
    src/runtime/Runtime.cpp
    src/runtime/Debugger.cpp
    src/runtime/Interpreter.cpp
    src/runtime/Profiler.cpp
    src/runtime/REPL.cpp
    src/runtime/StoryManager.cpp
    src/runtime/gem.cpp
    src/stdlib/IO.cpp
    src/stdlib/Math.cpp
    src/stdlib/Collection.cpp
    src/stdlib/Time.cpp
    src/stdlib/StdLib.cpp
    src/utils/Logger.cpp
    src/utils/Protocol.cpp
    src/utils/DocGenerator.cpp
    src/utils/CollectionParser.cpp
    src/services/llm/LLMClient.cpp
    src/services/GitIntegration/GitIntegration.cpp
    src/core/AutoSave.cpp
)

set(COMPILER_HEADERS
    src/compiler/Compiler.h
    src/compiler/Config.h
    src/compiler/BuildRulesParser.h
    src/lexer/Lexer.h
    src/lexer/Token.h
    src/parser/Parser.h
    src/parser/AST.h
    src/semantic/SemanticAnalyzer.h
    src/semantic/TypeChecker.h
    src/semantic/SymbolTable.h
    src/semantic/ArgumentValidator.h
    src/codegen/LLVMCodeGen.h
    src/codegen/Optimizer.h
    src/runtime/Runtime.h
    src/runtime/Debugger.h
    src/runtime/Interpreter.h
    src/runtime/Profiler.h
    src/runtime/REPL.h
    src/runtime/StoryManager.h
    src/runtime/gem.h
    src/stdlib/IO.h
    src/stdlib/Collection.h
    src/stdlib/StdLib.h
    src/stdlib/Time.h
    src/utils/Logger.h
    src/utils/Protocol.h
    src/utils/DocGenerator.h
    src/utils/CollectionParser.h
    src/services/llm/LLMClient.h
    src/services/GitIntegration/GitIntegration.h
    src/core/AutoSave.h
)

# Создать исполняемый файл компилятора
add_executable(proxima ${COMPILER_SOURCES} ${COMPILER_HEADERS})

target_link_libraries(proxima ${llvm_libs})
if(ENABLE_CURL)
    target_link_libraries(proxima ${CURL_LIBRARIES})
    target_compile_definitions(proxima PRIVATE USE_CURL)
endif()
if(ENABLE_CUDA)
    target_compile_definitions(proxima PRIVATE USE_CUDA)
endif()

target_include_directories(proxima PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Тесты
if(BUILD_TESTS)
    enable_testing()

    add_executable(lexer_test tests/unit/lexer_test.cpp)
    target_link_libraries(lexer_test proxima)
    add_test(NAME LexerTest COMMAND lexer_test)

    add_executable(parser_test tests/unit/parser_test.cpp)
    target_link_libraries(parser_test proxima)
    add_test(NAME ParserTest COMMAND parser_test)

    add_executable(runtime_test tests/unit/runtime_tests.cpp)
    target_link_libraries(runtime_test proxima)
    add_test(NAME RuntimeTest COMMAND runtime_test)
endif()

# IDE Centauri
if(BUILD_IDE)
    add_subdirectory(ide)
endif()

# Установка
install(TARGETS proxima DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY examples/ DESTINATION share/proxima/examples)

# Пакетирование
set(CPACK_PACKAGE_NAME "Proxima")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Proxima Programming Language")
set(CPACK_PACKAGE_VENDOR "Proxima Development Team")
include(CPack)

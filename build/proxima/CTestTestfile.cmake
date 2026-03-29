# CMake generated Testfile for 
# Source directory: /workspace/proxima
# Build directory: /workspace/build/proxima
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(LexerTest "/workspace/build/proxima/lexer_test")
set_tests_properties(LexerTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/proxima/CMakeLists.txt;163;add_test;/workspace/proxima/CMakeLists.txt;0;")
add_test(ParserTest "/workspace/build/proxima/parser_test")
set_tests_properties(ParserTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/proxima/CMakeLists.txt;167;add_test;/workspace/proxima/CMakeLists.txt;0;")
add_test(RuntimeTest "/workspace/build/proxima/runtime_test")
set_tests_properties(RuntimeTest PROPERTIES  _BACKTRACE_TRIPLES "/workspace/proxima/CMakeLists.txt;171;add_test;/workspace/proxima/CMakeLists.txt;0;")

# Run compiler tests
cd proxima-compiler/build
ctest

# Run IDE tests
cd centauri-ide/build
ctest

# Run integration tests
./run-tests.sh

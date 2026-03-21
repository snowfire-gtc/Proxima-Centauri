#!/bin/bash

# =============================================================================
# Proxima & Centauri Integration Test Runner
# =============================================================================
# License: GPLv3
# Version: 1.0.0
# Description: Запускает интеграционные тесты для компилятора Proxima и IDE Centauri
# =============================================================================

set -e

# =============================================================================
# Configuration
# =============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build"
TEST_DIR="${PROJECT_ROOT}/tests"
TEST_OUTPUT_DIR="${BUILD_DIR}/test-output"
LOG_FILE="${TEST_OUTPUT_DIR}/test-run.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

# =============================================================================
# Helper Functions
# =============================================================================

print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${CYAN}ℹ${NC} $1"
}

log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> "$LOG_FILE"
}

check_dependency() {
    if ! command -v $1 &> /dev/null; then
        print_warning "$1 is not installed"
        return 1
    fi
    return 0
}

run_test_suite() {
    local suite_name=$1
    local suite_path=$2
    
    print_header "Running Test Suite: $suite_name"
    log "Starting test suite: $suite_name"
    
    if [ -f "$suite_path" ]; then
        bash "$suite_path"
        local exit_code=$?
        
        if [ $exit_code -eq 0 ]; then
            print_success "Suite $suite_name passed"
            log "Suite $suite_name passed"
            ((TESTS_PASSED++))
        else
            print_error "Suite $suite_name failed (exit code: $exit_code)"
            log "Suite $suite_name failed with exit code: $exit_code"
            ((TESTS_FAILED++))
        fi
    else
        print_warning "Test suite not found: $suite_path"
        log "Test suite not found: $suite_path"
        ((TESTS_SKIPPED++))
    fi
    
    ((TESTS_RUN++))
}

# =============================================================================
# Test Setup
# =============================================================================

setup_test_environment() {
    print_header "Setting Up Test Environment"
    
    # Create test output directory
    mkdir -p "$TEST_OUTPUT_DIR"
    
    # Initialize log file
    echo "Test Run Log - $(date)" > "$LOG_FILE"
    echo "================================" >> "$LOG_FILE"
    
    # Check required dependencies
    print_info "Checking dependencies..."
    
    if check_dependency "proxima"; then
        print_success "Proxima compiler found"
        log "Proxima compiler: $(proxima --version 2>&1 | head -n1)"
    else
        print_warning "Proxima compiler not in PATH, trying build directory..."
        if [ -f "${BUILD_DIR}/proxima" ]; then
            export PATH="${BUILD_DIR}:$PATH"
            print_success "Proxima compiler found in build directory"
        else
            print_error "Proxima compiler not found!"
            log "ERROR: Proxima compiler not found"
            return 1
        fi
    fi
    
    if check_dependency "centauri"; then
        print_success "Centauri IDE found"
        log "Centauri IDE: $(centauri --version 2>&1 | head -n1)"
    else
        print_warning "Centauri IDE not in PATH"
        log "WARNING: Centauri IDE not found in PATH"
    fi
    
    # Check Qt for IDE tests
    if check_dependency "qmake"; then
        print_success "Qt development tools found"
    else
        print_warning "Qt development tools not found - IDE tests may be limited"
    fi
    
    # Create test projects directory
    mkdir -p "${TEST_OUTPUT_DIR}/projects"
    
    print_success "Test environment setup complete"
    log "Test environment setup complete"
    
    return 0
}

cleanup_test_environment() {
    print_header "Cleaning Up Test Environment"
    
    # Keep test output for review
    print_info "Test output preserved in: $TEST_OUTPUT_DIR"
    
    log "Test cleanup complete"
}

# =============================================================================
# Test Suites
# =============================================================================

run_compiler_tests() {
    print_header "Compiler Integration Tests"
    
    local test_script="${TEST_DIR}/integration/compiler_tests.sh"
    
    if [ -f "$test_script" ]; then
        bash "$test_script"
        return $?
    else
        print_warning "Compiler tests not found"
        return 1
    fi
}

run_ide_tests() {
    print_header "IDE Integration Tests"
    
    local test_script="${TEST_DIR}/integration/ide_tests.sh"
    
    if [ -f "$test_script" ]; then
        bash "$test_script"
        return $?
    else
        print_warning "IDE tests not found"
        return 1
    fi
}

run_language_tests() {
    print_header "Language Feature Tests"
    
    local test_script="${TEST_DIR}/integration/language_tests.sh"
    
    if [ -f "$test_script" ]; then
        bash "$test_script"
        return $?
    else
        print_warning "Language tests not found"
        return 1
    fi
}

run_performance_tests() {
    print_header "Performance Tests"
    
    local test_script="${TEST_DIR}/integration/performance_tests.sh"
    
    if [ -f "$test_script" ]; then
        bash "$test_script"
        return $?
    else
        print_warning "Performance tests not found"
        return 1
    fi
}

run_collaboration_tests() {
    print_header "Collaboration Tests"
    
    local test_script="${TEST_DIR}/integration/collaboration_tests.sh"
    
    if [ -f "$test_script" ]; then
        bash "$test_script"
        return $?
    else
        print_warning "Collaboration tests not found"
        return 1
    fi
}

# =============================================================================
# Main Test Execution
# =============================================================================

run_all_tests() {
    print_header "Running All Integration Tests"
    
    local start_time=$(date +%s)
    
    # Run test suites
    run_compiler_tests
    run_ide_tests
    run_language_tests
    run_performance_tests
    run_collaboration_tests
    
    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    print_header "Test Summary"
    echo -e "Tests Run:     ${TESTS_RUN}"
    echo -e "Tests Passed:  ${GREEN}${TESTS_PASSED}${NC}"
    echo -e "Tests Failed:  ${RED}${TESTS_FAILED}${NC}"
    echo -e "Tests Skipped: ${YELLOW}${TESTS_SKIPPED}${NC}"
    echo -e "Duration:      ${duration} seconds"
    echo ""
    echo "Log file: $LOG_FILE"
    echo "Output directory: $TEST_OUTPUT_DIR"
    
    log "Test run complete: ${TESTS_PASSED}/${TESTS_RUN} passed, ${TESTS_FAILED} failed, ${TESTS_SKIPPED} skipped"
    
    if [ $TESTS_FAILED -gt 0 ]; then
        print_error "Some tests failed!"
        return 1
    else
        print_success "All tests passed!"
        return 0
    fi
}

# =============================================================================
# Command Line Interface
# =============================================================================

show_help() {
    echo "Proxima & Centauri Integration Test Runner"
    echo ""
    echo "Usage: $0 [options] [test_suite]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -v, --verbose       Enable verbose output"
    echo "  -q, --quiet         Suppress non-essential output"
    echo "  -c, --clean         Clean test output before running"
    echo "  -l, --list          List available test suites"
    echo "  -o, --output DIR    Set test output directory"
    echo ""
    echo "Test Suites:"
    echo "  all                 Run all test suites (default)"
    echo "  compiler            Run compiler integration tests"
    echo "  ide                 Run IDE integration tests"
    echo "  language            Run language feature tests"
    echo "  performance         Run performance tests"
    echo "  collaboration       Run collaboration tests"
    echo ""
    echo "Examples:"
    echo "  $0                  Run all tests"
    echo "  $0 compiler         Run only compiler tests"
    echo "  $0 -c all           Clean and run all tests"
    echo "  $0 --verbose all    Run all tests with verbose output"
}

list_test_suites() {
    print_header "Available Test Suites"
    echo "1. compiler      - Compiler integration tests"
    echo "2. ide           - IDE integration tests"
    echo "3. language      - Language feature tests"
    echo "4. performance   - Performance tests"
    echo "5. collaboration - Collaboration tests"
    echo "6. all           - Run all test suites"
}

# =============================================================================
# Main Entry Point
# =============================================================================

main() {
    local verbose=false
    local quiet=false
    local clean=false
    local test_suite="all"
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                verbose=true
                shift
                ;;
            -q|--quiet)
                quiet=true
                shift
                ;;
            -c|--clean)
                clean=true
                shift
                ;;
            -l|--list)
                list_test_suites
                exit 0
                ;;
            -o|--output)
                TEST_OUTPUT_DIR="$2"
                shift 2
                ;;
            all|compiler|ide|language|performance|collaboration)
                test_suite="$1"
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Clean if requested
    if [ "$clean" = true ]; then
        print_info "Cleaning test output directory..."
        rm -rf "$TEST_OUTPUT_DIR"
        mkdir -p "$TEST_OUTPUT_DIR"
    fi
    
    # Setup environment
    if ! setup_test_environment; then
        print_error "Failed to setup test environment"
        exit 1
    fi
    
    # Run tests
    case $test_suite in
        all)
            run_all_tests
            ;;
        compiler)
            run_compiler_tests
            ;;
        ide)
            run_ide_tests
            ;;
        language)
            run_language_tests
            ;;
        performance)
            run_performance_tests
            ;;
        collaboration)
            run_collaboration_tests
            ;;
        *)
            print_error "Unknown test suite: $test_suite"
            exit 1
            ;;
    esac
    
    local exit_code=$?
    
    # Cleanup
    cleanup_test_environment
    
    exit $exit_code
}

# Run main function
main "$@"
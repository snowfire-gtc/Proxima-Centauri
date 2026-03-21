#!/bin/bash

# =============================================================================
# Proxima Compiler Integration Tests
# =============================================================================
# License: GPLv3
# Description: Integration tests for Proxima compiler functionality
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_OUTPUT="${SCRIPT_DIR}/../../build/test-output/compiler"
PROXIMA_CMD="proxima"

# Test counters
PASSED=0
FAILED=0

# =============================================================================
# Helper Functions
# =============================================================================

run_test() {
    local test_name=$1
    local test_file=$2
    local expected_output=$3
    
    echo -n "  Testing $test_name... "
    
    if [ -f "$test_file" ]; then
        local output=$("$PROXIMA_CMD" run "$test_file" 2>&1)
        local exit_code=$?
        
        if [ $exit_code -eq 0 ]; then
            if [ -n "$expected_output" ]; then
                if echo "$output" | grep -q "$expected_output"; then
                    echo -e "${GREEN}PASS${NC}"
                    ((PASSED++))
                    return 0
                else
                    echo -e "${RED}FAIL${NC} (unexpected output)"
                    echo "    Expected: $expected_output"
                    echo "    Got: $output"
                    ((FAILED++))
                    return 1
                fi
            else
                echo -e "${GREEN}PASS${NC}"
                ((PASSED++))
                return 0
            fi
        else
            echo -e "${RED}FAIL${NC} (exit code: $exit_code)"
            echo "    Output: $output"
            ((FAILED++))
            return 1
        fi
    else
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        return 1
    fi
}

# =============================================================================
# Test Cases
# =============================================================================

test_hello_world() {
    echo "Test: Hello World"
    
    local test_file="${TEST_OUTPUT}/hello_world.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    print("Hello, Proxima!\n");
    return 0;
end

end
EOF
    
    run_test "hello_world" "$test_file" "Hello, Proxima!"
}

test_variables() {
    echo "Test: Variable Declarations"
    
    local test_file="${TEST_OUTPUT}/variables.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    // Auto type inference
    a = 100;
    b : int32 = 200;
    c : double = 3.14;
    d : string = "test";
    
    print("a=%d, b=%d, c=%.2f, d=%s\n", a, b, c, d);
    return 0;
end

end
EOF
    
    run_test "variables" "$test_file" "a=100, b=200"
}

test_functions() {
    echo "Test: Function Definitions"
    
    local test_file="${TEST_OUTPUT}/functions.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

add(a : int32, b : int32) : int32
    return a + b;
end

main() : int32
    result : int32 = add(10, 20);
    print("Result: %d\n", result);
    return 0;
end

end
EOF
    
    run_test "functions" "$test_file" "Result: 30"
}

test_classes() {
    echo "Test: Class Definitions"
    
    local test_file="${TEST_OUTPUT}/classes.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

class Counter
public:
    constructor()
        .count = 0;
    end
    
    increment() : void
        .count = .count + 1;
    end
    
    get_count() : int32
        return .count;
    end
    
protected:
    count : int32;
end

main() : int32
    c : Counter = Counter();
    c.increment();
    c.increment();
    print("Count: %d\n", c.get_count());
    return 0;
end

end
EOF
    
    run_test "classes" "$test_file" "Count: 2"
}

test_vectors() {
    echo "Test: Vector Operations"
    
    local test_file="${TEST_OUTPUT}/vectors.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    v : vector<int32> = [1, 2, 3, 4, 5];
    sum : int32 = 0;
    
    for i in 1:5
        sum = sum + v[i];
    end
    
    print("Sum: %d\n", sum);
    return 0;
end

end
EOF
    
    run_test "vectors" "$test_file" "Sum: 15"
}

test_matrices() {
    echo "Test: Matrix Operations"
    
    local test_file="${TEST_OUTPUT}/matrices.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    A : matrix<double> = [1, 2,, 3, 4];
    B : matrix<double> = eye(2);
    C : matrix<double> = A * B;
    
    print("Matrix test passed\n");
    return 0;
end

end
EOF
    
    run_test "matrices" "$test_file" "Matrix test passed"
}

test_conditionals() {
    echo "Test: Conditional Statements"
    
    local test_file="${TEST_OUTPUT}/conditionals.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    x : int32 = 10;
    
    if x > 5
        print("x > 5\n");
    else
        print("x <= 5\n");
    end
    
    return 0;
end

end
EOF
    
    run_test "conditionals" "$test_file" "x > 5"
}

test_loops() {
    echo "Test: Loop Statements"
    
    local test_file="${TEST_OUTPUT}/loops.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    sum : int32 = 0;
    
    for i in 1:5
        sum = sum + i;
    end
    
    print("Loop sum: %d\n", sum);
    return 0;
end

end
EOF
    
    run_test "loops" "$test_file" "Loop sum: 15"
}

test_time_api() {
    echo "Test: Time API"
    
    local test_file="${TEST_OUTPUT}/time_api.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    t : time = time.now();
    print("Time API works\n");
    return 0;
end

end
EOF
    
    run_test "time_api" "$test_file" "Time API works"
}

test_regions() {
    echo "Test: Code Regions"
    
    local test_file="${TEST_OUTPUT}/regions.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

#region Test Region
test_function() : int32
    return 42;
end
#endregion Test Region

main() : int32
    print("Regions work\n");
    return 0;
end

end
EOF
    
    run_test "regions" "$test_file" "Regions work"
}

# =============================================================================
# Main Execution
# =============================================================================

main() {
    echo "========================================"
    echo "Proxima Compiler Integration Tests"
    echo "========================================"
    echo ""
    
    # Create test output directory
    mkdir -p "$TEST_OUTPUT"
    
    # Run all tests
    test_hello_world
    test_variables
    test_functions
    test_classes
    test_vectors
    test_matrices
    test_conditionals
    test_loops
    test_time_api
    test_regions
    
    # Summary
    echo ""
    echo "========================================"
    echo "Test Summary"
    echo "========================================"
    echo "Passed: $PASSED"
    echo "Failed: $FAILED"
    echo ""
    
    if [ $FAILED -gt 0 ]; then
        exit 1
    else
        exit 0
    fi
}

main "$@"
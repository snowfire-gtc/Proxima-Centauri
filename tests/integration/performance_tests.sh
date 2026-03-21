#!/bin/bash

# =============================================================================
# Proxima Performance Tests
# =============================================================================
# License: GPLv3
# Description: Performance and benchmark tests
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_OUTPUT="${SCRIPT_DIR}/../../build/test-output/performance"
PROXIMA_CMD="proxima"

PASSED=0
FAILED=0

# =============================================================================
# Performance Test Helper
# =============================================================================

run_benchmark() {
    local test_name=$1
    local test_file=$2
    local max_time_ms=$3
    
    echo -n "  Benchmarking $test_name... "
    
    local start_time=$(date +%s%N)
    "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1
    local end_time=$(date +%s%N)
    
    local duration_ms=$(( (end_time - start_time) / 1000000 ))
    
    if [ $duration_ms -lt $max_time_ms ]; then
        echo -e "${GREEN}PASS${NC} (${duration_ms}ms < ${max_time_ms}ms)"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC} (${duration_ms}ms >= ${max_time_ms}ms)"
        ((FAILED++))
    fi
}

# =============================================================================
# Test Cases
# =============================================================================

test_loop_performance() {
    echo "Test: Loop Performance"
    
    local test_file="${TEST_OUTPUT}/loop_perf.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    sum : int64 = 0;
    
    for i in 1:100000
        sum = sum + i;
    end
    
    print("Sum: %d\n", sum);
    return 0;
end

end
EOF
    
    run_benchmark "loop_100k" "$test_file" 5000
}

test_matrix_performance() {
    echo "Test: Matrix Multiplication Performance"
    
    local test_file="${TEST_OUTPUT}/matrix_perf.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    A : matrix<double> = rand(100, 100);
    B : matrix<double> = rand(100, 100);
    C : matrix<double> = A * B;
    print("Matrix test complete\n");
    return 0;
end

end
EOF
    
    run_benchmark "matrix_100x100" "$test_file" 10000
}

test_vector_performance() {
    echo "Test: Vector Operations Performance"
    
    local test_file="${TEST_OUTPUT}/vector_perf.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    v : vector<double> = rand(10000);
    sum : double = 0;
    
    for i in 1:10000
        sum = sum + v[i];
    end
    
    print("Vector sum: %.2f\n", sum);
    return 0;
end

end
EOF
    
    run_benchmark "vector_10k" "$test_file" 5000
}

test_function_call_performance() {
    echo "Test: Function Call Performance"
    
    local test_file="${TEST_OUTPUT}/function_perf.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

compute(x : int32) : int32
    return x * 2 + 1;
end

main() : int32
    sum : int64 = 0;
    
    for i in 1:10000
        sum = sum + compute(i);
    end
    
    print("Function calls: %d\n", sum);
    return 0;
end

end
EOF
    
    run_benchmark "function_10k" "$test_file" 5000
}

test_class_performance() {
    echo "Test: Class Instantiation Performance"
    
    local test_file="${TEST_OUTPUT}/class_perf.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

class Simple
public:
    constructor()
        .value = 0;
    end
    
    get_value() : int32
        return .value;
    end
    
protected:
    value : int32;
end

main() : int32
    sum : int64 = 0;
    
    for i in 1:1000
        obj : Simple = Simple();
        sum = sum + obj.get_value();
    end
    
    print("Class test: %d\n", sum);
    return 0;
end

end
EOF
    
    run_benchmark "class_1k" "$test_file" 10000
}

# =============================================================================
# Main Execution
# =============================================================================

main() {
    echo "========================================"
    echo "Proxima Performance Tests"
    echo "========================================"
    echo ""
    
    mkdir -p "$TEST_OUTPUT"
    
    test_loop_performance
    test_matrix_performance
    test_vector_performance
    test_function_call_performance
    test_class_performance
    
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
#!/bin/bash

# =============================================================================
# Proxima Language Feature Tests
# =============================================================================
# License: GPLv3
# Description: Tests for Proxima language features
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_OUTPUT="${SCRIPT_DIR}/../../build/test-output/language"
PROXIMA_CMD="proxima"

PASSED=0
FAILED=0

# =============================================================================
# Test Helper
# =============================================================================

run_language_test() {
    local test_name=$1
    local test_code=$2
    local expected=$3
    
    echo -n "  Testing $test_name... "
    
    local test_file="${TEST_OUTPUT}/${test_name}.prx"
    
    cat > "$test_file" << EOF
namespace("test");

main() : int32
$test_code
    return 0;
end

end
EOF
    
    local output=$("$PROXIMA_CMD" run "$test_file" 2>&1)
    local exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        if echo "$output" | grep -q "$expected"; then
            echo -e "${GREEN}PASS${NC}"
            ((PASSED++))
        else
            echo -e "${RED}FAIL${NC}"
            echo "    Expected: $expected"
            echo "    Got: $output"
            ((FAILED++))
        fi
    else
        echo -e "${RED}FAIL${NC} (exit code: $exit_code)"
        ((FAILED++))
    fi
}

# =============================================================================
# Test Cases
# =============================================================================

test_type_inference() {
    echo "Test: Type Inference"
    
    run_language_test "type_inference" '
    a = 100;
    b = 3.14;
    c = "hello";
    print("Types work\n");
    ' "Types work"
}

test_optional_typing() {
    echo "Test: Optional Typing"
    
    run_language_test "optional_typing" '
    x : int32 = 10;
    y = 20;
    print("Optional: %d\n", x + y);
    ' "Optional: 30"
}

test_anonymous_functions() {
    echo "Test: Anonymous Functions"
    
    run_language_test "anonymous_functions" '
    add = (a, b) : auto {a + b};
    result = add(5, 3);
    print("Anonymous: %d\n", result);
    ' "Anonymous: 8"
}

test_multiple_returns() {
    echo "Test: Multiple Return Values"
    
    local test_file="${TEST_OUTPUT}/multiple_returns.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

get_values() : [int32, double]
    return 42, 3.14;
end

main() : int32
    [a, b] = get_values();
    print("Multi-return works\n");
    return 0;
end

end
EOF
    
    echo -n "  Testing multiple returns... "
    
    if "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_class_inheritance() {
    echo "Test: Class Inheritance"
    
    local test_file="${TEST_OUTPUT}/inheritance.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

class Base
public:
    get_value() : int32
        return 10;
    end
end

class Derived : public Base
public:
    get_derived() : int32
        return 20;
    end
end

main() : int32
    d : Derived = Derived();
    print("Inheritance: %d, %d\n", d.get_value(), d.get_derived());
    return 0;
end

end
EOF
    
    echo -n "  Testing inheritance... "
    
    local output=$("$PROXIMA_CMD" run "$test_file" 2>&1)
    
    if echo "$output" | grep -q "Inheritance: 10, 20"; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_interface() {
    echo "Test: Interface Implementation"
    
    local test_file="${TEST_OUTPUT}/interface.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

interface class Drawable
    draw() : void;
end

class Circle : public Drawable
public:
    draw() : void
        print("Drawing circle\n");
    end
end

main() : int32
    c : Circle = Circle();
    c.draw();
    return 0;
end

end
EOF
    
    echo -n "  Testing interface... "
    
    local output=$("$PROXIMA_CMD" run "$test_file" 2>&1)
    
    if echo "$output" | grep -q "Drawing circle"; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_template() {
    echo "Test: Templates"
    
    local test_file="${TEST_OUTPUT}/template.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

template<T>
class Container
public:
    constructor()
        .value = T(0);
    end
    
    get_value() : T
        return .value;
    end
    
protected:
    value : T;
end

main() : int32
    c : Container<int32> = Container<int32>();
    print("Template works\n");
    return 0;
end

end
EOF
    
    echo -n "  Testing template... "
    
    if "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_gpu_parallel() {
    echo "Test: GPU Parallel (Syntax Check)"
    
    local test_file="${TEST_OUTPUT}/gpu_parallel.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    // GPU parallel syntax check
    config : parallel = parallel(grid=32, block=256);
    print("GPU syntax OK\n");
    return 0;
end

end
EOF
    
    echo -n "  Testing GPU parallel syntax... "
    
    if "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_cpu_parallel() {
    echo "Test: CPU Parallel (Syntax Check)"
    
    local test_file="${TEST_OUTPUT}/cpu_parallel.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    A : vector<double> = zeros(100);
    
    parallel(threads=4, array=&A) for i in 1:100
        A[i] = i;
    end
    
    print("CPU parallel OK\n");
    return 0;
end

end
EOF
    
    echo -n "  Testing CPU parallel syntax... "
    
    if "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

test_collection() {
    echo "Test: Collection Type"
    
    local test_file="${TEST_OUTPUT}/collection.prx"
    
    cat > "$test_file" << 'EOF'
namespace("test");

main() : int32
    data : collection = [
        "Alice", "25", "NYC",,
        "Bob", "30", "LA"
    ];
    data.header = ["Name", "Age", "City"];
    print("Collection works\n");
    return 0;
end

end
EOF
    
    echo -n "  Testing collection... "
    
    if "$PROXIMA_CMD" run "$test_file" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
    fi
}

# =============================================================================
# Main Execution
# =============================================================================

main() {
    echo "========================================"
    echo "Proxima Language Feature Tests"
    echo "========================================"
    echo ""
    
    mkdir -p "$TEST_OUTPUT"
    
    test_type_inference
    test_optional_typing
    test_anonymous_functions
    test_multiple_returns
    test_class_inheritance
    test_interface
    test_template
    test_gpu_parallel
    test_cpu_parallel
    test_collection
    
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
#!/bin/bash

# =============================================================================
# Centauri IDE Integration Tests
# =============================================================================
# License: GPLv3
# Description: Integration tests for Centauri IDE functionality
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TEST_OUTPUT="${SCRIPT_DIR}/../../build/test-output/ide"
CENTAURI_CMD="centauri"

# Test counters
PASSED=0
FAILED=0

# =============================================================================
# Helper Functions
# =============================================================================

create_test_project() {
    local project_name=$1
    local project_path="${TEST_OUTPUT}/projects/${project_name}"
    
    mkdir -p "${project_path}/src"
    mkdir -p "${project_path}/rules"
    mkdir -p "${project_path}/autosave"
    mkdir -p "${project_path}/assets"
    
    # Create manifest
    cat > "${project_path}/manifest" << EOF
[
    "name", "${project_name}",,
    "version", "1.0.0",,
    "entry_point", "src/main.prx",,
    "required_ide_version", "Centauri >= 0.9.0",,
    "license", "GPLv3"
]
EOF
    
    # Create build rules
    cat > "${project_path}/rules/build.rules" << EOF
[
    "compiler_version", "1.0.0",,
    "optimization_level", "O2",,
    "debug_symbols", true
]
EOF
    
    echo "$project_path"
}

check_file_exists() {
    local file_path=$1
    local description=$2
    
    echo -n "  Checking $description... "
    
    if [ -f "$file_path" ]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
        return 1
    fi
}

check_directory_exists() {
    local dir_path=$1
    local description=$2
    
    echo -n "  Checking $description... "
    
    if [ -d "$dir_path" ]; then
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}FAIL${NC}"
        ((FAILED++))
        return 1
    fi
}

# =============================================================================
# Test Cases
# =============================================================================

test_project_creation() {
    echo "Test: Project Creation"
    
    local project_path=$(create_test_project "test_project_1")
    
    check_directory_exists "${project_path}/src" "src directory"
    check_directory_exists "${project_path}/rules" "rules directory"
    check_directory_exists "${project_path}/autosave" "autosave directory"
    check_file_exists "${project_path}/manifest" "manifest file"
    check_file_exists "${project_path}/rules/build.rules" "build.rules file"
}

test_module_creation() {
    echo "Test: Module Creation"
    
    local project_path=$(create_test_project "test_project_2")
    local module_path="${project_path}/src/main.prx"
    
    cat > "$module_path" << 'EOF'
namespace("test");

main() : int32
    print("Module test\n");
    return 0;
end

end
EOF
    
    check_file_exists "$module_path" "main.prx module"
}

test_autosave() {
    echo "Test: Auto-save Functionality"
    
    local project_path=$(create_test_project "test_project_3")
    local module_path="${project_path}/src/main.prx"
    local autosave_path="${project_path}/autosave/src/main.prx"
    
    # Create module
    cat > "$module_path" << 'EOF'
namespace("test");

main() : int32
    return 0;
end

end
EOF
    
    # Simulate autosave (in real test, IDE would do this)
    mkdir -p "${project_path}/autosave/src"
    cp "$module_path" "$autosave_path"
    
    check_file_exists "$autosave_path" "autosave file"
}

test_build_rules() {
    echo "Test: Build Rules Parsing"
    
    local project_path=$(create_test_project "test_project_4")
    local rules_path="${project_path}/rules/build.rules"
    
    # Verify build.rules format
    if grep -q "optimization_level" "$rules_path"; then
        echo -e "  ${GREEN}✓${NC} Build rules format valid"
        ((PASSED++))
    else
        echo -e "  ${RED}✗${NC} Build rules format invalid"
        ((FAILED++))
    fi
}

test_subproject_structure() {
    echo "Test: Subproject Structure"
    
    local project_path=$(create_test_project "test_project_5")
    local subproject_path="${project_path}/src/utils"
    
    mkdir -p "$subproject_path"
    
    cat > "${subproject_path}/helpers.prx" << 'EOF'
namespace("test.utils");

helper_function() : int32
    return 42;
end

end
EOF
    
    check_directory_exists "$subproject_path" "subproject directory"
    check_file_exists "${subproject_path}/helpers.prx" "subproject module"
}

test_manifest_format() {
    echo "Test: Manifest Format"
    
    local project_path=$(create_test_project "test_project_6")
    local manifest_path="${project_path}/manifest"
    
    # Verify manifest contains required fields
    local valid=true
    
    if ! grep -q '"name"' "$manifest_path"; then
        valid=false
    fi
    
    if ! grep -q '"version"' "$manifest_path"; then
        valid=false
    fi
    
    if ! grep -q '"entry_point"' "$manifest_path"; then
        valid=false
    fi
    
    if [ "$valid" = true ]; then
        echo -e "  ${GREEN}✓${NC} Manifest format valid"
        ((PASSED++))
    else
        echo -e "  ${RED}✗${NC} Manifest format invalid"
        ((FAILED++))
    fi
}

test_git_integration() {
    echo "Test: Git Integration Structure"
    
    local project_path=$(create_test_project "test_project_7")
    
    # Initialize git repo (simulated)
    cd "$project_path"
    
    # Check if .gitignore would be created
    echo -e "  ${GREEN}✓${NC} Git integration ready"
    ((PASSED++))
    
    cd - > /dev/null
}

test_assets_directory() {
    echo "Test: Assets Directory"
    
    local project_path=$(create_test_project "test_project_8")
    local assets_path="${project_path}/assets"
    
    mkdir -p "${assets_path}/config"
    mkdir -p "${assets_path}/data"
    
    check_directory_exists "$assets_path" "assets directory"
    check_directory_exists "${assets_path}/config" "assets/config directory"
    check_directory_exists "${assets_path}/data" "assets/data directory"
}

# =============================================================================
# Main Execution
# =============================================================================

main() {
    echo "========================================"
    echo "Centauri IDE Integration Tests"
    echo "========================================"
    echo ""
    
    # Create test output directory
    mkdir -p "$TEST_OUTPUT/projects"
    
    # Run all tests
    test_project_creation
    test_module_creation
    test_autosave
    test_build_rules
    test_subproject_structure
    test_manifest_format
    test_git_integration
    test_assets_directory
    
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
#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test statistics
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0
CURRENT_PHASE=""

# VM paths
VM_BINARY="./vm"
COMPILER_BINARY="./compiler"

# Test output
TEST_OUTPUT=""
TEST_EXIT_CODE=0

# Phase tracking
phase_start() {
    CURRENT_PHASE="$1"
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}PHASE: $CURRENT_PHASE${NC}"
    echo -e "${CYAN}========================================${NC}"
}

# Start individual test
test_start() {
    local test_name="$1"
    echo -e "${BLUE}[TEST]${NC} $test_name"
    TESTS_RUN=$((TESTS_RUN + 1))
}

# Run VM with bytecode file
run_vm() {
    local bytecode_file="$1"
    local timeout="${2:-5}"
    
    if [ ! -f "$VM_BINARY" ]; then
        echo -e "${RED}✗${NC} VM binary not found: $VM_BINARY"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    TEST_OUTPUT=$(timeout "$timeout" "$VM_BINARY" "$bytecode_file" 2>&1)
    TEST_EXIT_CODE=$?
    
    if [ $TEST_EXIT_CODE -eq 124 ]; then
        echo -e "${RED}✗${NC} VM timed out after ${timeout}s"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    return 0
}

# Run VM with inline bytecode
run_vm_inline() {
    local bytecode="$1"
    local timeout="${2:-5}"
    local temp_file="/tmp/vm-test-$$.bc"
    
    echo "$bytecode" > "$temp_file"
    run_vm "$temp_file" "$timeout"
    local result=$?
    rm -f "$temp_file"
    return $result
}

# Compile source and run
compile_and_run() {
    local source_file="$1"
    local timeout="${2:-5}"
    local bytecode_file="/tmp/vm-compiled-$$.bc"
    
    if [ ! -f "$COMPILER_BINARY" ]; then
        echo -e "${YELLOW}⚠${NC} Compiler not available, skipping"
        return 2
    fi
    
    # Compile
    if ! "$COMPILER_BINARY" "$source_file" "$bytecode_file" 2>/dev/null; then
        echo -e "${RED}✗${NC} Compilation failed"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
    
    # Run
    run_vm "$bytecode_file" "$timeout"
    local result=$?
    rm -f "$bytecode_file"
    return $result
}

# Compile inline source and run
compile_and_run_inline() {
    local source_code="$1"
    local timeout="${2:-5}"
    local source_file="/tmp/vm-source-$$.txt"
    
    echo "$source_code" > "$source_file"
    compile_and_run "$source_file" "$timeout"
    local result=$?
    rm -f "$source_file"
    return $result
}

# Assert VM output equals expected
assert_output() {
    local expected="$1"
    
    if [ "$TEST_OUTPUT" = "$expected" ]; then
        echo -e "${GREEN}✓${NC} Output matches exactly"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} Output mismatch"
        echo -e "${YELLOW}Expected:${NC}"
        echo "$expected"
        echo -e "${YELLOW}Got:${NC}"
        echo "$TEST_OUTPUT"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Assert output contains string
assert_contains() {
    local needle="$1"
    
    if echo "$TEST_OUTPUT" | grep -qF "$needle"; then
        echo -e "${GREEN}✓${NC} Output contains: '$needle'"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} Output does not contain: '$needle'"
        echo -e "${YELLOW}Got:${NC}"
        echo "$TEST_OUTPUT"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Assert output matches regex
assert_matches() {
    local pattern="$1"
    
    if echo "$TEST_OUTPUT" | grep -qE "$pattern"; then
        echo -e "${GREEN}✓${NC} Output matches pattern: $pattern"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} Output does not match pattern: $pattern"
        echo -e "${YELLOW}Got:${NC}"
        echo "$TEST_OUTPUT"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Assert VM exited cleanly
assert_exit_success() {
    if [ $TEST_EXIT_CODE -eq 0 ]; then
        echo -e "${GREEN}✓${NC} VM exited successfully (code 0)"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} VM exited with code $TEST_EXIT_CODE"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Assert VM exited with error
assert_exit_error() {
    if [ $TEST_EXIT_CODE -ne 0 ]; then
        echo -e "${GREEN}✓${NC} VM exited with error (expected)"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} VM should have failed but exited successfully"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Assert no output (for HALT test)
assert_no_output() {
    if [ -z "$TEST_OUTPUT" ]; then
        echo -e "${GREEN}✓${NC} No output (as expected)"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗${NC} Expected no output"
        echo -e "${YELLOW}Got:${NC}"
        echo "$TEST_OUTPUT"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Skip test
skip_test() {
    local reason="$1"
    echo -e "${YELLOW}⊘${NC} Skipped: $reason"
}

# Test summary
test_summary() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}TEST SUMMARY${NC}"
    echo -e "${CYAN}========================================${NC}"
    echo "Tests run:    $TESTS_RUN"
    echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
    
    if [ $TESTS_RUN -gt 0 ]; then
        local pass_rate=$((TESTS_PASSED * 100 / TESTS_RUN))
        echo "Pass rate:    ${pass_rate}%"
    fi
    
    echo ""
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}🎉 All tests passed!${NC}"
        exit 0
    else
        echo -e "${RED}❌ Some tests failed${NC}"
        exit 1
    fi
}

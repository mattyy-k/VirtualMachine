#!/bin/bash

set -e

source "$(dirname "$0")/lib/test-framework.sh"

# Build VM
build_vm() {
    echo -e "${BLUE}[BUILD]${NC} Compiling VM..."
    
    if [ -f "Makefile" ]; then
        make clean >/dev/null 2>&1 || true
        if make 2>&1; then
            echo -e "${GREEN}✓${NC} VM compiled successfully"
            return 0
        fi
    else
        # Try direct compilation
        if g++ -std=c++17 -o vm src/*.cpp 2>&1; then
            echo -e "${GREEN}✓${NC} VM compiled successfully"
            return 0
        fi
    fi
    
    echo -e "${RED}✗${NC} VM compilation failed"
    exit 1
}

# Build compiler (optional, for phase 2+)
build_compiler() {
    if [ -d "compiler" ] || [ -f "src/compiler.cpp" ]; then
        echo -e "${BLUE}[BUILD]${NC} Compiling compiler..."
        # Add your compiler build command here
        # g++ -std=c++17 -o compiler src/compiler.cpp
    fi
}

# Cleanup
cleanup() {
    rm -f /tmp/vm-test-* /tmp/vm-source-* /tmp/vm-compiled-*
}

# Run specific phase tests
run_phase() {
    local phase="$1"
    local phase_dir="tests/$phase"
    
    if [ ! -d "$phase_dir" ]; then
        echo -e "${YELLOW}Phase directory not found: $phase_dir${NC}"
        return
    fi
    
    phase_start "$phase"
    
    for test_file in "$phase_dir"/*.test; do
        if [ -f "$test_file" ]; then
            source "$test_file"
        fi
    done
}

# Main
main() {
    local filter="${1:-all}"
    
    echo -e "${CYAN}╔════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║     VM Test Suite Runner          ║${NC}"
    echo -e "${CYAN}╚════════════════════════════════════╝${NC}"
    echo ""
    
    # Build
    build_vm
    build_compiler
    
    # Setup
    cleanup
    trap cleanup EXIT
    
    echo ""
    
    # Run tests
    if [ "$filter" = "all" ]; then
        # Run all phases in order
        for phase in phase0 phase1 phase2 phase3 phase4 phase5 phase6 phase7; do
            if [ -d "tests/$phase" ] && [ -n "$(ls -A tests/$phase/*.test 2>/dev/null)" ]; then
                run_phase "$phase"
            fi
        done
    else
        # Run specific phase
        run_phase "$filter"
    fi
    
    # Summary
    test_summary
}

main "$@"

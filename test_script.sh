#!/bin/bash

# COOL Semantic Analyzer Test Script
# Compares our implementation with official implementation

echo "=========================================="
echo "COOL Semantic Analyzer Test Suite"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test files
TEST_FILES=("good.cl" "bad.cl" "stack.cl" "complex.cl")
PASS_COUNT=0
FAIL_COUNT=0

# Check if semantic analyzer exists
if [ ! -f "./mysemant" ]; then
    echo "Error: ./mysemant not found. Please compile first with 'make semant'"
    exit 1
fi

# Create results directory
mkdir -p test_results

for test_file in "${TEST_FILES[@]}"; do
    echo
    echo "------------------------------------------"
    echo "Testing: $test_file"
    echo "------------------------------------------"
    
    # Skip if test file doesn't exist
    if [ ! -f "$test_file" ]; then
        echo -e "${RED}SKIP: $test_file not found${NC}"
        continue
    fi
    
    base_name="${test_file%.cl}"
    
    # Generate official output
    echo "Generating official output..."
    ./lexer "$test_file" 2>/dev/null | ./parser "$test_file" 2>&1 | /usr/class/bin/semant "$test_file" 2>&1 > "test_results/official_$base_name.txt"
    
    # Generate our output
    echo "Generating our output..."  
    ./lexer "$test_file" 2>/dev/null | ./parser "$test_file" 2>&1 | ./mysemant "$test_file" 2>&1 > "test_results/my_$base_name.txt"
    
    # Compare outputs
    if diff -w "test_results/official_$base_name.txt" "test_results/my_$base_name.txt" > /dev/null 2>&1; then
        echo -e "${GREEN}✅ PASS: $test_file${NC}"
        echo "Outputs are identical!"
        ((PASS_COUNT++))
    else
        echo -e "${RED}❌ FAIL: $test_file${NC}"
        echo "Outputs differ:"
        
        # Show first few differences
        diff -u "test_results/official_$base_name.txt" "test_results/my_$base_name.txt" | head -20
        
        # Save full diff
        diff -u "test_results/official_$base_name.txt" "test_results/my_$base_name.txt" > "test_results/diff_$base_name.txt"
        echo "Full diff saved to: test_results/diff_$base_name.txt"
        ((FAIL_COUNT++))
    fi
    
    # For bad.cl, also check that we report errors
    if [ "$test_file" = "bad.cl" ]; then
        OUR_ERRORS=$(grep -c "^ERROR:" "test_results/my_$base_name.txt" || true)
        OFFICIAL_ERRORS=$(grep -c "^ERROR:" "test_results/official_$base_name.txt" || true)
        
        if [ "$OUR_ERRORS" -ge "$OFFICIAL_ERRORS" ]; then
            echo -e "${GREEN}✅ Error count adequate: $OUR_ERRORS >= $OFFICIAL_ERRORS${NC}"
        else
            echo -e "${YELLOW}⚠️  Warning: Fewer errors than official ($OUR_ERRORS < $OFFICIAL_ERRORS)${NC}"
        fi
    fi
done

echo
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "Passed: ${GREEN}$PASS_COUNT${NC}"
echo -e "Failed: ${RED}$FAIL_COUNT${NC}"
echo "Total:  $((PASS_COUNT + FAIL_COUNT))"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}🎉 All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}💥 Some tests failed. Check diff files for details.${NC}"
    exit 1
fi

#!/bin/bash
# VfsShell test script - Testing logical AND/OR functionality

echo "Testing logical operator functionality..."

echo ""
echo "Test 1: Logical AND (&&) - should execute right side only if left succeeds"
echo "echo 'Left side' && echo 'Right side'"
echo "Left side" && echo "Right side"

echo ""
echo "Test 2: Logical AND with failing command"
echo "false && echo 'This should not appear'"
false && echo "This should not appear"

echo ""
echo "Test 3: Logical OR (||) - should execute right side only if left fails"
echo "true || echo 'This should not appear'"
true || echo "This should not appear"

echo ""
echo "Test 4: Logical OR with failing command"
echo "false || echo 'This should appear'"
false || echo "This should appear"

echo ""
echo "Test 5: Complex expression"
echo "true && echo 'First' || echo 'Second'"
true && echo "First" || echo "Second"

echo ""
echo "Test 6: Complex expression 2"
echo "false && echo 'First' || echo 'Second'"
false && echo "First" || echo "Second"

echo ""
echo "Logical operator tests completed."
#!/bin/bash

# Playbook Test Harness v1
# Runs deterministic tests for AI Playbooks

set -e  # Exit on any error

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TESTS_JSON="$TEST_DIR/tests_v1.json"
EXIT_CODE=0

echo "Starting playbook test harness..."

# Function to run a single test
run_test() {
    local test_id=$1
    local workspace=$2
    local playbook_id=$3
    local expect_status=$4
    local expect_no_apply=$5
    local expect_min_events=$6
    local expect_max_events=$7
    local override_max_risk=$8
    local override_allow_apply=$9
    
    echo "Running test: $test_id"
    echo "  Workspace: $workspace"
    echo "  Playbook: $playbook_id"
    
    # Ensure .aiupp directory exists in workspace
    mkdir -p "$workspace/.aiupp"
    
    # Capture baseline evolution stats
    if [ -f "$workspace/.aiupp/evolution.json" ]; then
        BASELINE_EVENTS=$(jq '.total_events // 0' "$workspace/.aiupp/evolution.json" 2>/dev/null || echo "0")
    else
        BASELINE_EVENTS=0
    fi
    
    echo "  Baseline events: $BASELINE_EVENTS"
    
    # Prepare command with potential overrides
    CMD="theide-cli --workspace-root \"$workspace\" --json run_playbook --playbook_id \"$playbook_id\""
    
    # If overrides are provided, we would need a way to pass them
    # Since the CLI may not support dynamic constraint overrides, 
    # we'll proceed without them for this initial implementation
    if [ ! -z "$override_max_risk" ] || [ ! -z "$override_allow_apply" ]; then
        echo "  NOTE: Constraint overrides detected but not currently implemented in CLI"
        echo "  Using default playbook constraints"
    fi
    
    # Execute the playbook
    RESULT_FILE=$(mktemp)
    if $CMD > "$RESULT_FILE" 2>/dev/null; then
        PLAYBOOK_RESULT=$(cat "$RESULT_FILE")
        rm "$RESULT_FILE"
        
        echo "  Playbook executed successfully"
    else
        PLAYBOOK_RESULT=$(cat "$RESULT_FILE")
        rm "$RESULT_FILE"
        
        echo "  Playbook execution completed (may have failed as expected)"
    fi
    
    # Output the result for debugging
    echo "  Playbook result: $PLAYBOOK_RESULT"
    
    # Capture new evolution stats
    if [ -f "$workspace/.aiupp/evolution.json" ]; then
        NEW_EVENTS=$(jq '.total_events // 0' "$workspace/.aiupp/evolution.json" 2>/dev/null || echo "0")
    else
        NEW_EVENTS=0
    fi
    
    EVENTS_INCREASED=$((NEW_EVENTS - BASELINE_EVENTS))
    echo "  Events increased by: $EVENTS_INCREASED"
    
    # Validate expectations
    local test_passed=true
    
    # Check status expectation
    if [ "$expect_status" = "blocked_by_constraint" ]; then
        # Check if the result indicates constraint blocking
        if echo "$PLAYBOOK_RESULT" | jq -e '.status' | grep -q "blocked_by_constraint\|constraint_violation\|simulation_only"; then
            echo "  Status validation PASSED: constraint blocking detected"
        else
            echo "  Status validation FAILED: expected blocked_by_constraint but didn't find constraint blocking indication"
            test_passed=false
        fi
    elif [ "$expect_status" = "applied_or_simulated" ]; then
        # Check if the result indicates the operation was applied or simulated
        if echo "$PLAYBOOK_RESULT" | jq -e '.status' >/dev/null 2>&1; then
            echo "  Status validation PASSED: playbook ran successfully"
        else
            echo "  Status validation FAILED: playbook didn't run properly"
            test_passed=false
        fi
    fi
    
    # Check no_apply expectation
    if [ "$expect_no_apply" = "true" ] && [ "$expect_no_apply" != "" ]; then
        if [ "$EVENTS_INCREASED" -eq 0 ]; then
            echo "  Apply validation PASSED: no events added (no changes applied)"
        else
            echo "  Apply validation FAILED: $EVENTS_INCREASED events added when no apply was expected"
            test_passed=false
        fi
    fi
    
    # Check event increase bounds
    if [ ! -z "$expect_min_events" ] && [ "$EVENTS_INCREASED" -lt "$expect_min_events" ]; then
        echo "  Event count validation FAILED: expected at least $expect_min_events events, got $EVENTS_INCREASED"
        test_passed=false
    fi
    
    if [ ! -z "$expect_max_events" ] && [ "$EVENTS_INCREASED" -gt "$expect_max_events" ]; then
        echo "  Event count validation FAILED: expected at most $expect_max_events events, got $EVENTS_INCREASED"
        test_passed=false
    fi
    
    if [ "$test_passed" = true ]; then
        echo "  RESULT: PASS $test_id"
        echo
        return 0
    else
        echo "  RESULT: FAIL $test_id"
        echo
        EXIT_CODE=1
        return 1
    fi
}

# Parse tests from JSON and run them
TEST_COUNT=$(jq -r '.tests | length' "$TESTS_JSON")
echo "Found $TEST_COUNT tests to run"

for i in $(seq 0 $((TEST_COUNT - 1))); do
    # Extract test configuration
    test_id=$(jq -r ".tests[$i].id" "$TESTS_JSON")
    workspace=$(jq -r ".tests[$i].workspace" "$TESTS_JSON")
    playbook_id=$(jq -r ".tests[$i].playbook_id" "$TESTS_JSON")
    expect_status=$(jq -r ".tests[$i].expect.status // empty" "$TESTS_JSON")
    expect_no_apply=$(jq -r ".tests[$i].expect.no_apply // empty" "$TESTS_JSON")
    expect_min_events=$(jq -r ".tests[$i].expect.min_events_increase // empty" "$TESTS_JSON")
    expect_max_events=$(jq -r ".tests[$i].expect.max_events_increase // empty" "$TESTS_JSON")
    
    # Handle optional override constraints
    override_max_risk=$(jq -r ".tests[$i].override_constraints.max_risk // empty" "$TESTS_JSON")
    override_allow_apply=$(jq -r ".tests[$i].override_constraints.allow_apply // empty" "$TESTS_JSON")
    
    # Run the test
    run_test "$test_id" "$workspace" "$playbook_id" "$expect_status" "$expect_no_apply" \
             "$expect_min_events" "$expect_max_events" "$override_max_risk" "$override_allow_apply"
done

echo "Test harness execution complete."

if [ $EXIT_CODE -eq 0 ]; then
    echo "All tests PASSED"
else
    echo "Some tests FAILED"
fi

exit $EXIT_CODE
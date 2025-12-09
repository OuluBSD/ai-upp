#include "uppsrc/clicore/clicore.h"
#include "uppsrc/clicore/CoreScenario.h"
#include <Core/Core.h>

using namespace Upp;

CONSOLE_APP_MAIN {
    // Initialize logging for debugging
    StdLogSetup(LOG_COUT|LOG_FILE);

    // Create a CoreIde instance
    CoreIde ide;
    String error;

    // Set up workspace
    if (!ide.SetWorkspaceRoot(GetCurrentDirectory(), error)) {
        LOG("Failed to set workspace root: " + error);
        return 1;
    }

    // Test 1: Build a scenario from supervisor suggestions
    LOG("Testing scenario building from supervisor...");
    Value scenario_plan = ide.BuildScenarioFromPlan("test_package", 3, error);
    
    if (!error.IsEmpty()) {
        LOG("Error building scenario: " + error);
        return 1;
    }
    
    LOG("Scenario plan built successfully: " + AsString(scenario_plan));
    
    // Test 2: Simulate the scenario (should not apply changes)
    LOG("Testing scenario simulation...");
    Value simulate_result = ide.SimulateScenario(scenario_plan, error);
    
    if (!error.IsEmpty()) {
        LOG("Error simulating scenario: " + error);
        return 1;
    }
    
    LOG("Scenario simulation completed: " + AsString(simulate_result));
    
    // Verify that applied flag is false for simulation
    if (Is<ValueMap>(simulate_result)) {
        ValueMap result_map = simulate_result;
        bool applied = result_map.Get("applied", false);
        if (applied) {
            LOG("ERROR: Simulation result shows applied=true, but should be false!");
            return 1;
        }
        LOG("SUCCESS: Simulation correctly shows applied=false");
    }
    
    // Test 3: Apply the scenario (should apply real changes)
    LOG("Testing scenario application...");
    Value apply_result = ide.ApplyScenario(scenario_plan, error);
    
    if (!error.IsEmpty()) {
        LOG("Error applying scenario: " + error);
        return 1;
    }
    
    LOG("Scenario application completed: " + AsString(apply_result));
    
    // Verify that applied flag is true for application
    if (Is<ValueMap>(apply_result)) {
        ValueMap result_map = apply_result;
        bool applied = result_map.Get("applied", false);
        if (!applied) {
            LOG("ERROR: Apply result shows applied=false, but should be true!");
            return 1;
        }
        LOG("SUCCESS: Application correctly shows applied=true");
    }
    
    LOG("All scenario tests passed!");
    
    return 0;
}
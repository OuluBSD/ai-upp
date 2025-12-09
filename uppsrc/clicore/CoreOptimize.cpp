#include "clicore.h"
#include "CoreOptimize.h"
#include "CoreIde.h"
#include "CoreRefactor.h"
#include "CoreTelemetry.h"

CoreOptimize::CoreOptimize() {
}

CoreOptimize::LoopResult CoreOptimize::RunOptimizationLoop(
    const String& package,
    const LoopConfig& config,
    CoreIde& ide,
    String& error
) {
    LoopResult result;
    result.success = false;
    
    // Get initial telemetry
    Value initial_telemetry = ide.GetPackageTelemetry(package, error);
    if (!error.IsEmpty()) {
        result.reason = "Failed to get initial telemetry: " + error;
        return result;
    }
    
    double initial_score = ComputeScore(initial_telemetry);
    double current_score = initial_score;
    
    for (int i = 0; i < config.max_iterations; i++) {
        LoopIteration iteration;
        iteration.index = i;
        
        // Store before state
        iteration.before_stats = ide.GetPackageTelemetry(package, error);
        if (!error.IsEmpty()) {
            result.reason = "Failed to get before telemetry: " + error;
            return result;
        }
        
        // Store current state for change detection
        CoreIde ide_before = ide.Clone();
        
        // Perform built-in refactor actions
        // For v1: remove dead includes, canonicalize includes
        bool changes_made = false;
        
        // Remove dead includes
        VectorMap<String, Vector<String>> dead_includes = ide.GetDeadIncludes(package, error);
        if (!error.IsEmpty()) {
            result.reason = "Failed to get dead includes: " + error;
            return result;
        }
        
        int removed_includes = 0;
        for (const auto& [file, includes] : dead_includes) {
            for (const auto& include : includes) {
                ide.RemoveInclude(file, include);
                removed_includes++;
            }
        }
        
        if (removed_includes > 0) {
            changes_made = true;
        }
        
        // Canonicalize includes (normalize format and order)
        ide.CanonicalizeIncludes(package, error);
        if (!error.IsEmpty()) {
            result.reason = "Failed to canonicalize includes: " + error;
            return result;
        }
        
        // Get after state
        iteration.after_stats = ide.GetPackageTelemetry(package, error);
        if (!error.IsEmpty()) {
            result.reason = "Failed to get after telemetry: " + error;
            return result;
        }
        
        // Detect changes
        iteration.changes = DetectChanges(ide_before, ide);
        
        // Compute scores
        double before_score = ComputeScore(iteration.before_stats);
        double after_score = ComputeScore(iteration.after_stats);
        iteration.score_delta = after_score - before_score;
        
        // Check termination conditions
        if (config.stop_on_worse && iteration.score_delta > 0) {
            // Score worsened
            result.reason = "regression";
            result.iterations.Add(iteration);
            return result;
        }
        
        if (config.stop_on_converge && fabs(iteration.score_delta) < config.converge_threshold) {
            // Converged
            result.reason = "converged";
            result.iterations.Add(iteration);
            result.success = true;
            return result;
        }
        
        if (!changes_made) {
            // No changes made
            result.reason = "no changes";
            result.iterations.Add(iteration);
            result.success = true;
            return result;
        }
        
        // Add iteration to results
        result.iterations.Add(iteration);
        current_score = after_score;
    }
    
    result.reason = "max iterations reached";
    result.success = true;
    return result;
}

double CoreOptimize::ComputeScore(const Value& telemetry) {
    // v1 heuristic:
    // lower complexity, fewer includes, fewer cycles, smaller average file size → better
    
    double score = 0.0;
    
    // Assuming telemetry is a ValueMap with various metrics
    if (telemetry.Is<ValueMap>()) {
        ValueMap tm = telemetry;
        
        // Get complexity metrics (these would need to be available in telemetry)
        double complexity = tm.Get("average_complexity", 0.0);
        double include_count = tm.Get("total_includes", 0.0);
        double cycle_count = tm.Get("dependency_cycles", 0.0);
        double avg_file_size = tm.Get("average_file_size", 0.0);
        double dead_includes = tm.Get("dead_includes", 0.0);
        
        // Calculate score - lower is better (negative score means better)
        // Weight different factors appropriately
        score = -0.1 * complexity + 
                -0.3 * include_count + 
                -0.4 * cycle_count + 
                -0.2 * avg_file_size +
                -0.5 * dead_includes;
    }
    
    return score;
}

Value CoreOptimize::DetectChanges(const CoreIde& ide_before, const CoreIde& ide_after) {
    ValueMap changes;
    
    // Compare various metrics to detect changes
    // For now, we'll focus on includes that were removed
    
    // This would typically involve comparing the file contents before and after
    // and counting differences like removed includes, changed lines, etc.
    
    int removed_includes_count = 0; // This would need to be computed
    
    changes.Set("removed_includes", removed_includes_count);
    changes.Set("description", "Includes removed and canonicalized");
    
    return changes;
}
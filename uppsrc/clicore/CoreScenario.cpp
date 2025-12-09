#include "clicore.h"
#include "CoreScenario.h"
#include <Core/Core.h>
#include <algorithm>
#include <vector>

struct FileSnapshot : Moveable<FileSnapshot> {
    String path;
    String content;
};

Vector<FileSnapshot> CaptureWorkspaceState(CoreIde& ide) {
    Vector<FileSnapshot> files;

    // Get all packages in the workspace
    const VectorMap<String, CoreWorkspace::Package>& packages = ide.GetWorkspace().GetPackages();

    for(int i = 0; i < packages.GetCount(); i++) {
        const CoreWorkspace::Package& pkg = packages[i];

        // Add all files from this package
        for(int j = 0; j < pkg.files.GetCount(); j++) {
            String filepath = pkg.path + "/" + pkg.files[j];

            // Load file content
            String content, error;
            if(ide.GetFileOps().OpenFile(filepath, content, error)) {
                FileSnapshot snapshot;
                snapshot.path = filepath;
                snapshot.content = content;
                files.Add(snapshot);
            }
        }
    }

    return files;
}

String GenerateUnifiedDiff(const Vector<FileSnapshot>& before,
                           const Vector<FileSnapshot>& after,
                           ValueArray& out_file_changes) {
    String diff;

    // Create a mapping from path to content for before and after states
    Index<String> all_paths;

    for(const auto& file : before) {
        all_paths.Add(file.path);
    }

    for(const auto& file : after) {
        if(all_paths.Find(file.path) < 0) {
            all_paths.Add(file.path);
        }
    }

    // Process each file to generate diff hunks
    for(int i = 0; i < all_paths.GetCount(); i++) {
        String path = all_paths[i];

        // Find before and after contents for this path
        const String* before_content = nullptr;
        const String* after_content = nullptr;

        for(const auto& file : before) {
            if(file.path == path) {
                before_content = &file.content;
                break;
            }
        }

        for(const auto& file : after) {
            if(file.path == path) {
                after_content = &file.content;
                break;
            }
        }

        // Determine if file was added, removed, or modified
        if(before_content == nullptr && after_content != nullptr) {
            // File was added
            diff += "+++ " + path + "\n";
            diff += "@@ -0,0 +1," + String().Cat() << after_content->GetCount() << " @@\n";

            // Add all lines
            Vector<String> after_lines = Split(*after_content, "\n");
            for(const auto& line : after_lines) {
                diff += "+" + line + "\n";
            }

            // Add to file changes
            ValueMap change;
            change.Set("path", path);
            change.Set("added_lines", after_lines.GetCount());
            change.Set("removed_lines", 0);
            change.Set("modified", true);
            out_file_changes.Add(change);
        }
        else if(before_content != nullptr && after_content == nullptr) {
            // File was removed
            Vector<String> before_lines = Split(*before_content, "\n");
            diff += "--- " + path + "\n";
            diff += "@@ -1," + String().Cat() << before_lines.GetCount() << " +0,0 @@\n";

            // Remove all lines
            for(const auto& line : before_lines) {
                diff += "-" + line + "\n";
            }

            // Add to file changes
            ValueMap change;
            change.Set("path", path);
            change.Set("added_lines", 0);
            change.Set("removed_lines", before_lines.GetCount());
            change.Set("modified", true);
            out_file_changes.Add(change);
        }
        else if(before_content != nullptr && after_content != nullptr) {
            // File was modified - use line-based comparison
            Vector<String> before_lines = Split(*before_content, "\n");
            Vector<String> after_lines = Split(*after_content, "\n");

            // A simple line-based diff algorithm for demonstration
            int added_lines = 0, removed_lines = 0;
            String file_diff;
            bool has_changes = false;

            // For simplicity, we'll just compare the contents
            if(*before_content != *after_content) {
                has_changes = true;
                file_diff += "--- " + path + "\n";
                file_diff += "+++ " + path + "\n";

                // Find common prefix and suffix to create a proper diff hunk
                int prefix_len = 0, suffix_len = 0;

                // Calculate prefix
                int min_len = min(before_lines.GetCount(), after_lines.GetCount());
                while(prefix_len < min_len && before_lines[prefix_len] == after_lines[prefix_len]) {
                    prefix_len++;
                }

                // Calculate suffix
                while(suffix_len < min_len - prefix_len &&
                      before_lines.GetCount() - suffix_len - 1 >= prefix_len &&
                      after_lines.GetCount() - suffix_len - 1 >= prefix_len &&
                      before_lines.GetCount() - suffix_len - 1 < before_lines.GetCount() &&
                      after_lines.GetCount() - suffix_len - 1 < after_lines.GetCount() &&
                      before_lines[before_lines.GetCount() - suffix_len - 1] == after_lines[after_lines.GetCount() - suffix_len - 1]) {
                    suffix_len++;
                }

                // Create diff hunk
                int before_start = prefix_len + 1;
                int before_count = before_lines.GetCount() - prefix_len - suffix_len;
                int after_start = prefix_len + 1;
                int after_count = after_lines.GetCount() - prefix_len - suffix_len;

                file_diff += "@@ -" + String().Cat() << before_start << "," << before_count << " +" << after_start << "," << after_count << " @@\n";

                // Output context lines
                for(int j = 0; j < prefix_len; j++) {
                    file_diff += " " + before_lines[j] + "\n";
                }

                // Output removed lines
                for(int j = prefix_len; j < before_lines.GetCount() - suffix_len; j++) {
                    file_diff += "-" + before_lines[j] + "\n";
                    removed_lines++;
                }

                // Output added lines
                for(int j = prefix_len; j < after_lines.GetCount() - suffix_len; j++) {
                    file_diff += "+" + after_lines[j] + "\n";
                    added_lines++;
                }

                // Output suffix context lines
                for(int j = after_lines.GetCount() - suffix_len; j < after_lines.GetCount(); j++) {
                    if(j < before_lines.GetCount()) {
                        file_diff += " " + before_lines[before_lines.GetCount() - suffix_len + (j - (after_lines.GetCount() - suffix_len))] + "\n";
                    } else {
                        file_diff += " " + after_lines[j] + "\n";
                    }
                }

                diff += file_diff;
            }

            if(has_changes) {
                ValueMap change;
                change.Set("path", path);
                change.Set("added_lines", added_lines);
                change.Set("removed_lines", removed_lines);
                change.Set("modified", true);
                out_file_changes.Add(change);
            } else {
                // File unchanged but still recorded
                ValueMap change;
                change.Set("path", path);
                change.Set("added_lines", 0);
                change.Set("removed_lines", 0);
                change.Set("modified", false);
                out_file_changes.Add(change);
            }
        }
    }

    return diff;
}

CoreScenario::CoreScenario() {
}

CoreScenario::ScenarioPlan CoreScenario::BuildPlanFromSupervisor(
    const Value& sup_plan_value,
    int max_actions) {

    ScenarioPlan plan;
    plan.name = "supervisor_generated_plan";

    // The incoming sup_plan_value is a Value (likely a ValueMap) containing the plan data
    // Convert from ValueMap to ScenarioPlan
    if (Is<ValueMap>(sup_plan_value)) {
        const ValueMap& plan_map = ~sup_plan_value;

        // Set plan name if available
        if (plan_map.Find("summary") >= 0) {
            plan.name = plan_map.Get("summary", String("supervisor_generated_plan"));
        }

        // Extract steps from the plan
        if (plan_map.Find("steps") >= 0) {
            ValueArray steps = plan_map.Get("steps");
            int action_count = 0;

            for (int i = 0; i < steps.GetCount() && action_count < max_actions; i++) {
                if (Is<ValueMap>(steps[i])) {
                    const ValueMap& step = ~steps[i];

                    ScenarioAction action;
                    action.type = step.Get("action", String("command"));
                    action.target = step.Get("target", String(""));
                    action.params = step.Get("params", ValueMap());

                    plan.actions.Add(action);
                    action_count++;
                }
            }
        }
    }

    return plan;
}

CoreScenario::ScenarioResult CoreScenario::Simulate(
    const ScenarioPlan& plan,
    CoreIde& ide,
    String& error) {

    ScenarioResult result;
    result.plan = plan;
    result.before = Snapshot(ide);
    result.applied = false;  // Simulation only

    // In simulation, we estimate the impact without making real changes
    // This would involve heuristics to estimate what would happen
    result.after = result.before;  // Placeholder - in real implementation, this would be adjusted

    // Apply heuristic adjustments based on the plan
    for(const auto& action : plan.actions) {
        if(action.type == "playbook") {
            // Estimate the impact of running a playbook
            // For example, if it's a cleanup playbook, complexity might decrease
            if(action.target.Find("cleanup") >= 0) {
                if(Is<ValueMap>(result.after.telemetry)) {
                    ValueMap& telemetry = result.after.telemetry;
                    if(telemetry.Find("complexity_index") >= 0) {
                        double current_complexity = telemetry.Get("complexity_index");
                        // Estimate complexity reduction
                        telemetry.Set("complexity_index", current_complexity * 0.9); // 10% reduction estimate
                    }
                }
            }
        }
        else if(action.type == "refactor") {
            // Estimate the impact of a refactoring action
            if(action.target == "rename_function" || action.target == "rename_class") {
                if(Is<ValueMap>(result.after.architecture)) {
                    ValueMap& arch = result.after.architecture;
                    if(arch.Find("scores") >= 0) {
                        ValueMap scores = arch.Get("scores");
                        if(scores.Find("cohesion") >= 0) {
                            double current_cohesion = scores.Get("cohesion");
                            // Estimate cohesion improvement
                            scores.Set("cohesion", current_cohesion * 1.1); // 10% improvement estimate
                            arch.Set("scores", scores);
                        }
                    }
                }
            }
        }
        else if(action.type == "command") {
            // Estimate the impact of running a command
            if(action.target == "optimize_package") {
                if(Is<ValueMap>(result.after.telemetry)) {
                    ValueMap& telemetry = result.after.telemetry;
                    if(telemetry.Find("complexity_index") >= 0) {
                        double current_complexity = telemetry.Get("complexity_index");
                        // Estimate complexity reduction from optimization
                        telemetry.Set("complexity_index", current_complexity * 0.85); // 15% reduction estimate
                    }

                    if(telemetry.Find("lines_of_code") >= 0) {
                        int current_loc = telemetry.Get("lines_of_code");
                        telemetry.Set("lines_of_code", int(current_loc * 0.95)); // 5% reduction estimate
                    }
                }
            }
        }
    }

    result.deltas = ComputeDeltas(result.before, result.after);

    // For v1, simulation doesn't compute a true diff (since no real edits)
    // We can set a placeholder for the unified_diff
    result.unified_diff = "simulation_only";  // Placeholder for simulation

    return result;
}

CoreScenario::ScenarioResult CoreScenario::Apply(
    const ScenarioPlan& plan,
    CoreIde& ide,
    String& error) {

    ScenarioResult result;
    result.plan = plan;
    result.before = Snapshot(ide);

    // Capture workspace state before applying changes
    Vector<FileSnapshot> before_files = CaptureWorkspaceState(ide);

    result.applied = true;

    // Actually apply the changes through CoreIde
    for(const auto& action : plan.actions) {
        if(action.type == "playbook") {
            // Execute playbook through CoreIde
            ValueMap params = action.params;
            params.Set("playbook_name", action.target);

            Value playbook_result = ide.RunPlaybook(params, error);
            if(!error.IsEmpty()) {
                return result; // Return early on error
            }
        }
        else if(action.type == "refactor") {
            // Execute refactoring through CoreIde
            ValueMap params = action.params;
            params.Set("refactor_type", action.target);

            Value refactor_result = ide.ExecuteRefactor(params, error);
            if(!error.IsEmpty()) {
                return result; // Return early on error
            }
        }
        else if(action.type == "command") {
            // Execute command through CoreIde
            ValueMap params = action.params;
            params.Set("command", action.target);

            Value command_result = ide.ExecuteCommand(params, error);
            if(!error.IsEmpty()) {
                return result; // Return early on error
            }
        }
    }

    // Take a snapshot after applying changes
    result.after = Snapshot(ide);
    result.deltas = ComputeDeltas(result.before, result.after);

    // Capture workspace state after applying changes
    Vector<FileSnapshot> after_files = CaptureWorkspaceState(ide);

    // Generate unified diff between before and after states
    result.unified_diff = GenerateUnifiedDiff(before_files, after_files, result.file_changes);

    return result;
}

CoreScenario::ScenarioResult CoreScenario::Revert(const String& patch_text,
                                                  CoreIde& ide,
                                                  String& error) {
    ScenarioResult result;
    result.plan.name = "revert";
    result.applied = true;

    // Capture the state before revert
    result.before = Snapshot(ide);
    Vector<FileSnapshot> before_files = CaptureWorkspaceState(ide);

    // Parse the patch and apply the inverse changes
    Vector<String> lines = Split(patch_text, "\n");
    String current_file_path;
    int line_idx = 0;

    while(line_idx < lines.GetCount()) {
        String line = lines[line_idx];

        if(line.StartsWith("--- ")) {
            // This is the old file path line
            String old_path = line.Mid(4);  // Remove "--- " prefix
            if(lines[line_idx + 1].StartsWith("+++ ")) {
                current_file_path = lines[line_idx + 1].Mid(4);  // Remove "+++ " prefix
                line_idx += 2;  // Skip both --- and +++ lines
            } else {
                error = "Invalid patch format: expected '+++ ' line after '--- ' line";
                return result;
            }
        }
        else if(line.StartsWith("@@ -") && current_file_path.GetCount() > 0) {
            // This is a hunk header: @@ -start,count +start,count @@
            // We need to parse this to understand the line ranges, but for now
            // we'll just find the next hunk or end of header

            // Skip hunk header line
            line_idx++;

            // Process hunk content
            while(line_idx < lines.GetCount()) {
                String hunk_line = lines[line_idx++];

                if(hunk_line.StartsWith("--- ") || hunk_line.StartsWith("++ ")) {
                    // This marks the beginning of a new file or hunk
                    line_idx--; // Go back to reprocess this line
                    break;
                }

                if(hunk_line.StartsWith("+")) {
                    // This line was added in the original diff, so we remove it in revert
                    // For now, we'll just mark this as a potential issue since we can't just remove
                    // a line without context
                    String line_content = hunk_line.Mid(1); // Remove the '+' prefix

                    // Read the current file content
                    String current_content, file_error;
                    if(!ide.GetFileOps().OpenFile(current_file_path, current_content, file_error)) {
                        error = "Could not read file " + current_file_path + ": " + file_error;
                        return result;
                    }

                    // Find and remove this line from the current file content
                    Vector<String> file_lines = Split(current_content, "\n");
                    Vector<String> new_file_lines;

                    bool removed = false;
                    for(auto& file_line : file_lines) {
                        if(!removed && file_line == line_content) {
                            removed = true; // Skip this line (remove it)
                        } else {
                            new_file_lines.Add(file_line);
                        }
                    }

                    if(removed) {
                        // Write the modified content back to the file
                        String new_content = Join(new_file_lines, "\n");
                        if(!ide.GetFileOps().SaveFile(current_file_path, new_content, file_error)) {
                            error = "Could not save file " + current_file_path + ": " + file_error;
                            return result;
                        }
                    } else {
                        error = "Could not find line to remove in revert: " + line_content;
                        return result;
                    }
                }
                else if(hunk_line.StartsWith("-")) {
                    // This line was removed in the original diff, so we add it back in revert
                    String line_content = hunk_line.Mid(1); // Remove the '-' prefix

                    // For now, we'll append to the file - a more sophisticated approach
                    // would require tracking the original position from the hunk header

                    // Read the current file content
                    String current_content, file_error;
                    if(!ide.GetFileOps().OpenFile(current_file_path, current_content, file_error)) {
                        error = "Could not read file " + current_file_path + ": " + file_error;
                        return result;
                    }

                    // Add the line back to the file
                    String new_content = current_content + "\n" + line_content;
                    if(!ide.GetFileOps().SaveFile(current_file_path, new_content, file_error)) {
                        error = "Could not save file " + current_file_path + ": " + file_error;
                        return result;
                    }
                }
                else if(hunk_line.StartsWith(" ")) {
                    // Context line - no action needed, just continue
                    continue;
                }
            }
        }
        else {
            line_idx++;
        }
    }

    // Capture the state after revert
    result.after = Snapshot(ide);

    // Generate the diff that represents this revert operation
    Vector<FileSnapshot> after_files = CaptureWorkspaceState(ide);
    result.unified_diff = GenerateUnifiedDiff(before_files, after_files, result.file_changes);

    return result;
}

CoreScenario::ScenarioSnapshot CoreScenario::Snapshot(CoreIde& ide) const {
    ScenarioSnapshot snapshot;
    
    // Get telemetry from the current state
    snapshot.telemetry = ide.GetTelemetryData();
    
    // Get semantic analysis data
    snapshot.semantic = ide.GetSemanticData();
    
    // Get architectural metrics
    snapshot.architecture = ide.GetArchitectureData();
    
    // Get behavioral data (if applicable)
    snapshot.behavior = ide.GetBehavioralData();
    
    // Calculate overall score based on metrics
    snapshot.score = ide.CalculateOverallScore(snapshot.telemetry, 
                                              snapshot.semantic, 
                                              snapshot.architecture,
                                              snapshot.behavior);
    
    return snapshot;
}

ValueMap CoreScenario::ComputeDeltas(const ScenarioSnapshot& before,
                                     const ScenarioSnapshot& after) const {
    ValueMap deltas;
    
    // Compute deltas for telemetry
    if(Is<ValueMap>(before.telemetry) && Is<ValueMap>(after.telemetry)) {
        const ValueMap& bt = before.telemetry;
        const ValueMap& at = after.telemetry;
        
        for(int i = 0; i < bt.GetCount(); i++) {
            String key = bt.GetKey(i);
            Value b_val = bt[i];
            if(at.Find(key) >= 0) {
                Value a_val = at[key];
                
                if(Is<double>(b_val) && Is<double>(a_val)) {
                    deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if(Is<int>(b_val) && Is<int>(a_val)) {
                    deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    deltas.Set(key, a_val); // Just store the new value if types don't match for subtraction
                }
            }
        }
    }
    
    // Compute deltas for semantic data
    if(Is<ValueMap>(before.semantic) && Is<ValueMap>(after.semantic)) {
        const ValueMap& bs = before.semantic;
        const ValueMap& as = after.semantic;
        
        for(int i = 0; i < bs.GetCount(); i++) {
            String key = bs.GetKey(i);
            Value b_val = bs[i];
            if(as.Find(key) >= 0) {
                Value a_val = as[key];
                
                if(Is<double>(b_val) && Is<double>(a_val)) {
                    deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if(Is<int>(b_val) && Is<int>(a_val)) {
                    deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    deltas.Set(key, a_val);
                }
            }
        }
    }
    
    // Compute deltas for architecture
    if(Is<ValueMap>(before.architecture) && Is<ValueMap>(after.architecture)) {
        const ValueMap& ba = before.architecture;
        const ValueMap& aa = after.architecture;
        
        for(int i = 0; i < ba.GetCount(); i++) {
            String key = ba.GetKey(i);
            Value b_val = ba[i];
            if(aa.Find(key) >= 0) {
                Value a_val = aa[key];
                
                if(Is<double>(b_val) && Is<double>(a_val)) {
                    deltas.Set(key, As<double>(a_val) - As<double>(b_val));
                } else if(Is<int>(b_val) && Is<int>(a_val)) {
                    deltas.Set(key, As<int>(a_val) - As<int>(b_val));
                } else {
                    deltas.Set(key, a_val);
                }
            }
        }
    }
    
    // Compute score delta
    deltas.Set("score", after.score - before.score);
    
    return deltas;
}
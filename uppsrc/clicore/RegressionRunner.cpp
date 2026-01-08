#include "RegressionRunner.h"
#include <Core/Util.h>
#include <Core/Json.h>
#include <Core/File.h>
#include <Core/Process.h>

NAMESPACE_UPP

RegressionRunner::RegressionRunner() 
    : workspace_temp_dir(GetSystemFolder(SYS_TEMP) + "/ai-upp-regression")
    , report_output_dir("./reports")
{
    // Ensure directories exist
    ForceDirectory(workspace_temp_dir);
    ForceDirectory(report_output_dir);
}

RegressionRunner::~RegressionRunner() {
    // Clean up temporary workspace
    // Note: Implement proper cleanup based on configuration
}

bool RegressionRunner::InitializeWorkspace(const String& source_path, const String& target_path) {
    try {
        // Clear target directory if it exists
        if (DirectoryExists(target_path)) {
            // For now, we'll remove and recreate the directory
            // In practice, you might want a more sophisticated approach
            DeleteFolderDeep(target_path);
        }
        
        // Create target directory
        if (!ForceDirectory(target_path)) {
            LOG("Failed to create target directory: " + target_path);
            return false;
        }
        
        // Copy all files from source to target
        // This is a simplified implementation; in practice you might want 
        // to use git clone or similar for version control
        Vector<String> files = ReadDirs(source_path, "*.cpp;*.h;*.hpp;*.cc;*.cxx;*.c;*.txt;*.json");
        
        for (const String& file : files) {
            String src_file = AppendFileName(source_path, file);
            String dst_file = AppendFileName(target_path, file);
            
            String dst_dir = GetFileFolder(dst_file);
            if (!ForceDirectory(dst_dir)) {
                LOG("Failed to create directory: " + dst_dir);
                continue;
            }
            
            if (!CopyFile(src_file, dst_file)) {
                LOG("Failed to copy file: " + src_file + " to " + dst_file);
                return false;
            }
        }
        
        LOG("Workspace initialized from " + source_path + " to " + target_path);
        return true;
    } catch (...) {
        LOG("Exception occurred while initializing workspace");
        return false;
    }
}

RegressionRunner::RegressionResult RegressionRunner::RunSpec(const RegressionSpec& spec) {
    RegressionResult result;
    result.spec_name = spec.GetAgentName();
    result.agent_name = spec.GetAgentName();
    
    try {
        // Setup temporary workspace for this test
        String temp_workspace = workspace_temp_dir + "/" + spec.GetAgentName() + "_" + 
                                String().Cat() << Upp::GetTickCount();
        if (!InitializeWorkspace(spec.GetWorkspaceRoot(), temp_workspace)) {
            result.error_msg = "Failed to initialize workspace";
            result.success = false;
            return result;
        }
        
        // Capture metrics before agent run
        ValueMap before_metrics;
        // Placeholder for actual metric collection
        
        // Prepare prompt for the agent
        String prompt = "Execute playbook '" + spec.GetPlaybookOrScenario() + 
                       "' in the provided workspace. Max actions: " + 
                       String().Cat() << spec.GetMaxActions() + 
                       ". Allowed risk level: " + String().Cat() << spec.GetAllowedRisk();
        
        // Invoke the MCP agent (stub implementation for v1)
        String agent_output;
        if (!InvokeMCPAgent(spec.GetAgentName(), prompt, agent_output)) {
            result.error_msg = "Agent invocation failed";
            result.success = false;
            return result;
        }
        
        // Capture metrics after agent run
        ValueMap after_metrics;
        // Placeholder for actual metric collection
        
        // Compute deltas
        result.metrics = ComputeDeltas(before_metrics, after_metrics);
        
        // Evaluate outcome
        result.success = true; // For now, assume success
        // In a real implementation, this would check various conditions
        
        // Generate report
        result.patch_summary.Add("size", agent_output.GetLength());
        result.patch_summary.Add("changed_files", 1); // Placeholder
        
        LOG("Regression spec '" + spec.GetAgentName() + "' completed successfully");
    } catch (...) {
        result.error_msg = "Exception occurred during regression run";
        result.success = false;
        LOG("Exception occurred during regression run for spec: " + spec.GetAgentName());
    }
    
    return result;
}

bool RegressionRunner::EvaluateOutcome(const RegressionResult& result, const RegressionSpec& spec) {
    // Check if the agent stayed within allowed risk
    if (result.metrics.Find("risk_delta") >= 0) {
        double risk_delta = result.metrics.Get("risk_delta").GetDouble();
        if (risk_delta > spec.GetAllowedRisk()) {
            LOG("Risk threshold exceeded: " + String().Cat() << risk_delta + 
                " > " + String().Cat() << spec.GetAllowedRisk());
            return false;
        }
    }
    
    // Check for violations
    if (result.violations.GetCount() > 0) {
        LOG("Violations detected: " + String().Join(result.violations, ", "));
        return false;
    }
    
    // Check for conflicts
    if (result.conflicts.GetCount() > 0) {
        LOG("Conflicts detected: " + String().Join(result.conflicts, ", "));
        return false;
    }
    
    // More evaluation criteria can be added here
    
    return true;
}

ValueMap RegressionRunner::ComputeDeltas(const ValueMap& before_metrics, const ValueMap& after_metrics) {
    ValueMap deltas;
    
    for (int i = 0; i < after_metrics.GetCount(); i++) {
        String key = after_metrics.GetKey(i);
        Value after_val = after_metrics.Get(key);
        int before_idx = before_metrics.Find(key);
        
        if (before_idx >= 0) {
            Value before_val = before_metrics.Get(key);
            
            // Calculate difference based on value type
            if (before_val.IsInt() && after_val.IsInt()) {
                deltas.Add(key, after_val.GetInt() - before_val.GetInt());
            } else if (before_val.IsDouble() && after_val.IsDouble()) {
                deltas.Add(key, after_val.GetDouble() - before_val.GetDouble());
            } else if (before_val.IsString() || after_val.IsString()) {
                // For strings, just store the new value as change
                deltas.Add(key, after_val);
            } else {
                // For other types, just store the after value
                deltas.Add(key, after_val);
            }
        } else {
            // New metric added
            deltas.Add(key, after_val);
        }
    }
    
    // For metrics that existed before but not after (removed metrics)
    for (int i = 0; i < before_metrics.GetCount(); i++) {
        String key = before_metrics.GetKey(i);
        if (after_metrics.Find(key) < 0) {
            deltas.Add(key, -before_metrics.Get(key)); // Indicate removal with negative value
        }
    }
    
    return deltas;
}

bool RegressionRunner::InvokeMCPAgent(const String& agent_identity, const String& prompt, String& output) {
    try {
        // This is a stub implementation for v1
        // Real agent invocation happens in v2 via MCP server
        
        // For testing purposes, we'll simulate a response
        output = "Agent " + agent_identity + " processed command: " + prompt + "\n";
        output += "Simulated actions performed:\n";
        output += "- Analyzed code structure\n";
        output += "- Applied requested changes\n";
        output += "- Verified code correctness\n";
        output += "- Generated metrics report\n";
        
        // In the actual implementation, this would call the MCP server
        // Example (to be implemented in v2):
        // return CallMCPServer(agent_identity, prompt, output);
        
        return true;
    } catch (...) {
        LOG("Exception occurred while invoking MCP agent: " + agent_identity);
        return false;
    }
}

bool RegressionRunner::SaveReport(const RegressionResult& result, const String& path) {
    try {
        ValueMap report;
        report.Add("spec", result.spec_name);
        report.Add("agent", result.agent_name);
        report.Add("success", result.success);
        report.Add("metrics", result.metrics);
        report.Add("patch_summary", result.patch_summary);
        report.Add("violations", result.violations);
        report.Add("conflicts", result.conflicts);
        report.Add("score", result.score);
        
        if (!result.error_msg.IsEmpty()) {
            report.Add("error", result.error_msg);
        }
        
        String json_str = AsJSON(report);
        return SaveFile(path, json_str);
    } catch (...) {
        LOG("Exception occurred while saving regression report");
        return false;
    }
}

String RegressionRunner::GenerateMarkdownReport(const RegressionResult& result) {
    String md;
    md << "# Regression Report\n\n";
    md << "**Spec:** " << result.spec_name << "\n";
    md << "**Agent:** " << result.agent_name << "\n";
    md << "**Success:** " << (result.success ? "Yes" : "No") << "\n\n";
    
    if (!result.error_msg.IsEmpty()) {
        md << "## Error\n";
        md << result.error_msg << "\n\n";
    }
    
    md << "## Metrics\n";
    for (int i = 0; i < result.metrics.GetCount(); i++) {
        md << "- **" << result.metrics.GetKey(i) << "**: " << result.metrics.Get(i) << "\n";
    }
    
    md << "\n## Patch Summary\n";
    for (int i = 0; i < result.patch_summary.GetCount(); i++) {
        md << "- **" << result.patch_summary.GetKey(i) << "**: " << result.patch_summary.Get(i) << "\n";
    }
    
    if (result.violations.GetCount() > 0) {
        md << "\n## Violations\n";
        for (const String& violation : result.violations) {
            md << "- " << violation << "\n";
        }
    }
    
    if (result.conflicts.GetCount() > 0) {
        md << "\n## Conflicts\n";
        for (const String& conflict : result.conflicts) {
            md << "- " << conflict << "\n";
        }
    }
    
    return md;
}

RegressionRunner& RegressionRunner::SetWorkspaceTempDir(const String& path) {
    workspace_temp_dir = path;
    return *this;
}

RegressionRunner& RegressionRunner::SetReportOutputDir(const String& path) {
    report_output_dir = path;
    return *this;
}

// Future compatibility hooks (placeholders for Task 44+)
void RegressionRunner::SetAgentPersonalityProfile(const ValueMap& profile) {
    // Placeholder for v2: agent personality profiles
}

ValueMap RegressionRunner::GetAgentPersonalityProfile() const {
    // Placeholder for v2: agent personality profiles
    return ValueMap();
}

void RegressionRunner::SetHistoricalComparisonEnabled(bool enabled) {
    // Placeholder for v2: historical comparison over time
}

bool RegressionRunner::IsHistoricalComparisonEnabled() const {
    // Placeholder for v2: historical comparison over time
    return false;
}

void RegressionRunner::EnableTournamentMode() {
    // Placeholder for v2: AI tournament mode
}

bool RegressionRunner::IsTournamentModeEnabled() const {
    // Placeholder for v2: AI tournament mode
    return false;
}

void RegressionRunner::EnableMultiAgentNegotiation() {
    // Placeholder for v2: multi-agent negotiation regression
}

bool RegressionRunner::IsMultiAgentNegotiationEnabled() const {
    // Placeholder for v2: multi-agent negotiation regression
    return false;
}

END_UPP_NAMESPACE
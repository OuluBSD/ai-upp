#ifndef REGRESSIONRUNNER_H
#define REGRESSIONRUNNER_H

#include <Core/Core.h>
#include "RegressionSpec.h"
#include <Core/Util.h>

NAMESPACE_UPP

class RegressionRunner {
public:
    struct RegressionResult {
        String spec_name;
        String agent_name;
        bool success = false;
        ValueMap metrics;
        ValueMap patch_summary;
        Array<String> violations;
        Array<String> conflicts;
        ValueMap score;
        String error_msg;
    };
    
private:
    String workspace_temp_dir;
    String report_output_dir;

public:
    RegressionRunner();
    ~RegressionRunner();

    bool InitializeWorkspace(const String& source_path, const String& target_path);
    RegressionResult RunSpec(const RegressionSpec& spec);
    bool EvaluateOutcome(const RegressionResult& result, const RegressionSpec& spec);
    ValueMap ComputeDeltas(const ValueMap& before_metrics, const ValueMap& after_metrics);
    
    // This is a stub implementation for v1 - real agent invocation happens in v2
    bool InvokeMCPAgent(const String& agent_identity, const String& prompt, String& output);
    
    bool SaveReport(const RegressionResult& result, const String& path);
    String GenerateMarkdownReport(const RegressionResult& result);
    
    RegressionRunner& SetWorkspaceTempDir(const String& path);
    RegressionRunner& SetReportOutputDir(const String& path);
    
    const String& GetWorkspaceTempDir() const { return workspace_temp_dir; }
    const String& GetReportOutputDir() const { return report_output_dir; }

    // Future compatibility hooks (placeholders for Task 44+)
    void SetAgentPersonalityProfile(const ValueMap& profile);  // Agent personality profiles
    ValueMap GetAgentPersonalityProfile() const;
    void SetHistoricalComparisonEnabled(bool enabled);  // Historical comparison over time
    bool IsHistoricalComparisonEnabled() const;
    void EnableTournamentMode();  // AI tournament mode
    bool IsTournamentModeEnabled() const;
    void EnableMultiAgentNegotiation();  // Multi-agent negotiation regression
    bool IsMultiAgentNegotiationEnabled() const;
};

END_UPP_NAMESPACE

#endif
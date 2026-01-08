#ifndef REGRESSIONSPEC_H
#define REGRESSIONSPEC_H

#include <Core/Core.h>

NAMESPACE_UPP

class RegressionSpec {
public:
    String agent_name;
    String workspace_root;
    String playbook_or_scenario;
    double allowed_risk;
    int max_actions;
    ValueMap comparison_baselines;

public:
    RegressionSpec();
    RegressionSpec(const String& agent_name, const String& workspace_root, const String& playbook_or_scenario);
    RegressionSpec& SetAgentName(const String& name);
    RegressionSpec& SetWorkspaceRoot(const String& root);
    RegressionSpec& SetPlaybookOrScenario(const String& scenario);
    RegressionSpec& SetAllowedRisk(double risk);
    RegressionSpec& SetMaxActions(int actions);
    RegressionSpec& SetComparisonBaselines(const ValueMap& baselines);

    const String& GetAgentName() const { return agent_name; }
    const String& GetWorkspaceRoot() const { return workspace_root; }
    const String& GetPlaybookOrScenario() const { return playbook_or_scenario; }
    double GetAllowedRisk() const { return allowed_risk; }
    int GetMaxActions() const { return max_actions; }
    const ValueMap& GetComparisonBaselines() const { return comparison_baselines; }

    bool LoadFromFile(const String& path);
    bool SaveToFile(const String& path);
};

class RegressionSuite {
private:
    VectorMap<String, RegressionSpec> specs;

public:
    RegressionSuite();
    
    RegressionSuite& LoadSpecs(const String& path);
    Vector<String> ListSpecs() const;
    bool HasSpec(const String& name) const;
    const RegressionSpec& GetSpec(const String& name) const;
    void AddSpec(const String& name, const RegressionSpec& spec);
    void RemoveSpec(const String& name);
    
    static RegressionSuite& Single(); // Singleton for global access
};

// Future compatibility: Placeholders for additional functionality
class RegressionConfig {
public:
    // Agent personality profiles
    ValueMap agent_personality_profiles;

    // Historical comparison settings
    bool historical_comparison_enabled = false;

    // AI tournament mode settings
    bool tournament_mode_enabled = false;

    // Multi-agent negotiation settings
    bool multi_agent_negotiation_enabled = false;

    // Additional placeholder settings can be added here
};

END_UPP_NAMESPACE

#endif
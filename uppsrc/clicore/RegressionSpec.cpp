#include "RegressionSpec.h"
#include <Core/Util.h>
#include <Core/Json.h>

NAMESPACE_UPP

// RegressionSpec implementation
RegressionSpec::RegressionSpec()
    : agent_name("Unknown")
    , workspace_root(".")
    , playbook_or_scenario("")
    , allowed_risk(0.0)
    , max_actions(100)
{
}

RegressionSpec::RegressionSpec(const String& agent_name, const String& workspace_root, const String& playbook_or_scenario)
    : agent_name(agent_name)
    , workspace_root(workspace_root)
    , playbook_or_scenario(playbook_or_scenario)
    , allowed_risk(0.5)
    , max_actions(100)
{
}

RegressionSpec& RegressionSpec::SetAgentName(const String& name) {
    agent_name = name;
    return *this;
}

RegressionSpec& RegressionSpec::SetWorkspaceRoot(const String& root) {
    workspace_root = root;
    return *this;
}

RegressionSpec& RegressionSpec::SetPlaybookOrScenario(const String& scenario) {
    playbook_or_scenario = scenario;
    return *this;
}

RegressionSpec& RegressionSpec::SetAllowedRisk(double risk) {
    allowed_risk = risk;
    return *this;
}

RegressionSpec& RegressionSpec::SetMaxActions(int actions) {
    max_actions = actions;
    return *this;
}

RegressionSpec& RegressionSpec::SetComparisonBaselines(const ValueMap& baselines) {
    comparison_baselines = baselines;
    return *this;
}

bool RegressionSpec::LoadFromFile(const String& path) {
    try {
        String content = LoadFile(path);
        if (content.IsEmpty()) {
            LOG("Failed to load regression spec from file: " + path);
            return false;
        }
        
        Value json_value = ParseJSON(content);
        if (!json_value.Is<ValueMap>()) {
            LOG("Invalid JSON format in regression spec file: " + path);
            return false;
        }
        
        ValueMap map = json_value;
        if (map.Find("agent_name") >= 0) agent_name = map.Get("agent_name");
        if (map.Find("workspace_root") >= 0) workspace_root = map.Get("workspace_root");
        if (map.Find("playbook_or_scenario") >= 0) playbook_or_scenario = map.Get("playbook_or_scenario");
        if (map.Find("allowed_risk") >= 0) allowed_risk = map.Get("allowed_risk").GetDouble();
        if (map.Find("max_actions") >= 0) max_actions = map.Get("max_actions").GetInt();
        if (map.Find("comparison_baselines") >= 0) comparison_baselines = map.Get("comparison_baselines");
        
        return true;
    } catch (...) {
        LOG("Exception occurred while loading regression spec from file: " + path);
        return false;
    }
}

bool RegressionSpec::SaveToFile(const String& path) {
    try {
        ValueMap map;
        map.Add("agent_name", agent_name);
        map.Add("workspace_root", workspace_root);
        map.Add("playbook_or_scenario", playbook_or_scenario);
        map.Add("allowed_risk", allowed_risk);
        map.Add("max_actions", max_actions);
        map.Add("comparison_baselines", comparison_baselines);
        
        String json_str = AsJSON(map);
        return SaveFile(path, json_str);
    } catch (...) {
        LOG("Exception occurred while saving regression spec to file: " + path);
        return false;
    }
}

// RegressionSuite implementation
RegressionSuite::RegressionSuite() {
}

RegressionSuite& RegressionSuite::LoadSpecs(const String& path) {
    try {
        String content = LoadFile(path);
        if (content.IsEmpty()) {
            LOG("Failed to load regression suite from file: " + path);
            return *this;
        }
        
        Value json_value = ParseJSON(content);
        if (!json_value.Is<Array>()) {
            LOG("Invalid JSON format in regression suite file: " + path);
            return *this;
        }
        
        Array<Value> specs_array = json_value;
        for (int i = 0; i < specs_array.GetCount(); i++) {
            if (specs_array[i].Is<ValueMap>()) {
                ValueMap spec_map = specs_array[i];
                
                String name = spec_map.Get("name");
                if (!name.IsEmpty()) {
                    RegressionSpec spec;
                    if (spec_map.Find("agent_name") >= 0) spec.agent_name = spec_map.Get("agent_name");
                    if (spec_map.Find("workspace_root") >= 0) spec.workspace_root = spec_map.Get("workspace_root");
                    if (spec_map.Find("playbook_or_scenario") >= 0) spec.playbook_or_scenario = spec_map.Get("playbook_or_scenario");
                    if (spec_map.Find("allowed_risk") >= 0) spec.allowed_risk = spec_map.Get("allowed_risk").GetDouble();
                    if (spec_map.Find("max_actions") >= 0) spec.max_actions = spec_map.Get("max_actions").GetInt();
                    if (spec_map.Find("comparison_baselines") >= 0) spec.comparison_baselines = spec_map.Get("comparison_baselines");
                    
                    AddSpec(name, spec);
                }
            }
        }
        
        return *this;
    } catch (...) {
        LOG("Exception occurred while loading regression suite from file: " + path);
        return *this;
    }
}

Vector<String> RegressionSuite::ListSpecs() const {
    Vector<String> result;
    for (int i = 0; i < specs.GetCount(); i++) {
        result.Add(specs.GetKey(i));
    }
    return result;
}

bool RegressionSuite::HasSpec(const String& name) const {
    return specs.Find(name) >= 0;
}

const RegressionSpec& RegressionSuite::GetSpec(const String& name) const {
    int idx = specs.Find(name);
    if (idx < 0) {
        THROW("Regression spec not found: " + name);
    }
    return specs[idx];
}

void RegressionSuite::AddSpec(const String& name, const RegressionSpec& spec) {
    specs.Add(name, spec);
}

void RegressionSuite::RemoveSpec(const String& name) {
    int idx = specs.Find(name);
    if (idx >= 0) {
        specs.Remove(idx);
    }
}

RegressionSuite& RegressionSuite::Single() {
    static RegressionSuite instance;
    return instance;
}

END_UPP_NAMESPACE
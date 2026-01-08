#include "AgentRegistry.h"

bool AgentRegistry::Load(const String& path, String& error) {
    // Load the JSON file
    String json_content = LoadFile(path);
    if (json_content.IsEmpty()) {
        error = "Could not load file: " + path;
        return false;
    }
    
    // Parse the JSON content
    Value json_value = ParseJSON(json_content);
    if (IsError(json_value)) {
        error = "Failed to parse JSON: " + AsString(GetError(json_value));
        return false;
    }
    
    // Parse agent profiles from the JSON
    return ParseAgentProfiles(json_value, error);
}

Vector<AgentProfile> AgentRegistry::GetAll() const {
    return agents;
}

bool AgentRegistry::ParseAgentProfiles(const Value& json_value, String& error) {
    if (!IsValueMap(json_value)) {
        error = "JSON root must be an object";
        return false;
    }
    
    const ValueMap& root_map = ValueTo<ValueMap>(json_value);
    
    // Get the "agents" array
    int agents_idx = root_map.Find("agents");
    if (agents_idx < 0) {
        error = "JSON must contain 'agents' array";
        return false;
    }
    
    if (!IsValueArray(root_map.Get("agents"))) {
        error = "'agents' must be an array";
        return false;
    }
    
    ValueArray agents_array = root_map.Get("agents");
    
    // Clear existing agents
    agents.Clear();
    
    // Parse each agent profile
    for (int i = 0; i < agents_array.GetCount(); i++) {
        if (!IsValueMap(agents_array[i])) {
            error = "Agent profile must be an object at index " + AsString(i);
            return false;
        }
        
        const ValueMap& agent_map = ValueTo<ValueMap>(agents_array[i]);
        
        AgentProfile profile;
        if (!ParseAgentProfile(agent_map, profile, error)) {
            return false;
        }
        
        agents.Add(profile);
    }
    
    return true;
}

bool AgentRegistry::ParseAgentProfile(const ValueMap& agent_map, AgentProfile& profile, String& error) {
    // Get name
    int name_idx = agent_map.Find("name");
    if (name_idx < 0) {
        error = "Agent profile must contain 'name'";
        return false;
    }
    profile.name = agent_map.Get("name");
    
    // Get preferences (optional)
    int prefs_idx = agent_map.Find("preferences");
    if (prefs_idx >= 0) {
        if (!IsValueMap(agent_map.Get("preferences"))) {
            error = "'preferences' must be an object";
            return false;
        }
        profile.preferences = agent_map.Get("preferences");
    }
    
    // Get goals
    int goals_idx = agent_map.Find("goals");
    if (goals_idx < 0) {
        error = "Agent profile must contain 'goals'";
        return false;
    }
    
    if (!IsValueArray(agent_map.Get("goals"))) {
        error = "'goals' must be an array";
        return false;
    }
    
    ValueArray goals_array = agent_map.Get("goals");
    
    for (int i = 0; i < goals_array.GetCount(); i++) {
        if (!IsValueMap(goals_array[i])) {
            error = "Goal must be an object at index " + AsString(i);
            return false;
        }
        
        const ValueMap& goal_map = ValueTo<ValueMap>(goals_array[i]);
        
        Goal goal;
        if (!ParseGoal(goal_map, goal, error)) {
            return false;
        }
        
        profile.goals.Add(goal);
    }
    
    return true;
}

bool AgentRegistry::ParseGoal(const ValueMap& goal_map, Goal& goal, String& error) {
    // Get id
    int id_idx = goal_map.Find("id");
    if (id_idx < 0) {
        error = "Goal must contain 'id'";
        return false;
    }
    goal.id = goal_map.Get("id");
    
    // Get description
    int desc_idx = goal_map.Find("description");
    if (desc_idx >= 0) {
        goal.description = goal_map.Get("description");
    }
    
    // Get weights
    int weights_idx = goal_map.Find("weights");
    if (weights_idx < 0) {
        error = "Goal must contain 'weights'";
        return false;
    }
    
    if (!IsValueMap(goal_map.Get("weights"))) {
        error = "'weights' must be an object";
        return false;
    }
    goal.weights = goal_map.Get("weights");
    
    // Get priority
    int priority_idx = goal_map.Find("priority");
    if (priority_idx < 0) {
        error = "Goal must contain 'priority'";
        return false;
    }
    goal.priority = goal_map.Get("priority", 0.0);
    
    return true;
}
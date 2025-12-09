#include "GlobalKnowledge.h"

GlobalKnowledge::GlobalKnowledge() {
    // Initialize with default structure
    data = ValueMap();
    storage_path = GetHomeDir() + "/.aiupp/global_knowledge.json";
}

void GlobalKnowledge::Load(const String& path) {
    storage_path = path;
    
    if (FileExists(path)) {
        String json_content = LoadFile(path);
        if (!json_content.IsEmpty()) {
            data = Jsonize(json_content);
        }
    }
    
    // Initialize with default structure if empty
    if (data.GetCount() == 0) {
        data.Add("workspace_snapshots", Array<Json>());
        data.Add("pattern_stats", ValueMap());
        data.Add("refactor_stats", ValueMap());
        data.Add("topology_stats", ValueMap());
    }
}

void GlobalKnowledge::Save(const String& path) const {
    String json_content = Jsonize(data);
    SaveFile(path, json_content);
}

void GlobalKnowledge::RecordWorkspaceSnapshot(const ValueMap& snapshot) {
    if (!data.IsKey("workspace_snapshots")) {
        data.Add("workspace_snapshots", Array<Json>());
    }
    
    Array<Json>& snapshots = ~data.Get("workspace_snapshots");
    snapshots.Add(Jsonize(snapshot));
}

void GlobalKnowledge::RecordPatternOutcome(const String& pattern, bool success, const ValueMap& deltas) {
    if (!data.IsKey("pattern_stats")) {
        data.Add("pattern_stats", ValueMap());
    }
    
    ValueMap& pattern_stats = ~data.Get("pattern_stats");
    
    if (!pattern_stats.IsKey(pattern)) {
        pattern_stats.Add(pattern, ValueMap());
        ValueMap& stats = ~pattern_stats.Get(pattern);
        stats.Add("occurrences", 0);
        stats.Add("successes", 0);
        stats.Add("failures", 0);
        stats.Add("avg_benefit", 0.0);
        stats.Add("avg_risk", 0.0);
        stats.Add("avg_confidence", 0.0);
    }
    
    ValueMap& stats = ~pattern_stats.Get(pattern);
    int occurrences = stats.Get("occurrences").GetInt();
    int successes = stats.Get("successes").GetInt();
    int failures = stats.Get("failures").GetInt();
    double avg_benefit = stats.Get("avg_benefit").GetDouble();
    double avg_risk = stats.Get("avg_risk").GetDouble();
    double avg_confidence = stats.Get("avg_confidence").GetDouble();
    
    occurrences++;
    if (success) successes++; else failures++;
    
    // Update averages with deterministic merging
    if (deltas.IsKey("benefit")) {
        double new_benefit = deltas.Get("benefit").GetDouble();
        avg_benefit = (avg_benefit * (occurrences - 1) + new_benefit) / occurrences;
    }
    
    if (deltas.IsKey("risk")) {
        double new_risk = deltas.Get("risk").GetDouble();
        avg_risk = (avg_risk * (occurrences - 1) + new_risk) / occurrences;
    }
    
    if (deltas.IsKey("confidence")) {
        double new_confidence = deltas.Get("confidence").GetDouble();
        avg_confidence = (avg_confidence * (occurrences - 1) + new_confidence) / occurrences;
    }
    
    stats.Set("occurrences", occurrences);
    stats.Set("successes", successes);
    stats.Set("failures", failures);
    stats.Set("avg_benefit", avg_benefit);
    stats.Set("avg_risk", avg_risk);
    stats.Set("avg_confidence", avg_confidence);
}

void GlobalKnowledge::RecordRefactorOutcome(const String& type, bool success, const ValueMap& deltas) {
    if (!data.IsKey("refactor_stats")) {
        data.Add("refactor_stats", ValueMap());
    }
    
    ValueMap& refactor_stats = ~data.Get("refactor_stats");
    
    if (!refactor_stats.IsKey(type)) {
        refactor_stats.Add(type, ValueMap());
        ValueMap& stats = ~refactor_stats.Get(type);
        stats.Add("uses", 0);
        stats.Add("successful", 0);
        stats.Add("reverted", 0);
        stats.Add("avg_delta_complexity", 0.0);
    }
    
    ValueMap& stats = ~refactor_stats.Get(type);
    int uses = stats.Get("uses").GetInt();
    int successful = stats.Get("successful").GetInt();
    int reverted = stats.Get("reverted").GetInt();
    double avg_delta_complexity = stats.Get("avg_delta_complexity").GetDouble();
    
    uses++;
    if (success) successful++; else reverted++;
    
    // Update averages with deterministic merging
    if (deltas.IsKey("delta_complexity")) {
        double new_complexity = deltas.Get("delta_complexity").GetDouble();
        avg_delta_complexity = (avg_delta_complexity * (uses - 1) + new_complexity) / uses;
    }
    
    stats.Set("uses", uses);
    stats.Set("successful", successful);
    stats.Set("reverted", reverted);
    stats.Set("avg_delta_complexity", avg_delta_complexity);
}

GlobalKnowledge::PatternStats GlobalKnowledge::GetPatternStats(const String& pattern) const {
    PatternStats result;
    
    if (!data.IsKey("pattern_stats")) {
        return result;
    }
    
    const ValueMap& pattern_stats = ~data.Get("pattern_stats");
    
    if (!pattern_stats.IsKey(pattern)) {
        return result;
    }
    
    const ValueMap& stats = ~pattern_stats.Get(pattern);
    
    result.occurrences = stats.Get("occurrences").GetInt();
    result.successes = stats.Get("successes").GetInt();
    result.failures = stats.Get("failures").GetInt();
    result.avg_benefit = stats.Get("avg_benefit").GetDouble();
    result.avg_risk = stats.Get("avg_risk").GetDouble();
    result.avg_confidence = stats.Get("avg_confidence").GetDouble();
    
    return result;
}

GlobalKnowledge::RefactorStats GlobalKnowledge::GetRefactorStats(const String& refactor) const {
    RefactorStats result;
    
    if (!data.IsKey("refactor_stats")) {
        return result;
    }
    
    const ValueMap& refactor_stats = ~data.Get("refactor_stats");
    
    if (!refactor_stats.IsKey(refactor)) {
        return result;
    }
    
    const ValueMap& stats = ~refactor_stats.Get(refactor);
    
    result.uses = stats.Get("uses").GetInt();
    result.successful = stats.Get("successful").GetInt();
    result.reverted = stats.Get("reverted").GetInt();
    result.avg_delta_complexity = stats.Get("avg_delta_complexity").GetDouble();
    
    return result;
}

GlobalKnowledge::TopologyStats GlobalKnowledge::GetTopologyStats() const {
    TopologyStats result;
    
    if (!data.IsKey("topology_stats")) {
        return result;
    }
    
    const ValueMap& topology_stats = ~data.Get("topology_stats");
    
    result.avg_cycles = topology_stats.Get("avg_cycles").GetDouble();
    result.avg_depth = topology_stats.Get("avg_depth").GetDouble();
    result.avg_coupling = topology_stats.Get("avg_coupling").GetDouble();
    result.count = topology_stats.Get("count").GetInt();
    
    return result;
}
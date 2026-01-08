#include "CoreEvolution.h"
#include <Core/Core.h>

NAMESPACE_UPP

CoreEvolution::CoreEvolution() {}

void CoreEvolution::Load(const String& path) {
    storage_path = path;
    
    if (!FileExists(path)) {
        events.Clear();
        return;
    }
    
    String content = LoadFile(path);
    if (content.IsEmpty()) {
        events.Clear();
        return;
    }
    
    Value json_value = ParseJSON(content);
    if (!json_value.IsArray()) {
        events.Clear();
        return;
    }
    
    ValueArray arr = json_value;
    events.Clear();
    
    for (int i = 0; i < arr.GetCount(); i++) {
        ValueMap map = arr[i];
        EvolutionEvent ev;
        
        if (map.Find("timestamp") >= 0) {
            String time_str = map.Get("timestamp");
            ev.timestamp = ScanTime(time_str);  // This may need adjustment depending on stored format
        }
        
        if (map.Find("id") >= 0) ev.id = map.Get("id");
        if (map.Find("package") >= 0) ev.package = map.Get("package");
        if (map.Find("agent_name") >= 0) ev.agent_name = map.Get("agent_name");
        if (map.Find("scenario_id") >= 0) ev.scenario_id = map.Get("scenario_id");
        if (map.Find("lifecycle_phase") >= 0) ev.lifecycle_phase = map.Get("lifecycle_phase");
        if (map.Find("strategy") >= 0) ev.strategy = map.Get("strategy");
        if (map.Find("context") >= 0) ev.context = map.Get("context");
        
        if (map.Find("change_kinds") >= 0) {
            ValueArray change_arr = map.Get("change_kinds");
            for (int j = 0; j < change_arr.GetCount(); j++) {
                ev.change_kinds.Add(change_arr[j]);
            }
        }
        
        if (map.Find("metrics_before") >= 0) ev.metrics_before = map.Get("metrics_before");
        if (map.Find("metrics_after") >= 0) ev.metrics_after = map.Get("metrics_after");
        if (map.Find("deltas") >= 0) ev.deltas = map.Get("deltas");
        
        if (map.Find("succeeded") >= 0) ev.succeeded = map.Get("succeeded");
        if (map.Find("reverted") >= 0) ev.reverted = map.Get("reverted");
        
        events.Add(ev);
    }
}

void CoreEvolution::Save(const String& path) const {
    ValueArray arr;
    
    for (const EvolutionEvent& ev : events) {
        ValueMap map;
        
        map.Set("timestamp", ev.timestamp.ToString());  // Need to store as string format
        map.Set("id", ev.id);
        map.Set("package", ev.package);
        map.Set("agent_name", ev.agent_name);
        map.Set("scenario_id", ev.scenario_id);
        map.Set("lifecycle_phase", ev.lifecycle_phase);
        map.Set("strategy", ev.strategy);
        map.Set("context", ev.context);
        
        ValueArray change_arr;
        for (const String& kind : ev.change_kinds) {
            change_arr.Add(kind);
        }
        map.Set("change_kinds", change_arr);
        
        map.Set("metrics_before", ev.metrics_before);
        map.Set("metrics_after", ev.metrics_after);
        map.Set("deltas", ev.deltas);
        
        map.Set("succeeded", ev.succeeded);
        map.Set("reverted", ev.reverted);
        
        arr.Add(map);
    }
    
    String json_str = AsJSON(arr);
    SaveFile(path, json_str);
}

void CoreEvolution::RecordEvent(const EvolutionEvent& ev) {
    events.Add(ev);
    
    // Auto-save if storage path is set
    if (!storage_path.IsEmpty()) {
        Save(storage_path);
    }
}

Vector<EvolutionEvent> CoreEvolution::GetTimeline() const {
    return events;
}

EvolutionSummary CoreEvolution::Summarize() const {
    EvolutionSummary summary;
    summary.total_events = events.GetCount();
    
    for (const EvolutionEvent& ev : events) {
        if (ev.succeeded) {
            summary.successful++;
        }
        if (ev.reverted) {
            summary.reverted_count++;
        }
        
        // Count by change kind
        for (const String& kind : ev.change_kinds) {
            ValueMap* stats = summary.by_change_kind.GetPtr(kind);
            if (!stats) {
                ValueMap new_stats;
                new_stats.Set("count", 1);
                new_stats.Set("success_count", ev.succeeded ? 1 : 0);
                new_stats.Set("total_delta", 0.0); // Placeholder - would need actual metric calculation
                
                if (ev.deltas.GetCount() > 0) {
                    double total_delta = 0.0;
                    for (int i = 0; i < ev.deltas.GetCount(); i++) {
                        if (ev.deltas.GetValue(i).IsNumber()) {
                            total_delta += ev.deltas.GetValue(i);
                        }
                    }
                    new_stats.Set("total_delta", total_delta);
                }
                
                summary.by_change_kind.Set(kind, new_stats);
            } else {
                ValueMap& stats_ref = *stats;
                stats_ref.Set("count", stats_ref.Get("count") + 1);
                stats_ref.Set("success_count", stats_ref.Get("success_count") + (ev.succeeded ? 1 : 0));
                
                if (ev.deltas.GetCount() > 0) {
                    double current_total_delta = stats_ref.Get("total_delta");
                    double new_delta = 0.0;
                    
                    for (int i = 0; i < ev.deltas.GetCount(); i++) {
                        if (ev.deltas.GetValue(i).IsNumber()) {
                            new_delta += ev.deltas.GetValue(i);
                        }
                    }
                    
                    stats_ref.Set("total_delta", current_total_delta + new_delta);
                }
            }
        }
        
        // Count by strategy
        if (!ev.strategy.IsEmpty()) {
            ValueMap* strategy_stats = summary.by_strategy.GetPtr(ev.strategy);
            if (!strategy_stats) {
                ValueMap new_stats;
                new_stats.Set("count", 1);
                new_stats.Set("success_count", ev.succeeded ? 1 : 0);
                new_stats.Set("total_delta", 0.0); // Placeholder - would need actual metric calculation
                
                if (ev.deltas.GetCount() > 0) {
                    double total_delta = 0.0;
                    for (int i = 0; i < ev.deltas.GetCount(); i++) {
                        if (ev.deltas.GetValue(i).IsNumber()) {
                            total_delta += ev.deltas.GetValue(i);
                        }
                    }
                    new_stats.Set("total_delta", total_delta);
                }
                
                summary.by_strategy.Set(ev.strategy, new_stats);
            } else {
                ValueMap& stats_ref = *strategy_stats;
                stats_ref.Set("count", stats_ref.Get("count") + 1);
                stats_ref.Set("success_count", stats_ref.Get("success_count") + (ev.succeeded ? 1 : 0));
                
                if (ev.deltas.GetCount() > 0) {
                    double current_total_delta = stats_ref.Get("total_delta");
                    double new_delta = 0.0;
                    
                    for (int i = 0; i < ev.deltas.GetCount(); i++) {
                        if (ev.deltas.GetValue(i).IsNumber()) {
                            new_delta += ev.deltas.GetValue(i);
                        }
                    }
                    
                    stats_ref.Set("total_delta", current_total_delta + new_delta);
                }
            }
        }
        
        // Count by phase
        if (!ev.lifecycle_phase.IsEmpty()) {
            ValueMap* phase_stats = summary.by_phase.GetPtr(ev.lifecycle_phase);
            if (!phase_stats) {
                ValueMap new_stats;
                new_stats.Set("count", 1);
                new_stats.Set("success_count", ev.succeeded ? 1 : 0);
                
                ValueArray change_kinds_arr;
                for (const String& kind : ev.change_kinds) {
                    change_kinds_arr.Add(kind);
                }
                new_stats.Set("change_kinds", change_kinds_arr);
                
                summary.by_phase.Set(ev.lifecycle_phase, new_stats);
            } else {
                ValueMap& stats_ref = *phase_stats;
                stats_ref.Set("count", stats_ref.Get("count") + 1);
                stats_ref.Set("success_count", stats_ref.Get("success_count") + (ev.succeeded ? 1 : 0));
                
                // Add new change kinds to the list for this phase
                ValueArray current_kinds = stats_ref.Get("change_kinds");
                for (const String& kind : ev.change_kinds) {
                    bool found = false;
                    for (int i = 0; i < current_kinds.GetCount(); i++) {
                        if (current_kinds[i] == kind) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        current_kinds.Add(kind);
                    }
                }
                stats_ref.Set("change_kinds", current_kinds);
            }
        }
    }
    
    // Calculate averages and rates
    for (int i = 0; i < summary.by_change_kind.GetCount(); i++) {
        ValueMap& stats = summary.by_change_kind.GetValue(i);
        int count = stats.Get("count");
        int success_count = stats.Get("success_count");
        double avg_success_rate = count > 0 ? (double)success_count / count : 0.0;
        stats.Set("avg_success_rate", avg_success_rate);
        
        double total_delta = stats.Get("total_delta");
        double avg_delta = count > 0 ? total_delta / count : 0.0;
        stats.Set("avg_delta", avg_delta);
    }
    
    for (int i = 0; i < summary.by_strategy.GetCount(); i++) {
        ValueMap& stats = summary.by_strategy.GetValue(i);
        int count = stats.Get("count");
        int success_count = stats.Get("success_count");
        double avg_success_rate = count > 0 ? (double)success_count / count : 0.0;
        stats.Set("avg_success_rate", avg_success_rate);
        
        double total_delta = stats.Get("total_delta");
        double avg_delta = count > 0 ? total_delta / count : 0.0;
        stats.Set("avg_delta", avg_delta);
    }
    
    return summary;
}

END_UPP_NAMESPACE
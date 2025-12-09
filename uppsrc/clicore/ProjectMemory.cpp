#include "clicore.h"
#include "ProjectMemory.h"

namespace Upp {

ProjectMemory::ProjectMemory() {}

void ProjectMemory::Load(const String& path) {
    storage_path = path;
    FileIn in(path);
    if (!in.IsOpen()) {
        history.Clear();
        return;
    }
    
    Value v = LoadJSON(in);
    if (IsError(v)) {
        history.Clear();
        return;
    }
    
    if (v.IsArray()) {
        history.Clear();
        for (int i = 0; i < v.GetCount(); i++) {
            const Value& entryVal = v[i];
            if (entryVal.IsMap()) {
                Entry e;
                e.timestamp = entryVal.Get("timestamp", Time(0));
                e.proposal_id = entryVal.Get("proposal_id", String());
                e.metrics_before = entryVal.Get("metrics_before", ValueMap());
                e.metrics_after = entryVal.Get("metrics_after", ValueMap());
                e.deltas = entryVal.Get("deltas", ValueMap());
                e.benefit_score = entryVal.Get("benefit_score", 0.0);
                e.risk_score = entryVal.Get("risk_score", 0.0);
                e.confidence_score = entryVal.Get("confidence_score", 0.0);
                e.applied = entryVal.Get("applied", false);
                e.reverted = entryVal.Get("reverted", false);
                
                if (e.proposal_id.IsEmpty()) continue; // Skip invalid entries
                
                history.Add(std::move(e));
            }
        }
    }
}

void ProjectMemory::Save(const String& path) const {
    Array<Value> array;
    for (const Entry& e : history) {
        ValueMap entryMap;
        entryMap.Set("timestamp", e.timestamp);
        entryMap.Set("proposal_id", e.proposal_id);
        entryMap.Set("metrics_before", e.metrics_before);
        entryMap.Set("metrics_after", e.metrics_after);
        entryMap.Set("deltas", e.deltas);
        entryMap.Set("benefit_score", e.benefit_score);
        entryMap.Set("risk_score", e.risk_score);
        entryMap.Set("confidence_score", e.confidence_score);
        entryMap.Set("applied", e.applied);
        entryMap.Set("reverted", e.reverted);
        
        array.Add(std::move(entryMap));
    }
    
    String jsonStr = AsJSON(array);
    FileOut out(path);
    if (out.IsOpen()) {
        out.Write(jsonStr);
        out.Close();
    }
}

void ProjectMemory::Record(const Entry& e) {
    history.Add(e);
}

Vector<ProjectMemory::Entry> ProjectMemory::GetHistory() const {
    return history;
}

double ProjectMemory::AverageBenefit() const {
    if (history.IsEmpty()) return 0.0;
    
    double sum = 0.0;
    for (const Entry& e : history) {
        sum += e.benefit_score;
    }
    return sum / history.GetCount();
}

double ProjectMemory::AverageRisk() const {
    if (history.IsEmpty()) return 0.0;
    
    double sum = 0.0;
    for (const Entry& e : history) {
        sum += e.risk_score;
    }
    return sum / history.GetCount();
}

double ProjectMemory::AverageConfidence() const {
    if (history.IsEmpty()) return 0.0;
    
    double sum = 0.0;
    for (const Entry& e : history) {
        sum += e.confidence_score;
    }
    return sum / history.GetCount();
}

int ProjectMemory::CountHighValueChanges() const {
    int count = 0;
    for (const Entry& e : history) {
        if (e.applied && e.benefit_score > 0.7) { // Threshold for "high value"
            count++;
        }
    }
    return count;
}

int ProjectMemory::CountFailedChanges() const {
    int count = 0;
    for (const Entry& e : history) {
        if (e.applied && e.reverted) {
            count++;
        }
    }
    return count;
}

}
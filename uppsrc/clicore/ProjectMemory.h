#ifndef _clicore_ProjectMemory_h_
#define _clicore_ProjectMemory_h_

#include <Core/Core.h>

namespace Upp {

class ProjectMemory : Moveable<ProjectMemory> {
public:
    struct Entry : Moveable<Entry> {
        Time timestamp;
        String proposal_id;
        ValueMap metrics_before;
        ValueMap metrics_after;
        ValueMap deltas;
        double benefit_score;
        double risk_score;
        double confidence_score;
        bool applied;
        bool reverted;
        
        Entry() : benefit_score(0.0), risk_score(0.0), confidence_score(0.0), 
                 applied(false), reverted(false) {}
    };

    ProjectMemory();

    void Load(const String& path);
    void Save(const String& path) const;

    void Record(const Entry& e);

    Vector<Entry> GetHistory() const;

    // Derived analytics
    double AverageBenefit() const;
    double AverageRisk() const;
    double AverageConfidence() const;

    int CountHighValueChanges() const;
    int CountFailedChanges() const;

private:
    Vector<Entry> history;
    String storage_path;
};

}

#endif
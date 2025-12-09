#ifndef _clicore_StrategyProfile_h_
#define _clicore_StrategyProfile_h_

#include <Core/Core.h>

NAMESPACE_UPP

struct StrategyProfile : Moveable<StrategyProfile> {
    String name;
    String description;
    ValueMap weights;            // e.g. weights["include_density"] = 1.0
    ValueMap thresholds;         // nested maps for playbooks (raw JSON-like)
    ValueMap objective_weights;  // weights for multi-objective optimization (benefit, cost, risk, confidence)
};

class StrategyRegistry {
public:
    StrategyRegistry();

    bool Load(const String& path, String& error);

    const StrategyProfile* Find(const String& name) const;
    const Vector<StrategyProfile>& GetAll() const;

private:
    Vector<StrategyProfile> profiles;
};

END_UPP_NAMESPACE

#endif
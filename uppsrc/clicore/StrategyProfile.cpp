#include "clicore.h"
#include "StrategyProfile.h"

NAMESPACE_UPP

StrategyRegistry::StrategyRegistry() {
}

bool StrategyRegistry::Load(const String& path, String& error) {
    try {
        String json_data = LoadFile(path);
        if (json_data.IsEmpty()) {
            error = "Could not load strategies file: " + path;
            return false;
        }

        Value json_value = ParseJSON(json_data);
        if (!json_value.Is<ValueArray>()) {
            Value strategies_obj = json_value;
            json_value = strategies_obj["strategies"];
        }

        if (!json_value.Is<ValueArray>()) {
            error = "Invalid strategies format: 'strategies' should be an array";
            return false;
        }

        ValueArray strategies_array = json_value;
        profiles.Clear();

        for (int i = 0; i < strategies_array.GetCount(); i++) {
            Value strategy_val = strategies_array[i];
            if (!strategy_val.Is<ValueMap>()) {
                error = "Invalid strategy format at index " + AsString(i);
                return false;
            }

            ValueMap strategy_map = strategy_val;
            StrategyProfile profile;
            
            // Extract name
            if (strategy_map.Find("name").IsVoid()) {
                error = "Strategy at index " + AsString(i) + " missing 'name' field";
                return false;
            }
            profile.name = strategy_map.Get("name");
            
            // Extract description
            if (strategy_map.Find("description").IsVoid()) {
                error = "Strategy at index " + AsString(i) + " missing 'description' field";
                return false;
            }
            profile.description = strategy_map.Get("description");
            
            // Extract weights (optional)
            if (!strategy_map.Get("weights").IsVoid()) {
                profile.weights = strategy_map.Get("weights");
            }
            
            // Extract thresholds (optional)
            if (!strategy_map.Get("playbook_thresholds").IsVoid()) {
                profile.thresholds = strategy_map.Get("playbook_thresholds");
            }

            // Extract objective_weights (optional)
            if (!strategy_map.Get("objective_weights").IsVoid()) {
                profile.objective_weights = strategy_map.Get("objective_weights");
            }

            profiles.Add(std::move(profile));
        }

        return true;
    }
    catch (const std::exception& e) {
        error = String(e.what());
        return false;
    }
    catch (...) {
        error = "Unknown error occurred while loading strategies";
        return false;
    }
}

const StrategyProfile* StrategyRegistry::Find(const String& name) const {
    for (const auto& profile : profiles) {
        if (profile.name == name) {
            return &profile;
        }
    }
    return nullptr;
}

const Vector<StrategyProfile>& StrategyRegistry::GetAll() const {
    return profiles;
}

END_UPP_NAMESPACE
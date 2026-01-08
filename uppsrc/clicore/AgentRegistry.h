#ifndef _clicore_AgentRegistry_h_
#define _clicore_AgentRegistry_h_

#include <Core/Core.h>
#include <Core/ValueMap.h>
#include <Core/Json.h>

#include "StrategicNavigator.h"

class AgentRegistry {
public:
    bool Load(const String& path, String& error);
    Vector<AgentProfile> GetAll() const;
    
private:
    Vector<AgentProfile> agents;
    
    bool ParseAgentProfiles(const Value& json_value, String& error);
    bool ParseAgentProfile(const ValueMap& agent_map, AgentProfile& profile, String& error);
    bool ParseGoal(const ValueMap& goal_map, Goal& goal, String& error);
};

#endif
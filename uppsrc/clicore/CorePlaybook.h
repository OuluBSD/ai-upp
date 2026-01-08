#ifndef _clicore_CorePlaybook_h_
#define _clicore_CorePlaybook_h_

#include <Core/Value.h>
#include <Core/Vector.h>
#include <Core/String.h>
#include <Core/Moveable.h>

#include "CoreIde.h"

namespace Upp {

struct PlaybookStep : Moveable<PlaybookStep> {
    String id;          // e.g. "scan_workspace", "propose_changes", "apply_safe_subset"
    String action;      // logical action name, not CLI command
    ValueMap params;    // parameters for the action
};

struct Playbook : Moveable<Playbook> {
    String id;                 // e.g. "safe_cleanup_cycle"
    String description;
    double safety_level;       // 0..1, higher = more conservative
    ValueMap constraints;      // e.g. max_risk, max_actions, allow_apply (bool)
    Vector<PlaybookStep> steps;
};

class CorePlaybook {
public:
    CorePlaybook();

    bool Load(const String& path, String& error);
    Vector<Playbook> GetAll() const;
    const Playbook* Find(const String& id) const;

    Value Run(const Playbook& pb, CoreIde& ide, String& error) const;

private:
    Vector<Playbook> playbooks;

    Value RunStep(const Playbook& pb,
                  const PlaybookStep& step,
                  CoreIde& ide,
                  Value& shared_state,
                  String& error) const;
};

}

#endif
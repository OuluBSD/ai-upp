#include "RunTarget.h"

NAMESPACE_UPP

RunTargetRegistry& RunTargetRegistry::Get()
{
	static RunTargetRegistry reg;
	return reg;
}

void RunTargetRegistry::Register(IRunTarget& target)
{
	if (Find(target.GetID()))
		return;
	targets.Add(&target);
}

void RunTargetRegistry::Unregister(const String& id)
{
	for (int i = 0; i < targets.GetCount(); i++) {
		if (targets[i]->GetID() == id) {
			targets.Remove(i);
			return;
		}
	}
}

IRunTarget* RunTargetRegistry::Find(const String& id)
{
	for (auto& t : targets)
		if (t->GetID() == id)
			return t;
	return nullptr;
}

const Vector<IRunTarget*>& RunTargetRegistry::GetAll() const
{
	return targets;
}

IRunTarget* RunTargetRegistry::Resolve(const RunTargetContext& ctx, const String& preferred_id)
{
	// 1. User-selected target for this extension (from RunSettings)
	if (preferred_id.GetCount()) {
		IRunTarget* preferred = Find(preferred_id);
		if (preferred && preferred->CanRun(ctx))
			return preferred;
	}

	// 2. First target that CanRun() returns true for
	for (auto& t : targets) {
		if (t->GetID() == "local.terminal")
			continue;
		if (t->CanRun(ctx))
			return t;
	}

	// 3. Hard fallback: "local.terminal" (PythonCLI)
	if (IRunTarget* terminal = Find("local.terminal"))
		if (terminal->CanRun(ctx))
			return terminal;
	return nullptr;
}

END_UPP_NAMESPACE

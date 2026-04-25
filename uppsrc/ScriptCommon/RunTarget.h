#ifndef _ScriptCommon_RunTarget_h_
#define _ScriptCommon_RunTarget_h_

#include <Core/Core.h>

NAMESPACE_UPP

enum class RunMode { Run, Debug, Profile };

struct RunTargetContext {
    String              file_path;
    String              working_dir;
    RunMode             mode;
};

class IRunTarget {
public:
    virtual ~IRunTarget() {}

    // Unique ID, e.g. "local.game_window", "local.terminal", "local.external_process"
    virtual String GetID() const = 0;

    // Human-readable name shown in Run Options menu/settings
    virtual String GetName() const = 0;

    // Which file extensions / mime types this target supports.
    // Empty = supports all.
    virtual Vector<String> GetSupportedExtensions() const { return {}; }

    // Can this target handle the given context?
    virtual bool CanRun(const RunTargetContext& ctx) const = 0;

    // Launch execution. Called from GUI thread.
    // May spawn threads, open windows, etc.
    virtual void Run(const RunTargetContext& ctx) = 0;
};

class RunTargetRegistry {
public:
    static RunTargetRegistry& Get();

    void Register(IRunTarget& target);
    void Unregister(const String& id);

    IRunTarget* Find(const String& id);
    const Vector<IRunTarget*>& GetAll() const;

    /*
	  Target resolution order:
	  1. User-selected target for this extension (from RunSettings)
	  2. First target that CanRun() returns true for
	  3. Hard fallback: "local.terminal" (PythonCLI)
	*/
    IRunTarget* Resolve(const RunTargetContext& ctx,
                        const String& preferred_id = String());

private:
    Vector<IRunTarget*> targets;
};

END_UPP_NAMESPACE

#endif

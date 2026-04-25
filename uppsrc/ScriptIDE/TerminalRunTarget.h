#ifndef _ScriptIDE_TerminalRunTarget_h_
#define _ScriptIDE_TerminalRunTarget_h_

NAMESPACE_UPP

class TerminalRunTarget : public IRunTarget {
public:
	virtual String GetID() const override { return "local.terminal"; }
	virtual String GetName() const override { return "Terminal (PythonCLI)"; }
	virtual bool   CanRun(const RunTargetContext& ctx) const override;
	virtual void   Run(const RunTargetContext& ctx) override;
};

void RegisterTerminalRunTarget();

END_UPP_NAMESPACE

#endif

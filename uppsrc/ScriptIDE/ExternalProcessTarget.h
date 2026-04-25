#ifndef _ScriptIDE_ExternalProcessTarget_h_
#define _ScriptIDE_ExternalProcessTarget_h_

NAMESPACE_UPP

class ExternalProcessTarget : public IRunTarget {
public:
	virtual String GetID() const override { return "local.external_process"; }
	virtual String GetName() const override { return "External process"; }
	virtual bool   CanRun(const RunTargetContext& ctx) const override;
	virtual void   Run(const RunTargetContext& ctx) override;

	void SetConfig(const ExternalProcessSettings& cfg);

private:
	ExternalProcessSettings config;

	String ResolveBinaryPath() const;
	String ResolvePythonCliEntry(const String& path) const;
	String BuildCommandLine(const String& binary, const Vector<String>& args) const;
	Vector<String> ParseExtraArgs(const String& text) const;
};

void RegisterExternalProcessTarget();
ExternalProcessTarget* GetExternalProcessTarget();
int GetExternalProcessLaunchCount();
void ResetExternalProcessLaunchCount();

END_UPP_NAMESPACE

#endif

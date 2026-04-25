#ifndef _ScriptIDE_EmbeddedGameWindowTarget_h_
#define _ScriptIDE_EmbeddedGameWindowTarget_h_

NAMESPACE_UPP

class EmbeddedGameWindowTarget : public IRunTarget {
public:
	virtual String GetID() const override { return "local.game_window"; }
	virtual String GetName() const override { return "Embedded game window"; }
	virtual Vector<String> GetSupportedExtensions() const override;
	virtual bool   CanRun(const RunTargetContext& ctx) const override;
	virtual void   Run(const RunTargetContext& ctx) override;
};

void RegisterEmbeddedGameWindowTarget();

END_UPP_NAMESPACE

#endif

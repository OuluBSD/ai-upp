#include "ScriptIDE.h"

NAMESPACE_UPP

Vector<String> EmbeddedGameWindowTarget::GetSupportedExtensions() const
{
	Vector<String> exts;
	exts.Add(".gamestate");
	return exts;
}

bool EmbeddedGameWindowTarget::CanRun(const RunTargetContext& ctx) const
{
	return ToLower(GetFileExt(ctx.file_path)) == ".gamestate";
}

void EmbeddedGameWindowTarget::Run(const RunTargetContext& ctx)
{
	if(!CanRun(ctx))
		return;
	OpenStandaloneGameWindow(ctx.file_path, ctx.mode);
}

void RegisterEmbeddedGameWindowTarget()
{
	static EmbeddedGameWindowTarget target;
	RunTargetRegistry::Get().Register(target);
}

END_UPP_NAMESPACE

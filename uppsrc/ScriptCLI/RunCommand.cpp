#include "ScriptCLI.h"

NAMESPACE_UPP

int RunFileCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "run: missing file argument\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	String path = GetFullPath(args[1]);
	if(!FileExists(path)) {
		Cerr() << "run: file not found: " << path << "\n";
		return SCRIPTCLI_INFRA_ERROR;
	}

	PyVM vm;

	// Wire up print → stdout
	vm.WhenPrint = [](const String& s) { Cout() << s; };

	String ext = ToLower(GetFileExt(path));
	Cerr() << "DEBUG ext='" << ext << "'\n";

	if(ext == ".gamestate") {
		Cerr() << "DEBUG: routing to plugin\n";
		// Use plugin system: initialise all internal plugins and execute
		HeadlessPluginContext ctx(vm);
		Vector<One<IPlugin>> plugins;
		for(auto factory : GetInternalPluginFactories()) {
			One<IPlugin> p(factory());
			p->Init(ctx);
			plugins.Add(pick(p));
		}
		ctx.SyncBindings();

		ICustomExecuteProvider* provider = ctx.FindExecuteProvider(path);
		if(!provider) {
			Cerr() << "run: no plugin handles " << ext << "\n";
			return SCRIPTCLI_INFRA_ERROR;
		}

		try {
			provider->Execute(path);
		} catch(Exc& e) {
			Cerr() << "run: " << e << "\n";
			for(auto& p : plugins) p->Shutdown();
			return SCRIPTCLI_RUNTIME_ERROR;
		}

		for(auto& p : plugins) p->Shutdown();
		return SCRIPTCLI_OK;
	}

	// Plain Python script
	String code = LoadFile(path);
	RunManager run(vm);

	bool had_error = false;
	run.WhenError = [&](const String& err) {
		had_error = true;
		Cerr() << "run: " << err << "\n";
	};

	run.Run(code, path);
	return had_error ? SCRIPTCLI_RUNTIME_ERROR : SCRIPTCLI_OK;
}

END_UPP_NAMESPACE

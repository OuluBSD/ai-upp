#include "ScriptCLI.h"

NAMESPACE_UPP

int PluginCommand(const Vector<String>& args)
{
	if(args.GetCount() < 2) {
		Cerr() << "plugin: expected subcommand (list|test)\n";
		return SCRIPTCLI_USAGE_ERROR;
	}

	if(args[1] == "list") {
		Cout() << "Internal Plugins:\n";
		auto& factories = GetInternalPluginFactories();
		for(auto f : factories) {
			One<IPlugin> p(f());
			Cout() << "- " << p->GetID() << " (" << p->GetName() << "): " << p->GetDescription() << "\n";
		}
		return SCRIPTCLI_OK;
	}

	if(args[1] == "test") {
		if(args.GetCount() < 3) {
			Cerr() << "plugin test: missing <plugin-id>\n";
			return SCRIPTCLI_USAGE_ERROR;
		}
		
		String plugin_id = args[2];
		IPlugin* plugin = nullptr;
		auto& factories = GetInternalPluginFactories();
		for(auto f : factories) {
			One<IPlugin> p(f());
			if(p->GetID() == plugin_id) {
				plugin = p.Detach();
				break;
			}
		}
		
		if(!plugin) {
			Cerr() << "plugin test: plugin '" << plugin_id << "' not found\n";
			return SCRIPTCLI_INFRA_ERROR;
		}
		
		One<IPlugin> p(plugin);
		Cout() << "Testing plugin: " << p->GetID() << "\n";
		
		PyVM vm;
		HeadlessPluginContext ctx(vm);
		p->Init(ctx);
		ctx.SyncBindings();
		
		String test_dir = AppendFileName(GetHomeDirectory(), "Dev/ai-upp/autotest/Plugins/" + plugin_id + "/tests");
		if(!DirectoryExists(test_dir)) {
			Cout() << "No tests found in " << test_dir << "\n";
			return SCRIPTCLI_OK;
		}
		
		int failed = 0;
		int passed = 0;
		
		FindFile ff(AppendFileName(test_dir, "*.py"));
		while(ff) {
			if(ff.IsFile()) {
				String script_path = ff.GetPath();
				Cout() << "Running test: " << ff.GetName() << "... ";
				
				try {
					String code = LoadFile(script_path);
					Tokenizer tk;
					if(tk.Process(code, ff.GetName())) {
						tk.NewlineToEndStatement();
						tk.CombineTokens();
						PyCompiler compiler(tk.GetTokens(), ff.GetName());
						Vector<PyIR> ir;
						compiler.Compile(ir);
						vm.SetIR(ir);
						vm.Run();
						Cout() << "PASSED\n";
						passed++;
					} else {
						Cout() << "FAILED (Tokenization error)\n";
						failed++;
					}
				}
				catch(Exc& e) {
					Cout() << "FAILED: " << e << "\n";
					failed++;
				}
			}
			ff.Next();
		}
		
		p->Shutdown();
		
		Cout() << "Summary: " << passed << " passed, " << failed << " failed.\n";
		return failed == 0 ? SCRIPTCLI_OK : SCRIPTCLI_INFRA_ERROR;
	}

	Cerr() << "plugin: unknown subcommand '" << args[1] << "'\n";
	return SCRIPTCLI_USAGE_ERROR;
}

END_UPP_NAMESPACE

#include "ScriptCommon.h"

NAMESPACE_UPP

ScriptServices::ScriptServices(const String& workspace)
{
	this->workspace = workspace.IsEmpty() ? GetCurrentDirectory() : NormalizePath(workspace);
}

String ScriptServices::GetWorkspace() const
{
	return workspace;
}

String ScriptServices::ResolvePath(const String& path) const
{
	if(path.IsEmpty())
		return path;
	if(IsFullPath(path))
		return NormalizePath(path);
	return NormalizePath(AppendFileName(workspace, path));
}

ScriptRunResult ScriptServices::RunFile(const ScriptRunRequest& req) const
{
	ScriptRunResult result;
	result.path = ResolvePath(req.path);
	if(result.path.IsEmpty()) {
		result.error = "path is required";
		return result;
	}
	if(!FileExists(result.path)) {
		result.error = "file not found: " + result.path;
		return result;
	}

	PyVM vm;
	vm.WhenPrint = [&](const String& s) {
		req.WhenStdout(s);
	};

	String ext = ToLower(GetFileExt(result.path));
	if(ext == ".gamestate") {
		result.kind = "gamestate";
		HeadlessPluginContext ctx(vm);
		Vector<One<IPlugin>> plugins;
		for(auto factory : GetInternalPluginFactories()) {
			One<IPlugin> p(factory());
			p->Init(ctx);
			plugins.Add(pick(p));
		}
		ctx.SyncBindings();

		ICustomExecuteProvider* provider = ctx.FindExecuteProvider(result.path);
		if(!provider) {
			result.error = "no plugin handles " + ext;
			for(auto& p : plugins)
				p->Shutdown();
			return result;
		}

		try {
			provider->Execute(result.path);
			result.ok = true;
		}
		catch(Exc& e) {
			result.runtime_error = true;
			result.error = e;
		}

		for(auto& p : plugins)
			p->Shutdown();
		return result;
	}

	result.kind = "python";
	String code = LoadFile(result.path);
	RunManager run(vm);
	run.WhenError = [&](const String& err) {
		result.runtime_error = true;
		result.error = err;
		req.WhenStderr(err);
	};

	run.Run(code, result.path);
	result.ok = !result.runtime_error && result.error.IsEmpty();
	return result;
}

ScriptLintResult ScriptServices::LintFile(const String& path) const
{
	ScriptLintResult result;
	result.path = ResolvePath(path);
	if(result.path.IsEmpty()) {
		ScriptLintIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.text = "path is required";
		return result;
	}
	if(!FileExists(result.path)) {
		ScriptLintIssue& issue = result.issues.Add();
		issue.severity = "error";
		issue.text = "file not found: " + result.path;
		return result;
	}

	String code = LoadFile(result.path);
	Linter lint;
	Vector<Linter::Message> msgs = lint.Analyze(code, result.path);
	for(const auto& m : msgs) {
		ScriptLintIssue& issue = result.issues.Add();
		issue.line = m.line;
		issue.column = m.column;
		issue.severity = m.is_error ? "error" : "warning";
		issue.text = m.text;
	}
	result.ok = result.issues.IsEmpty();
	return result;
}

Vector<ScriptPluginInfo> ScriptServices::ListPlugins() const
{
	Vector<ScriptPluginInfo> out;
	auto& factories = GetInternalPluginFactories();
	for(auto factory : factories) {
		One<IPlugin> p(factory());
		ScriptPluginInfo& info = out.Add();
		info.id = p->GetID();
		info.name = p->GetName();
		info.description = p->GetDescription();
	}
	return out;
}

ScriptPluginTestResult ScriptServices::TestPlugin(const String& plugin_id, const String& case_name) const
{
	ScriptPluginTestResult result;
	result.plugin_id = plugin_id;
	if(plugin_id.IsEmpty()) {
		result.error = "plugin_id is required";
		return result;
	}

	IPlugin* plugin = nullptr;
	auto& factories = GetInternalPluginFactories();
	for(auto factory : factories) {
		One<IPlugin> p(factory());
		if(p->GetID() == plugin_id) {
			plugin = p.Detach();
			break;
		}
	}
	if(!plugin) {
		result.error = "plugin not found: " + plugin_id;
		return result;
	}

	One<IPlugin> p(plugin);
	PyVM vm;
	HeadlessPluginContext ctx(vm);
	p->Init(ctx);
	ctx.SyncBindings();

	String test_dir = AppendFileName(GetHomeDirectory(), "Dev/ai-upp/autotest/Plugins/" + plugin_id + "/tests");
	if(!DirectoryExists(test_dir)) {
		result.ok = true;
		p->Shutdown();
		return result;
	}

	FindFile ff(AppendFileName(test_dir, "*.py"));
	while(ff) {
		if(ff.IsFile()) {
			String test_name = ff.GetName();
			if(!case_name.IsEmpty() && case_name != test_name) {
				ff.Next();
				continue;
			}

			ScriptPluginTestCaseResult& case_result = result.results.Add();
			case_result.name = test_name;
			try {
				String code = LoadFile(ff.GetPath());
				Tokenizer tk;
				if(tk.Process(code, test_name)) {
					tk.NewlineToEndStatement();
					tk.CombineTokens();
					PyCompiler compiler(tk.GetTokens(), test_name);
					Vector<PyIR> ir;
					compiler.Compile(ir);
					vm.SetIR(ir);
					vm.Run();
					case_result.ok = true;
					result.passed++;
				}
				else {
					case_result.message = "Tokenization error";
					result.failed++;
				}
			}
			catch(Exc& e) {
				case_result.message = e;
				result.failed++;
			}
		}
		ff.Next();
	}

	p->Shutdown();
	result.ok = result.failed == 0;
	return result;
}

END_UPP_NAMESPACE

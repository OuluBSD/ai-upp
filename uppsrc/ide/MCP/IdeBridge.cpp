#ifdef flagGUI
#include "MCP.h"
#include <ide/ide.h>

NAMESPACE_UPP

IdeBridge sIdeBridge;

bool IdeBridge::RunOnGui(Function<void()> fn, int timeout_ms) const
{
	Semaphore done;
	PostCallback([fn = pick(fn), &done]() mutable { fn(); done.Release(); });
	return done.Wait(timeout_ms);
}

// ---- mainconfig ------------------------------------------------------------

ValueArray IdeBridge::ListMainConfigs() const
{
	ValueArray out;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		for(int i = 0; i < ide->mainconfiglist.GetCount(); i++) {
			ValueMap m;
			m.Add("param", AsString(ide->mainconfiglist.GetKey(i)));
			m.Add("name",  AsString(ide->mainconfiglist.GetValue(i)));
			out.Add(m);
		}
	});
	return out;
}

String IdeBridge::GetMainConfig() const
{
	String r;
	GuiLock __;
	if(Ide* ide = TheIde()) r = ide->mainconfigparam;
	return r;
}

String IdeBridge::SetMainConfig(const String& param)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->mainconfigparam = param;
		ide->SetMainConfigList();
	});
	return err;
}

// ---- build methods ---------------------------------------------------------

ValueArray IdeBridge::ListBuildMethods() const
{
	ValueArray out;
	RunOnGui([&] {
		if(IdeContext* ctx = TheIdeContext())
			for(FindFile ff(ConfigFile(ctx->GetMethodName("*"))); ff; ff.Next())
				out.Add(GetFileTitle(ff.GetName()));
	});
	return out;
}

String IdeBridge::GetBuildMethod() const
{
	String r;
	GuiLock __;
	if(MakeBuild* mb = TheIde()) r = mb->method;
	return r;
}

String IdeBridge::SetBuildMethod(const String& name)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->SetMethod(name);
	});
	return err;
}

// ---- build mode ------------------------------------------------------------

String IdeBridge::GetBuildMode() const
{
	String r;
	GuiLock __;
	if(MakeBuild* mb = TheIde()) r = mb->targetmode == 1 ? "release" : "debug";
	return r;
}

String IdeBridge::SetBuildMode(const String& mode)
{
	if(mode != "debug" && mode != "release")
		return "mode must be 'debug' or 'release'";
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->targetmode = (mode == "release") ? 1 : 0;
		ide->SyncBuildMode();
	});
	return err;
}

// ---- packages --------------------------------------------------------------

ValueArray IdeBridge::ListAssemblyPackages() const
{
	ValueArray out;
	RunOnGui([&] {
		const Workspace& wspc = GetIdeWorkspace();
		for(int i = 0; i < wspc.GetCount(); i++)
			out.Add(wspc[i]);
	});
	return out;
}

String IdeBridge::GetActivePackage() const
{
	String r;
	GuiLock __;
	if(Ide* ide = TheIde()) r = ide->GetActivePackage();
	return r;
}

String IdeBridge::SetActivePackage(const String& name)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		if(!ide->package.FindSetCursor(name))
			err = "Package not found: " + name;
	});
	return err;
}

// ---- files in package ------------------------------------------------------

ValueArray IdeBridge::ListPackageFiles(const String& pkg) const
{
	ValueArray out;
	RunOnGui([&] {
		const Workspace& wspc = GetIdeWorkspace();
		int q = wspc.package.Find(pkg);
		if(q < 0) return;
		const Package& p = wspc.package[q];
		for(int i = 0; i < p.file.GetCount(); i++)
			out.Add(SourcePath(pkg, p.file[i]));
	});
	return out;
}

String IdeBridge::GetActiveFile() const
{
	String r;
	GuiLock __;
	if(Ide* ide = TheIde()) r = ide->editfile;
	return r;
}

String IdeBridge::SetActiveFile(const String& path)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->EditFile(path);
	});
	return err;
}

// ---- editor cursor / content -----------------------------------------------

IdeBridge::EditorPos IdeBridge::GetEditorCursor(int editor_idx) const
{
	EditorPos r;
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return r;
	CodeEditor& ed = editor_idx == 0 ? (CodeEditor&)ide->editor : (CodeEditor&)ide->editor2;
	int64 cur = ed.GetCursor64();
	r.cursor = cur;
	r.line   = ed.GetLine(cur);
	r.col    = (int)(cur - ed.GetPos(r.line));
	return r;
}

String IdeBridge::SetEditorCursor(int line, int col, int editor_idx)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		CodeEditor& ed = editor_idx == 0 ? (CodeEditor&)ide->editor : (CodeEditor&)ide->editor2;
		int pos = ed.GetPos(line, col);
		ed.SetCursor(pos);
	});
	return err;
}

String IdeBridge::SetEditorCursorPos(int64 pos, int editor_idx)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		CodeEditor& ed = editor_idx == 0 ? (CodeEditor&)ide->editor : (CodeEditor&)ide->editor2;
		ed.SetCursor(pos);
	});
	return err;
}

int IdeBridge::GetEditorLineCount(int editor_idx) const
{
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return 0;
	CodeEditor& ed = editor_idx == 0 ? (CodeEditor&)ide->editor : (CodeEditor&)ide->editor2;
	return ed.GetLineCount();
}

String IdeBridge::GetEditorLine(int line, int editor_idx) const
{
	String r;
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return r;
	CodeEditor& ed = editor_idx == 0 ? (CodeEditor&)ide->editor : (CodeEditor&)ide->editor2;
	if(line >= 0 && line < ed.GetLineCount())
		r = ed.GetUtf8Line(line);
	return r;
}

String IdeBridge::GetEditorPath(int editor_idx) const
{
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return String();
	return editor_idx == 0 ? ide->editfile : ide->editfile2;
}

String IdeBridge::InsertEditorText(const String& text)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->InsertText(text);
	});
	return err;
}

// ---- console output --------------------------------------------------------

String IdeBridge::GetConsoleText() const
{
	String r;
	GuiLock __;
	if(Ide* ide = TheIde()) {
		int n = ide->console.GetLineCount();
		for(int i = 0; i < n; i++) {
			if(i) r.Cat('\n');
			r.Cat(ide->console.GetUtf8Line(i));
		}
	}
	return r;
}

String IdeBridge::GetConsoleTail(int lines) const
{
	String r;
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return r;
	int total = ide->console.GetLineCount();
	int start = max(0, total - lines);
	for(int i = start; i < total; i++) {
		if(i > start) r.Cat('\n');
		r.Cat(ide->console.GetUtf8Line(i));
	}
	return r;
}

// ---- error list ------------------------------------------------------------

Vector<IdeBridge::BuildError> IdeBridge::GetErrors() const
{
	Vector<BuildError> out;
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return out;
	for(int i = 0; i < ide->error.GetCount(); i++) {
		Value fv = ide->error.Get(i, 0);
		Value lv = ide->error.Get(i, 1);
		Value tv = ide->error.Get(i, 2);
		if(IsNull(fv) && IsNull(lv)) continue; // separator row (e.g. "Linking has failed")
		BuildError e;
		e.file = AsString(fv);
		e.line = (int)lv;
		// tv may be AttrText — convert via AsString
		String msg = AsString(tv);
		// heuristic: warnings typically contain "warning:" in the message
		e.is_warning = ToLower(msg).Find("warning:") >= 0;
		e.text = msg;
		out.Add(pick(e));
	}
	return out;
}

int IdeBridge::GetErrorCount() const
{
	GuiLock __;
	return TheIde() ? TheIde()->error_count : 0;
}

int IdeBridge::GetWarningCount() const
{
	GuiLock __;
	return TheIde() ? TheIde()->warning_count : 0;
}

// ---- find in files ---------------------------------------------------------

String IdeBridge::FindInFiles(const String& pattern, bool replace)
{
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		ide->ff.find <<= pattern;
		ide->FindInFiles(replace);
	});
	return String();
}

// ---- find in editor --------------------------------------------------------

String IdeBridge::EditorFindNext()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->EditFindNext();
	});
	return err;
}

String IdeBridge::EditorFindPrev()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->EditFindPrevious();
	});
	return err;
}

// ---- valgrind --------------------------------------------------------------

String IdeBridge::RunValgrind()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		if(!ide->IsValgrind()) { err = "Valgrind not found"; return; }
		ide->Valgrind();
	});
	return err;
}

// ---- assist suggestions ----------------------------------------------------

ValueArray IdeBridge::GetAssistSuggestions() const
{
	ValueArray out;
	GuiLock __;
	Ide* ide = TheIde();
	if(!ide) return out;
	for(int i = 0; i < ide->editor.assist_item.GetCount(); i++) {
		const AssistEditor::AssistItem& it = ide->editor.assist_item[i];
		ValueMap m;
		m.Add("name",   it.name);
		m.Add("pretty", it.pretty);
		m.Add("kind",   it.kind);
		out.Add(m);
	}
	return out;
}

// ---- assist menu actions ---------------------------------------------------

String IdeBridge::AssistGoto()
{
#ifndef flagV1
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->Goto();
	});
	return err;
#else
	return "assist.goto requires non-V1 build";
#endif
}

String IdeBridge::AssistUsage()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->Usage();
	});
	return err;
}

String IdeBridge::AssistQueryId()
{
#ifndef flagV1
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->QueryId();
	});
	return err;
#else
	return "assist.query requires non-V1 build";
#endif
}

String IdeBridge::AssistContextGoto()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->ContextGoto();
	});
	return err;
}

// ---- workspace management --------------------------------------------------

String IdeBridge::WorkspaceOpen(const String& package)
{
	String err;
	RunOnGui([&] {
		// If the "Select main package" dialog is open, feed it the package directly
		if(sActiveSelectPkgDlg) {
			sActiveSelectPkgDlg->selected      = package;
			sActiveSelectPkgDlg->selected_nest = String();
			sActiveSelectPkgDlg->Break(IDYES);
			return;
		}
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		InvalidatePackageCache();
		ide->main.Clear();
		ide->SetMain(package);
	});
	return err;
}

String IdeBridge::WorkspaceReload()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		String pkg = ide->main;
		if(pkg.IsEmpty()) { err = "No package open"; return; }
		ide->SetMain(pkg);
	});
	return err;
}

String IdeBridge::WorkspaceClose()
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->FlushFile();
		ide->SaveWorkspace();
		ide->main.Clear();
		ide->editfile.Clear();
		IdeAgain = true;
		ide->Break(IDOK);
	});
	return err;
}

// ---- assembly management ---------------------------------------------------

Vector<IdeBridge::AssemblyInfo> IdeBridge::ListAssemblies() const
{
	Vector<AssemblyInfo> out;
	// Read config files directly — no GUI lock needed
	for(FindFile ff(ConfigFile("*.var")); ff; ff.Next()) {
		AssemblyInfo a;
		a.name = GetFileTitle(ff.GetName());
		a.path = ff.GetPath();
		VectorMap<String, String> vars;
		LoadVarFile(ff.GetPath(), vars);
		a.upp_dirs  = SplitDirs(vars.Get("UPP",    String()));
		a.output_dir = vars.Get("OUTPUT", String());
		a.include    = vars.Get("INCLUDE", String());
		out.Add(pick(a));
	}
	return out;
}

String IdeBridge::GetActiveAssembly() const
{
	return GetVarsName();
}

String IdeBridge::SwitchAssembly(const String& name)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		if(!LoadVars(name)) { err = "Assembly not found: " + name; return; }
		// Reload the current package under the new assembly
		String pkg = ide->main;
		if(pkg.GetCount())
			ide->SetMain(pkg);
	});
	return err;
}

String IdeBridge::GetAssemblyPath(const String& name) const
{
	return VarFilePath(name);
}

ValueArray IdeBridge::ListPackagesInAssembly(const String& vars_name,
                                              const String& filter,
                                              const String& search) const
{
	ValueArray out;
	// Determine which var file to read
	String vname = vars_name.IsEmpty() ? GetVarsName() : vars_name;
	VectorMap<String, String> vars;
	if(!LoadVarFile(VarFilePath(vname), vars))
		return out;
	Vector<String> upp_dirs = SplitDirs(vars.Get("UPP", String()));

	bool want_main    = (filter == "main"    || filter == "all" || filter.IsEmpty());
	bool want_nonmain = (filter == "nonmain" || filter == "all" || filter.IsEmpty());

	Index<String> seen;
	for(const String& dir : upp_dirs) {
		// Each package is a subdirectory containing a <name>.upp file
		for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next()) {
			String pkg_name = ff.GetName();
			if(pkg_name == "." || pkg_name == "..") continue;
			String pkg_dir = AppendFileName(dir, pkg_name);
			if(!DirectoryExists(pkg_dir)) continue;
			String upp_file = AppendFileName(pkg_dir, pkg_name + ".upp");
			if(!FileExists(upp_file)) continue;
			if(!search.IsEmpty() && ToLower(pkg_name).Find(ToLower(search)) < 0)
				continue;
			if(seen.Find(pkg_name) >= 0) continue;
			seen.Add(pkg_name);
			out.Add(pkg_name);
		}
	}
	return out;
}

// ---- UppHub ----------------------------------------------------------------

String IdeBridge::OpenUppHub()
{
	String err;
	RunOnGui([&] {
		if(!TheIde()) { err = "IDE not available"; return; }
		UppHub();
	});
	return err;
}

END_UPP_NAMESPACE
#endif // flagGUI

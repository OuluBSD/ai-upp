#include "Pkg.h"
#include <cstdio>

namespace Upp {

void PkgUseMap::Jsonize(JsonIO& jio)
{
	jio("upp_flag", upp_flag)("scope", scope)("thread_model", thread_model);
}

void PkgUsePolicy::Jsonize(JsonIO& jio)
{
	jio("name", name)("description", description)("default_on", default_on);
	jio("maps_to", maps_to);
}

void PkgTargetProfile::Jsonize(JsonIO& jio)
{
	jio("name", name)("thread_model", thread_model)("compiler", compiler)("linker", linker)
	   ("sdk", sdk)("summary", summary);
	jio("forced_use", forced_use);
	jio("masked_use", masked_use);
	jio("provider_preferences", provider_preferences);
}

void PkgStateRecord::Jsonize(JsonIO& jio)
{
	jio("atom", atom)("target", target)("toolchain", toolchain)("build_status", build_status)
	   ("artifact_path", artifact_path)("timestamp", timestamp);
	jio("selected_use", selected_use);
	jio("declared_use", declared_use);
	jio("effective_flags", effective_flags);
	jio("providers", providers);
}

void PkgState::Jsonize(JsonIO& jio)
{
	jio("target", target)("toolchain", toolchain);
	jio("records", records);
}

void PkgEselectState::Jsonize(JsonIO& jio)
{
	jio("compiler", compiler)("linker", linker)("target", target)("provider", provider);
}

static String sJoin(const Vector<String>& v, const char *sep = " ")
{
	String out;
	for(int i = 0; i < v.GetCount(); i++) {
		if(i)
			out << sep;
		out << v[i];
	}
	return out;
}

static String sJoinPath(const Vector<String>& parts, int from, int to)
{
	String out;
	for(int i = from; i < to; i++) {
		if(i > from)
			out << '/';
		out << parts[i];
	}
	return out;
}

static String sLower(const String& s)
{
	return ToLower(s);
}

static bool sContainsWord(const String& text, const String& word)
{
	return sLower(text).Find(sLower(word)) >= 0;
}

static String sTrimPath(const String& path)
{
	String p = UnixPath(NormalizePath(path));
	while(p.EndsWith("/"))
		p.Remove(p.GetCount() - 1);
	return p;
}

static String sParentDir(String dir)
{
	dir = sTrimPath(dir);
	int p = dir.ReverseFind('/');
	if(p < 0)
		return Null;
	return dir.Left(p);
}

String PkgRepoRoot()
{
	String dir = sTrimPath(GetCurrentDirectory());
	for(int i = 0; i < 32; i++) {
		if(DirectoryExists(AppendFileName(dir, "uppsrc")) && FileExists(AppendFileName(dir, "README.md")))
			return dir;
		String parent = sParentDir(dir);
		if(parent.IsEmpty() || parent == dir)
			break;
		dir = parent;
	}
	return sTrimPath(GetCurrentDirectory());
}

PkgConfigPaths FindPkgConfigPaths(const String& root)
{
	PkgConfigPaths p;
	p.root = root;
	p.ai_dir = AppendFileName(root, ".ai-upp");
	p.world = AppendFileName(p.ai_dir, "world");
	p.package_use = AppendFileName(p.ai_dir, "package.use");
	p.package_provider = AppendFileName(p.ai_dir, "package.provider");
	p.package_target = AppendFileName(p.ai_dir, "package.target");
	p.state = AppendFileName(p.ai_dir, "state.json");
	p.eselect = AppendFileName(p.ai_dir, "eselect.json");
	return p;
}

static String sPackageAtomFromUpp(const String& repo_root, const String& upp_path, String& nest_out)
{
	String rel = UnixPath(NormalizePath(upp_path));
	rel.Remove(0, sTrimPath(repo_root).GetCount());
	while(rel.GetCount() && (rel[0] == '/' || rel[0] == '\\'))
		rel.Remove(0, 1);
	Vector<String> parts = Split(rel, '/');
	if(parts.GetCount() < 3)
		return Null;
	nest_out = parts[0];
	return sJoinPath(parts, 1, parts.GetCount() - 1);
}

static Vector<String> sExpandSourceFiles(const String& dir)
{
	static const char *patterns[] = {
		"*.cpp", "*.c", "*.h", "*.hpp", "*.icpp", "*.usc", "*.tpp"
	};
	Index<String> seen;
	Vector<String> out;
	for(const char *pattern : patterns) {
		Vector<String> found = FindAllPaths(dir, pattern);
		for(const String& f : found)
			if(seen.Find(f) < 0) {
				seen.Add(f);
				out.Add(f);
			}
	}
	return pick(out);
}

static String sPackageNameFromPath(const String& atom, const String& nest)
{
	if(nest == "uppsrc")
		return atom;
	return nest + "/" + atom;
}

void PkgRepository::Discover()
{
	root = PkgRepoRoot();
	paths = FindPkgConfigPaths(root);
	packages.Clear();
	nests.Clear();
	Vector<String> upp = FindAllPaths(root, "*.upp");
	for(const String& upp_path : upp) {
		String nest;
		String atom = sPackageAtomFromUpp(root, upp_path, nest);
		if(atom.IsEmpty())
			continue;
		PkgPackage p;
		Package pkg;
		if(!pkg.Load(upp_path))
			continue;
		p.atom = atom;
		p.name = sPackageNameFromPath(atom, nest);
		p.nest = nest;
		p.path = upp_path;
		p.dir = GetFileFolder(upp_path);
		p.description = pkg.description;
		p.accepts.Clear();
		for(int i = 0; i < pkg.accepts.GetCount(); i++)
			p.accepts.Add(pkg.accepts[i]);
		p.mtime = pkg.time;
		for(int i = 0; i < pkg.uses.GetCount(); i++)
			p.uses.Add(pkg.uses[i].text);
		for(int i = 0; i < pkg.config.GetCount(); i++)
			p.mainconfig.Add(pkg.config[i].name);
		Vector<String> sources = sExpandSourceFiles(p.dir);
		for(const String& s : sources)
			p.source_files.Add(s);
		packages.Add(pick(p));
		if(FindIndex(nests, nest) < 0)
			nests.Add(nest);
	}
}

static int sPkgScore(const PkgPackage& p, const String& atom)
{
	if(p.name == atom)
		return p.nest == "uppsrc" ? 0 : 1;
	if(p.atom == atom)
		return p.nest == "uppsrc" ? 0 : 1;
	String qualified = p.nest + "/" + p.atom;
	if(qualified == atom)
		return 0;
	return 1000;
}

const PkgPackage* PkgRepository::Find(const String& atom) const
{
	int best = -1;
	int best_score = 1000000;
	for(int i = 0; i < packages.GetCount(); i++) {
		int q = sPkgScore(packages[i], atom);
		if(q < best_score) {
			best_score = q;
			best = i;
		}
	}
	return best >= 0 && best_score < 1000 ? &packages[best] : nullptr;
}

Vector<const PkgPackage*> PkgRepository::Search(const String& query) const
{
	Vector<const PkgPackage*> found;
	String q = sLower(query);
	for(int i = 0; i < packages.GetCount(); i++) {
		const PkgPackage& p = packages[i];
		String hay = sLower(p.atom + " " + p.name + " " + p.nest + " " + p.description + " " + p.path);
		if(hay.Find(q) >= 0)
			found.Add(&p);
	}
	return pick(found);
}

bool PkgRepository::HasPackage(const String& atom) const
{
	return Find(atom) != nullptr;
}

static Vector<PkgUsePolicy> sUsePolicies()
{
	Vector<PkgUsePolicy> v;

	{
		PkgUsePolicy& p = v.Add();
		p.name = "sqlite";
		p.description = "Enable SQLite support";
		p.default_on = false;
		PkgUseMap& m = p.maps_to.Add();
		m.upp_flag = "SQLITE";
		m.scope = "accepted";
	}
	{
		PkgUsePolicy& p = v.Add();
		p.name = "st";
		p.description = "Build single-threaded variant";
		p.default_on = false;
		PkgUseMap& m0 = p.maps_to.Add();
		m0.thread_model = "st";
		PkgUseMap& m1 = p.maps_to.Add();
		m1.upp_flag = "ST";
		m1.scope = "global";
	}
	{
		PkgUsePolicy& p = v.Add();
		p.name = "gtk";
		p.description = "Enable GTK UI integration";
		p.default_on = false;
		PkgUseMap& m = p.maps_to.Add();
		m.upp_flag = "GTK";
		m.scope = "accepted";
	}
	return pick(v);
}

static const PkgUsePolicy* sFindPolicy(const String& name)
{
	static Vector<PkgUsePolicy> policies = sUsePolicies();
	for(int i = 0; i < policies.GetCount(); i++)
		if(policies[i].name == name)
			return &policies[i];
	return nullptr;
}

static Vector<PkgTargetProfile> sTargets()
{
	Vector<PkgTargetProfile> v;
	{
		PkgTargetProfile& p = v.Add();
		p.name = "native";
		p.thread_model = "mt";
		p.compiler = "any";
		p.linker = "any";
		p.summary = "Native host target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "native-clang";
		p.thread_model = "mt";
		p.compiler = "clang";
		p.linker = "lld";
		p.summary = "Native clang target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "native-gcc";
		p.thread_model = "mt";
		p.compiler = "gcc";
		p.linker = "ld";
		p.summary = "Native gcc target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "windows-msvc";
		p.thread_model = "mt";
		p.compiler = "msvc";
		p.linker = "link";
		p.summary = "Windows MSVC target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "windows-mingw";
		p.thread_model = "mt";
		p.compiler = "mingw";
		p.linker = "ld";
		p.summary = "Windows MinGW target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "wasm-browser";
		p.thread_model = "st";
		p.compiler = "emcc";
		p.linker = "emcc";
		p.summary = "Browser wasm target";
		p.forced_use.Add("st");
		p.masked_use.Add("gtk");
		p.masked_use.Add("gui");
		p.provider_preferences.Add("upp-plugin");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "wasm-node";
		p.thread_model = "st";
		p.compiler = "emcc";
		p.linker = "emcc";
		p.summary = "Node wasm target";
		p.forced_use.Add("st");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "linux-fb";
		p.thread_model = "mt";
		p.compiler = "gcc";
		p.linker = "ld";
		p.summary = "Linux framebuffer target";
		p.masked_use.Add("gui");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "retro-x86";
		p.thread_model = "st";
		p.compiler = "gcc";
		p.linker = "ld";
		p.summary = "Retro x86 target";
		p.forced_use.Add("st");
		p.masked_use.Add("gui");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "freebsd-native";
		p.thread_model = "mt";
		p.compiler = "clang";
		p.linker = "ld";
		p.summary = "FreeBSD native target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "macos-native";
		p.thread_model = "mt";
		p.compiler = "clang";
		p.linker = "ld64";
		p.summary = "macOS native target";
	}
	return pick(v);
}

static const PkgTargetProfile* sFindTarget(const String& name)
{
	static Vector<PkgTargetProfile> targets = sTargets();
	for(int i = 0; i < targets.GetCount(); i++)
		if(targets[i].name == name)
			return &targets[i];
	return nullptr;
}

static Vector<PkgProvider> sProviders()
{
	Vector<PkgProvider> v;
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sqlite";
		p.kind = "upp-plugin";
		p.provider = "plugin/sqlite3";
		p.details = "Repository-local sqlite3 plugin";
		p.priority = 100;
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/ssl";
		p.kind = "upp-plugin";
		p.provider = "Core/SSL";
		p.details = "Core SSL package";
		p.priority = 90;
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sdl2";
		p.kind = "system";
		p.provider = "SDL2";
		p.details = "System SDL2 package";
		p.priority = 10;
		p.system_install = true;
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/opengl";
		p.kind = "upp-plugin";
		p.provider = "GLDraw";
		p.details = "Repository-local GLDraw package";
		p.priority = 80;
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/gui-runtime";
		p.kind = "system";
		p.provider = "desktop-gui";
		p.details = "Host GUI runtime";
		p.priority = 5;
		p.system_install = true;
	}
	return pick(v);
}

static Vector<PkgProvider> sProvidersFor(const String& capability)
{
	Vector<PkgProvider> all = sProviders();
	Vector<PkgProvider> out;
	for(const PkgProvider& p : all)
		if(p.capability == capability) {
			PkgProvider& q = out.Add();
			q.capability = p.capability;
			q.kind = p.kind;
			q.provider = p.provider;
			q.details = p.details;
			q.priority = p.priority;
			q.system_install = p.system_install;
			for(const String& s : p.upp_flags)
				q.upp_flags.Add(s);
			for(const String& s : p.uses)
				q.uses.Add(s);
		}
	return pick(out);
}

static bool sIsSelected(const Vector<String>& v, const String& s)
{
	return FindIndex(v, s) >= 0;
}

static String sUppFlagSpelling(const String& name, bool accepted)
{
	return accepted ? "." + name : name;
}

static String sMacroName(const String& name)
{
	return "flag" + name;
}

bool IsGlobalAuditFlag(const String& flag)
{
	static const char *skip[] = {
		"GUI", "DEBUG", "RELEASE", "POSIX", "WIN32", "LINUX",
		"MSC", "GCC", "CLANG", "BLITZ", "MAIN", "X11", "COCOA",
		"FREEBSD", "MACOS", "ANDROID", "MINGW", "MSVC"
	};
	for(const char *s : skip)
		if(flag == s)
			return true;
	return false;
}

static Vector<String> sReadWorld(const PkgConfigPaths& paths)
{
	Vector<String> v;
	String txt = LoadFile(paths.world);
	Vector<String> lines = Split(txt, '\n');
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.IsEmpty() || line.StartsWith("#"))
			continue;
		v.Add(line);
	}
	return pick(v);
}

static void sWriteWorld(const PkgConfigPaths& paths, const Vector<String>& world)
{
	RealizePath(paths.ai_dir);
	String out;
	for(const String& s : world)
		out << s << '\n';
	SaveFile(paths.world, out);
}

static void sAddWorld(const PkgConfigPaths& paths, const String& atom)
{
	Vector<String> world = sReadWorld(paths);
	if(FindIndex(world, atom) < 0) {
		world.Add(atom);
		sWriteWorld(paths, world);
	}
}

static bool sPromptYesNo()
{
	Cout() << "Would you like to build these packages? [Yes/No] ";
	Cout().Flush();
	char buf[64];
	if(!fgets(buf, sizeof(buf), stdin))
		return false;
	String ans = TrimBoth(String(buf));
	return ans.StartsWith("y") || ans.StartsWith("Y");
}

static bool sIsLongOpt(const String& s, const char *name)
{
	return s == String("--") + name || s.StartsWith(String("--") + name + "=");
}

static String sOptValue(const String& s)
{
	int p = s.Find('=');
	return p >= 0 ? s.Mid(p + 1) : Null;
}

static bool sParseColorValue(const String& v, PkgColorMode& mode)
{
	String x = ToLower(TrimBoth(v));
	if(x == "y" || x == "yes" || x == "on" || x == "1")
		mode = PKG_COLOR_YES;
	else if(x == "n" || x == "no" || x == "off" || x == "0")
		mode = PKG_COLOR_NO;
	else if(x == "auto")
		mode = PKG_COLOR_AUTO;
	else
		return false;
	return true;
}

static bool sIsOption(const String& s)
{
	return s.GetCount() > 1 && s[0] == '-';
}

bool ParsePkgArgs(const Vector<String>& args, PkgInvocation& inv, String& error)
{
	Vector<String> positional;
	for(int i = 1; i < args.GetCount(); i++) {
		String a = args[i];
		if(a == "--")
			continue;
		if(a.StartsWith("--")) {
			if(a == "--help") inv.command = PKG_CMD_HELP;
			else if(a == "--version") inv.command = PKG_CMD_VERSION;
			else if(a == "--info") inv.info = true, inv.command = PKG_CMD_INFO;
			else if(a == "--metadata") inv.metadata = true, inv.command = PKG_CMD_METADATA;
			else if(a == "--list-sets") inv.list_sets = true;
			else if(a == "--pretend") inv.pretend = true;
			else if(a == "--ask") inv.ask = true;
			else if(a == "--verbose") inv.verbose = true;
			else if(a == "--update") inv.update = true;
			else if(a == "--deep") inv.deep = true;
			else if(a == "--newuse") inv.newuse = true;
			else if(a == "--resume") inv.resume = true;
			else if(a == "--oneshot") inv.oneshot = true;
			else if(a == "--plan") inv.plan = true;
			else if(a == "--install") inv.install = true;
			else if(a == "--patch") inv.audit_patch = true;
			else if(a == "--search") inv.command = PKG_CMD_SEARCH;
			else if(a == "--color" || a.StartsWith("--color=")) {
				String v = a == "--color" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				if(!sParseColorValue(v, inv.color)) {
					error = "invalid --color value: " + v;
					return false;
				}
			}
			else if(a == "--colour" || a.StartsWith("--colour=")) {
				String v = a == "--colour" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				if(!sParseColorValue(v, inv.color)) {
					error = "invalid --colour value: " + v;
					return false;
				}
			}
			else if(a == "--target" || a.StartsWith("--target=")) {
				inv.target = a == "--target" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--root" || a.StartsWith("--root=")) {
				inv.root = a == "--root" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--sysroot" || a.StartsWith("--sysroot=")) {
				inv.sysroot = a == "--sysroot" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else
				inv.extra.Add(a);
			continue;
		}
		if(a.StartsWith("-") && a.GetCount() > 1) {
			for(int j = 1; j < a.GetCount(); j++) {
				switch(a[j]) {
				case 'a': inv.ask = true; break;
				case 'v': inv.verbose = true; break;
				case 'u': inv.update = true; break;
				case 'D': inv.deep = true; break;
				case 'N': inv.newuse = true; break;
				case 'p': inv.pretend = true; inv.plan = true; break;
				case 's': inv.command = PKG_CMD_SEARCH; break;
				default:
					String msg;
					msg << "unknown option: -" << a[j];
					error = msg;
					return false;
				}
			}
			continue;
		}
		positional.Add(a);
	}

	if(inv.command == PKG_CMD_NONE) {
		if(positional.GetCount()) {
			String cmd = positional[0];
			if(cmd == "help") inv.command = PKG_CMD_HELP;
			else if(cmd == "version") inv.command = PKG_CMD_VERSION;
			else if(cmd == "info") inv.command = PKG_CMD_INFO;
			else if(cmd == "metadata") inv.command = PKG_CMD_METADATA;
			else if(cmd == "explain-use") inv.command = PKG_CMD_EXPLAIN_USE;
			else if(cmd == "deps") inv.command = PKG_CMD_DEPS;
			else if(cmd == "target") inv.command = PKG_CMD_TARGET;
			else if(cmd == "eselect") inv.command = PKG_CMD_ESELECT;
			else if(cmd == "audit-acceptflags") inv.command = PKG_CMD_AUDIT_ACCEPTFLAGS;
			else if(cmd == "resume") inv.command = PKG_CMD_RESUME;
			else if(cmd == "search") inv.command = PKG_CMD_SEARCH;
			else inv.command = PKG_CMD_PLAN;
		}
		else if(inv.list_sets)
			inv.command = PKG_CMD_INFO;
		else if(inv.metadata)
			inv.command = PKG_CMD_METADATA;
		else if(inv.info)
			inv.command = PKG_CMD_INFO;
		else if(inv.search)
			inv.command = PKG_CMD_SEARCH;
		else if(inv.resume)
			inv.command = PKG_CMD_RESUME;
		else if(inv.ask || inv.verbose || inv.update || inv.deep || inv.newuse || inv.pretend)
			inv.command = PKG_CMD_PLAN;
		else
			inv.command = PKG_CMD_HELP;
	}

	if(positional.GetCount()) {
		int start = 0;
		if(positional[0] == "help" || positional[0] == "version" || positional[0] == "info" ||
		   positional[0] == "metadata" || positional[0] == "explain-use" || positional[0] == "deps" ||
		   positional[0] == "target" || positional[0] == "eselect" || positional[0] == "audit-acceptflags" ||
		   positional[0] == "resume" || positional[0] == "search")
			start = 1;

		Vector<String> rest;
		for(int i = start; i < positional.GetCount(); i++)
			rest.Add(positional[i]);

		if(inv.command == PKG_CMD_SEARCH) {
			inv.query = rest.GetCount() ? rest[0] : Null;
		}
		else if(inv.command == PKG_CMD_TARGET) {
			inv.subcommand = rest.GetCount() ? rest[0] : "list";
			if(rest.GetCount() > 1)
				inv.target = rest[1];
		}
		else if(inv.command == PKG_CMD_ESELECT) {
			inv.module = rest.GetCount() ? rest[0] : "help";
			if(rest.GetCount() > 1)
				inv.subcommand = rest[1];
			if(rest.GetCount() > 2)
				inv.target = rest[2];
		}
		else if(inv.command == PKG_CMD_EXPLAIN_USE || inv.command == PKG_CMD_DEPS ||
		        inv.command == PKG_CMD_PLAN || inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS ||
		        inv.command == PKG_CMD_RESUME) {
			if(rest.GetCount())
				inv.atom = rest[0];
			for(int i = 1; i < rest.GetCount(); i++)
				inv.use_args.Add(rest[i]);
		}
		else if(inv.command == PKG_CMD_INFO || inv.command == PKG_CMD_METADATA || inv.command == PKG_CMD_VERSION ||
		        inv.command == PKG_CMD_HELP || inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS) {
			if(rest.GetCount())
				inv.atom = rest[0];
			for(int i = 1; i < rest.GetCount(); i++)
				inv.extra.Add(rest[i]);
		}
		else {
			if(rest.GetCount())
				inv.atom = rest[0];
			for(int i = 1; i < rest.GetCount(); i++)
				inv.use_args.Add(rest[i]);
		}
	}

	if(inv.command == PKG_CMD_SEARCH && inv.query.IsEmpty() && !inv.atom.IsEmpty())
		inv.query = inv.atom;
	if(inv.command == PKG_CMD_PLAN && inv.atom.IsEmpty() && positional.GetCount())
		inv.atom = positional.Top();

	return true;
}

static String sAnsi(const char *code, const String& text, bool color)
{
	return color ? String("\x1b[") + code + "m" + text + "\x1b[0m" : text;
}

static bool sUseColor(const PkgInvocation& inv)
{
	String force = GetEnv("CLICOLOR_FORCE");
	if(inv.color == PKG_COLOR_NO)
		return false;
	if(inv.color == PKG_COLOR_YES)
		return true;
	if(!GetEnv("NO_COLOR").IsEmpty())
		return false;
	if(!force.IsEmpty() && force != "0")
		return true;
	String term = GetEnv("TERM");
	return !term.IsEmpty();
}

static String sFmtList(const Vector<String>& v)
{
	return v.GetCount() ? sJoin(v, " ") : String("[none]");
}

static void sPrintHelp(bool color)
{
	Cout()
		<< "Usage: pkg [options] <command> [args]\n\n"
		<< "Common options:\n"
		<< "  -a, --ask            show a plan and ask before changing anything\n"
		<< "  -v, --verbose        show USE, target, provider, and U++ projection details\n"
		<< "  -u, --update         update selected packages to the best available revision\n"
		<< "  -D, --deep           resolve the full dependency graph\n"
		<< "  -N, --newuse         include packages whose USE metadata changed\n"
		<< "  -p, --pretend        print a plan without changing anything\n"
		<< "      --color <y|n|auto>\n"
		<< "      --colour <yes|no|auto>\n\n"
		<< "Commands:\n"
		<< "  --help, help\n"
		<< "  --version, version\n"
		<< "  --info, info\n"
		<< "  --metadata, metadata\n"
		<< "  -s, --search <query>\n"
		<< "  deps <atom> [USE flags...] --plan\n"
		<< "  explain-use <atom> [USE flags...]\n"
		<< "  target list|info <name>|set <name>\n"
		<< "  eselect ...\n"
		<< "  audit-acceptflags [atom] [--patch]\n"
		<< "  -avuDN @world\n\n"
		<< "Recognized sets: @world, @system, @toolchain\n";
	(void)color;
}

static void sPrintVersion()
{
	Cout() << "pkg 0.1\n";
}

static void sPrintSets(const PkgRepository& repo)
{
	Vector<String> world = sReadWorld(repo.paths);
	Cout() << "Built-in sets:\n";
	Cout() << "  @system    bootstrap essentials: build, umk, pkg\n";
	Cout() << "  @world     user-selected top-level packages (" << world.GetCount() << " entries)\n";
	Cout() << "  @toolchain selected compiler/linker/sdk/tool profiles\n";
}

static const PkgTargetProfile& sDefaultTargetProfile(const String& target)
{
	static Vector<PkgTargetProfile> targets = sTargets();
	const PkgTargetProfile* p = sFindTarget(target);
	if(p)
		return *p;
	return targets[0];
}

static void sTokenizeUseArgs(const Vector<String>& use_args, Vector<String>& selected, Vector<String>& disabled)
{
	for(const String& a : use_args) {
		if(a.StartsWith("+"))
			selected.Add(a.Mid(1));
		else if(a.StartsWith("-"))
			disabled.Add(a.Mid(1));
		else
			selected.Add(a);
	}
}

static void sPolicyFlags(const Vector<String>& use_args, const String& target, Vector<String>& selected, Vector<String>& disabled, Vector<String>& defaulted, Vector<String>& effective)
{
	Vector<String> target_forced;
	Vector<String> target_masked;
	const PkgTargetProfile& tp = sDefaultTargetProfile(target);
	selected = Vector<String>();
	disabled = Vector<String>();
	defaulted = Vector<String>();
	for(int i = 0; i < tp.forced_use.GetCount(); i++)
		target_forced.Add(tp.forced_use[i]);
	for(int i = 0; i < tp.masked_use.GetCount(); i++)
		target_masked.Add(tp.masked_use[i]);
	sTokenizeUseArgs(use_args, selected, disabled);
	for(const String& f : target_forced)
		selected.Add(f);
	for(const String& f : target_masked)
		disabled.Add(f);
	const Vector<PkgUsePolicy> policies = sUsePolicies();
	for(const PkgUsePolicy& p : policies) {
		bool sel = sIsSelected(selected, p.name);
		bool dis = sIsSelected(disabled, p.name);
		if(!sel && !dis && p.default_on)
			defaulted.Add(p.name);
	}
	for(const PkgUsePolicy& p : policies)
		if(sIsSelected(selected, p.name) || sIsSelected(defaulted, p.name))
			effective.Add(p.name);
}

static void sExplainUse(const PkgInvocation& inv, const PkgRepository& repo)
{
	(void)repo;
	Vector<String> selected;
	Vector<String> disabled;
	Vector<String> defaulted;
	Vector<String> effective;
	sPolicyFlags(inv.use_args, inv.target, selected, disabled, defaulted, effective);
	const PkgTargetProfile& tp = sDefaultTargetProfile(inv.target);

	Cout() << "Use policy for " << (inv.atom.IsEmpty() ? String("[unknown atom]") : inv.atom) << "\n";
	Cout() << "Target: " << (inv.target.IsEmpty() ? String("native") : inv.target) << "\n";
	Cout() << "Thread model: " << tp.thread_model << "\n\n";

	Cout() << "Selected USE:\n";
	for(const String& s : selected)
		Cout() << "  +" << s << "\n";
	Cout() << "Defaulted USE:\n";
	if(defaulted.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& s : defaulted)
			Cout() << "  " << s << "\n";
	Cout() << "Disabled USE:\n";
	if(disabled.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& s : disabled)
			Cout() << "  -" << s << "\n";

	Cout() << "\nMapped U++ flags:\n";
	Index<String> macros;
	Vector<String> uppflags;
	for(const String& s : effective) {
		const PkgUsePolicy* p = sFindPolicy(s);
		if(!p)
			continue;
		for(const PkgUseMap& m : p->maps_to) {
			String spelling = sUppFlagSpelling(m.upp_flag, m.scope == "accepted");
			Cout() << "  " << p->name << " -> " << spelling << " (scope: " << m.scope << ")\n";
			macros.Add(sMacroName(m.upp_flag));
			uppflags.Add(spelling);
		}
	}
	if(macros.IsEmpty())
		Cout() << "  [none]\n";
	else {
		Vector<String> macro_list;
		for(int i = 0; i < macros.GetCount(); i++)
			macro_list.Add(macros[i]);
		Cout() << "\nC++ macros:\n  " << sJoin(macro_list, ", ") << "\n";
	}
	Cout() << "\numk spelling: " << (uppflags.IsEmpty() ? String("[none]") : sJoin(uppflags, " ")) << "\n\n";
}

static String sTargetThreadModel(const String& target)
{
	const PkgTargetProfile& tp = sDefaultTargetProfile(target);
	return tp.thread_model;
}

static Vector<String> sTargetEffectiveFlags(const String& target)
{
	Vector<String> flags;
	const PkgTargetProfile& tp = sDefaultTargetProfile(target);
	for(const String& s : tp.forced_use)
		flags.Add(s);
	return flags;
}

static void sPrintTargetInfo(const String& name)
{
	const PkgTargetProfile* t = sFindTarget(name);
	if(!t) {
		Cout() << "Unknown target: " << name << "\n";
		return;
	}
	Cout() << "target: " << t->name << "\n";
	Cout() << "thread_model: " << t->thread_model << "\n";
	Cout() << "compiler: " << t->compiler << "\n";
	Cout() << "linker: " << t->linker << "\n";
	Cout() << "sdk: " << (t->sdk.IsEmpty() ? String("[none]") : t->sdk) << "\n";
	Cout() << "forced_use: " << sFmtList(t->forced_use) << "\n";
	Cout() << "masked_use: " << sFmtList(t->masked_use) << "\n";
	Cout() << "provider_preferences: " << sFmtList(t->provider_preferences) << "\n";
	Cout() << "summary: " << t->summary << "\n";
}

static void sListTargets()
{
	Vector<PkgTargetProfile> targets = sTargets();
	for(const PkgTargetProfile& t : targets)
		Cout() << t.name << " [" << t.thread_model << "] - " << t.summary << "\n";
}

static String sResolveProvider(const String& capability, const PkgRepository& repo)
{
	Vector<PkgProvider> providers = sProvidersFor(capability);
	for(const PkgProvider& p : providers) {
		if(p.kind == "upp-plugin" && repo.HasPackage(p.provider))
			return p.provider;
	}
	return providers.GetCount() ? providers[0].provider : Null;
}

static void sPrintDeps(const PkgInvocation& inv, const PkgRepository& repo)
{
	Vector<String> selected;
	Vector<String> disabled;
	Vector<String> defaulted;
	Vector<String> effective;
	sPolicyFlags(inv.use_args, inv.target, selected, disabled, defaulted, effective);
	const PkgTargetProfile& tp = sDefaultTargetProfile(inv.target);
	Vector<String> virtuals;
	if(sIsSelected(effective, "sqlite"))
		virtuals.Add("virtual/sqlite");
	if(sIsSelected(effective, "st"))
		virtuals.Add("virtual/gui-runtime");
	if(sIsSelected(effective, "gtk"))
		virtuals.Add("virtual/gui-runtime");

	Cout() << "Dependencies for " << inv.atom << "\n";
	Cout() << "Target: " << (inv.target.IsEmpty() ? String("native") : inv.target) << " (" << tp.thread_model << ")\n";
	Cout() << "Virtual requirements:\n";
	if(virtuals.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& v : virtuals)
			Cout() << "  " << v << "\n";

	Cout() << "\nSelected providers:\n";
	Vector<String> providers;
	for(const String& v : virtuals) {
		String provider = sResolveProvider(v, repo);
		if(!provider.IsEmpty()) {
			providers.Add(provider);
			Cout() << "  " << v << " -> " << provider << "\n";
		}
	}
	if(providers.IsEmpty())
		Cout() << "  [none]\n";

	Cout() << "\nU++ uses additions:\n";
	if(providers.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& p : providers)
			Cout() << "  " << p << "\n";

	Cout() << "\nU++ flag additions:\n";
	Vector<String> uppflags;
	for(const String& s : effective) {
		const PkgUsePolicy* p = sFindPolicy(s);
		if(!p)
			continue;
		for(const PkgUseMap& m : p->maps_to)
			uppflags.Add(sUppFlagSpelling(m.upp_flag, m.scope == "accepted"));
	}
	if(uppflags.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& f : uppflags)
			Cout() << "  " << f << "\n";

	bool need_system = false;
	for(const String& v : virtuals) {
		Vector<PkgProvider> choices = sProvidersFor(v);
		for(const PkgProvider& p : choices)
			if(p.kind == "system")
				need_system = true;
	}
	Cout() << "\nSystem installation required: " << (need_system ? "yes" : "no") << "\n";
}

static void sPrintSearch(const PkgRepository& repo, const String& query)
{
	Vector<const PkgPackage*> found = repo.Search(query);
	Cout() << "[ Results for search key : " << query << " ]\n";
	Cout() << "Searching...\n\n";
	for(const PkgPackage* p : found) {
		Cout() << "*  " << p->name << "\n";
		Cout() << "      Latest version available: 0\n";
		Cout() << "      Latest version installed: [ Not Installed ]\n";
		Cout() << "      Homepage:      [ none ]\n";
		Cout() << "      Description:   " << (p->description.IsEmpty() ? "U++ package " + p->name : p->description) << "\n";
		Cout() << "      Repository:    " << p->nest << "\n\n";
	}
	Cout() << "[ Packages found : " << found.GetCount() << " ]\n";
}

static void sPrintMetadata(const PkgRepository& repo)
{
	Cout() << "Repository root: " << repo.paths.root << "\n";
	Cout() << "Package count: " << repo.packages.GetCount() << "\n";
	Cout() << "Nest count: " << repo.nests.GetCount() << "\n";
	Cout() << "State file: " << repo.paths.state << "\n";
	Cout() << "World file: " << repo.paths.world << "\n";
	Cout() << "Package.use: " << repo.paths.package_use << "\n";
	Cout() << "Package.provider: " << repo.paths.package_provider << "\n";
	Cout() << "Package.target: " << repo.paths.package_target << "\n";
}

static void sPrintInfo(const PkgRepository& repo, const PkgInvocation& inv)
{
	Vector<String> world = sReadWorld(repo.paths);
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	PkgEselectState eselect;
	LoadFromJsonFile(eselect, repo.paths.eselect);
	Cout() << "Repository root: " << repo.paths.root << "\n";
	Cout() << "State file: " << repo.paths.state << "\n";
	Cout() << "World file: " << repo.paths.world << "\n";
	Cout() << "Package.use: " << repo.paths.package_use << "\n";
	Cout() << "Package.provider: " << repo.paths.package_provider << "\n";
	Cout() << "Package.target: " << repo.paths.package_target << "\n";
	Cout() << "Eselect file: " << repo.paths.eselect << "\n";
	Cout() << "World entries: " << world.GetCount() << "\n";
	Cout() << "Active target: " << (!inv.target.IsEmpty() ? inv.target : (state.target.IsEmpty() ? String("[none]") : state.target)) << "\n";
	Cout() << "Active toolchain: " << (state.toolchain.IsEmpty() ? String("[none]") : state.toolchain) << "\n";
	Cout() << "Selected compiler: " << (eselect.compiler.IsEmpty() ? String("[none]") : eselect.compiler) << "\n";
	Cout() << "Selected linker: " << (eselect.linker.IsEmpty() ? String("[none]") : eselect.linker) << "\n";
	Cout() << "Selected provider: " << (eselect.provider.IsEmpty() ? String("[none]") : eselect.provider) << "\n";
}

static void sWriteState(const PkgRepository& repo, const PkgInvocation& inv, const Vector<String>& selected, const Vector<String>& effective, const Vector<String>& providers)
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	state.target = inv.target;
	if(state.toolchain.IsEmpty())
		state.toolchain = "native";
	PkgStateRecord& rec = state.records.Add();
	rec.atom = inv.atom;
	rec.target = inv.target;
	rec.toolchain = state.toolchain;
	rec.build_status = inv.pretend ? "pretend" : "mock";
	rec.artifact_path = repo.root + "/bin/umk.exe";
	rec.timestamp = GetSysTime();
	for(const String& s : selected)
		rec.selected_use.Add(s);
	for(const String& s : effective)
		rec.effective_flags.Add(s);
	for(const String& s : providers)
		rec.providers.Add(s);
	RealizePath(repo.paths.ai_dir);
	StoreAsJsonFile(state, repo.paths.state, true);
}

static void sPrintPlan(const PkgInvocation& inv, const PkgRepository& repo)
{
	Vector<String> selected;
	Vector<String> disabled;
	Vector<String> defaulted;
	Vector<String> effective;
	sPolicyFlags(inv.use_args, inv.target, selected, disabled, defaulted, effective);
	const PkgTargetProfile& tp = sDefaultTargetProfile(inv.target);
	Vector<String> providers;
	Vector<String> virtuals;
	Vector<String> uppflags;
	if(sIsSelected(effective, "sqlite"))
		virtuals.Add("virtual/sqlite");
	if(sIsSelected(effective, "gtk") || sIsSelected(effective, "gui") || tp.thread_model == "st")
		virtuals.Add("virtual/gui-runtime");
	for(const String& v : virtuals) {
		String provider = sResolveProvider(v, repo);
		if(!provider.IsEmpty())
			providers.Add(provider);
	}
	for(const String& s : effective) {
		const PkgUsePolicy* p = sFindPolicy(s);
		if(!p)
			continue;
		for(const PkgUseMap& m : p->maps_to)
			uppflags.Add(sUppFlagSpelling(m.upp_flag, m.scope == "accepted"));
	}

	Cout() << "These are the packages that would be built, in order:\n\n";
	Cout() << "Calculating dependencies... done!\n";
	Cout() << "Dependency resolution took 0.00 s (backtrack: 0/20).\n\n";
	Cout() << "[ebuild  N    ] " << inv.atom << " USE=\"" << sJoin(effective, " ") << "\" TARGET=\""
	     << (inv.target.IsEmpty() ? String("native") : inv.target) << "\"";
	if(!inv.use_args.IsEmpty())
		Cout() << " UPPFLAGS=\"" << (uppflags.IsEmpty() ? String("[none]") : sJoin(uppflags, " ")) << "\"";
	Cout() << "\n";
	for(const String& p : providers)
		Cout() << "  provider: " << p << "\n";
	Cout() << "\n";
	sWriteState(repo, inv, selected, effective, providers);
	if(!inv.oneshot && !inv.pretend)
		sAddWorld(repo.paths, inv.atom);
}

static void sPrintTargetCommand(const PkgInvocation& inv)
{
	if(inv.subcommand.IsEmpty() || inv.subcommand == "list")
		sListTargets();
	else if(inv.subcommand == "info")
		sPrintTargetInfo(inv.target);
	else if(inv.subcommand == "set") {
		RealizePath(FindPkgConfigPaths(PkgRepoRoot()).ai_dir);
		PkgEselectState st;
		LoadFromJsonFile(st, FindPkgConfigPaths(PkgRepoRoot()).eselect);
		st.target = inv.target;
		StoreAsJsonFile(st, FindPkgConfigPaths(PkgRepoRoot()).eselect, true);
		Cout() << "target set to " << inv.target << "\n";
	}
	else
		Cout() << "unknown target subcommand: " << inv.subcommand << "\n";
}

static void sPrintEselectCommand(const PkgInvocation& inv)
{
	Cout() << "Usage: pkg eselect <global options> <module name> <module options>\n\n";
	Cout() << "Built-in modules:\n";
	Cout() << "  help\n  usage\n  version\n\n";
	Cout() << "Extra modules:\n";
	Cout() << "  compiler\n  linker\n  binutils\n  visualstudio\n  vcpkg\n  msys2\n  emscripten\n  target\n  profile\n  provider\n  repository\n  news\n";
	if(inv.module == "compiler" && inv.subcommand == "set") {
		PkgEselectState st;
		LoadFromJsonFile(st, FindPkgConfigPaths(PkgRepoRoot()).eselect);
		st.compiler = inv.target;
		StoreAsJsonFile(st, FindPkgConfigPaths(PkgRepoRoot()).eselect, true);
		Cout() << "compiler set to " << inv.target << "\n";
	}
	else if(inv.module == "target" && inv.subcommand == "set") {
		PkgInvocation t;
		t.subcommand = "set";
		t.target = inv.target;
		sPrintTargetCommand(t);
	}
}

static void sAuditAcceptFlags(const PkgInvocation& inv, const PkgRepository& repo)
{
	Vector<const PkgPackage*> targets;
	if(inv.atom.IsEmpty()) {
		for(const PkgPackage& p : repo.packages)
			targets.Add(&p);
	}
	else {
		const PkgPackage* p = repo.Find(inv.atom);
		if(p)
			targets.Add(p);
	}

	for(const PkgPackage* p : targets) {
		Index<String> used;
		for(const String& src : p->source_files) {
			String txt = LoadFile(src);
			if(txt.IsEmpty())
				continue;
			Vector<String> lines = Split(txt, '\n');
			for(String line : lines) {
				int sl = line.Find("//");
				if(sl >= 0)
					line = line.Left(sl);
				for(;;) {
					int q = line.Find("flag");
					if(q < 0)
						break;
					bool before = q == 0 || !(IsAlNum(line[q - 1]) || line[q - 1] == '_');
					int s = q + 4;
					if(before && s < line.GetCount() && (IsAlNum(line[s]) || line[s] == '_')) {
						String flag;
						while(s < line.GetCount() && (IsAlNum(line[s]) || line[s] == '_'))
							flag.Cat(line[s++]);
						if(!IsGlobalAuditFlag(flag))
							used.Add(flag);
						line = line.Mid(s);
					}
					else
						line = line.Mid(q + 4);
				}
			}
		}
		Index<String> accepts;
		for(const String& a : p->accepts)
			accepts.Add(a);
		for(int i = 0; i < used.GetCount(); i++) {
			if(accepts.Find(used[i]) < 0) {
				Cout() << "[warn] " << p->name << " uses flag" << used[i] << " but "
				     << GetFileName(p->path) << " does not accept " << used[i] << "\n";
				Cout() << "[hint] add: acceptflags " << used[i] << ";\n";
			}
		}
		for(const String& a : p->accepts)
			if(used.Find(a) < 0)
				Cout() << "[info] " << p->name << " accepts " << a << " but no source flag was found\n";
	}
	if(inv.audit_patch)
		Cout() << "\n[patch] suggestion only - no files modified\n";
}

int RunPkg(const Vector<String>& args)
{
	PkgInvocation inv;
	String error;
	if(!ParsePkgArgs(args, inv, error)) {
		Cerr() << error << "\n";
		return 1;
	}
	bool color = sUseColor(inv);
	(void)color;

	if(inv.command == PKG_CMD_HELP) {
		sPrintHelp(color);
		return 0;
	}
	if(inv.command == PKG_CMD_VERSION) {
		sPrintVersion();
		return 0;
	}

	bool need_repo = inv.command == PKG_CMD_INFO || inv.command == PKG_CMD_METADATA ||
	                 inv.command == PKG_CMD_SEARCH || inv.command == PKG_CMD_EXPLAIN_USE ||
	                 inv.command == PKG_CMD_DEPS || inv.command == PKG_CMD_PLAN ||
	                 inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS || inv.command == PKG_CMD_ESELECT ||
	                 inv.command == PKG_CMD_TARGET;
	PkgRepository repo;
	if(need_repo)
		repo.Discover();

	if(inv.command == PKG_CMD_INFO) {
		if(inv.list_sets)
			sPrintSets(repo);
		else
			sPrintInfo(repo, inv);
		return 0;
	}
	if(inv.command == PKG_CMD_METADATA) {
		sPrintMetadata(repo);
		return 0;
	}
	if(inv.command == PKG_CMD_SEARCH) {
		sPrintSearch(repo, inv.query);
		return 0;
	}
	if(inv.command == PKG_CMD_TARGET) {
		sPrintTargetCommand(inv);
		return 0;
	}
	if(inv.command == PKG_CMD_ESELECT) {
		sPrintEselectCommand(inv);
		return 0;
	}
	if(inv.command == PKG_CMD_EXPLAIN_USE) {
		sExplainUse(inv, repo);
		return 0;
	}
	if(inv.command == PKG_CMD_DEPS) {
		sPrintDeps(inv, repo);
		return 0;
	}
	if(inv.command == PKG_CMD_PLAN) {
		if(inv.ask && !sPromptYesNo())
			return 0;
		if(inv.install) {
			Cout() << "system package installation is not implemented yet\n";
			return 1;
		}
		sPrintPlan(inv, repo);
		return 0;
	}
	if(inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS) {
		sAuditAcceptFlags(inv, repo);
		return 0;
	}
	if(inv.command == PKG_CMD_RESUME) {
		Cout() << "resuming previous transaction (mock)\n";
		if(inv.pretend)
			Cout() << "pretend mode active\n";
		return 0;
	}

	Cerr() << "not implemented yet\n";
	return 1;
}

} // namespace Upp

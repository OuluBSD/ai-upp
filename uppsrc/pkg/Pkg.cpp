#include "Pkg.h"
#include <cstdio>

#define Cout() PkgOut()
#define Cerr() PkgErr()

namespace Upp {

struct PkgConsole {
	PkgConsole(FILE *f) : file(f) {}

	~PkgConsole()
	{
		Flush();
	}

	template <class T>
	PkgConsole& operator<<(const T& v)
	{
		buffer << v;
		return *this;
	}

	void Flush()
	{
		String s = buffer;
		if(!s.IsEmpty()) {
			fwrite(~s, 1, s.GetCount(), file);
			fflush(file);
			buffer.Clear();
		}
	}

private:
	String buffer;
	FILE *file;
};

static PkgConsole PkgOut()
{
	return PkgConsole(stdout);
}

static PkgConsole PkgErr()
{
	return PkgConsole(stderr);
}

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
	jio("name", name)("host_platform", host_platform)("build_platform", build_platform)
	   ("target_platform", target_platform)("runtime_environment", runtime_environment)
	   ("architecture", architecture)("toolchain", toolchain)("sysroot", sysroot)
	   ("thread_model", thread_model)("compiler", compiler)("linker", linker)
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

static String sTrimPath(const String& path);
static String sEselectRoot(const PkgInvocation& inv)
{
	return inv.root.IsEmpty() ? PkgRepoRoot() : sTrimPath(inv.root);
}

static String sEselectPath(const PkgInvocation& inv)
{
	return FindPkgConfigPaths(sEselectRoot(inv)).eselect;
}

static PkgEselectState sLoadEselectState(const PkgInvocation& inv)
{
	PkgEselectState st;
	LoadFromJsonFile(st, sEselectPath(inv));
	return st;
}

static void sStoreEselectState(const PkgInvocation& inv, const PkgEselectState& st)
{
	StoreAsJsonFile(st, sEselectPath(inv), true);
}

static const char * const sEselectBuiltins[] = {
	"help",
	"usage",
	"version",
};

static const char * const sEselectModules[] = {
	"compiler",
	"linker",
	"binutils",
	"visualstudio",
	"vcpkg",
	"msys2",
	"emscripten",
	"target",
	"profile",
	"provider",
	"repository",
	"news",
};

static const String *sEselectSlot(const PkgEselectState& st, const String& module);
static String *sEselectSlot(PkgEselectState& st, const String& module);
static Vector<PkgTargetProfile> sTargets();

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
	p.eselect = AppendFileName(root, "eselect.json");
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

static int sSearchScore(const PkgPackage& p, const String& query)
{
	String q = ToLower(TrimBoth(query));
	String atom = ToLower(p.atom);
	String name = ToLower(p.name);
	String path = ToLower(UnixPath(NormalizePath(p.path)));
	String desc = ToLower(p.description);
	String nest = ToLower(p.nest);

	if(q.IsEmpty())
		return 1000;
	if(q == atom || q == name || q == GetFileName(path))
		return 0;
	if(path.EndsWith(q) || atom.EndsWith(q) || name.EndsWith(q))
		return 1;
	if(atom.Find(q) >= 0 || name.Find(q) >= 0)
		return 2;
	if(path.Find(q) >= 0)
		return 3;
	if(desc.Find(q) >= 0)
		return 4;
	if(nest.Find(q) >= 0)
		return 5;
	return 1000;
}

static bool sStateHasAtom(const PkgState& state, const String& atom)
{
	for(const PkgStateRecord& rec : state.records)
		if(rec.atom == atom)
			return true;
	return false;
}

static String sPlanStatusText(char status)
{
	switch(status) {
	case 'N': return "new";
	case 'U': return "update";
	case 'D': return "downgrade";
	case 'R': return "rebuild";
	case 'r': return "reverse rebuild";
	case 'B': return "blocker";
	case 'b': return "resolved blocker";
	case 'F': return "fetch/manual action";
	case 'I': return "interactive";
	default:  return "unknown";
	}
}

static String sPlanTag(char status)
{
	String tag;
	switch(status) {
	case 'B': return "[blocked  B    ]";
	case 'F': return "[fetch    F    ]";
	case 'I': return "[ebuild   I    ]";
	}
	tag << "[ebuild   " << status << "    ]";
	return tag;
}

static String sPlanLabel(const PkgPlanItem& item)
{
	String s = item.atom;
	if(!item.description.IsEmpty())
		s << " - " << item.description;
	return s;
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
	{
		PkgUsePolicy& p = v.Add();
		p.name = "virtualgui";
		p.description = "Enable the VirtualGui transport";
		p.default_on = false;
		PkgUseMap& m0 = p.maps_to.Add();
		m0.upp_flag = "GUI";
		m0.scope = "global";
		PkgUseMap& m1 = p.maps_to.Add();
		m1.upp_flag = "SDL2GL";
		m1.scope = "global";
		PkgUseMap& m2 = p.maps_to.Add();
		m2.upp_flag = "VIRTUALGUI";
		m2.scope = "accepted";
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
		p.host_platform = "host";
		p.build_platform = "host";
		p.target_platform = "native";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "auto";
		p.sysroot = "";
		p.thread_model = "mt";
		p.compiler = "any";
		p.linker = "any";
		p.summary = "Native host target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "native-clang";
		p.host_platform = "host";
		p.build_platform = "host";
		p.target_platform = "native";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "clang";
		p.sysroot = "";
		p.thread_model = "mt";
		p.compiler = "clang";
		p.linker = "lld";
		p.summary = "Native clang target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "native-gcc";
		p.host_platform = "host";
		p.build_platform = "host";
		p.target_platform = "native";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "gcc";
		p.sysroot = "";
		p.thread_model = "mt";
		p.compiler = "gcc";
		p.linker = "ld";
		p.summary = "Native gcc target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "windows-msvc";
		p.host_platform = "windows";
		p.build_platform = "windows";
		p.target_platform = "windows";
		p.runtime_environment = "desktop";
		p.architecture = "x64";
		p.toolchain = "msvc";
		p.sysroot = "vcvars";
		p.thread_model = "mt";
		p.compiler = "msvc";
		p.linker = "link";
		p.summary = "Windows MSVC target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "windows-mingw";
		p.host_platform = "windows";
		p.build_platform = "windows";
		p.target_platform = "windows";
		p.runtime_environment = "desktop";
		p.architecture = "x64";
		p.toolchain = "mingw";
		p.sysroot = "mingw";
		p.thread_model = "mt";
		p.compiler = "mingw";
		p.linker = "ld";
		p.summary = "Windows MinGW target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "wasm-browser";
		p.host_platform = "any";
		p.build_platform = "host";
		p.target_platform = "wasm32";
		p.runtime_environment = "browser";
		p.architecture = "wasm32";
		p.toolchain = "emcc";
		p.sysroot = "emscripten";
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
		p.host_platform = "any";
		p.build_platform = "host";
		p.target_platform = "wasm32";
		p.runtime_environment = "node";
		p.architecture = "wasm32";
		p.toolchain = "emcc";
		p.sysroot = "emscripten";
		p.thread_model = "st";
		p.compiler = "emcc";
		p.linker = "emcc";
		p.summary = "Node wasm target";
		p.forced_use.Add("st");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "linux-fb";
		p.host_platform = "linux";
		p.build_platform = "linux";
		p.target_platform = "linux";
		p.runtime_environment = "framebuffer";
		p.architecture = "native";
		p.toolchain = "gcc";
		p.sysroot = "";
		p.thread_model = "mt";
		p.compiler = "gcc";
		p.linker = "ld";
		p.summary = "Linux framebuffer target";
		p.masked_use.Add("gui");
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "retro-x86";
		p.host_platform = "any";
		p.build_platform = "host";
		p.target_platform = "x86";
		p.runtime_environment = "retro";
		p.architecture = "x86";
		p.toolchain = "gcc";
		p.sysroot = "retro";
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
		p.host_platform = "freebsd";
		p.build_platform = "freebsd";
		p.target_platform = "freebsd";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "clang";
		p.sysroot = "";
		p.thread_model = "mt";
		p.compiler = "clang";
		p.linker = "ld";
		p.summary = "FreeBSD native target";
	}
	{
		PkgTargetProfile& p = v.Add();
		p.name = "macos-native";
		p.host_platform = "macos";
		p.build_platform = "macos";
		p.target_platform = "macos";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "clang";
		p.sysroot = "";
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

static void sAddUnique(Vector<String>& dst, const String& s)
{
	if(!s.IsEmpty() && FindIndex(dst, s) < 0)
		dst.Add(s);
}

static void sAppendUseExpr(Vector<String>& out, const Vector<String>& selected, const Vector<String>& disabled)
{
	for(const String& s : selected)
		out.Add(s);
	for(const String& s : disabled)
		out.Add(String("-") + s);
}

static String sFormatUseRequest(const PkgUseModel& use)
{
	if(!use.requested.IsEmpty())
		return sJoin(use.requested, " ");
	Vector<String> expr;
	for(const String& s : use.selected)
		expr.Add(s);
	for(const String& s : use.disabled)
		expr.Add(String("-") + s);
	return sJoin(expr, " ");
}

static String sFormatUseList(const Vector<String>& v)
{
	return v.IsEmpty() ? String("[none]") : sJoin(v, " ");
}

static String sFormatUseModel(const PkgUseModel& use)
{
	String s = sFormatUseRequest(use);
	if(!s.IsEmpty())
		return s;
	s = sFormatUseList(use.effective);
	return s;
}

static String sFormatUppFlag(const PkgUppFlag& flag)
{
	switch(flag.scope) {
	case PKG_UPP_ACCEPTED: return "." + flag.name;
	case PKG_UPP_MAIN_ONLY: return "!" + flag.name;
	default:                return flag.name;
	}
}

static int sUppGlobalRank(const String& s)
{
	if(s == "GUI")
		return 0;
	if(s == "ST")
		return 1;
	if(s == "SDL2GL")
		return 2;
	return 100 + ToUpper(s)[0];
}

static String sFormatUppProjection(const PkgUppProjection& proj)
{
	Vector<String> out;
	Vector<String> global;
	for(const String& s : proj.global)
		global.Add(s);
	Sort(global, [](const String& a, const String& b) {
		int ra = sUppGlobalRank(a);
		int rb = sUppGlobalRank(b);
		if(ra != rb)
			return ra < rb;
		return a < b;
	});
	for(const String& s : global)
		out.Add(s);
	Vector<String> accepted;
	for(const String& s : proj.accepted)
		accepted.Add(s);
	Sort(accepted, [](const String& a, const String& b) { return a < b; });
	for(const String& s : accepted)
		out.Add(s);
	Vector<String> main_only;
	for(const String& s : proj.main_only)
		main_only.Add(s);
	Sort(main_only, [](const String& a, const String& b) { return a < b; });
	for(const String& s : main_only)
		out.Add(s);
	return out.IsEmpty() ? String("[none]") : sJoin(out, " ");
}

static void sAddProjection(PkgUppProjection& proj, int scope, const String& name, const String& reason)
{
	PkgUppFlag& flag = proj.flags.Add();
	flag.name = name;
	flag.scope = scope;
	flag.reason = reason;
	String spell = sFormatUppFlag(flag);
	sAddUnique(scope == PKG_UPP_ACCEPTED ? proj.accepted : scope == PKG_UPP_MAIN_ONLY ? proj.main_only : proj.global, spell);
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

static String sRepoRelativePath(const PkgRepository& repo, const String& path)
{
	String full = UnixPath(NormalizePath(path));
	String root = UnixPath(NormalizePath(repo.root));
	if(full.StartsWith(root)) {
		full = full.Mid(root.GetCount());
		while(full.GetCount() && (full[0] == '/' || full[0] == '\\'))
			full.Remove(0, 1);
	}
	return full;
}

static void sPrintAcceptFlagsPatch(const PkgRepository& repo, const PkgPackage& p, const Vector<String>& missing)
{
	Vector<String> merged;
	for(const String& s : p.accepts)
		merged.Add(s);
	for(const String& m : missing)
		if(FindIndex(merged, m) < 0)
			merged.Add(m);
	Sort(merged, [](const String& a, const String& b) { return a < b; });

	Cout() << "--- a/" << sRepoRelativePath(repo, p.path) << "\n";
	Cout() << "+++ b/" << sRepoRelativePath(repo, p.path) << "\n";
	Cout() << "@@\n";
	if(p.accepts.IsEmpty()) {
		Cout() << "+acceptflags\n";
		for(int i = 0; i < merged.GetCount(); i++) {
			Cout() << "+" << '\t' << merged[i];
			Cout() << (i + 1 < merged.GetCount() ? ",\n" : ";\n");
		}
		return;
	}

	Cout() << "-acceptflags\n";
	for(int i = 0; i < p.accepts.GetCount(); i++) {
		Cout() << "-" << '\t' << p.accepts[i];
		Cout() << (i + 1 < p.accepts.GetCount() ? ",\n" : ";\n");
	}
	Cout() << "+acceptflags\n";
	for(int i = 0; i < merged.GetCount(); i++) {
		Cout() << "+" << '\t' << merged[i];
		Cout() << (i + 1 < merged.GetCount() ? ",\n" : ";\n");
	}
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
	Cout() << "Would you like to continue? [Yes/No] ";
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
	for(int i = 0; i < args.GetCount(); i++) {
		String a = args[i];
		if(a == "--")
			continue;
		if(a.StartsWith("--")) {
			if(a == "--help") inv.command = PKG_CMD_HELP;
			else if(a == "--version") inv.command = PKG_CMD_VERSION;
			else if(a == "--info") inv.info = true, inv.command = PKG_CMD_INFO;
			else if(a == "--metadata") inv.metadata = true, inv.command = PKG_CMD_METADATA;
			else if(a == "--list-sets") inv.list_sets = true, inv.command = PKG_CMD_LIST_SETS;
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
		if(a.StartsWith("USE=")) {
			Vector<String> flags = Split(a.Mid(4), CharFilterWhitespace);
			for(const String& f : flags)
				inv.use_args.Add(f);
			continue;
		}
		if(a.StartsWith("UPPFLAGS=")) {
			inv.extra.Add(a);
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
			else if(cmd == "list-sets") inv.command = PKG_CMD_LIST_SETS;
			else if(cmd == "explain-use") inv.command = PKG_CMD_EXPLAIN_USE;
			else if(cmd == "explain-target") inv.command = PKG_CMD_EXPLAIN_TARGET;
			else if(cmd == "deps") inv.command = PKG_CMD_DEPS;
			else if(cmd == "target") inv.command = PKG_CMD_TARGET;
			else if(cmd == "eselect") inv.command = PKG_CMD_ESELECT;
			else if(cmd == "audit-acceptflags") inv.command = PKG_CMD_AUDIT_ACCEPTFLAGS;
			else if(cmd == "resume") inv.command = PKG_CMD_RESUME;
			else if(cmd == "search") inv.command = PKG_CMD_SEARCH;
			else inv.command = PKG_CMD_PLAN;
		}
		else if(inv.list_sets)
			inv.command = PKG_CMD_LIST_SETS;
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
		   positional[0] == "metadata" || positional[0] == "list-sets" || positional[0] == "explain-use" || positional[0] == "explain-target" || positional[0] == "deps" ||
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
		else if(inv.command == PKG_CMD_EXPLAIN_TARGET) {
			if(rest.GetCount())
				inv.target = rest[0];
		}
		else if(inv.command == PKG_CMD_EXPLAIN_USE || inv.command == PKG_CMD_DEPS ||
		        inv.command == PKG_CMD_PLAN || inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS ||
		        inv.command == PKG_CMD_RESUME) {
			if(rest.GetCount())
				inv.atom = rest[0];
			for(int i = 1; i < rest.GetCount(); i++)
				inv.use_args.Add(rest[i]);
		}
		else if(inv.command == PKG_CMD_INFO || inv.command == PKG_CMD_METADATA || inv.command == PKG_CMD_LIST_SETS || inv.command == PKG_CMD_VERSION ||
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
	if(inv.audit_patch && inv.command == PKG_CMD_PLAN && !inv.atom.IsEmpty())
		inv.command = PKG_CMD_AUDIT_ACCEPTFLAGS;

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

static void sFlushConsole()
{
	Cout().Flush();
	Cerr().Flush();
}

static void sPrintEselectModules(const PkgInvocation& inv)
{
	Cout() << "Usage: pkg eselect <global options> <module name> <module options>\n\n";
	Cout() << "Global options:\n";
	Cout() << "  --brief\n";
	Cout() << "  --colour=<yes|no|auto>\n";
	Cout() << "  --root=<path>\n";
	Cout() << "  --sysroot=<path>\n\n";
	Cout() << "Built-in modules:\n";
	for(const char *m : sEselectBuiltins)
		Cout() << "  " << m << "\n";
	Cout() << "\nExtra modules:\n";
	for(const char *m : sEselectModules)
		Cout() << "  " << m << "\n";
	Cout() << "\nRepository root: " << sEselectRoot(inv) << "\n";
	Cout() << "Sysroot: " << (inv.sysroot.IsEmpty() ? String("[none]") : inv.sysroot) << "\n";
}

static void sPrintEselectValueList(const String& module)
{
	if(module == "compiler") {
		Cout() << "Available compiler selectors:\n";
		Cout() << "  auto\n  clang\n  gcc\n  msvc\n  mingw\n  emcc\n";
		return;
	}
	if(module == "linker") {
		Cout() << "Available linker selectors:\n";
		Cout() << "  auto\n  ld\n  lld\n  link\n  emcc\n";
		return;
	}
	if(module == "target") {
		Vector<PkgTargetProfile> targets = sTargets();
		for(const PkgTargetProfile& t : targets)
			Cout() << t.name << " [" << t.thread_model << "] - " << t.summary << "\n";
		return;
	}
	if(module == "provider") {
		Cout() << "Available provider selectors:\n";
		Cout() << "  auto\n  system\n  repository\n  plugin\n";
		return;
	}
	Cout() << "Module " << module << " is recognized but has no selector list yet.\n";
}

static void sPrintEselectValueShow(const PkgEselectState& st, const String& module)
{
	const String *slot = sEselectSlot(st, module);
	if(!slot || slot->IsEmpty())
		Cout() << module << ": [none]\n";
	else
		Cout() << module << ": " << *slot << "\n";
}

static void sPrintEselectValueSet(const PkgInvocation& inv, const String& module)
{
	PkgEselectState st = sLoadEselectState(inv);
	String *slot = sEselectSlot(st, module);
	if(!slot) {
		Cout() << "module not yet backed by a stored selection: " << module << "\n";
		Cout() << "stored repository-local selections currently cover compiler, linker, target, and provider\n";
		return;
	}
	*slot = inv.target;
	sStoreEselectState(inv, st);
	Cout() << module << " set to " << inv.target << "\n";
}

static const String *sEselectSlot(const PkgEselectState& st, const String& module)
{
	if(module == "compiler") return &st.compiler;
	if(module == "linker") return &st.linker;
	if(module == "target") return &st.target;
	if(module == "provider") return &st.provider;
	return nullptr;
}

static String *sEselectSlot(PkgEselectState& st, const String& module)
{
	if(module == "compiler") return &st.compiler;
	if(module == "linker") return &st.linker;
	if(module == "target") return &st.target;
	if(module == "provider") return &st.provider;
	return nullptr;
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
		<< "  --list-sets\n"
		<< "  -s, --search <query>\n"
		<< "  deps <atom> [USE flags...] --plan\n"
		<< "  explain-use <atom> [USE flags...]\n"
		<< "  explain-target <name>\n"
		<< "  target list|info <name>|explain <name>|set <name>\n"
		<< "  eselect ...\n"
		<< "  resume [--pretend]\n"
		<< "  audit-acceptflags [atom] [--patch]  global/platform flags are skipped\n"
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

static String sTargetNameText(const String& target)
{
	return target.IsEmpty() ? String("native") : target;
}

static String sTargetThreadReason(const PkgTargetProfile& tp)
{
	if(tp.thread_model == "st") {
		if(FindIndex(tp.forced_use, "st") >= 0)
			return "forced by the target profile";
		return "selected by the target profile";
	}
	return "MT is the default for this target profile";
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

static void sProjectUpp(PkgUppProjection& proj, const PkgUseModel& use);
static void sUseModelApplyPolicy(PkgUseModel& use, const PkgUsePolicy& p, bool default_on)
{
	if(default_on)
		sAddUnique(use.defaults, p.name);
}

static void sBuildUseModel(PkgUseModel& use, const PkgPackage *pkg, const Vector<String>& use_args, const String& target)
{
	const PkgTargetProfile& tp = sDefaultTargetProfile(target);
	use.requested.Clear();
	use.declared.Clear();
	use.defaults.Clear();
	use.forced.Clear();
	use.masked.Clear();
	use.selected.Clear();
	use.disabled.Clear();
	use.effective.Clear();
	use.transitions.Clear();
	for(const String& s : use_args)
		use.requested.Add(s);

	Index<String> declared;
	if(pkg)
		for(const String& s : pkg->accepts)
			if(declared.Find(s) < 0)
				declared.Add(s);
	for(const PkgUsePolicy& p : sUsePolicies())
		if(declared.Find(p.name) < 0)
			declared.Add(p.name);
	for(const String& a : use.requested) {
		String token = a;
		if(token.StartsWith("+") || token.StartsWith("-"))
			token = token.Mid(1);
		if(!token.IsEmpty() && declared.Find(token) < 0)
			declared.Add(token);
	}
	for(int i = 0; i < declared.GetCount(); i++)
		use.declared.Add(declared[i]);

	for(int i = 0; i < tp.forced_use.GetCount(); i++)
		sAddUnique(use.forced, tp.forced_use[i]);
	for(int i = 0; i < tp.masked_use.GetCount(); i++)
		sAddUnique(use.masked, tp.masked_use[i]);

	sTokenizeUseArgs(use.requested, use.selected, use.disabled);
	for(const String& s : use.forced)
		sAddUnique(use.selected, s);
	for(const String& s : use.masked)
		sAddUnique(use.disabled, s);

	const Vector<PkgUsePolicy> policies = sUsePolicies();
	for(const PkgUsePolicy& p : policies)
		sUseModelApplyPolicy(use, p, p.default_on);

	for(const String& s : use.selected) {
		PkgUseTransition& tr = use.transitions.Add();
		tr.flag = s;
		tr.marker = "+";
		tr.reason = "selected";
	}
	for(const String& s : use.defaults)
		if(!sIsSelected(use.selected, s) && !sIsSelected(use.disabled, s)) {
			PkgUseTransition& tr = use.transitions.Add();
			tr.flag = s;
			tr.marker = "+";
			tr.reason = "default";
		}
	for(const String& s : use.forced) {
		PkgUseTransition& tr = use.transitions.Add();
		tr.flag = s;
		tr.marker = "+";
		tr.reason = "forced";
	}
	for(const String& s : use.masked) {
		PkgUseTransition& tr = use.transitions.Add();
		tr.flag = s;
		tr.marker = "-";
		tr.reason = "masked";
	}

	Index<String> effective;
	for(const String& s : use.selected)
		if(effective.Find(s) < 0)
			effective.Add(s);
	for(const String& s : use.defaults)
		if(!sIsSelected(use.disabled, s) && effective.Find(s) < 0)
			effective.Add(s);
	for(const String& s : use.forced)
		if(effective.Find(s) < 0)
			effective.Add(s);
	for(const String& s : use.masked) {
		int q = effective.Find(s);
		if(q >= 0)
			effective.Remove(q);
	}
	for(int i = 0; i < effective.GetCount(); i++)
		use.effective.Add(effective[i]);

}

static void sPolicyFlags(const Vector<String>& use_args, const String& target, Vector<String>& selected, Vector<String>& disabled, Vector<String>& defaulted, Vector<String>& effective,
                         Vector<String>* target_forced_out = nullptr, Vector<String>* target_masked_out = nullptr)
{
	PkgUseModel use;
	sBuildUseModel(use, nullptr, use_args, target);
	selected.Clear();
	disabled.Clear();
	defaulted.Clear();
	effective.Clear();
	for(const String& s : use.selected)
		selected.Add(s);
	for(const String& s : use.disabled)
		disabled.Add(s);
	for(const String& s : use.defaults)
		defaulted.Add(s);
	for(const String& s : use.effective)
		effective.Add(s);
	if(target_forced_out) {
		target_forced_out->Clear();
		for(const String& s : use.forced)
			target_forced_out->Add(s);
	}
	if(target_masked_out) {
		target_masked_out->Clear();
		for(const String& s : use.masked)
			target_masked_out->Add(s);
	}
}

static void sExplainUse(const PkgInvocation& inv, const PkgRepository& repo)
{
	const PkgPackage *pkg = inv.atom.IsEmpty() ? nullptr : repo.Find(inv.atom);
	PkgUseModel use;
	sBuildUseModel(use, pkg, inv.use_args, inv.target);
	PkgUppProjection upp;
	sProjectUpp(upp, use);
	const PkgTargetProfile& tp = sDefaultTargetProfile(inv.target);

	Cout() << "Use policy for " << (inv.atom.IsEmpty() ? String("[unknown atom]") : inv.atom) << "\n";
	Cout() << "Target: " << sTargetNameText(inv.target) << "\n";
	Cout() << "Thread model: " << tp.thread_model << "\n";
	Cout() << "Thread model reason: " << sTargetThreadReason(tp) << "\n";
	Cout() << "Target platform: " << (tp.target_platform.IsEmpty() ? String("[none]") : tp.target_platform) << "\n";
	Cout() << "Runtime environment: " << (tp.runtime_environment.IsEmpty() ? String("[none]") : tp.runtime_environment) << "\n";
	Cout() << "USE: " << sFormatUseModel(use) << "\n";
	Cout() << "Effective USE: " << sFormatUseList(use.effective) << "\n";
	Cout() << "Declared USE: " << sFormatUseList(use.declared) << "\n";
	Cout() << "Defaults: " << sFormatUseList(use.defaults) << "\n";
	Cout() << "Forced: " << sFormatUseList(use.forced) << "\n";
	Cout() << "Masked: " << sFormatUseList(use.masked) << "\n\n";

	Cout() << "USE transitions:\n";
	if(use.transitions.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgUseTransition& tr : use.transitions)
			Cout() << "  " << tr.marker << tr.flag << " (" << tr.reason << ")\n";

	Cout() << "\nMapped U++ flags:\n";
	if(upp.flags.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgUppFlag& flag : upp.flags)
			Cout() << "  " << sFormatUppFlag(flag) << " (" << (flag.scope == PKG_UPP_ACCEPTED ? "accepted" : flag.scope == PKG_UPP_MAIN_ONLY ? "main-only" : "global") << ")\n";
	Cout() << "\nUPPFLAGS: " << sFormatUppProjection(upp) << "\n\n";
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
	Cout() << "host_platform: " << (t->host_platform.IsEmpty() ? String("[none]") : t->host_platform) << "\n";
	Cout() << "build_platform: " << (t->build_platform.IsEmpty() ? String("[none]") : t->build_platform) << "\n";
	Cout() << "target_platform: " << (t->target_platform.IsEmpty() ? String("[none]") : t->target_platform) << "\n";
	Cout() << "runtime_environment: " << (t->runtime_environment.IsEmpty() ? String("[none]") : t->runtime_environment) << "\n";
	Cout() << "architecture: " << (t->architecture.IsEmpty() ? String("[none]") : t->architecture) << "\n";
	Cout() << "toolchain: " << (t->toolchain.IsEmpty() ? String("[none]") : t->toolchain) << "\n";
	Cout() << "sysroot: " << (t->sysroot.IsEmpty() ? String("[none]") : t->sysroot) << "\n";
	Cout() << "thread_model: " << t->thread_model << "\n";
	Cout() << "thread_model_reason: " << sTargetThreadReason(*t) << "\n";
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

static void sExplainTarget(const String& name)
{
	const PkgTargetProfile* t = sFindTarget(name);
	if(!t) {
		Cout() << "Unknown target: " << name << "\n";
		return;
	}
	Cout() << "Target profile: " << t->name << "\n";
	Cout() << "Thread model: " << t->thread_model << "\n";
	Cout() << "Thread model reason: " << sTargetThreadReason(*t) << "\n";
	Cout() << "Host platform: " << (t->host_platform.IsEmpty() ? String("[none]") : t->host_platform) << "\n";
	Cout() << "Build platform: " << (t->build_platform.IsEmpty() ? String("[none]") : t->build_platform) << "\n";
	Cout() << "Target platform: " << (t->target_platform.IsEmpty() ? String("[none]") : t->target_platform) << "\n";
	Cout() << "Runtime environment: " << (t->runtime_environment.IsEmpty() ? String("[none]") : t->runtime_environment) << "\n";
	Cout() << "Architecture: " << (t->architecture.IsEmpty() ? String("[none]") : t->architecture) << "\n";
	Cout() << "Compiler: " << (t->compiler.IsEmpty() ? String("[none]") : t->compiler) << "\n";
	Cout() << "Toolchain: " << (t->toolchain.IsEmpty() ? String("[none]") : t->toolchain) << "\n";
	Cout() << "Sysroot: " << (t->sysroot.IsEmpty() ? String("[none]") : t->sysroot) << "\n";
	Cout() << "Forced USE: " << sFmtList(t->forced_use) << "\n";
	Cout() << "Masked USE: " << sFmtList(t->masked_use) << "\n";
	Cout() << "Provider preferences: " << sFmtList(t->provider_preferences) << "\n";
	Cout() << "Summary: " << t->summary << "\n";
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
	const PkgPackage *pkg = inv.atom.IsEmpty() ? nullptr : repo.Find(inv.atom);
	PkgUseModel use;
	PkgUppProjection upp;
	sBuildUseModel(use, pkg, inv.use_args, inv.target);
	sProjectUpp(upp, use);
	const PkgTargetProfile& tp = sDefaultTargetProfile(inv.target);
	Vector<String> virtuals;
	if(sIsSelected(use.effective, "sqlite"))
		virtuals.Add("virtual/sqlite");
	if(sIsSelected(use.effective, "st"))
		virtuals.Add("virtual/gui-runtime");
	if(sIsSelected(use.effective, "gtk"))
		virtuals.Add("virtual/gui-runtime");

	Cout() << "Dependencies for " << inv.atom << "\n";
	Cout() << "Target: " << sTargetNameText(inv.target) << " (" << tp.thread_model << ")\n";
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
	if(upp.flags.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgUppFlag& flag : upp.flags)
			Cout() << "  " << sFormatUppFlag(flag) << "\n";

	bool need_system = false;
	for(const String& v : virtuals) {
		Vector<PkgProvider> choices = sProvidersFor(v);
		for(const PkgProvider& p : choices)
			if(p.kind == "system")
				need_system = true;
	}
	Cout() << "\nSystem installation required: " << (need_system ? "yes" : "no") << "\n";
}

static void sPrintSearch(const PkgRepository& repo, const String& query, bool color)
{
	Vector<const PkgPackage*> found = repo.Search(query);
	Sort(found, [&](const PkgPackage* a, const PkgPackage* b) {
		int sa = sSearchScore(*a, query);
		int sb = sSearchScore(*b, query);
		if(sa != sb)
			return sa < sb;
		if(a->name != b->name)
			return a->name < b->name;
		if(a->nest != b->nest)
			return a->nest < b->nest;
		return a->path < b->path;
	});

	Cout() << "[ Results for search key : " << query << " ]\n";
	if(found.IsEmpty()) {
		Cout() << "No packages found.\n";
		Cout() << "[ Packages found : 0 ]\n";
		return;
	}

	Cout() << "Searching...\n\n";
	for(const PkgPackage* p : found) {
		String title = p->name;
		if(!p->atom.IsEmpty() && p->atom != p->name)
			title << " (" << p->atom << ")";
		Cout() << "*  " << sAnsi("36;1", title, color) << "\n";
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
	String active_target = !inv.target.IsEmpty() ? inv.target : (!eselect.target.IsEmpty() ? eselect.target : (state.target.IsEmpty() ? String("native") : state.target));
	const PkgTargetProfile& active_profile = sDefaultTargetProfile(active_target);
	Cout() << "Active target: " << active_target << "\n";
	Cout() << "Active target thread model: " << active_profile.thread_model << "\n";
	Cout() << "Active target reason: " << sTargetThreadReason(active_profile) << "\n";
	Cout() << "Active target compiler: " << active_profile.compiler << "\n";
	Cout() << "Active target toolchain: " << active_profile.toolchain << "\n";
	Cout() << "Selected target: " << (eselect.target.IsEmpty() ? String("[none]") : eselect.target) << "\n";
	Cout() << "Active toolchain: " << (state.toolchain.IsEmpty() ? String("[none]") : state.toolchain) << "\n";
	Cout() << "Selected compiler: " << (eselect.compiler.IsEmpty() ? String("[none]") : eselect.compiler) << "\n";
	Cout() << "Selected linker: " << (eselect.linker.IsEmpty() ? String("[none]") : eselect.linker) << "\n";
	Cout() << "Selected provider: " << (eselect.provider.IsEmpty() ? String("[none]") : eselect.provider) << "\n";
}

static void sPrintResumeState(const PkgState& state)
{
	if(state.records.IsEmpty()) {
		Cout() << "No saved transaction found.\n";
		return;
	}

	const PkgStateRecord& rec = state.records.Top();
	Cout() << "Last transaction:\n";
	Cout() << "  atom: " << (rec.atom.IsEmpty() ? String("[none]") : rec.atom) << "\n";
	Cout() << "  target: " << (rec.target.IsEmpty() ? String("[none]") : rec.target) << "\n";
	Cout() << "  toolchain: " << (rec.toolchain.IsEmpty() ? String("[none]") : rec.toolchain) << "\n";
	Cout() << "  build_status: " << (rec.build_status.IsEmpty() ? String("[none]") : rec.build_status) << "\n";
	Cout() << "  artifact_path: " << (rec.artifact_path.IsEmpty() ? String("[none]") : rec.artifact_path) << "\n";
	Cout() << "  selected_use: " << sFmtList(rec.selected_use) << "\n";
	Cout() << "  effective_flags: " << sFmtList(rec.effective_flags) << "\n";
	Cout() << "  providers: " << sFmtList(rec.providers) << "\n";
	Cout() << "  timestamp: " << Format(rec.timestamp) << "\n";
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
	rec.artifact_path = repo.root + "/bin/build.exe";
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

static int sUppScopeFromText(const String& scope)
{
	String s = ToLower(TrimBoth(scope));
	if(s == "accepted" || s == "accept" || s == "dot")
		return PKG_UPP_ACCEPTED;
	if(s == "main" || s == "main-only" || s == "main_only")
		return PKG_UPP_MAIN_ONLY;
	return PKG_UPP_GLOBAL;
}

static void sProjectUpp(PkgUppProjection& proj, const PkgUseModel& use)
{
	proj.flags.Clear();
	proj.global.Clear();
	proj.accepted.Clear();
	proj.main_only.Clear();
	proj.transitions.Clear();
	Index<String> seen;
	for(const String& s : use.effective) {
		const PkgUsePolicy* p = sFindPolicy(s);
		if(p) {
			for(const PkgUseMap& m : p->maps_to) {
				int scope = sUppScopeFromText(m.scope);
				String spell = sUppFlagSpelling(m.upp_flag, scope == PKG_UPP_ACCEPTED);
				if(seen.Find(spell) >= 0)
					continue;
				seen.Add(spell);
				sAddProjection(proj, scope, m.upp_flag, p->name);
			}
			continue;
		}
		String raw = ToUpper(s);
		String spell = raw;
		if(seen.Find(spell) < 0) {
			seen.Add(spell);
			sAddProjection(proj, PKG_UPP_ACCEPTED, raw, "implicit projection");
		}
	}
}

static void sPlanAddItem(PkgPlan& plan, char base_status, const String& atom, const PkgPackage* pkg, const String& reason, const String& provider, bool blocker, bool resolved, int depth)
{
	PkgPlanItem& item = plan.items.Add();
	item.status = plan.ask && depth == 0 ? 'I' : base_status;
	item.atom = atom;
	item.provider = provider;
	item.blocker = blocker;
	item.resolved = resolved;
	item.interactive = plan.ask && depth == 0;
	item.target = sTargetNameText(plan.target);
	item.use = sFormatUseModel(plan.use);
	item.uppflags = sFormatUppProjection(plan.upp);
	item.reason = reason;
	if(item.interactive) {
		String note = sPlanStatusText(base_status);
		item.reason = item.reason.IsEmpty() ? note : note + "; " + item.reason;
	}
	if(pkg) {
		item.repository = pkg->nest;
		item.description = pkg->description;
	}
}

static void sPlanWalkAtom(PkgPlan& plan, const PkgRepository& repo, const PkgState& state, const String& atom, const String& reason, Index<String>& seen, int depth)
{
	String key = ToLower(TrimBoth(atom));
	if(key.IsEmpty())
		return;

	if(atom == "@world") {
		Vector<String> world = sReadWorld(repo.paths);
		if(world.IsEmpty()) {
			sPlanAddItem(plan, 'F', atom, nullptr, reason.IsEmpty() ? String("world file is empty") : reason, String(), false, false, depth);
			return;
		}
		for(const String& w : world)
			sPlanWalkAtom(plan, repo, state, w, "member of @world", seen, depth);
		return;
	}

	if(atom == "@system" || atom == "@toolchain") {
		sPlanAddItem(plan, 'N', atom, nullptr, reason.IsEmpty() ? String("synthetic set") : reason, String(), false, false, depth);
		return;
	}

	if(atom.StartsWith("virtual/")) {
		String provider = sResolveProvider(atom, repo);
		if(provider.IsEmpty()) {
			sPlanAddItem(plan, 'B', atom, nullptr, reason.IsEmpty() ? String("no provider available") : reason, String(), true, false, depth);
			return;
		}
		const PkgPackage* provider_pkg = repo.Find(provider);
		sPlanAddItem(plan, provider_pkg ? 'b' : 'B', provider, provider_pkg, reason.IsEmpty() ? String("provider for ") + atom : reason, atom, false, provider_pkg != nullptr, depth);
		if(provider_pkg && plan.deep)
			sPlanWalkAtom(plan, repo, state, provider_pkg->atom, String("provider for ") + atom, seen, depth + 1);
		return;
	}

	const PkgPackage* pkg = repo.Find(atom);
	if(!pkg) {
		String provider = sResolveProvider(atom, repo);
		if(provider.IsEmpty() && !atom.StartsWith("virtual/"))
			provider = sResolveProvider(String("virtual/") + atom, repo);
		if(!provider.IsEmpty()) {
			const PkgPackage* provider_pkg = repo.Find(provider);
			sPlanAddItem(plan, provider_pkg ? 'b' : 'B', provider, provider_pkg, reason.IsEmpty() ? String("provider for ") + atom : reason, atom, false, provider_pkg != nullptr, depth);
		}
		else
			sPlanAddItem(plan, 'F', atom, nullptr, reason.IsEmpty() ? String("package metadata not found") : reason, String(), false, false, depth);
		return;
	}

	if(FindIndex(seen, key) >= 0) {
		if(depth > 0)
			sPlanAddItem(plan, 'r', atom, pkg, reason.IsEmpty() ? String("reverse dependency already scheduled") : reason, String(), false, false, depth);
		return;
	}
	seen.Add(key);

	char status = sStateHasAtom(state, atom) ? (plan.update ? 'U' : 'R') : 'N';
	String msg = reason;
	if(msg.IsEmpty())
		msg = status == 'N' ? "new package" : (status == 'U' ? "update requested" : "rebuild requested");
	sPlanAddItem(plan, status, atom, pkg, msg, String(), false, false, depth);

	if(!plan.deep)
		return;

	for(const String& dep : pkg->uses) {
		String dep_reason = String("dependency of ") + atom;
		sPlanWalkAtom(plan, repo, state, dep, dep_reason, seen, depth + 1);
	}
}

static PkgPlan sBuildPlan(const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, bool color)
{
	PkgPlan plan;
	plan.atom = inv.atom;
	plan.target = inv.target;
	plan.color = color;
	plan.ask = inv.ask;
	plan.verbose = inv.verbose;
	plan.pretend = inv.pretend;
	plan.update = inv.update;
	plan.deep = inv.deep;
	plan.newuse = inv.newuse;
	const PkgPackage *root_pkg = inv.atom.IsEmpty() ? nullptr : repo.Find(inv.atom);
	sBuildUseModel(plan.use, root_pkg, inv.use_args, inv.target);
	sProjectUpp(plan.upp, plan.use);
	plan.selected_use.Clear();
	plan.disabled_use.Clear();
	plan.defaulted_use.Clear();
	plan.effective_use.Clear();
	plan.target_forced.Clear();
	plan.target_masked.Clear();
	plan.uppflags.Clear();
	for(const String& s : plan.use.selected)
		plan.selected_use.Add(s);
	for(const String& s : plan.use.disabled)
		plan.disabled_use.Add(s);
	for(const String& s : plan.use.defaults)
		plan.defaulted_use.Add(s);
	for(const String& s : plan.use.effective)
		plan.effective_use.Add(s);
	for(const String& s : plan.use.forced)
		plan.target_forced.Add(s);
	for(const String& s : plan.use.masked)
		plan.target_masked.Add(s);
	for(const String& s : Split(sFormatUppProjection(plan.upp), CharFilterWhitespace))
		plan.uppflags.Add(s);

	if(sIsSelected(plan.effective_use, "sqlite") && FindIndex(plan.virtuals, "virtual/sqlite") < 0)
		plan.virtuals.Add("virtual/sqlite");
	if((sIsSelected(plan.effective_use, "st") || sIsSelected(plan.effective_use, "gtk")) && FindIndex(plan.virtuals, "virtual/gui-runtime") < 0)
		plan.virtuals.Add("virtual/gui-runtime");
	for(const String& v : plan.virtuals) {
		String provider = sResolveProvider(v, repo);
		if(!provider.IsEmpty() && FindIndex(plan.providers, provider) < 0)
			plan.providers.Add(provider);
	}

	Index<String> seen;
	if(!inv.atom.IsEmpty())
		sPlanWalkAtom(plan, repo, state, inv.atom, String(), seen, 0);
	else if(inv.command == PKG_CMD_PLAN && inv.ask)
		sPlanWalkAtom(plan, repo, state, "@world", String(), seen, 0);

	plan.backtrack = 0;
	plan.dependency_seconds = 0.00;
	return plan;
}

static void sPrintPlanItem(const PkgPlan& plan, const PkgPlanItem& item)
{
	String tag = sPlanTag(item.status);
	String use = String("USE=\"") + (item.use.IsEmpty() ? String("[none]") : item.use) + "\"";
	String target = String("TARGET=\"") + (item.target.IsEmpty() ? String("native") : item.target) + "\"";
	String upp = item.uppflags.IsEmpty() ? String() : String("UPPFLAGS=\"") + item.uppflags + "\"";
	String color_code = item.status == 'B' ? "31;1" : item.status == 'F' ? "33;1" : item.status == 'I' ? "36;1" : "32;1";

	Cout() << sAnsi(color_code, tag, plan.color) << ' '
	       << sAnsi("36", item.atom, plan.color) << ' '
	       << sAnsi("33", use, plan.color) << ' '
	       << sAnsi("35", target, plan.color);
	if(!upp.IsEmpty())
		Cout() << ' ' << sAnsi("33", upp, plan.color);
	Cout() << "\n";
	if(!item.provider.IsEmpty())
		Cout() << "  " << sAnsi("34", "provider:", plan.color) << ' ' << item.provider << "\n";
	if(!item.reason.IsEmpty())
		Cout() << "  " << sAnsi("90", "reason:", plan.color) << ' ' << item.reason << "\n";
	if(!item.repository.IsEmpty())
		Cout() << "  " << sAnsi("90", "repository:", plan.color) << ' ' << item.repository << "\n";
	if(!item.description.IsEmpty())
		Cout() << "  " << sAnsi("90", "description:", plan.color) << ' ' << item.description << "\n";
	if(plan.verbose) {
		Cout() << "  " << sAnsi("90", "declared USE:", plan.color) << ' ' << sFormatUseList(plan.use.declared) << "\n";
		Cout() << "  " << sAnsi("90", "selected USE:", plan.color) << ' ' << sFormatUseList(plan.use.selected) << "\n";
		Cout() << "  " << sAnsi("90", "default USE:", plan.color) << ' ' << sFormatUseList(plan.use.defaults) << "\n";
		Cout() << "  " << sAnsi("90", "forced USE:", plan.color) << ' ' << sFormatUseList(plan.use.forced) << "\n";
		Cout() << "  " << sAnsi("90", "masked USE:", plan.color) << ' ' << sFormatUseList(plan.use.masked) << "\n";
		Cout() << "  " << sAnsi("90", "effective USE:", plan.color) << ' ' << sFormatUseList(plan.use.effective) << "\n";
		if(!plan.use.transitions.IsEmpty()) {
			Cout() << "  " << sAnsi("90", "USE transitions:", plan.color) << '\n';
			for(const PkgUseTransition& tr : plan.use.transitions)
				Cout() << "    " << tr.marker << tr.flag << " (" << tr.reason << ")\n";
		}
		Cout() << "  " << sAnsi("90", "UPP global:", plan.color) << ' ' << sFormatUseList(plan.upp.global) << "\n";
		Cout() << "  " << sAnsi("90", "UPP accepted:", plan.color) << ' ' << sFormatUseList(plan.upp.accepted) << "\n";
		Cout() << "  " << sAnsi("90", "UPP main-only:", plan.color) << ' ' << sFormatUseList(plan.upp.main_only) << "\n";
	}
	Cout() << "\n";
}

static void sPrintPlan(const PkgInvocation& inv, const PkgRepository& repo, bool color)
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	PkgPlan plan = sBuildPlan(inv, repo, state, color);

	Cout() << "These are the packages that would be built, in order:\n\n";
	Cout() << "Calculating dependencies... done!\n";
	Cout() << "Dependency resolution took " << FormatDouble(plan.dependency_seconds, 2) << " s (backtrack: "
	     << plan.backtrack << '/' << plan.backtrack_limit << ").\n\n";

	if(plan.items.IsEmpty())
		Cout() << "[ no packages selected ]\n\n";
	else
		for(const PkgPlanItem& item : plan.items)
			sPrintPlanItem(plan, item);

	Cout() << "USE: " << sFormatUseModel(plan.use) << "\n";
	Cout() << "Effective USE: " << sFmtList(plan.effective_use) << "\n";
	Cout() << "TARGET: " << sTargetNameText(plan.target) << "\n";
	Cout() << "UPPFLAGS: " << sFormatUppProjection(plan.upp) << "\n";
	if(!plan.providers.IsEmpty())
		Cout() << "Providers: " << sFmtList(plan.providers) << "\n";
	if(!plan.virtuals.IsEmpty())
		Cout() << "Virtual requirements: " << sFmtList(plan.virtuals) << "\n";
	Cout() << "Total packages: " << plan.items.GetCount() << "\n";
}

static void sPrintTargetCommand(const PkgInvocation& inv)
{
	if(inv.subcommand.IsEmpty() || inv.subcommand == "list")
		sListTargets();
	else if(inv.subcommand == "info")
		sPrintTargetInfo(inv.target);
	else if(inv.subcommand == "explain")
		sExplainTarget(inv.target);
	else if(inv.subcommand == "set") {
		PkgEselectState st = sLoadEselectState(inv);
		st.target = inv.target;
		sStoreEselectState(inv, st);
		Cout() << "target set to " << inv.target << "\n";
	}
	else
		Cout() << "unknown target subcommand: " << inv.subcommand << "\n";
}

static void sPrintEselectCommand(const PkgInvocation& inv)
{
	if(inv.module.IsEmpty() || inv.module == "help") {
		sPrintEselectModules(inv);
		return;
	}
	if(inv.module == "usage") {
		sPrintEselectModules(inv);
		return;
	}
	if(inv.module == "version") {
		Cout() << "pkg eselect 0.1\n";
		Cout() << "Repository root: " << sEselectRoot(inv) << "\n";
		Cout() << "Sysroot: " << (inv.sysroot.IsEmpty() ? String("[none]") : inv.sysroot) << "\n";
		return;
	}
	if(inv.module == "modules" && inv.subcommand == "list") {
		Cout() << "Modules available in this repository-local selector:\n";
		Cout() << "Built-in modules:\n";
		for(const char *m : sEselectBuiltins)
			Cout() << "  " << m << "\n";
		Cout() << "Extra modules:\n";
		for(const char *m : sEselectModules)
			Cout() << "  " << m << "\n";
		Cout() << "Repository root: " << sEselectRoot(inv) << "\n";
		Cout() << "Sysroot: " << (inv.sysroot.IsEmpty() ? String("[none]") : inv.sysroot) << "\n";
		return;
	}
	if(inv.subcommand == "list") {
		sPrintEselectValueList(inv.module);
		return;
	}
	if(inv.subcommand == "show") {
		PkgEselectState st = sLoadEselectState(inv);
		sPrintEselectValueShow(st, inv.module);
		return;
	}
	if(inv.subcommand == "set") {
		if(inv.module == "target") {
			PkgInvocation t;
			t.subcommand = "set";
			t.target = inv.target;
			t.root = inv.root;
			t.sysroot = inv.sysroot;
			sPrintTargetCommand(t);
			return;
		}
		sPrintEselectValueSet(inv, inv.module);
		return;
	}
	Cout() << "unknown eselect module command: " << inv.module;
	if(!inv.subcommand.IsEmpty())
		Cout() << ' ' << inv.subcommand;
	Cout() << "\n";
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
	Sort(targets, [](const PkgPackage* a, const PkgPackage* b) { return a->name < b->name; });

	for(const PkgPackage* p : targets) {
		Vector<String> used;
		Vector<String> skipped;
		for(const String& src : p->source_files) {
			String txt = LoadFile(src);
			if(txt.IsEmpty())
				continue;
			Vector<String> lines = Split(txt, '\n');
			for(String line : lines) {
				int sl = line.Find("//");
				if(sl >= 0)
					line = line.Left(sl);
				line = TrimLeft(line);
				if(line.IsEmpty() || line[0] != '#')
					continue;
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
						if(!IsGlobalAuditFlag(flag)) {
							if(FindIndex(used, flag) < 0)
								used.Add(flag);
						}
						else if(FindIndex(skipped, flag) < 0)
							skipped.Add(flag);
						line = line.Mid(s);
					}
					else
						line = line.Mid(q + 4);
				}
			}
		}
		Sort(skipped, [](const String& a, const String& b) { return a < b; });
		for(const String& s : skipped)
			Cout() << "[info] " << p->name << " uses flag" << s << " but " << s
			     << " is skipped as a global/platform flag\n";

		Sort(used, [](const String& a, const String& b) { return a < b; });
		Vector<String> accepts;
		for(const String& s : p->accepts)
			accepts.Add(s);
		Sort(accepts, [](const String& a, const String& b) { return a < b; });

		Vector<String> missing;
		for(const String& s : used) {
			if(FindIndex(accepts, s) < 0) {
				missing.Add(s);
				Cout() << "[warn] " << p->name << " uses flag" << s << " but "
				     << GetFileName(p->path) << " does not accept " << s << "\n";
				Cout() << "[hint] add: acceptflags " << s << ";\n";
			}
		}
		for(const String& a : accepts)
			if(FindIndex(used, a) < 0)
				Cout() << "[info] " << p->name << " accepts " << a << " but no source flag was found\n";
		if(inv.audit_patch && !missing.IsEmpty()) {
			Cout() << '\n';
			sPrintAcceptFlagsPatch(repo, *p, missing);
		}
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
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_VERSION) {
		sPrintVersion();
		sFlushConsole();
		return 0;
	}

	bool need_repo = inv.command == PKG_CMD_INFO || inv.command == PKG_CMD_METADATA || inv.command == PKG_CMD_LIST_SETS ||
	                 inv.command == PKG_CMD_SEARCH || inv.command == PKG_CMD_EXPLAIN_USE ||
	                 inv.command == PKG_CMD_EXPLAIN_TARGET || inv.command == PKG_CMD_DEPS || inv.command == PKG_CMD_PLAN ||
	                 inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS || inv.command == PKG_CMD_ESELECT ||
	                 inv.command == PKG_CMD_TARGET;
	PkgRepository repo;
	if(need_repo)
		repo.Discover();

	if(inv.command == PKG_CMD_LIST_SETS) {
		sPrintSets(repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_INFO) {
		sPrintInfo(repo, inv);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_METADATA) {
		sPrintMetadata(repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_SEARCH) {
		sPrintSearch(repo, inv.query, color);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_TARGET) {
		sPrintTargetCommand(inv);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_ESELECT) {
		sPrintEselectCommand(inv);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_EXPLAIN_USE) {
		sExplainUse(inv, repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_EXPLAIN_TARGET) {
		sExplainTarget(inv.target);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_DEPS) {
		sPrintDeps(inv, repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_PLAN) {
		sPrintPlan(inv, repo, color);
		if(inv.ask) {
			if(!sPromptYesNo()) {
				Cout() << "Aborted.\n";
				sFlushConsole();
				return 0;
			}
			Cout() << "Continuing.\n";
		}
		if(inv.install && !inv.pretend) {
			Cout() << "system package installation is not implemented yet\n";
			sFlushConsole();
			return 1;
		}
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS) {
		sAuditAcceptFlags(inv, repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_RESUME) {
		PkgState state;
		LoadFromJsonFile(state, repo.paths.state);
		sPrintResumeState(state);
		if(inv.pretend)
			Cout() << "pretend mode active\n";
		sFlushConsole();
		return 0;
	}

	Cerr() << "not implemented yet\n";
	sFlushConsole();
	return 1;
}

} // namespace Upp

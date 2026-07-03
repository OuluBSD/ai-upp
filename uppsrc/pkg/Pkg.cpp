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

void PkgUppFlag::Jsonize(JsonIO& jio)
{
	jio("name", name)("scope", scope)("reason", reason);
}

void PkgTargetProfile::Jsonize(JsonIO& jio)
{
	jio("name", name)("host_platform", host_platform)("build_platform", build_platform)
	   ("target_platform", target_platform)("runtime_environment", runtime_environment)
	   ("architecture", architecture)("toolchain", toolchain)("sysroot", sysroot)
	   ("thread_model", thread_model)("compiler", compiler)("linker", linker)
	   ("sdk", sdk)("summary", summary);
	jio("default_use", default_use);
	jio("forced_use", forced_use);
	jio("masked_use", masked_use);
	jio("provider_preferences", provider_preferences);
	jio("upp_add", upp_add);
	jio("notes", notes);
	jio("warnings", warnings);
}

void PkgProviderPreference::Jsonize(JsonIO& jio)
{
	jio("capability", capability)("provider_id", provider_id)("reason", reason)("priority", priority);
}

void PkgUppkgManifest::Jsonize(JsonIO& jio)
{
	jio("path", path)("present", present)("ok", ok)("target", target)("provider", provider);
	jio("use_default", use_default)("use_forced", use_forced)("use_masked", use_masked);
	jio("provider_preferences", provider_preferences);
	jio("notes", notes)("warnings", warnings)("errors", errors)("unknown_keys", unknown_keys);
}

void PkgStateRecord::Jsonize(JsonIO& jio)
{
	jio("atom", atom)("target", target)("toolchain", toolchain)("build_status", build_status)
	   ("artifact_path", artifact_path)("build_method", build_method)("command_line", command_line)
	   ("output_path", output_path)("staged", staged)("success", success)("owned", owned)("timestamp", timestamp);
	jio("selected_use", selected_use);
	jio("declared_use", declared_use);
	jio("effective_use", effective_use);
	jio("effective_uppflags", effective_uppflags);
	jio("accepted_flags", accepted_flags);
	jio("providers", providers);
}

void PkgState::Jsonize(JsonIO& jio)
{
	jio("target", target)("toolchain", toolchain);
	jio("records", records);
}

void PkgBuildStep::Jsonize(JsonIO& jio)
{
	jio("atom", atom)("path", path)("reason", reason)("command", command)("result", result)
	   ("exit_code", exit_code)("requested", requested)("provider_added", provider_added)
	   ("set_member", set_member)("root", root)("executed", executed)("completed", completed)
	   ("skipped", skipped)("failed", failed)("output_path", output_path)
	   ("build_method", build_method)("staged", staged);
}

void PkgTransaction::Jsonize(JsonIO& jio)
{
	jio("command_line", command_line)("target", target)("provider", provider)("compiler", compiler)
	   ("linker", linker)("toolchain", toolchain)("build_method", build_method)("result", result)
	   ("jobs", jobs)("failed_index", failed_index)("pretend", pretend)("ask", ask)
	   ("resume", resume)("staged", staged)("staged_runner", staged_runner)
	   ("keep_going", keep_going)("skip_first", skip_first);
	jio("requested_atoms", requested_atoms);
	jio("completed_steps", completed_steps);
	jio("resume_data", resume_data);
	jio("failed_step", failed_step);
	jio("steps", steps);
	jio("timestamp", timestamp);
}

void PkgEselectState::Jsonize(JsonIO& jio)
{
	jio("compiler", compiler)("linker", linker)("target", target)("provider", provider);
	jio("profile", profile)("repository", repository);
	jio("vcpkg_root", vcpkg_root)("vcpkg_triplet", vcpkg_triplet);
	jio("emscripten_profile", emscripten_profile);
}

void PkgPackage::Jsonize(JsonIO& jio)
{
	jio("atom", atom)("name", name)("nest", nest)("path", path)("dir", dir);
	jio("description", description);
	jio("accepts", accepts)("uses", uses)("mainconfig", mainconfig);
	jio("source_files", source_files)("manifest", manifest)("mtime", mtime);
}

void PkgMetadataCacheEntry::Jsonize(JsonIO& jio)
{
	jio("path", path)("manifest_path", manifest_path)("size", size)("mtime", mtime)
	   ("manifest_size", manifest_size)("manifest_mtime", manifest_mtime)("pkg", pkg);
}

void PkgMetadataCache::Jsonize(JsonIO& jio)
{
	jio("schema", schema)("root", root)("entries", entries)("saved_at", saved_at);
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
	String path = sEselectPath(inv);
	if(FileExists(path))
		LoadFromJsonFile(st, path);
	else {
		String legacy = AppendFileName(sEselectRoot(inv), "eselect.json");
		if(FileExists(legacy))
			LoadFromJsonFile(st, legacy);
	}
	return st;
}

static void sStoreEselectState(const PkgInvocation& inv, const PkgEselectState& st)
{
	String path = sEselectPath(inv);
	RealizeDirectory(GetFileFolder(path));
	StoreAsJsonFile(st, path, true);
}

static void sCopyInvocation(PkgInvocation& dst, const PkgInvocation& src)
{
	dst.command = src.command;
	dst.color = src.color;
	dst.atom = src.atom;
	dst.query = src.query;
	dst.provider_query = src.provider_query;
	dst.target = src.target;
	dst.provider = src.provider;
	dst.compiler = src.compiler;
	dst.linker = src.linker;
	dst.profile = src.profile;
	dst.repository = src.repository;
	dst.vcpkg_root = src.vcpkg_root;
	dst.vcpkg_triplet = src.vcpkg_triplet;
	dst.emscripten_profile = src.emscripten_profile;
	dst.value = src.value;
	dst.module = src.module;
	dst.subcommand = src.subcommand;
	dst.root = src.root;
	dst.sysroot = src.sysroot;
	dst.command_line = src.command_line;
	dst.jobs = src.jobs;
	dst.target_explicit = src.target_explicit;
	dst.provider_explicit = src.provider_explicit;
	dst.ask = src.ask;
	dst.verbose = src.verbose;
	dst.update = src.update;
	dst.deep = src.deep;
	dst.newuse = src.newuse;
	dst.changed_use = src.changed_use;
	dst.strict = src.strict;
	dst.nodeps = src.nodeps;
	dst.summary = src.summary;
	dst.quiet = src.quiet;
	dst.ci = src.ci;
	dst.keep_going = src.keep_going;
	dst.skip_first = src.skip_first;
	dst.probe = src.probe;
	dst.debug_timing = src.debug_timing;
	dst.pretend = src.pretend;
	dst.resume = src.resume;
	dst.oneshot = src.oneshot;
	dst.plan = src.plan;
	dst.metadata = src.metadata;
	dst.metadata_cache = src.metadata_cache;
	dst.brief = src.brief;
	dst.list_sets = src.list_sets;
	dst.targets = src.targets;
	dst.providers = src.providers;
	dst.info = src.info;
	dst.doctor = src.doctor;
	dst.search = src.search;
	dst.install = src.install;
	dst.audit_patch = src.audit_patch;
	dst.bins = src.bins;
	dst.clean = src.clean;
	dst.depclean = src.depclean;
	dst.all = src.all;
	dst.staged = src.staged;
	dst.report = src.report;
	dst.limit = src.limit;
	dst.use_cache = src.use_cache;
	dst.rebuild_cache = src.rebuild_cache;
	dst.baseline = src.baseline;
	dst.update_baseline = src.update_baseline;
	dst.show_baseline = src.show_baseline;
	dst.fail_on_baseline = src.fail_on_baseline;
	dst.argv.Clear();
	for(const String& s : src.argv)
		dst.argv.Add(s);
	dst.use_args.Clear();
	for(const String& s : src.use_args)
		dst.use_args.Add(s);
	dst.extra.Clear();
	for(const String& s : src.extra)
		dst.extra.Add(s);
}

static const PkgTargetProfile* sFindTarget(const String& name);

struct PkgPolicyLine : Moveable<PkgPolicyLine> {
	String file;
	int line = 0;
	String atom;
	Vector<String> values;
	bool package_mask = false;
};

struct PkgLocalPolicy : Moveable<PkgLocalPolicy> {
	String make_conf_path;
	String package_use_path;
	String package_provider_path;
	String package_target_path;
	String package_mask_path;
	String package_force_path;
	String make_use;
	String make_target;
	String make_provider;
	String make_compiler;
	String make_linker;
	String make_profile;
	String make_repository;
	String make_vcpkg_root;
	String make_vcpkg_triplet;
	String make_emscripten_profile;
	int make_jobs = 0;
	Vector<PkgPolicyLine> package_use;
	Vector<PkgPolicyLine> package_provider;
	Vector<PkgPolicyLine> package_target;
	Vector<PkgPolicyLine> package_mask;
	Vector<PkgPolicyLine> package_force;
	Vector<String> warnings;
	Vector<String> errors;
};

struct PkgPolicyAtomRules {
	bool masked = false;
	bool has_target = false;
	bool has_provider = false;
	bool has_compiler = false;
	bool has_linker = false;
	bool has_profile = false;
	bool has_repository = false;
	bool has_vcpkg_root = false;
	bool has_vcpkg_triplet = false;
	bool has_emscripten_profile = false;
	String target;
	String provider;
	String compiler;
	String linker;
	String profile;
	String repository;
	String vcpkg_root;
	String vcpkg_triplet;
	String emscripten_profile;
	Vector<String> use_args;
	Vector<String> warnings;
};

static void sCopyInvocation(PkgInvocation& dst, const PkgInvocation& src);
static String sResolveEselectTarget(const PkgInvocation& inv, const PkgEselectState& st);

static String sPolicyTrimComment(String line)
{
	int hash = -1;
	bool quoted = false;
	for(int i = 0; i < line.GetCount(); i++) {
		char c = line[i];
		if(c == '"') {
			quoted = !quoted;
			continue;
		}
		if(c == '#' && !quoted) {
			hash = i;
			break;
		}
	}
	if(hash >= 0)
		line = line.Left(hash);
	return TrimBoth(line);
}

static String sPolicyUnquote(const String& value)
{
	String out = TrimBoth(value);
	if(out.GetCount() >= 2) {
		char q = out[0];
		if((q == '"' || q == '\'') && out[out.GetCount() - 1] == q) {
			out = out.Mid(1, out.GetCount() - 2);
			out.Replace("\\\"", "\"");
			out.Replace("\\'", "'");
		}
	}
	return TrimBoth(out);
}

static Vector<String> sPolicyWords(const String& line)
{
	Vector<String> out = Split(line, CharFilterWhitespace);
	return pick(out);
}

static void sPolicyWarn(PkgLocalPolicy& policy, const String& file, int line, const String& message)
{
	String s = file;
	if(line > 0)
		s << ':' << line;
	s << ": " << message;
	policy.warnings.Add(s);
}

static void sPolicyError(PkgLocalPolicy& policy, const String& file, int line, const String& message)
{
	String s = file;
	if(line > 0)
		s << ':' << line;
	s << ": " << message;
	policy.errors.Add(s);
}

static bool sPolicyIsCapabilityName(const String& s)
{
	String t = TrimBoth(s);
	return t.StartsWith("virtual/") ||
	       t == "virtual/sqlite" || t == "virtual/ssl" || t == "virtual/sdl2" ||
	       t == "virtual/opengl" || t == "virtual/gui-runtime";
}

static String sPackageManifestPath(const String& upp_path)
{
	return ForceExt(upp_path, ".uppkg");
}

static void sManifestWarn(PkgUppkgManifest& manifest, const String& path, const String& message)
{
	String s = path;
	s << ": " << message;
	manifest.warnings.Add(s);
}

static void sManifestError(PkgUppkgManifest& manifest, const String& path, const String& message)
{
	String s = path;
	s << ": " << message;
	manifest.errors.Add(s);
}

static bool sManifestAllowedKey(const String& key)
{
	return key == "path" ||
	       key == "present" ||
	       key == "ok" ||
	       key == "target" ||
	       key == "provider" ||
	       key == "use_default" ||
	       key == "use_forced" ||
	       key == "use_masked" ||
	       key == "provider_preferences" ||
	       key == "notes";
}

static bool sManifestReadStringList(PkgUppkgManifest& manifest, const Value& v, Vector<String>& out, const String& path, const String& key)
{
	out.Clear();
	if(IsNull(v))
		return true;
	if(!v.Is<ValueArray>()) {
		sManifestError(manifest, path, String("field `") + key + "` must be an array of strings");
		return false;
	}
	ValueArray arr = v;
	for(int i = 0; i < arr.GetCount(); i++) {
		const Value& item = arr[i];
		if(!IsString(item)) {
			sManifestError(manifest, path, String("field `") + key + "` must contain only strings");
			continue;
		}
		String s = TrimBoth((String)item);
		if(s.IsEmpty()) {
			sManifestWarn(manifest, path, String("field `") + key + "` contains an empty string");
			continue;
		}
		out.Add(s);
	}
	return true;
}

static bool sManifestReadProviderPreferences(PkgUppkgManifest& manifest, const Value& v, const String& path)
{
	manifest.provider_preferences.Clear();
	if(IsNull(v))
		return true;
	if(!v.Is<ValueArray>()) {
		sManifestError(manifest, path, "field `provider_preferences` must be an array");
		return false;
	}
	ValueArray arr = v;
	for(int i = 0; i < arr.GetCount(); i++) {
		const Value& item = arr[i];
		if(!item.Is<ValueMap>()) {
			sManifestError(manifest, path, "provider_preferences entry must be an object");
			continue;
		}
		ValueMap obj = item;
		Index<String> allowed;
		allowed.Add("capability");
		allowed.Add("provider_id");
		allowed.Add("reason");
		allowed.Add("priority");
		for(int k = 0; k < obj.GetCount(); k++) {
			String key = obj.GetKey(k);
			if(allowed.Find(key) < 0)
				sManifestWarn(manifest, path, String("unknown provider_preferences key: ") + key);
		}
		PkgProviderPreference pref;
		LoadFromJsonValue(pref, item);
		if(pref.capability.IsEmpty()) {
			sManifestError(manifest, path, "provider_preferences entry missing capability");
			continue;
		}
		if(pref.provider_id.IsEmpty()) {
			sManifestWarn(manifest, path, "provider_preferences entry missing provider_id");
		}
		manifest.provider_preferences.Add(pref);
	}
	return true;
}

static bool sLoadUppkgManifest(PkgUppkgManifest& manifest, const String& path)
{
	manifest = PkgUppkgManifest();
	manifest.path = path;
	if(path.IsEmpty() || !FileExists(path)) {
		manifest.present = false;
		manifest.ok = true;
		return true;
	}
	manifest.present = true;
	Value root = ParseJSON(LoadFile(path));
	if(IsNull(root) || !root.Is<ValueMap>()) {
		sManifestError(manifest, path, "manifest root must be a JSON object");
		manifest.ok = false;
		return false;
	}
	ValueMap obj = root;
	for(int i = 0; i < obj.GetCount(); i++) {
		String key = obj.GetKey(i);
		if(!sManifestAllowedKey(key))
			manifest.unknown_keys.Add(key);
	}
	Value v;
	v = obj["target"];
	if(!IsNull(v)) {
		if(!IsString(v)) {
			sManifestError(manifest, path, "field `target` must be a string");
		}
		else
			manifest.target = TrimBoth((String)v);
	}
	v = obj["provider"];
	if(!IsNull(v)) {
		if(!IsString(v)) {
			sManifestError(manifest, path, "field `provider` must be a string");
		}
		else
			manifest.provider = TrimBoth((String)v);
	}
	sManifestReadStringList(manifest, obj["use_default"], manifest.use_default, path, "use_default");
	sManifestReadStringList(manifest, obj["use_forced"], manifest.use_forced, path, "use_forced");
	sManifestReadStringList(manifest, obj["use_masked"], manifest.use_masked, path, "use_masked");
	sManifestReadProviderPreferences(manifest, obj["provider_preferences"], path);
	sManifestReadStringList(manifest, obj["notes"], manifest.notes, path, "notes");
	for(const String& key : manifest.unknown_keys)
		sManifestWarn(manifest, path, String("unknown manifest key: ") + key);
	for(const String& flag : manifest.use_default)
		if(flag.StartsWith("."))
			sManifestWarn(manifest, path, String("use_default should use bare flag names: ") + flag);
	for(const String& flag : manifest.use_forced)
		if(flag.StartsWith("."))
			sManifestWarn(manifest, path, String("use_forced should use bare flag names: ") + flag);
	for(const String& flag : manifest.use_masked)
		if(flag.StartsWith("."))
			sManifestWarn(manifest, path, String("use_masked should use bare flag names: ") + flag);
	if(!manifest.target.IsEmpty() && !sFindTarget(manifest.target))
		sManifestWarn(manifest, path, String("unknown target override: ") + manifest.target);
	manifest.ok = manifest.errors.IsEmpty();
	return manifest.ok;
}

static void sParseMakeConfFile(PkgLocalPolicy& policy, const String& path)
{
	if(path.IsEmpty() || !FileExists(path))
		return;
	policy.make_conf_path = path;
	Vector<String> lines = Split(LoadFile(path), '\n');
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = sPolicyTrimComment(lines[i]);
		if(line.IsEmpty())
			continue;
		int eq = line.Find('=');
		if(eq < 0) {
			sPolicyWarn(policy, path, i + 1, "malformed make.conf line");
			continue;
		}
		String key = ToUpper(TrimBoth(line.Left(eq)));
		String value = sPolicyUnquote(line.Mid(eq + 1));
		if(key == "USE")
			policy.make_use = value;
		else if(key == "TARGET")
			policy.make_target = value;
		else if(key == "PROVIDER")
			policy.make_provider = value;
		else if(key == "COMPILER")
			policy.make_compiler = value;
		else if(key == "LINKER")
			policy.make_linker = value;
		else if(key == "PROFILE")
			policy.make_profile = value;
		else if(key == "REPOSITORY")
			policy.make_repository = value;
		else if(key == "VCPKG_ROOT")
			policy.make_vcpkg_root = value;
		else if(key == "VCPKG_TRIPLET")
			policy.make_vcpkg_triplet = value;
		else if(key == "EMSCRIPTEN_PROFILE")
			policy.make_emscripten_profile = value;
		else if(key == "JOBS")
			policy.make_jobs = ScanInt(value);
		else if(key == "ROOT")
			policy.make_repository = value;
		else if(key == "SYSROOT")
			; // accepted for compatibility; currently reported via info only
		else
			sPolicyWarn(policy, path, i + 1, "unknown make.conf directive: " + key);
	}
}

static void sParsePolicyFile(PkgLocalPolicy& policy, const String& path, Vector<PkgPolicyLine>& dst, bool allow_empty_values = false, bool package_mask = false)
{
	if(path.IsEmpty() || !FileExists(path))
		return;
	Vector<String> lines = Split(LoadFile(path), '\n');
	for(int i = 0; i < lines.GetCount(); i++) {
		String line = sPolicyTrimComment(lines[i]);
		if(line.IsEmpty())
			continue;
		Vector<String> words = sPolicyWords(line);
		if(words.IsEmpty())
			continue;
		if(words.GetCount() < 2 && !allow_empty_values) {
			sPolicyWarn(policy, path, i + 1, "malformed policy line");
			continue;
		}
		PkgPolicyLine& item = dst.Add();
		item.file = path;
		item.line = i + 1;
		item.atom = words[0];
		item.package_mask = package_mask && words.GetCount() < 2;
		for(int j = 1; j < words.GetCount(); j++)
			item.values.Add(words[j]);
	}
}

static bool sPolicyRuleMatches(const PkgRepository& repo, const PkgPackage *pkg, const String& rule_atom)
{
	String rule = TrimBoth(rule_atom);
	if(rule.IsEmpty() || !pkg)
		return false;
	if(ToLower(UnixPath(NormalizePath(rule))) == ToLower(UnixPath(NormalizePath(pkg->path))))
		return true;
	String low = ToLower(rule);
	if(low == ToLower(pkg->atom) || low == ToLower(pkg->name) || low == ToLower(pkg->nest + "/" + pkg->atom))
		return true;
	PkgLookupResult lookup = repo.Resolve(rule);
	return lookup.pkg == pkg && !lookup.ambiguous;
}

static bool sPolicyRuleMatchesAny(const PkgRepository& repo, const String& rule_atom, String& status)
{
	PkgLookupResult lookup = repo.Resolve(rule_atom);
	if(lookup.ambiguous) {
		status = "ambiguous";
		return false;
	}
	if(!lookup.pkg) {
		status = "missing";
		return false;
	}
	status = "ok";
	return true;
}

static PkgLocalPolicy sLoadLocalPolicy(const PkgConfigPaths& paths, const PkgRepository& repo)
{
	PkgLocalPolicy policy;
	policy.make_conf_path = paths.make_conf;
	policy.package_use_path = paths.package_use;
	policy.package_provider_path = paths.package_provider;
	policy.package_target_path = paths.package_target;
	policy.package_mask_path = paths.package_mask;
	policy.package_force_path = paths.package_force;
	sParseMakeConfFile(policy, paths.make_conf);
	sParsePolicyFile(policy, paths.package_use, policy.package_use);
	sParsePolicyFile(policy, paths.package_provider, policy.package_provider);
	sParsePolicyFile(policy, paths.package_target, policy.package_target);
	sParsePolicyFile(policy, paths.package_mask, policy.package_mask, true, true);
	sParsePolicyFile(policy, paths.package_force, policy.package_force);

	Index<String> seen;
	auto validate = [&](const Vector<PkgPolicyLine>& lines, const char *kind) {
		for(const PkgPolicyLine& line : lines) {
			String status;
			if(!sPolicyRuleMatchesAny(repo, line.atom, status)) {
				String msg = String(kind) + " rule for unknown package atom: " + line.atom;
				if(status == "ambiguous")
					msg = String(kind) + " rule has ambiguous package atom: " + line.atom;
				String key = line.file + ":" + AsString(line.line) + ":" + msg;
				if(seen.Find(key) < 0) {
					seen.Add(key);
					if(status == "ambiguous")
						sPolicyWarn(policy, line.file, line.line, msg);
					else
						sPolicyWarn(policy, line.file, line.line, msg);
				}
			}
		}
	};
	validate(policy.package_use, "package.use");
	validate(policy.package_target, "package.target");
	validate(policy.package_mask, "package.mask");
	validate(policy.package_force, "package.force");
	for(const PkgPolicyLine& line : policy.package_provider) {
		if(sPolicyIsCapabilityName(line.atom))
			continue;
		String status;
		if(!sPolicyRuleMatchesAny(repo, line.atom, status)) {
			String msg = String("package.provider rule for unknown package atom: ") + line.atom;
			if(status == "ambiguous")
				msg = String("package.provider rule has ambiguous package atom: ") + line.atom;
			sPolicyWarn(policy, line.file, line.line, msg);
		}
	}
	return policy;
}

static String sPolicyCapabilityProvider(const PkgLocalPolicy& policy, const String& capability);

static void sAppendPolicyValues(Vector<String>& dst, const Vector<String>& src)
{
	for(const String& s : src)
		if(FindIndex(dst, s) < 0)
			dst.Add(s);
}

static Vector<String> sMergeUseArgs(const Vector<String>& base, const Vector<String>& override)
{
	Index<String> seen;
	Vector<String> order;
	VectorMap<String, bool> state;
	auto add = [&](const String& token) {
		String t = TrimBoth(token);
		if(t.IsEmpty())
			return;
		bool selected = true;
		if(t.StartsWith("+"))
			t = t.Mid(1);
		else if(t.StartsWith("-")) {
			t = t.Mid(1);
			selected = false;
		}
		if(t.IsEmpty())
			return;
		int i = state.Find(t);
		if(i < 0) {
			state.Add(t, selected);
			order.Add(t);
		}
		else
			state[i] = selected;
	};
	for(const String& s : base)
		add(s);
	for(const String& s : override)
		add(s);
	Vector<String> out;
	for(const String& s : order) {
		int i = state.Find(s);
		bool selected = i >= 0 ? state[i] : true;
		out.Add(selected ? s : String("-") + s);
	}
	return out;
}

static bool sPolicyPackageMasked(const PkgLocalPolicy& policy, const PkgRepository& repo, const PkgPackage *pkg)
{
	if(!pkg)
		return false;
	for(const PkgPolicyLine& line : policy.package_mask)
		if(line.package_mask && sPolicyRuleMatches(repo, pkg, line.atom))
			return true;
	return false;
}

static void sApplyPolicyLayer(PkgInvocation& out, const PkgInvocation& base, const PkgEselectState& eselect, const PkgLocalPolicy& policy, const PkgRepository& repo, const PkgPackage *pkg)
{
	String pkg_target;
	String pkg_provider;
	String pkg_compiler;
	String pkg_linker;
	String pkg_profile;
	String pkg_repository;
	String pkg_vcpkg_root;
	String pkg_vcpkg_triplet;
	String pkg_emscripten_profile;
	Vector<String> pkg_use = Split(policy.make_use, CharFilterWhitespace);

	if(pkg) {
		if(pkg->manifest.present && pkg->manifest.ok) {
			if(!base.target_explicit && !pkg->manifest.target.IsEmpty())
				pkg_target = pkg->manifest.target;
			if(!base.provider_explicit && !pkg->manifest.provider.IsEmpty())
				pkg_provider = pkg->manifest.provider;
		}
		for(const PkgPolicyLine& line : policy.package_use)
			if(sPolicyRuleMatches(repo, pkg, line.atom))
				sAppendPolicyValues(pkg_use, line.values);
		for(const PkgPolicyLine& line : policy.package_force)
			if(sPolicyRuleMatches(repo, pkg, line.atom))
				for(const String& s : line.values)
					pkg_use.Add(String("+") + s);
		for(const PkgPolicyLine& line : policy.package_mask)
			if(sPolicyRuleMatches(repo, pkg, line.atom))
				for(const String& s : line.values)
					pkg_use.Add(String("-") + s);
		for(const PkgPolicyLine& line : policy.package_target)
			if(sPolicyRuleMatches(repo, pkg, line.atom) && !line.values.IsEmpty())
				pkg_target = line.values.Top();
		for(const PkgPolicyLine& line : policy.package_provider)
			if(sPolicyRuleMatches(repo, pkg, line.atom) && !line.values.IsEmpty())
				pkg_provider = line.values.Top();
	}

	out.use_args = sMergeUseArgs(pkg_use, base.use_args);

	if(!base.target_explicit) {
		if(!policy.make_target.IsEmpty())
			out.target = policy.make_target;
		PkgInvocation target_base;
		sCopyInvocation(target_base, base);
		if(target_base.compiler.IsEmpty())
			target_base.compiler = !policy.make_compiler.IsEmpty() ? policy.make_compiler : eselect.compiler;
		String esa_target = sResolveEselectTarget(target_base, eselect);
		if(out.target.IsEmpty() && !esa_target.IsEmpty())
			out.target = esa_target;
		if(!pkg_target.IsEmpty())
			out.target = pkg_target;
	}

	if(!base.provider_explicit) {
		if(!policy.make_provider.IsEmpty())
			out.provider = policy.make_provider;
		if(out.provider.IsEmpty() && !eselect.provider.IsEmpty())
			out.provider = eselect.provider;
		if(!pkg_provider.IsEmpty())
			out.provider = pkg_provider;
	}

	if(base.compiler.IsEmpty()) {
		if(!policy.make_compiler.IsEmpty())
			out.compiler = policy.make_compiler;
		if(out.compiler.IsEmpty() && !eselect.compiler.IsEmpty())
			out.compiler = eselect.compiler;
		if(!pkg_compiler.IsEmpty())
			out.compiler = pkg_compiler;
	}

	if(base.linker.IsEmpty()) {
		if(!policy.make_linker.IsEmpty())
			out.linker = policy.make_linker;
		if(out.linker.IsEmpty() && !eselect.linker.IsEmpty())
			out.linker = eselect.linker;
		if(!pkg_linker.IsEmpty())
			out.linker = pkg_linker;
	}

	if(base.profile.IsEmpty()) {
		if(!policy.make_profile.IsEmpty())
			out.profile = policy.make_profile;
		if(out.profile.IsEmpty() && !eselect.profile.IsEmpty())
			out.profile = eselect.profile;
		if(!pkg_profile.IsEmpty())
			out.profile = pkg_profile;
	}

	if(base.repository.IsEmpty()) {
		if(!policy.make_repository.IsEmpty())
			out.repository = policy.make_repository;
		if(out.repository.IsEmpty() && !eselect.repository.IsEmpty())
			out.repository = eselect.repository;
		if(!pkg_repository.IsEmpty())
			out.repository = pkg_repository;
	}

	if(base.vcpkg_root.IsEmpty()) {
		if(!policy.make_vcpkg_root.IsEmpty())
			out.vcpkg_root = policy.make_vcpkg_root;
		if(out.vcpkg_root.IsEmpty() && !eselect.vcpkg_root.IsEmpty())
			out.vcpkg_root = eselect.vcpkg_root;
		if(!pkg_vcpkg_root.IsEmpty())
			out.vcpkg_root = pkg_vcpkg_root;
	}

	if(base.vcpkg_triplet.IsEmpty()) {
		if(!policy.make_vcpkg_triplet.IsEmpty())
			out.vcpkg_triplet = policy.make_vcpkg_triplet;
		if(out.vcpkg_triplet.IsEmpty() && !eselect.vcpkg_triplet.IsEmpty())
			out.vcpkg_triplet = eselect.vcpkg_triplet;
		if(!pkg_vcpkg_triplet.IsEmpty())
			out.vcpkg_triplet = pkg_vcpkg_triplet;
	}

	if(base.emscripten_profile.IsEmpty()) {
		if(!policy.make_emscripten_profile.IsEmpty())
			out.emscripten_profile = policy.make_emscripten_profile;
		if(out.emscripten_profile.IsEmpty() && !eselect.emscripten_profile.IsEmpty())
			out.emscripten_profile = eselect.emscripten_profile;
		if(!pkg_emscripten_profile.IsEmpty())
			out.emscripten_profile = pkg_emscripten_profile;
	}

	if(base.jobs <= 0 && policy.make_jobs > 0)
		out.jobs = policy.make_jobs;
}

static PkgInvocation sEffectiveInvocation(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, const PkgPackage *pkg)
{
	PkgInvocation out;
	sCopyInvocation(out, inv);
	PkgEselectState eselect = sLoadEselectState(inv);
	sApplyPolicyLayer(out, inv, eselect, policy, repo, pkg);
	if(out.use_args.IsEmpty() && !policy.make_use.IsEmpty())
		out.use_args = Split(policy.make_use, CharFilterWhitespace);
	if(out.target.IsEmpty())
		out.target = sResolveEselectTarget(out, eselect);
	if(out.target.IsEmpty())
		out.target = "native";
	return out;
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
static void sCopyPkg(PkgPackage& dst, const PkgPackage& src);
static void sFlushConsole();
static bool sLoadMetadataCache(const String& path, PkgMetadataCache& cache, bool& corrupt);
static void sStoreMetadataCache(const String& path, const PkgMetadataCache& cache);
static Time sPkgFileStamp(const String& path);
static int64 sPkgFileSize(const String& path);
static int sFindMetadataCacheEntry(const Vector<PkgMetadataCacheEntry>& entries, const String& path);

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
	p.cache_dir = AppendFileName(p.ai_dir, "cache");
	p.metadata_cache = AppendFileName(p.cache_dir, "package-metadata.json");
	p.sets_dir = AppendFileName(p.ai_dir, "sets");
	p.make_conf = AppendFileName(p.ai_dir, "make.conf");
	p.system_set = AppendFileName(p.sets_dir, "system");
	p.toolchain_set = AppendFileName(p.sets_dir, "toolchain");
	p.world = AppendFileName(p.ai_dir, "world");
	p.package_use = AppendFileName(p.ai_dir, "package.use");
	p.package_provider = AppendFileName(p.ai_dir, "package.provider");
	p.package_target = AppendFileName(p.ai_dir, "package.target");
	p.package_mask = AppendFileName(p.ai_dir, "package.mask");
	p.package_force = AppendFileName(p.ai_dir, "package.force");
	p.state = AppendFileName(p.ai_dir, "state.json");
	p.eselect = AppendFileName(p.ai_dir, "eselect.json");
	p.transaction = AppendFileName(p.ai_dir, "last-transaction.json");
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

static String sPackageResolvePath(const String& path)
{
	return ToLower(UnixPath(NormalizePath(path)));
}

static int sPackageResolveScore(const PkgPackage& p, const String& query)
{
	String q = TrimBoth(query);
	if(q.IsEmpty())
		return 1000;
	String qpath = sPackageResolvePath(q);
	String qlow = ToLower(q);
	String path = sPackageResolvePath(p.path);
	String atom = ToLower(p.atom);
	String name = ToLower(p.name);
	String qualified = ToLower(p.nest + "/" + p.atom);
	String title = ToLower(GetFileName(path));

	if(qpath == path || q == p.path)
		return 0;
	if(qlow == name || qlow == atom || qlow == qualified)
		return 1;
	if(path.EndsWith(qpath) || qpath.EndsWith(path))
		return 2;
	if(atom.EndsWith(qlow) || name.EndsWith(qlow) || title == qlow)
		return 3;
	return 1000;
}

static String sPackageLookupLabel(const PkgPackage& p)
{
	return p.name + " <" + p.path + ">";
}

PkgLookupResult PkgRepository::Resolve(const String& atom) const
{
	++resolve_calls;
	PkgLookupResult r;
	r.query = TrimBoth(atom);
	if(r.query.IsEmpty())
		return r;

	String cache_key = ToLower(sPackageResolvePath(r.query));
	if(!cache_key.IsEmpty()) {
		int h = resolve_cache.Find(cache_key);
		if(h >= 0) {
			++resolve_cache_hits;
			const PkgResolveCacheEntry& e = resolve_cache[h];
			r.canonical = e.canonical;
			r.path = e.path;
			r.direct_path = e.direct_path;
			r.ambiguous = e.ambiguous;
			if(e.pkg_index >= 0 && e.pkg_index < packages.GetCount())
				r.pkg = &packages[e.pkg_index];
			for(int idx : e.candidates)
				if(idx >= 0 && idx < packages.GetCount())
					r.candidates.Add(&packages[idx]);
			return r;
		}
	}

	Index<int> candidate_index;
	auto add_candidates = [&](const VectorMap<String, Vector<int> >& index, const String& key) {
		int pos = index.Find(key);
		if(pos < 0)
			return;
		for(int idx : index[pos])
			if(candidate_index.Find(idx) < 0)
				candidate_index.Add(idx);
	};

	String qlow = ToLower(r.query);
	String qpath = sPackageResolvePath(r.query);
	add_candidates(resolve_path_index, qpath);
	add_candidates(resolve_atom_index, qlow);
	add_candidates(resolve_name_index, qlow);
	add_candidates(resolve_qualified_index, qlow);
	bool used_index = !candidate_index.IsEmpty();
	if(used_index)
		++resolve_index_hits;

	int best = 1000;
	auto scan = [&](const Index<int>& candidates) {
		for(int idx : candidates) {
			const PkgPackage& p = packages[idx];
			int score = sPackageResolveScore(p, r.query);
			if(score >= 1000)
				continue;
			if(score < best) {
				best = score;
				r.candidates.Clear();
			}
			if(score == best)
				r.candidates.Add(&p);
		}
	};

	if(candidate_index.IsEmpty()) {
		Index<int> all;
		for(int i = 0; i < packages.GetCount(); i++)
			all.Add(i);
		scan(all);
	}
	else {
		scan(candidate_index);
		if(r.candidates.IsEmpty()) {
			Index<int> all;
			for(int i = 0; i < packages.GetCount(); i++)
				all.Add(i);
			scan(all);
		}
	}

	if(r.candidates.IsEmpty())
	{
		if(!cache_key.IsEmpty())
		{
			PkgResolveCacheEntry& e = resolve_cache.GetAdd(cache_key);
			e.canonical.Clear();
			e.path.Clear();
			e.candidates.Clear();
			e.pkg_index = -1;
			e.direct_path = false;
			e.ambiguous = false;
		}
		return r;
	}

	if(r.candidates.GetCount() == 1) {
		r.pkg = r.candidates[0];
		r.canonical = r.pkg->name;
		r.path = r.pkg->path;
		r.direct_path = sPackageResolvePath(r.query) == sPackageResolvePath(r.pkg->path) || r.query.EndsWith(".upp") || r.query.EndsWith(".xupp");
		if(!cache_key.IsEmpty())
		{
			PkgResolveCacheEntry& e = resolve_cache.GetAdd(cache_key);
			e.canonical = r.canonical;
			e.path = r.path;
			e.candidates.Clear();
			e.pkg_index = r.pkg ? int(r.pkg - packages.Begin()) : -1;
			e.direct_path = r.direct_path;
			e.ambiguous = false;
		}
		return r;
	}

	r.ambiguous = true;
	if(!cache_key.IsEmpty())
	{
		PkgResolveCacheEntry& e = resolve_cache.GetAdd(cache_key);
		e.canonical.Clear();
		e.path.Clear();
		e.candidates.Clear();
		for(const PkgPackage* c : r.candidates)
			if(c)
				e.candidates.Add(int(c - packages.Begin()));
		e.pkg_index = -1;
		e.direct_path = false;
		e.ambiguous = true;
	}
	return r;
}

void PkgRepository::Discover()
{
	TimeStop ts;
	root = PkgRepoRoot();
	paths = FindPkgConfigPaths(root);
	packages.Clear();
	nests.Clear();
	cache_used = false;
	cache_ok = false;
	cache_corrupt = false;
	cache_loaded_entries = 0;
	cache_reused_entries = 0;
	cache_reparsed_entries = 0;
	cache_stale_entries = 0;
	cache_schema = 0;
	resolve_cache.Clear();
	resolve_atom_index.Clear();
	resolve_name_index.Clear();
	resolve_path_index.Clear();
	resolve_qualified_index.Clear();
	resolve_calls = 0;
	resolve_cache_hits = 0;
	resolve_index_hits = 0;
	search_calls = 0;
	find_calls = 0;
	discover_paths = 0;
	loaded_packages = 0;
	Vector<String> upp = FindAllPaths(root, "*.upp");
	discover_paths = upp.GetCount();
	cache_used = use_cache;

	PkgMetadataCache cache;
	bool cache_is_corrupt = false;
	if(use_cache && !rebuild_cache) {
		if(sLoadMetadataCache(paths.metadata_cache, cache, cache_is_corrupt)) {
			cache_ok = true;
			cache_schema = cache.schema;
			cache_loaded_entries = cache.entries.GetCount();
		}
		else {
			cache_corrupt = cache_is_corrupt;
			cache_ok = !FileExists(paths.metadata_cache);
			if(cache_corrupt)
				Cerr() << "pkg: metadata cache is corrupt, rebuilding in memory: " << paths.metadata_cache << "\n";
		}
	}

	bool cache_changed = false;
	for(const String& upp_path : upp) {
		String nest;
		String atom = sPackageAtomFromUpp(root, upp_path, nest);
		if(atom.IsEmpty())
			continue;
		int64 size = sPkgFileSize(upp_path);
		Time mtime = FileGetTime(upp_path);
		String uppkg_path = sPackageManifestPath(upp_path);
		bool uppkg_exists = FileExists(uppkg_path);
		int64 uppkg_size = uppkg_exists ? sPkgFileSize(uppkg_path) : 0;
		Time uppkg_mtime = uppkg_exists ? FileGetTime(uppkg_path) : Time();
		PkgPackage p;
		bool reused = false;
		if(use_cache && !rebuild_cache && cache_ok) {
			int ci = sFindMetadataCacheEntry(cache.entries, upp_path);
			if(ci >= 0) {
				const PkgMetadataCacheEntry& ce = cache.entries[ci];
				if(ce.size == size && ce.mtime == mtime &&
				   sPackageResolvePath(ce.path) == sPackageResolvePath(upp_path) &&
				   sPackageResolvePath(ce.manifest_path) == sPackageResolvePath(uppkg_path) &&
				   ce.manifest_size == uppkg_size && ce.manifest_mtime == uppkg_mtime) {
					sCopyPkg(p, ce.pkg);
					reused = true;
					cache_reused_entries++;
				}
				else
					cache_stale_entries++;
			}
			else
				cache_stale_entries++;
		}
		if(!reused) {
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
			p.manifest.path = uppkg_path;
			if(uppkg_exists)
				sLoadUppkgManifest(p.manifest, uppkg_path);
			else {
				p.manifest.present = false;
				p.manifest.ok = true;
			}
			for(int i = 0; i < pkg.uses.GetCount(); i++)
				p.uses.Add(pkg.uses[i].text);
			for(int i = 0; i < pkg.config.GetCount(); i++)
				if(!pkg.config[i].name.IsEmpty())
					p.mainconfig.Add(pkg.config[i].name);
			Vector<String> sources = sExpandSourceFiles(p.dir);
			for(const String& s : sources)
				p.source_files.Add(s);
			cache_reparsed_entries++;
			cache_changed = true;
		}
		else {
			p.atom = atom;
			if(p.name.IsEmpty())
				p.name = sPackageNameFromPath(atom, nest);
			if(p.nest.IsEmpty())
				p.nest = nest;
			p.path = upp_path;
			if(p.dir.IsEmpty())
				p.dir = GetFileFolder(upp_path);
			if(!p.mtime.IsValid())
				p.mtime = mtime;
			if(p.manifest.path.IsEmpty())
				p.manifest.path = uppkg_path;
			if(uppkg_exists && !p.manifest.present) {
				sLoadUppkgManifest(p.manifest, uppkg_path);
			}
		}
		packages.Add(pick(p));
		if(FindIndex(nests, nest) < 0)
			nests.Add(nest);
	}
	for(int i = 0; i < packages.GetCount(); i++) {
		const PkgPackage& p = packages[i];
		resolve_path_index.GetAdd(sPackageResolvePath(p.path)).Add(i);
		resolve_atom_index.GetAdd(ToLower(p.atom)).Add(i);
		resolve_name_index.GetAdd(ToLower(p.name)).Add(i);
		resolve_qualified_index.GetAdd(ToLower(p.nest + "/" + p.atom)).Add(i);
	}
	loaded_packages = packages.GetCount();
	discover_seconds = ts.Seconds();

	if(use_cache && (cache_changed || rebuild_cache || !cache_ok)) {
		cache.schema = 2;
		cache.root = root;
		cache.saved_at = GetSysTime();
		cache.entries.Clear();
		for(const PkgPackage& p : packages) {
			PkgMetadataCacheEntry& e = cache.entries.Add();
			e.path = p.path;
			e.manifest_path = p.manifest.present ? p.manifest.path : String();
			e.size = sPkgFileSize(p.path);
			e.mtime = sPkgFileStamp(p.path);
			e.manifest_size = p.manifest.present ? sPkgFileSize(p.manifest.path) : 0;
			e.manifest_mtime = p.manifest.present ? sPkgFileStamp(p.manifest.path) : Time();
			sCopyPkg(e.pkg, p);
		}
		sStoreMetadataCache(paths.metadata_cache, cache);
		cache_ok = true;
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
	++find_calls;
	PkgLookupResult r = Resolve(atom);
	if(r.ambiguous || !r.pkg)
		return nullptr;
	return r.pkg;
}

Vector<const PkgPackage*> PkgRepository::Search(const String& query) const
{
	++search_calls;
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

static const PkgStateRecord* sFindStateRecord(const PkgState& state, const String& atom)
{
	for(int i = state.records.GetCount() - 1; i >= 0; --i)
		if(state.records[i].atom == atom)
			return &state.records[i];
	return nullptr;
}

static String sFormatUppProjection(const PkgUppProjection& proj);
static const PkgTargetProfile& sDefaultTargetProfile(const String& target);
static String sTargetNameText(const String& target);
static Vector<String> sLoadSet(const PkgConfigPaths& paths, const String& name, const String& path);
static PkgPlan sBuildPlan(const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgLocalPolicy& policy, bool color);

static bool sSameStringList(const Vector<String>& a, const Vector<String>& b)
{
	if(a.GetCount() != b.GetCount())
		return false;
	for(int i = 0; i < a.GetCount(); i++)
		if(a[i] != b[i])
			return false;
	return true;
}

static Vector<String> sSplitSpelling(const String& s)
{
	Vector<String> v = Split(s, CharFilterWhitespace);
	return pick(v);
}

static Vector<String> sPlanUppFlags(const PkgUppProjection& upp)
{
	Vector<String> out;
	for(const String& s : sSplitSpelling(sFormatUppProjection(upp)))
		out.Add(s);
	return pick(out);
}

static bool sRecordMatchesNewUse(const PkgStateRecord& rec, const PkgPlan& plan, const PkgPackage *pkg)
{
	Vector<String> accepted;
	if(pkg)
		for(const String& s : pkg->accepts)
			accepted.Add(s);
	if(!sSameStringList(rec.declared_use, plan.use.declared))
		return false;
	if(!sSameStringList(rec.effective_use, plan.use.effective))
		return false;
	if(!sSameStringList(rec.effective_uppflags, sPlanUppFlags(plan.upp)))
		return false;
	if(!sSameStringList(rec.providers, plan.providers))
		return false;
	if(!sSameStringList(rec.accepted_flags, accepted))
		return false;
	if(rec.target != sTargetNameText(plan.target))
		return false;
	if(rec.toolchain != sDefaultTargetProfile(plan.target).toolchain)
		return false;
	return true;
}

static bool sRecordMatchesChangedUse(const PkgStateRecord& rec, const PkgPlan& plan)
{
	if(!sSameStringList(rec.effective_use, plan.use.effective))
		return false;
	if(!sSameStringList(rec.effective_uppflags, sPlanUppFlags(plan.upp)))
		return false;
	if(!sSameStringList(rec.providers, plan.providers))
		return false;
	if(rec.target != sTargetNameText(plan.target))
		return false;
	return true;
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
	return Resolve(atom).pkg != nullptr;
}

static void sCopyPkg(PkgPackage& dst, const PkgPackage& src)
{
	dst.atom = src.atom;
	dst.name = src.name;
	dst.nest = src.nest;
	dst.path = src.path;
	dst.dir = src.dir;
	dst.description = src.description;
	dst.accepts.Clear();
	for(const String& s : src.accepts)
		dst.accepts.Add(s);
	dst.uses.Clear();
	for(const String& s : src.uses)
		dst.uses.Add(s);
	dst.mainconfig.Clear();
	for(const String& s : src.mainconfig)
		dst.mainconfig.Add(s);
	dst.source_files.Clear();
	for(const String& s : src.source_files)
		dst.source_files.Add(s);
	dst.manifest.path = src.manifest.path;
	dst.manifest.present = src.manifest.present;
	dst.manifest.ok = src.manifest.ok;
	dst.manifest.target = src.manifest.target;
	dst.manifest.provider = src.manifest.provider;
	dst.manifest.use_default.Clear();
	for(const String& s : src.manifest.use_default)
		dst.manifest.use_default.Add(s);
	dst.manifest.use_forced.Clear();
	for(const String& s : src.manifest.use_forced)
		dst.manifest.use_forced.Add(s);
	dst.manifest.use_masked.Clear();
	for(const String& s : src.manifest.use_masked)
		dst.manifest.use_masked.Add(s);
	dst.manifest.provider_preferences.Clear();
	for(const PkgProviderPreference& p : src.manifest.provider_preferences)
		dst.manifest.provider_preferences.Add(p);
	dst.manifest.notes.Clear();
	for(const String& s : src.manifest.notes)
		dst.manifest.notes.Add(s);
	dst.manifest.warnings.Clear();
	for(const String& s : src.manifest.warnings)
		dst.manifest.warnings.Add(s);
	dst.manifest.errors.Clear();
	for(const String& s : src.manifest.errors)
		dst.manifest.errors.Add(s);
	dst.manifest.unknown_keys.Clear();
	for(const String& s : src.manifest.unknown_keys)
		dst.manifest.unknown_keys.Add(s);
	dst.mtime = src.mtime;
}

static Time sPkgFileStamp(const String& path)
{
	return FileGetTime(path);
}

static int64 sPkgFileSize(const String& path)
{
	return GetFileLength(path);
}

static int sFindMetadataCacheEntry(const Vector<PkgMetadataCacheEntry>& entries, const String& path)
{
	String key = sPackageResolvePath(path);
	for(int i = 0; i < entries.GetCount(); i++)
		if(sPackageResolvePath(entries[i].path) == key)
			return i;
	return -1;
}

static bool sLoadMetadataCache(const String& path, PkgMetadataCache& cache, bool& corrupt)
{
	corrupt = false;
	if(!FileExists(path))
		return false;
	if(!LoadFromJsonFile(cache, path)) {
		corrupt = true;
		return false;
	}
	if(cache.schema <= 0)
		cache.schema = 1;
	return true;
}

static void sStoreMetadataCache(const String& path, const PkgMetadataCache& cache)
{
	RealizeDirectory(GetFileFolder(path));
	StoreAsJsonFile(cache, path, true);
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

static void sAddTargetUpp(PkgTargetProfile& p, const String& name, int scope, const String& reason)
{
	PkgUppFlag& f = p.upp_add.Add();
	f.name = name;
	f.scope = scope;
	f.reason = reason;
}

static void sAddTargetProviderPreference(PkgTargetProfile& p, const String& capability, const String& provider_id, const String& reason, int priority = 100)
{
	PkgProviderPreference& pref = p.provider_preferences.Add();
	pref.capability = capability;
	pref.provider_id = provider_id;
	pref.reason = reason;
	pref.priority = priority;
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
		p.name = "native-msvc";
		p.host_platform = "host";
		p.build_platform = "host";
		p.target_platform = "native";
		p.runtime_environment = "desktop";
		p.architecture = "native";
		p.toolchain = "msvc";
		p.sysroot = "vcvars";
		p.thread_model = "mt";
		p.compiler = "msvc";
		p.linker = "link";
		p.summary = "Native MSVC target";
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
		p.default_use.Add("st");
		p.forced_use.Add("st");
		p.forced_use.Add("virtualgui");
		p.masked_use.Add("nativegui");
		p.masked_use.Add("gtk");
		p.masked_use.Add("x11");
		sAddTargetUpp(p, "ST", PKG_UPP_GLOBAL, "wasm browser thread model");
		sAddTargetUpp(p, "GUI", PKG_UPP_GLOBAL, "wasm browser gui runtime");
		sAddTargetUpp(p, "SDL2GL", PKG_UPP_GLOBAL, "wasm browser gui runtime");
		sAddTargetUpp(p, "VIRTUALGUI", PKG_UPP_ACCEPTED, "wasm browser gui runtime");
		sAddTargetProviderPreference(p, "virtual/sdl2", "emscripten-sdl2", "browser sdl2 runtime");
		sAddTargetProviderPreference(p, "virtual/opengl", "webgl", "browser opengl runtime");
		sAddTargetProviderPreference(p, "virtual/gui-runtime", "virtualgui-sdl2gl", "browser gui runtime");
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
		p.default_use.Add("st");
		p.forced_use.Add("st");
		p.masked_use.Add("nativegui");
		p.masked_use.Add("gtk");
		p.masked_use.Add("x11");
		sAddTargetUpp(p, "ST", PKG_UPP_GLOBAL, "wasm node thread model");
		sAddTargetProviderPreference(p, "virtual/sdl2", "emscripten-sdl2", "node wasm sdl2 runtime");
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
		p.masked_use.Add("nativegui");
		p.masked_use.Add("x11");
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
		p.default_use.Add("st");
		p.forced_use.Add("st");
		p.masked_use.Add("gui");
		p.masked_use.Add("nativegui");
		p.masked_use.Add("x11");
		sAddTargetUpp(p, "ST", PKG_UPP_GLOBAL, "retro thread model");
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

static void sAddProviderFlag(PkgProvider& p, const String& name, int scope, const String& reason)
{
	PkgUppFlag& f = p.upp_add.Add();
	f.name = name;
	f.scope = scope;
	f.reason = reason;
}

static Vector<PkgProvider> sProviders()
{
	Vector<PkgProvider> v;
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sqlite";
		p.id = "upp-plugin-sqlite3";
		p.kind = "upp-plugin";
		p.provider = "plugin/sqlite3";
		p.details = "Repository-local sqlite3 plugin";
		p.priority = 100;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		p.uses_add.Add("plugin/sqlite3");
		sAddProviderFlag(p, "SQLITE", PKG_UPP_ACCEPTED, "sqlite provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sqlite";
		p.id = "system-sqlite";
		p.kind = "system";
		p.provider = "sqlite3";
		p.details = "System SQLite package";
		p.priority = 80;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		sAddProviderFlag(p, "SQLITE", PKG_UPP_ACCEPTED, "system sqlite provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sqlite";
		p.id = "vcpkg-sqlite3";
		p.kind = "vcpkg";
		p.provider = "sqlite3";
		p.details = "vcpkg sqlite3 package";
		p.priority = 60;
		p.targets.Add("windows");
		sAddProviderFlag(p, "SQLITE", PKG_UPP_ACCEPTED, "vcpkg sqlite provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/ssl";
		p.id = "upp-plugin-core-ssl";
		p.kind = "upp-plugin";
		p.provider = "Core/SSL";
		p.details = "Core SSL package";
		p.priority = 100;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		p.uses_add.Add("Core/SSL");
		sAddProviderFlag(p, "SSL", PKG_UPP_ACCEPTED, "ssl provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/ssl";
		p.id = "system-ssl";
		p.kind = "system";
		p.provider = "openssl";
		p.details = "System OpenSSL package";
		p.priority = 75;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		sAddProviderFlag(p, "SSL", PKG_UPP_ACCEPTED, "system ssl provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sdl2";
		p.id = "system-sdl2";
		p.kind = "system";
		p.provider = "SDL2";
		p.details = "System SDL2 package";
		p.priority = 70;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		sAddProviderFlag(p, "SDL2", PKG_UPP_GLOBAL, "system sdl2 provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sdl2";
		p.id = "vcpkg-sdl2";
		p.kind = "vcpkg";
		p.provider = "SDL2";
		p.details = "vcpkg SDL2 package";
		p.priority = 60;
		p.targets.Add("windows");
		sAddProviderFlag(p, "SDL2", PKG_UPP_GLOBAL, "vcpkg sdl2 provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sdl2";
		p.id = "msys2-sdl2";
		p.kind = "msys2";
		p.provider = "mingw-w64-sdl2";
		p.details = "MSYS2 SDL2 package";
		p.priority = 50;
		p.targets.Add("windows");
		sAddProviderFlag(p, "SDL2", PKG_UPP_GLOBAL, "msys2 sdl2 provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/sdl2";
		p.id = "emscripten-sdl2";
		p.kind = "emscripten";
		p.provider = "SDL2";
		p.details = "Emscripten SDL2 package";
		p.priority = 40;
		p.targets.Add("wasm-browser");
		p.targets.Add("wasm-node");
		sAddProviderFlag(p, "SDL2", PKG_UPP_GLOBAL, "emscripten sdl2 provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/opengl";
		p.id = "system-opengl";
		p.kind = "system";
		p.provider = "opengl32";
		p.details = "System OpenGL runtime";
		p.priority = 70;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "system opengl provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/opengl";
		p.id = "webgl";
		p.kind = "web";
		p.provider = "WebGL";
		p.details = "Browser WebGL runtime";
		p.priority = 60;
		p.targets.Add("wasm-browser");
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "webgl provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/opengl";
		p.id = "angle-manual";
		p.kind = "manual";
		p.provider = "ANGLE";
		p.details = "Manual ANGLE setup";
		p.priority = 10;
		p.manual = true;
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "angle provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/gui-runtime";
		p.id = "native-gui";
		p.kind = "system";
		p.provider = "desktop-gui";
		p.details = "Host GUI runtime";
		p.priority = 70;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "native gui runtime");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/gui-runtime";
		p.id = "virtualgui-sdl2gl";
		p.kind = "upp-plugin";
		p.provider = "rainbow/SDL2GL";
		p.details = "VirtualGui over SDL2GL";
		p.priority = 100;
		p.targets.Add("native");
		p.targets.Add("windows");
		p.targets.Add("linux");
		p.targets.Add("macos");
		p.uses_add.Add("rainbow/SDL2GL");
		p.uses_add.Add("rainbow/VirtualGui");
		p.targets.Add("wasm-browser");
		p.targets.Add("wasm-node");
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "virtualgui provider");
		sAddProviderFlag(p, "SDL2GL", PKG_UPP_GLOBAL, "virtualgui provider");
		sAddProviderFlag(p, "VIRTUALGUI", PKG_UPP_ACCEPTED, "virtualgui provider");
	}
	{
		PkgProvider& p = v.Add();
		p.capability = "virtual/gui-runtime";
		p.id = "framebuffer";
		p.kind = "system";
		p.provider = "framebuffer";
		p.details = "Framebuffer runtime";
		p.priority = 50;
		p.targets.Add("linux");
		p.targets.Add("native");
		sAddProviderFlag(p, "GUI", PKG_UPP_GLOBAL, "framebuffer provider");
	}
	return pick(v);
}

static String sTargetFamily(String target)
{
	target = ToLower(TrimBoth(target));
	if(target.IsEmpty())
		return "native";
	if(target.StartsWith("native-") || target == "native")
		return "native";
	if(target.StartsWith("windows-") || target == "windows")
		return "windows";
	if(target.StartsWith("linux-") || target == "linux")
		return "linux";
	if(target.StartsWith("macos-") || target == "macos")
		return "macos";
	if(target.StartsWith("wasm-") || target == "wasm32" || target == "wasm")
		return "wasm";
	if(target.StartsWith("freebsd-") || target == "freebsd")
		return "freebsd";
	return target;
}

static bool sProviderTargetMatch(const PkgProvider& p, const String& target)
{
	if(p.targets.IsEmpty())
		return true;
	String t = sTargetFamily(target);
	for(const String& s : p.targets)
		if(sTargetFamily(s) == t)
			return true;
	return false;
}

static int sProviderPreferenceRank(const PkgProvider& p, const String& pref)
{
	String want = ToLower(pref);
	String kind = ToLower(p.kind);
	if(want == "system") {
		if(kind == "system")
			return 0;
		if(kind == "upp-plugin")
			return 20;
		if(kind == "vcpkg" || kind == "msys2" || kind == "web")
			return 30;
		return 40;
	}
	if(want == "portable") {
		if(kind == "upp-plugin")
			return 0;
		if(kind == "vcpkg" || kind == "msys2" || kind == "web")
			return 10;
		if(kind == "system")
			return 30;
		return 40;
	}
	if(!want.IsEmpty() && (ToLower(p.id) == want || kind == want || ToLower(p.provider) == want))
		return 0;
	if(kind == "upp-plugin")
		return 0;
	if(kind == "vcpkg" || kind == "msys2" || kind == "web")
		return 10;
	if(kind == "system")
		return 20;
	if(kind == "manual")
		return 90;
	return 50;
}

static const PkgProviderPreference* sFindTargetProviderPreference(const PkgTargetProfile *tp, const String& capability)
{
	if(!tp)
		return nullptr;
	for(const PkgProviderPreference& pref : tp->provider_preferences) {
		if(pref.capability.IsEmpty() || pref.capability == "*" || pref.capability == capability)
			return &pref;
	}
	return nullptr;
}

static const PkgProviderPreference* sFindProviderPreference(const Vector<PkgProviderPreference>& prefs, const String& capability)
{
	for(const PkgProviderPreference& pref : prefs) {
		if(pref.capability.IsEmpty() || pref.capability == "*" || pref.capability == capability)
			return &pref;
	}
	return nullptr;
}

static bool sProviderMatchesTargetPreference(const PkgProvider& p, const PkgProviderPreference& pref)
{
	if(!pref.provider_id.IsEmpty()) {
		String want = ToLower(pref.provider_id);
		return ToLower(p.id) == want || ToLower(p.provider) == want || ToLower(p.kind) == want;
	}
	return true;
}

static String sProbeResolvePath(const String& exe, const String& root = Null)
{
	String path;
	if(!root.IsEmpty()) {
		path = AppendFileName(root, exe);
		if(FileExists(path))
			return path;
	}
	path = GetFileOnPath(exe, GetEnv("PATH"), true);
	if(!path.IsEmpty())
		return path;
	return FileExists(exe) ? exe : String();
}

static String sProbeCommandText(const String& exe, const Vector<String>& args)
{
	String cmd;
	if(exe.FindFirstOf(" \t\"") >= 0)
		cmd << '"' << exe << '"';
	else
		cmd << exe;
	for(const String& a : args) {
		cmd << ' ';
		if(a.FindFirstOf(" \t\"") >= 0)
		{
			String q = a;
			q.Replace("\"", "\\\"");
			cmd << '"' << q << '"';
		}
		else
			cmd << a;
	}
	return cmd;
}

static bool sProbeCommandAllowed(const String& exe, const Vector<String>& args, String& why)
{
	String base = ToLower(GetFileTitle(exe));
	if(base == "pkg-config" || base == "pkgconf") {
		if(args.GetCount() == 2 && (args[0] == "--exists" || args[0] == "--modversion"))
			return true;
		why = "pkg-config probe arguments must be --exists <pkg> or --modversion <pkg>";
		return false;
	}
	if(base == "vcpkg") {
		if(args.GetCount() == 1 && args[0] == "list")
			return true;
		why = "vcpkg probe arguments must be exactly: list";
		return false;
	}
	if(base == "emcc" || base == "clang" || base == "gcc") {
		if(args.GetCount() == 1 && args[0] == "--version")
			return true;
		why = "compiler probe arguments must be exactly: --version";
		return false;
	}
	why = "command is not on the read-only probe allowlist";
	return false;
}

static int sRunProbeCommand(const String& exe, const Vector<String>& args, String& out, String& command, String& path, String& why)
{
	why.Clear();
	if(!sProbeCommandAllowed(exe, args, why))
		return -1;
	path = sProbeResolvePath(exe);
	if(path.IsEmpty()) {
		why = "command not found: " + exe;
		return -1;
	}
	command = sProbeCommandText(path, args);
	int rc = Sys(command, out);
	if(rc < 0) {
		why = "failed to launch: " + path;
		return -1;
	}
	return rc;
}

static String sProbeFirstLine(const String& s)
{
	if(IsVoid(s))
		return String();
	int p = s.Find('\n');
	if(p < 0)
		p = s.Find('\r');
	return TrimBoth(p < 0 ? s : s.Left(p));
}

static void sProbePkgConfig(PkgProvider& q, const Vector<String>& names, const String& exe, bool probe)
{
	if(!probe) {
		q.probe_status = "probe_not_run";
		q.probe_reason = "probe disabled";
		q.probe_command = "pkg-config";
		return;
	}
	String pkgconf = sProbeResolvePath(exe);
	if(pkgconf.IsEmpty() && exe != "pkgconf")
		pkgconf = sProbeResolvePath("pkgconf");
	if(pkgconf.IsEmpty()) {
		q.probe_status = "unknown";
		q.probe_reason = exe + " not found";
		return;
	}

	for(const String& name : names) {
		Vector<String> args;
		args.Add("--exists");
		args.Add(name);
		String out, command, path, why;
		int rc = sRunProbeCommand(pkgconf, args, out, command, path, why);
		q.probe_command = command;
		q.probe_path = path;
		if(rc < 0) {
			q.probe_status = "probe_error";
			q.probe_reason = why;
			return;
		}
		if(rc == 0) {
			q.probe_status = "available";
			q.probe_reason = name + " found";
			Vector<String> modv;
			modv.Add("--modversion");
			modv.Add(name);
			String ver;
			String ver_cmd, ver_path, ver_why;
			if(sRunProbeCommand(pkgconf, modv, ver, ver_cmd, ver_path, ver_why) >= 0) {
				q.probe_command = ver_cmd;
				q.probe_path = ver_path;
				q.probe_version = sProbeFirstLine(ver);
			}
			return;
		}
		q.probe_reason = name + " not found";
	}

	q.probe_status = "missing";
}

static void sProbeVcpkg(PkgProvider& q, const PkgInvocation& inv, const String& package, bool probe)
{
	if(!probe) {
		q.probe_status = "probe_not_run";
		q.probe_reason = "probe disabled";
		q.probe_command = "vcpkg list";
		return;
	}

	String root = !inv.vcpkg_root.IsEmpty() ? inv.vcpkg_root : GetEnv("VCPKG_ROOT");
	if(root.IsEmpty()) {
		q.probe_status = "unknown";
		q.probe_reason = "VCPKG_ROOT not configured";
		return;
	}
#ifdef flagWIN32
	String exe = sProbeResolvePath("vcpkg.exe", root);
#else
	String exe = sProbeResolvePath("vcpkg", root);
#endif
	if(exe.IsEmpty())
		exe = sProbeResolvePath("vcpkg");
	if(exe.IsEmpty()) {
		q.probe_status = "unknown";
		q.probe_reason = "vcpkg executable not found";
		return;
	}

	Vector<String> args;
	args.Add("list");
	String out, command, path, why;
	int rc = sRunProbeCommand(exe, args, out, command, path, why);
	q.probe_command = command;
	q.probe_path = path;
	if(rc < 0) {
		q.probe_status = "probe_error";
		q.probe_reason = why;
		return;
	}
	String low = ToLower(out);
	String needle = ToLower(package) + ":";
	if(low.Find(needle) >= 0 || low.Find(ToLower(package) + " ") >= 0 || low.Find(ToLower(package)) >= 0) {
		q.probe_status = "available";
		q.probe_reason = package + " found in vcpkg list";
		q.probe_version = sProbeFirstLine(out);
	}
	else {
		q.probe_status = "missing";
		q.probe_reason = package + " not found in vcpkg list";
	}
}

static void sProbeCompiler(PkgProvider& q, const String& exe_name, bool probe)
{
	if(!probe) {
		q.probe_status = "probe_not_run";
		q.probe_reason = "probe disabled";
		q.probe_command = exe_name + " --version";
		return;
	}
	String exe = sProbeResolvePath(exe_name);
	if(exe.IsEmpty()) {
		q.probe_status = "unknown";
		q.probe_reason = exe_name + " not found";
		return;
	}
	Vector<String> args;
	args.Add("--version");
	String out, command, path, why;
	int rc = sRunProbeCommand(exe, args, out, command, path, why);
	q.probe_command = command;
	q.probe_path = path;
	if(rc < 0) {
		q.probe_status = "probe_error";
		q.probe_reason = why;
		return;
	}
	q.probe_version = sProbeFirstLine(out);
	q.probe_status = rc == 0 ? "available" : "missing";
	q.probe_reason = q.probe_version.IsEmpty() ? exe_name + " version query " + (rc == 0 ? "succeeded" : "failed") : q.probe_version;
}

static void sProbeProviderInfo(PkgProvider& q, const PkgRepository& repo, const PkgInvocation& inv, bool probe)
{
	q.probe_command.Clear();
	q.probe_path.Clear();
	q.probe_version.Clear();
	q.probe_reason.Clear();
	if(q.manual) {
		q.probe_status = "manual";
		q.probe_reason = "manual setup required";
		return;
	}
	if(q.kind == "upp-plugin") {
		bool ok = repo.HasPackage(q.provider);
		q.probe_status = ok ? "available" : "missing";
		q.probe_reason = ok ? "bundled provider package present" : "bundled provider package missing";
		return;
	}
	if(!probe) {
		q.probe_status = "probe_not_run";
		q.probe_reason = "probe disabled";
		return;
	}

	if(q.id == "system-sqlite") {
		Vector<String> names;
		names.Add("sqlite3");
		names.Add("sqlite");
		sProbePkgConfig(q, names, "pkg-config", true);
		return;
	}
	if(q.id == "system-ssl") {
		Vector<String> names;
		names.Add("openssl");
		names.Add("libssl");
		sProbePkgConfig(q, names, "pkg-config", true);
		return;
	}
	if(q.id == "vcpkg-sqlite3") {
		sProbeVcpkg(q, inv, "sqlite3", true);
		return;
	}
	if(q.id == "vcpkg-sdl2") {
		sProbeVcpkg(q, inv, "sdl2", true);
		return;
	}
	if(q.id == "system-sdl2") {
		Vector<String> names;
		names.Add("SDL2");
		names.Add("sdl2");
		sProbePkgConfig(q, names, "pkg-config", true);
		return;
	}
	if(q.id == "msys2-sdl2") {
		if(GetEnv("MSYSTEM").IsEmpty()) {
			q.probe_status = "unknown";
			q.probe_reason = "MSYSTEM not configured";
			return;
		}
		Vector<String> names;
		names.Add("SDL2");
		names.Add("sdl2");
		sProbePkgConfig(q, names, "pkg-config", true);
		if(q.probe_status == "available")
			q.probe_reason = "MSYS2 SDL2 package found";
		else if(q.probe_status == "missing" && q.probe_reason.IsEmpty())
			q.probe_reason = "SDL2 not found in pkg-config";
		return;
	}
	if(q.id == "emscripten-sdl2") {
		String emsdk = GetEnv("EMSDK");
		String emcfg = GetEnv("EM_CONFIG");
		if(emsdk.IsEmpty() && emcfg.IsEmpty()) {
			q.probe_status = "unknown";
			q.probe_reason = "EMSDK/EM_CONFIG not configured";
			return;
		}
		sProbeCompiler(q, "emcc", true);
		if(q.probe_status == "available")
			q.probe_reason = "emcc available";
		return;
	}
	if(q.id == "system-opengl") {
#ifdef flagWIN32
		q.probe_status = "available";
		q.probe_reason = "Windows OpenGL runtime is available";
#else
		Vector<String> names;
		names.Add("gl");
		names.Add("opengl");
		sProbePkgConfig(q, names, "pkg-config", true);
		if(q.probe_status == "missing" && q.probe_reason.IsEmpty())
			q.probe_reason = "OpenGL package not found";
#endif
		return;
	}
	if(q.id == "webgl") {
		sProbeCompiler(q, "emcc", true);
		if(q.probe_status == "available")
			q.probe_reason = "emcc available for WebGL";
		return;
	}
	if(q.id == "native-gui") {
#ifdef flagWIN32
		q.probe_status = "available";
		q.probe_reason = "Windows desktop GUI available";
#else
		if(!GetEnv("DISPLAY").IsEmpty() || !GetEnv("WAYLAND_DISPLAY").IsEmpty()) {
			q.probe_status = "available";
			q.probe_reason = "desktop display environment present";
		}
		else {
			q.probe_status = "missing";
			q.probe_reason = "no display environment present";
		}
#endif
		return;
	}
	if(q.id == "framebuffer") {
#ifdef flagWIN32
		q.probe_status = "unknown";
		q.probe_reason = "framebuffer runtime is platform-specific";
#else
		q.probe_status = FileExists("/dev/fb0") ? "available" : "missing";
		q.probe_reason = FileExists("/dev/fb0") ? "/dev/fb0 present" : "/dev/fb0 missing";
#endif
		return;
	}

	q.probe_status = "unknown";
	q.probe_reason = "no probe adapter";
}

static String sProviderProbeStatus(const PkgProvider& p, const PkgRepository& repo)
{
	if(!p.probe_status.IsEmpty())
		return p.probe_status;
	if(p.manual)
		return "manual";
	if(p.kind == "upp-plugin")
		return repo.HasPackage(p.provider) ? "available" : "missing";
	if(p.kind == "manual")
		return "manual";
	if(p.kind == "system" || p.kind == "vcpkg" || p.kind == "msys2" || p.kind == "web" || p.kind == "emscripten" || p.kind == "freebsd" || p.kind == "debian" || p.kind == "macos")
		return "probe_not_run";
	return "unknown";
}

static Vector<PkgProvider> sProvidersFor(const String& capability, const PkgRepository& repo, const PkgInvocation& inv, bool probe, const String& target = Null)
{
	Vector<PkgProvider> all = sProviders();
	Vector<PkgProvider> out;
	for(const PkgProvider& p : all)
		if(p.capability == capability && sProviderTargetMatch(p, target)) {
			PkgProvider& q = out.Add();
			q.capability = p.capability;
			q.id = p.id;
			q.kind = p.kind;
			q.provider = p.provider;
			q.details = p.details;
			q.priority = p.priority;
			q.system_install = p.system_install;
			q.manual = p.manual;
			sProbeProviderInfo(q, repo, inv, probe);
			for(const String& s : p.targets)
				q.targets.Add(s);
			for(const String& s : p.uses_add)
				q.uses_add.Add(s);
			for(const PkgUppFlag& f : p.upp_add)
				q.upp_add.Add(f);
		}
	Sort(out, [&](const PkgProvider& a, const PkgProvider& b) {
		if(a.priority != b.priority)
			return a.priority > b.priority;
		if(a.kind != b.kind)
			return a.kind < b.kind;
		return a.id < b.id;
	});
	return pick(out);
}

static String sCapabilityTitle(const String& capability)
{
	if(capability == "virtual/sqlite")
		return "SQLite";
	if(capability == "virtual/ssl")
		return "SSL";
	if(capability == "virtual/sdl2")
		return "SDL2";
	if(capability == "virtual/opengl")
		return "OpenGL";
	if(capability == "virtual/gui-runtime")
		return "GUI runtime";
	return GetFileName(capability);
}

static int sProviderSelectionScore(const PkgProvider& p, const PkgRepository& repo, const PkgTargetProfile *tp, const PkgProviderPreference *extra_pref, const String& pref, const String& capability)
{
	String want = ToLower(TrimBoth(pref));
	String kind = ToLower(p.kind);
	int score = 0;
	bool explicit_pref = !want.IsEmpty();
	if(want == "system") {
		if(kind == "system")
			score += 0;
		else if(kind == "upp-plugin")
			score += 100;
		else
			score += 40;
	}
	else if(want == "portable") {
		if(kind == "upp-plugin")
			score += 0;
		else if(kind == "vcpkg" || kind == "msys2" || kind == "web" || kind == "emscripten")
			score += 20;
		else
			score += 60;
	}
	else if(!want.IsEmpty() && (ToLower(p.id) == want || kind == want || ToLower(p.provider) == want)) {
		score += 0;
	}
	else {
		if(kind == "upp-plugin")
			score += 0;
		else if(kind == "vcpkg" || kind == "msys2" || kind == "web" || kind == "emscripten")
			score += 20;
		else if(kind == "system")
			score += 40;
		else if(kind == "manual")
			score += 90;
		else
			score += 50;
	}
	if(!explicit_pref) {
		const PkgProviderPreference* pref_match = extra_pref ? extra_pref : sFindTargetProviderPreference(tp, capability);
		if(!pref_match)
			pref_match = sFindTargetProviderPreference(tp, capability);
		if(pref_match && sProviderMatchesTargetPreference(p, *pref_match))
			score -= 80 + pref_match->priority / 10;
	}
	if(p.kind == "upp-plugin" && !repo.HasPackage(p.provider))
		score += 1000;
	if(p.manual)
		score += 500;
	if(p.probe_status == "available")
		score -= 20;
	else if(p.probe_status == "missing" || p.probe_status == "probe_error")
		score += 500;
	else if(p.probe_status == "unknown")
		score += 20;
	score -= p.priority / 10;
	return score;
}

static PkgProviderResolution sMakeResolution(const PkgProvider& p, const PkgRepository& repo, const String& capability, bool selected)
{
	PkgProviderResolution r;
	r.capability = capability;
	r.provider_id = p.id;
	r.provider_kind = p.kind;
	r.provider = p.provider;
	r.external_package = p.provider;
	r.details = p.details;
	r.probe_status = sProviderProbeStatus(p, repo);
	r.probe_command = p.probe_command;
	r.probe_path = p.probe_path;
	r.probe_version = p.probe_version;
	r.probe_reason = p.probe_reason;
	r.priority = p.priority;
	r.selected = selected;
	r.manual = p.manual;
	for(const String& s : p.targets)
		r.targets.Add(s);
	for(const String& s : p.uses_add)
		r.uses_add.Add(s);
	for(const PkgUppFlag& f : p.upp_add)
		r.upp_add.Add(f);
	return r;
}

static const PkgProvider* sSelectProviderCandidate(const Vector<PkgProvider>& candidates, const PkgRepository& repo, const PkgTargetProfile *tp, const PkgProviderPreference *extra_pref, const String& pref, const String& capability)
{
	const PkgProvider* best = nullptr;
	int best_score = INT_MAX;
	for(const PkgProvider& p : candidates) {
		int score = sProviderSelectionScore(p, repo, tp, extra_pref, pref, capability);
		if(!best || score < best_score || (score == best_score && (p.priority > best->priority || (p.priority == best->priority && p.id < best->id)))) {
			best = &p;
			best_score = score;
		}
	}
	return best;
}

static Vector<PkgVirtualCapability> sProviderCapabilities()
{
	Vector<PkgVirtualCapability> v;
	auto add = [&](const String& capability, const String& description) {
		PkgVirtualCapability& c = v.Add();
		c.capability = capability;
		c.description = description;
	};
	add("virtual/sqlite", "SQLite backend");
	add("virtual/ssl", "SSL backend");
	add("virtual/sdl2", "SDL2 backend");
	add("virtual/opengl", "OpenGL backend");
	add("virtual/gui-runtime", "GUI runtime");
	return pick(v);
}

static void sBuildProviderPlan(PkgProviderPlan& plan, const PkgRepository& repo, const PkgLocalPolicy& policy, const PkgTargetProfile *tp, const String& target, const Vector<String>& virtuals, const String& provider_pref, const PkgInvocation& inv, const Vector<PkgProviderPreference> *extra_prefs)
{
	plan.capabilities.Clear();
	plan.resolutions.Clear();
	plan.uses_additions.Clear();
	plan.upp_additions.Clear();
	plan.warnings.Clear();
	for(const PkgVirtualCapability& c : sProviderCapabilities()) {
		PkgVirtualCapability& q = plan.capabilities.Add();
		q.capability = c.capability;
		q.description = c.description;
		for(const String& s : c.provider_ids)
			q.provider_ids.Add(s);
	}
	for(const String& capability : virtuals) {
		PkgVirtualCapability* cap = nullptr;
		for(PkgVirtualCapability& c : plan.capabilities)
			if(c.capability == capability) {
				cap = &c;
				break;
			}
		if(!cap) {
			PkgVirtualCapability& c = plan.capabilities.Add();
			c.capability = capability;
			c.description = sCapabilityTitle(capability);
			cap = &c;
		}
		Vector<PkgProvider> candidates = sProvidersFor(capability, repo, inv, inv.probe, target);
		for(const PkgProvider& p : candidates)
			cap->provider_ids.Add(p.id);
		String capability_pref = sPolicyCapabilityProvider(policy, capability);
		String effective_pref = !capability_pref.IsEmpty() ? capability_pref : provider_pref;
		const PkgProviderPreference* pref_match = extra_prefs ? sFindProviderPreference(*extra_prefs, capability) : nullptr;
		const PkgProvider* selected = sSelectProviderCandidate(candidates, repo, tp, pref_match, effective_pref, capability);
		if(selected) {
			PkgProviderResolution r = sMakeResolution(*selected, repo, capability, true);
			PkgProviderResolution& q = plan.resolutions.Add();
			q.capability = r.capability;
			q.provider_id = r.provider_id;
			q.provider_kind = r.provider_kind;
			q.provider = r.provider;
			q.external_package = r.external_package;
			q.details = r.details;
			q.probe_status = r.probe_status;
			q.probe_command = r.probe_command;
			q.probe_path = r.probe_path;
			q.probe_version = r.probe_version;
			q.probe_reason = r.probe_reason;
			q.priority = r.priority;
			q.selected = r.selected;
			q.manual = r.manual;
			for(const String& s : r.targets)
				q.targets.Add(s);
			for(const String& s : r.uses_add)
				q.uses_add.Add(s);
			for(const PkgUppFlag& f : r.upp_add)
				q.upp_add.Add(f);
			if(r.selected) {
				for(const String& s : r.uses_add)
					if(FindIndex(plan.uses_additions, s) < 0)
						plan.uses_additions.Add(s);
				for(const PkgUppFlag& f : r.upp_add)
					plan.upp_additions.Add(f);
			}
		}
		else {
			PkgProviderResolution r;
			r.capability = capability;
			r.provider_id = "manual";
			r.provider_kind = "manual";
			r.provider = capability;
			r.external_package = Null;
			r.details = "no provider selected";
			r.probe_status = "missing";
			r.probe_reason = "no provider available";
			r.manual = true;
			PkgProviderResolution& q = plan.resolutions.Add();
			q.capability = r.capability;
			q.provider_id = r.provider_id;
			q.provider_kind = r.provider_kind;
			q.provider = r.provider;
			q.external_package = r.external_package;
			q.details = r.details;
			q.probe_status = r.probe_status;
			q.probe_command = r.probe_command;
			q.probe_path = r.probe_path;
			q.probe_version = r.probe_version;
			q.probe_reason = r.probe_reason;
			q.priority = r.priority;
			q.selected = r.selected;
			q.manual = r.manual;
			if(FindIndex(plan.warnings, capability + ": no provider available") < 0)
				plan.warnings.Add(capability + ": no provider available");
		}
	}
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

static String sPolicyCapabilityProvider(const PkgLocalPolicy& policy, const String& capability)
{
	String out;
	for(const PkgPolicyLine& line : policy.package_provider)
		if(sPolicyIsCapabilityName(line.atom) && ToLower(line.atom) == ToLower(capability) && !line.values.IsEmpty())
			out = line.values.Top();
	return out;
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
	Vector<String> expr;
	for(const String& s : use.selected)
		sAddUnique(expr, s);
	for(const String& s : use.defaults)
		if(!sIsSelected(use.disabled, s))
			sAddUnique(expr, s);
	for(const String& s : use.forced)
		sAddUnique(expr, s);
	for(const String& s : use.disabled)
		expr.Add(String("-") + s);
	if(expr.IsEmpty())
		return sFormatUseList(use.effective);
	return sJoin(expr, " ");
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

static bool sHasProjectionFlag(const PkgUppProjection& proj, int scope, const String& name)
{
	for(const PkgUppFlag& f : proj.flags)
		if(f.scope == scope && f.name == name)
			return true;
	return false;
}

static void sAddProjectionUnique(PkgUppProjection& proj, int scope, const String& name, const String& reason)
{
	if(sHasProjectionFlag(proj, scope, name))
		return;
	sAddProjection(proj, scope, name, reason);
}

static void sApplyUppAdditions(PkgUppProjection& proj, const Vector<PkgUppFlag>& add)
{
	for(const PkgUppFlag& f : add)
		sAddProjectionUnique(proj, f.scope, f.name, f.reason);
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
		"FREEBSD", "MACOS", "ANDROID", "MINGW", "MSVC",
		"ST", "MT", "USEMALLOC", "SHARED", "STATIC", "NDEBUG",
		"DEBUG_RT", "RELEASE_RT"
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

static String sAnsi(const char *code, const String& text, bool color);
static String sFmtList(const Vector<String>& v);

struct PkgAuditHit : Moveable<PkgAuditHit> {
	String flag;
	String path;
	int line = 0;
	String snippet;
};

struct PkgAuditPackage : Moveable<PkgAuditPackage> {
	const PkgPackage *pkg = nullptr;
	Vector<String> used;
	Vector<String> accepted;
	Vector<PkgAuditHit> hits;
	Vector<String> missing;
};

static bool sAuditScanPathAllowed(const String& path)
{
	String p = ToLower(UnixPath(NormalizePath(path)));
	if(p.Find("/.ai-upp/") >= 0 || p.StartsWith(".ai-upp/"))
		return false;
	if(p.Find("/build/") >= 0 || p.Find("/out/") >= 0 || p.Find("/bin/") >= 0 || p.Find("/obj/") >= 0)
		return false;
	if(p.Find("/tmp/") >= 0 || p.Find("/cache/") >= 0)
		return false;
	if(p.Find("/generated/") >= 0 || p.Find("/autogen/") >= 0 || p.Find("/gen/") >= 0)
		return false;
	return true;
}

static bool sAuditWordBefore(const String& text, int pos)
{
	return pos <= 0 || !(IsAlNum(text[pos - 1]) || text[pos - 1] == '_');
}

static String sAuditSnippet(const String& line)
{
	return TrimBoth(line);
}

static void sAuditScanFile(const String& path, Vector<PkgAuditHit>& hits)
{
	if(!sAuditScanPathAllowed(path))
		return;
	String txt = LoadFile(path);
	if(txt.IsEmpty())
		return;
	if(txt.Find((char)0) >= 0)
		return;

	int line = 1;
	bool line_comment = false;
	bool block_comment = false;
	char quote = 0;
	for(int i = 0; i < txt.GetCount(); ) {
		char c = txt[i];
		char n = i + 1 < txt.GetCount() ? txt[i + 1] : 0;

		if(c == '\n') {
			++line;
			line_comment = false;
			++i;
			continue;
		}

		if(line_comment) {
			++i;
			continue;
		}
		if(block_comment) {
			if(c == '*' && n == '/') {
				block_comment = false;
				i += 2;
			}
			else
				++i;
			continue;
		}
		if(quote) {
			if(c == '\\' && n) {
				i += 2;
				continue;
			}
			if(c == quote)
				quote = 0;
			++i;
			continue;
		}

		if(c == '/' && n == '/') {
			line_comment = true;
			i += 2;
			continue;
		}
		if(c == '/' && n == '*') {
			block_comment = true;
			i += 2;
			continue;
		}
		if(c == '"' || c == '\'') {
			quote = c;
			++i;
			continue;
		}

		if(c == 'f' && i + 4 < txt.GetCount() && txt[i + 1] == 'l' && txt[i + 2] == 'a' && txt[i + 3] == 'g' &&
		   sAuditWordBefore(txt, i)) {
			int j = i + 4;
			if(j < txt.GetCount() && ((txt[j] >= 'A' && txt[j] <= 'Z') || txt[j] == '_')) {
				int k = j + 1;
				bool ok = true;
				while(k < txt.GetCount() && (IsAlNum(txt[k]) || txt[k] == '_')) {
					if(!((txt[k] >= 'A' && txt[k] <= 'Z') || (txt[k] >= '0' && txt[k] <= '9') || txt[k] == '_'))
						ok = false;
					++k;
				}
				if(ok) {
					String flag = txt.Mid(i + 4, k - (i + 4));
					PkgAuditHit& hit = hits.Add();
					hit.flag = flag;
					hit.path = path;
					hit.line = line;
					int eol = txt.Find('\n', i);
					if(eol < 0)
						eol = txt.GetCount();
					hit.snippet = sAuditSnippet(txt.Mid(i, eol - i));
					i = k;
					continue;
				}
			}
		}

		++i;
	}
}

static Vector<String> sAuditRelevantFlags(const Vector<PkgAuditHit>& hits, Vector<PkgAuditHit>& relevant_hits)
{
	Index<String> seen;
	Vector<String> used;
	for(const PkgAuditHit& hit : hits) {
		if(IsGlobalAuditFlag(hit.flag))
			continue;
		if(seen.Find(hit.flag) < 0) {
			seen.Add(hit.flag);
			used.Add(hit.flag);
		}
		relevant_hits.Add(hit);
	}
	return pick(used);
}

static void sPrintAuditLine(const PkgAuditPackage& a, const PkgPackage& p, bool color)
{
	String prefix;
	if(!a.missing.IsEmpty())
		prefix = sAnsi("33;1", "[warn]", color);
	else if(a.used.IsEmpty())
		prefix = sAnsi("90", "[skip]", color);
	else
		prefix = sAnsi("32;1", "[ok]", color);

	Cout() << prefix << ' ' << p.name;
	if(!a.used.IsEmpty())
		Cout() << ' ' << sAnsi("90", "uses", color) << ' ' << sFmtList(a.used);
	if(!a.accepted.IsEmpty())
		Cout() << ' ' << sAnsi("90", "accepts", color) << ' ' << sFmtList(a.accepted);
	if(a.used.IsEmpty())
		Cout() << ' ' << sAnsi("90", "only global/system flags", color);
	Cout() << "\n";
}

static void sPrintAuditHitDetail(const PkgRepository& repo, const PkgAuditHit& hit, bool color)
{
	Cout() << "         " << sAnsi("90", "source:", color) << ' ' << sRepoRelativePath(repo, hit.path) << ':' << hit.line;
	if(!hit.snippet.IsEmpty())
		Cout() << "  " << sAnsi("90", hit.snippet, color);
	Cout() << "\n";
}

static void sAuditPrintPackage(const PkgRepository& repo, const PkgPackage& p, bool color, bool patch, int& warnings, int& ok, int& skipped)
{
	Vector<PkgAuditHit> hits;
	for(const String& src : p.source_files)
		sAuditScanFile(src, hits);

	Vector<PkgAuditHit> relevant_hits;
	Vector<String> used = sAuditRelevantFlags(hits, relevant_hits);
	Sort(used, [](const String& a, const String& b) { return a < b; });

	Vector<String> accepted;
	for(const String& s : p.accepts)
		accepted.Add(s);
	Sort(accepted, [](const String& a, const String& b) { return a < b; });

	PkgAuditPackage audit;
	audit.pkg = &p;
	for(const String& s : used)
		audit.used.Add(s);
	for(const String& s : accepted)
		audit.accepted.Add(s);
	for(const PkgAuditHit& hit : relevant_hits)
		audit.hits.Add(hit);

	if(audit.used.IsEmpty()) {
		++skipped;
		sPrintAuditLine(audit, p, color);
		return;
	}

	Index<String> accepted_set;
	for(const String& s : accepted)
		accepted_set.Add(s);
	for(const String& s : used)
		if(accepted_set.Find(s) < 0)
			audit.missing.Add(s);

	if(audit.missing.IsEmpty())
		++ok;
	else
		++warnings;

	sPrintAuditLine(audit, p, color);
	Vector<String> missing;
	for(const String& s : audit.missing)
		missing.Add(s);
	if(!missing.IsEmpty()) {
		for(const String& flag : missing) {
			PkgAuditHit best;
			bool found = false;
			for(const PkgAuditHit& hit : audit.hits) {
				if(hit.flag == flag) {
					best = hit;
					found = true;
					break;
				}
			}
			if(found)
				sPrintAuditHitDetail(repo, best, color);
			Cout() << "         " << sAnsi("90", "hint:", color) << " add `acceptflags " << flag << ";`\n";
		}
		if(patch) {
			Cout() << '\n';
			sPrintAcceptFlagsPatch(repo, p, audit.missing);
		}
	}
}

struct PkgLintFinding : Moveable<PkgLintFinding> {
	String atom;
	String name;
	String path;
	String severity;
	String code;
	String message;
	String fingerprint;
	String baseline_state;
	Vector<String> details;
	Vector<String> candidates;

	void Jsonize(JsonIO& jio)
	{
		jio("atom", atom)("name", name)("path", path)("severity", severity)
		   ("code", code)("message", message)("fingerprint", fingerprint)
		   ("baseline_state", baseline_state)("details", details)("candidates", candidates);
	}
};

struct PkgLintBaseline : Moveable<PkgLintBaseline> {
	int schema = 1;
	String root;
	Time timestamp;
	Vector<PkgLintFinding> entries;

	void Jsonize(JsonIO& jio)
	{
		jio("schema", schema)("root", root)("timestamp", timestamp)("entries", entries);
	}
};

struct PkgLintSummary {
	int errors = 0;
	int warnings = 0;
	int info = 0;
	int ok = 0;
	int new_errors = 0;
	int new_warnings = 0;
	int baseline_known = 0;
	int baseline_new = 0;
	int baseline_resolved = 0;
	int packages = 0;
	bool quiet = false;
	bool ci = false;
	bool show_baseline = true;
	bool fail_on_baseline = false;
	bool update_baseline = false;
	bool baseline_loaded = false;
	bool baseline_corrupt = false;
	bool baseline_present = false;
	String baseline_path;
	Vector<PkgLintFinding> findings;
	Vector<PkgLintFinding> baseline_resolved_findings;
	VectorMap<String, int> code_counts;
	Vector<PkgLintFinding> baseline_entries;
	VectorMap<String, int> baseline_index;
};

struct PkgLintReportCategory : Moveable<PkgLintReportCategory> {
	String code;
	int count = 0;

	void Jsonize(JsonIO& jio)
	{
		jio("code", code)("count", count);
	}
};

struct PkgLintReport {
	String command_line;
	Time timestamp;
	String target;
	String provider;
	String compiler;
	String linker;
	String profile;
	String repository;
	String atom;
	String report_path;
	String format;
	String baseline_path;
	bool ci = false;
	bool quiet = false;
	bool summary = false;
	bool strict = false;
	bool use_cache = true;
	bool rebuild_cache = false;
	bool update_baseline = false;
	bool baseline_loaded = false;
	bool baseline_corrupt = false;
	bool show_baseline = true;
	bool fail_on_baseline = false;
	int package_count = 0;
	int scanned_count = 0;
	int skipped_count = 0;
	int errors = 0;
	int warnings = 0;
	int info = 0;
	int ok = 0;
	int new_errors = 0;
	int new_warnings = 0;
	int baseline_known = 0;
	int baseline_new = 0;
	int baseline_resolved = 0;
	int baseline_entries = 0;
	int cache_loaded_entries = 0;
	int cache_reused_entries = 0;
	int cache_reparsed_entries = 0;
	int cache_stale_entries = 0;
	bool cache_used = false;
	Vector<PkgLintFinding> findings;
	Vector<PkgLintFinding> resolved;
	Vector<PkgLintReportCategory> categories;

	void Jsonize(JsonIO& jio)
	{
		jio("command_line", command_line)("timestamp", timestamp)("target", target)
		   ("provider", provider)("compiler", compiler)("linker", linker)
		   ("profile", profile)("repository", repository)("atom", atom)
		   ("report_path", report_path)("format", format)("baseline_path", baseline_path)
		   ("ci", ci)("quiet", quiet)("summary", summary)("strict", strict)
		   ("use_cache", use_cache)("rebuild_cache", rebuild_cache)
		   ("update_baseline", update_baseline)("baseline_loaded", baseline_loaded)
		   ("baseline_corrupt", baseline_corrupt)("show_baseline", show_baseline)
		   ("fail_on_baseline", fail_on_baseline)
		   ("package_count", package_count)("scanned_count", scanned_count)
		   ("skipped_count", skipped_count)("errors", errors)("warnings", warnings)
		   ("info", info)("ok", ok)("new_errors", new_errors)("new_warnings", new_warnings)
		   ("baseline_known", baseline_known)("baseline_new", baseline_new)
		   ("baseline_resolved", baseline_resolved)("baseline_entries", baseline_entries)
		   ("cache_loaded_entries", cache_loaded_entries)
		   ("cache_reused_entries", cache_reused_entries)
		   ("cache_reparsed_entries", cache_reparsed_entries)
		   ("cache_stale_entries", cache_stale_entries)
		   ("cache_used", cache_used)("findings", findings)("resolved", resolved)
		   ("categories", categories);
	}
};

struct PkgLintMetrics {
	double analysis_seconds = 0.0;
	int source_scan_count = 0;
	int duplicate_packages = 0;
	int root_count = 0;
	int root_skipped = 0;
};

static String sLintBaselineDefaultPath(const PkgConfigPaths& paths)
{
	return AppendFileName(paths.root, "metadata/pkg_lint_baseline.json");
}

static String sLintNormalizeFingerprintText(const String& s)
{
	String out;
	out.Reserve(s.GetCount());
	bool digit = false;
	for(int i = 0; i < s.GetCount(); i++) {
		int c = (unsigned char)s[i];
		if(IsDigit(c)) {
			if(!digit) {
				out.Cat('#');
				digit = true;
			}
			continue;
		}
		digit = false;
		out.Cat(c);
	}
	return out;
}

static String sLintFingerprint(const PkgLintFinding& finding)
{
	String out;
	out << sLintNormalizeFingerprintText(finding.code) << '\n'
	    << sLintNormalizeFingerprintText(finding.atom) << '\n'
	    << sLintNormalizeFingerprintText(finding.name) << '\n'
	    << sLintNormalizeFingerprintText(finding.path) << '\n'
	    << sLintNormalizeFingerprintText(finding.message) << '\n';
	for(const String& d : finding.details)
		out << sLintNormalizeFingerprintText(d) << '\n';
	for(const String& c : finding.candidates)
		out << sLintNormalizeFingerprintText(c) << '\n';
	return out;
}

static void sCopyLintFinding(PkgLintFinding& dst, const PkgLintFinding& src)
{
	dst.atom = src.atom;
	dst.name = src.name;
	dst.path = src.path;
	dst.severity = src.severity;
	dst.code = src.code;
	dst.message = src.message;
	dst.fingerprint = src.fingerprint;
	dst.baseline_state = src.baseline_state;
	dst.details.Clear();
	for(const String& d : src.details)
		dst.details.Add(d);
	dst.candidates.Clear();
	for(const String& c : src.candidates)
		dst.candidates.Add(c);
}

static bool sLoadLintBaseline(const String& path, PkgLintBaseline& baseline, bool& corrupt)
{
	corrupt = false;
	baseline.schema = 1;
	baseline.root.Clear();
	baseline.timestamp = Time();
	baseline.entries.Clear();
	if(path.IsEmpty() || !FileExists(path))
		return false;
	if(!LoadFromJsonFile(baseline, path)) {
		corrupt = true;
		return false;
	}
	if(baseline.schema <= 0)
		baseline.schema = 1;
	return true;
}

static bool sStoreLintBaseline(const String& path, const PkgLintBaseline& baseline)
{
	if(path.IsEmpty())
		return false;
	RealizeDirectory(GetFileFolder(path));
	return StoreAsJsonFile(baseline, path, true);
}

static void sLintLine(PkgLintSummary& sum, const String& level, const String& finding_code, const String& message, bool color, const String& atom = Null, const String& name = Null, const String& path = Null)
{
	String tag = String("[") + level + "]";
	String ansi = "36";
	if(level == String("ok")) {
		sum.ok++;
		ansi = "32;1";
	}
	else if(level == String("warn")) {
		sum.warnings++;
		ansi = "33;1";
	}
	else if(level == String("info")) {
		sum.info++;
		ansi = "36";
	}
	else if(level == String("error")) {
		sum.errors++;
		ansi = "31;1";
	}
	PkgLintFinding& finding = sum.findings.Add();
	finding.atom = atom;
	finding.name = name.IsEmpty() ? atom : name;
	finding.path = path;
	finding.severity = level;
	finding.code = finding_code;
	finding.message = message;
	finding.fingerprint = sLintFingerprint(finding);
	if(sum.baseline_loaded && finding.severity != "ok") {
		int bi = sum.baseline_index.Find(finding.fingerprint);
		if(bi >= 0) {
			sum.baseline_known++;
			finding.baseline_state = "known";
			sum.baseline_entries[bi].baseline_state = "matched";
		}
		else {
			sum.baseline_new++;
			finding.baseline_state = "new";
		}
	}
	if(finding.severity == "error" || finding.severity == "warn") {
		bool fail = !sum.baseline_loaded || finding.baseline_state != "known" || sum.fail_on_baseline;
		if(fail) {
			if(finding.severity == "error")
				sum.new_errors++;
			else
				sum.new_warnings++;
		}
	}
	if(level != "ok")
	{
		int qi = sum.code_counts.Find(finding_code);
		if(qi < 0)
			sum.code_counts.Add(finding_code, 1);
		else
			sum.code_counts[qi]++;
	}
	if(!sum.quiet)
		Cout() << "  " << sAnsi(ansi, sum.baseline_loaded && finding.severity != "ok" && sum.show_baseline
			? String("[") + (finding.baseline_state.IsEmpty() ? String("new") : finding.baseline_state) + "]"
			: tag, color) << "  " << message << "\n";
}

static void sLintSubline(PkgLintSummary& sum, const String& label, const String& value)
{
	if(sum.findings.GetCount())
		sum.findings.Top().details.Add(label + value);
	if(!sum.quiet)
		Cout() << "          " << label << value << "\n";
}

static void sLintPrintTiming(const PkgRepository& repo, const PkgLintMetrics& metrics, double command_seconds)
{
	Cout() << "\nTiming:\n";
	Cout() << "  metadata discovery time: " << FormatDouble(repo.discover_seconds, 3) << " s\n";
	Cout() << "  lint analysis time: " << FormatDouble(metrics.analysis_seconds, 3) << " s\n";
	Cout() << "  package metadata load count: " << repo.loaded_packages << "\n";
	Cout() << "  .upp parse count: " << repo.discover_paths << "\n";
	Cout() << "  dependency graph resolve count: " << repo.resolve_calls << "\n";
	Cout() << "  resolve cache hits: " << repo.resolve_cache_hits << "\n";
	Cout() << "  resolve index hits: " << repo.resolve_index_hits << "\n";
	Cout() << "  source scan count: " << metrics.source_scan_count << "\n";
	Cout() << "  total packages visited: " << metrics.root_count << "\n";
	Cout() << "  duplicate package loads: " << metrics.duplicate_packages << "\n";
	Cout() << "  total elapsed: " << FormatDouble(command_seconds, 3) << " s\n";
}

static Vector<PkgLintReportCategory> sLintSortedCategories(const PkgLintSummary& sum)
{
	Vector<PkgLintReportCategory> out;
	for(int i = 0; i < sum.code_counts.GetCount(); i++) {
		PkgLintReportCategory& c = out.Add();
		c.code = sum.code_counts.GetKey(i);
		c.count = sum.code_counts[i];
	}
	Sort(out, [](const PkgLintReportCategory& a, const PkgLintReportCategory& b) {
		if(a.count != b.count)
			return a.count > b.count;
		return a.code < b.code;
	});
	return out;
}

static void sLintPrintTopCategories(const PkgLintSummary& sum, bool color)
{
	Vector<PkgLintReportCategory> cats = sLintSortedCategories(sum);
	Cout() << "Top issue categories:\n";
	if(cats.IsEmpty()) {
		Cout() << "  none\n";
		return;
	}
	int limit = min(10, cats.GetCount());
	for(int i = 0; i < limit; i++) {
		const PkgLintReportCategory& c = cats[i];
		String tag = String("  [") + AsString(c.count) + "]";
		Cout() << "  " << sAnsi("36", tag, color) << "  " << c.code << "\n";
	}
}

static void sLintFinalizeBaseline(PkgLintSummary& sum)
{
	if(!sum.baseline_loaded)
		return;
	sum.baseline_resolved_findings.Clear();
	for(int i = 0; i < sum.baseline_entries.GetCount(); i++) {
		const PkgLintFinding& b = sum.baseline_entries[i];
		if(b.baseline_state == "matched")
			continue;
		PkgLintFinding& resolved = sum.baseline_resolved_findings.Add();
		sCopyLintFinding(resolved, b);
		resolved.severity = "resolved";
		resolved.code = "PKG-LINT-BASELINE-RESOLVED";
		resolved.message = "baseline finding resolved";
		resolved.baseline_state = "resolved";
		sum.baseline_resolved++;
	}
}

static int sLintExitCode(const PkgInvocation& inv, const PkgLintSummary& sum, bool report_ok)
{
	bool check = inv.check || inv.ci;
	if(inv.update_baseline)
		return report_ok ? 0 : 1;
	if(!sum.baseline_path.IsEmpty()) {
		if(!check)
			return report_ok ? 0 : 1;
		return sum.new_errors > 0 || (inv.strict && sum.new_warnings > 0) || (inv.fail_on_baseline && sum.baseline_known > 0) || !report_ok ? 1 : 0;
	}
	return sum.errors > 0 || (inv.strict && sum.warnings > 0) || !report_ok ? 1 : 0;
}

static String sLintReportFormat(const String& path)
{
	String ext = ToLower(GetFileExt(path));
	if(ext == ".md" || ext == ".markdown")
		return "markdown";
	return "json";
}

static bool sWriteLintReportFile(const PkgLintReport& report)
{
	if(report.report_path.IsEmpty())
		return true;
	RealizeDirectory(GetFileFolder(report.report_path));
	String format = sLintReportFormat(report.report_path);
	if(format == "markdown") {
		String out;
		out << "# pkg lint report\n\n";
		out << "- command_line: `" << report.command_line << "`\n";
		out << "- timestamp: " << AsString(report.timestamp) << "\n";
		out << "- report_path: `" << report.report_path << "`\n";
		out << "- format: markdown\n";
		out << "- target: " << (report.target.IsEmpty() ? String("[none]") : report.target) << "\n";
		out << "- provider: " << (report.provider.IsEmpty() ? String("[none]") : report.provider) << "\n";
		out << "- compiler: " << (report.compiler.IsEmpty() ? String("[none]") : report.compiler) << "\n";
		out << "- linker: " << (report.linker.IsEmpty() ? String("[none]") : report.linker) << "\n";
		out << "- baseline_path: " << (report.baseline_path.IsEmpty() ? String("[none]") : report.baseline_path) << "\n";
		out << "- baseline_loaded: " << (report.baseline_loaded ? "yes" : "no") << "\n";
		out << "- baseline_corrupt: " << (report.baseline_corrupt ? "yes" : "no") << "\n";
		out << "- package_count: " << report.package_count << "\n";
		out << "- scanned_count: " << report.scanned_count << "\n";
		out << "- skipped_count: " << report.skipped_count << "\n";
		out << "- errors: " << report.errors << "\n";
		out << "- warnings: " << report.warnings << "\n";
		out << "- info: " << report.info << "\n";
		out << "- ok: " << report.ok << "\n";
		out << "- new_errors: " << report.new_errors << "\n";
		out << "- new_warnings: " << report.new_warnings << "\n";
		out << "- baseline_entries: " << report.baseline_entries << "\n";
		out << "- baseline_known: " << report.baseline_known << "\n";
		out << "- baseline_new: " << report.baseline_new << "\n";
		out << "- baseline_resolved: " << report.baseline_resolved << "\n";
		out << "- cache_used: " << (report.cache_used ? "yes" : "no") << "\n";
		out << "- cache_loaded_entries: " << report.cache_loaded_entries << "\n";
		out << "- cache_reused_entries: " << report.cache_reused_entries << "\n";
		out << "- cache_reparsed_entries: " << report.cache_reparsed_entries << "\n";
		out << "- cache_stale_entries: " << report.cache_stale_entries << "\n\n";
		out << "## Top issue categories\n";
		if(report.categories.IsEmpty())
			out << "- none\n";
		else
			for(const PkgLintReportCategory& c : report.categories)
				out << "- `" << c.code << "`: " << c.count << "\n";
		out << "\n## Findings\n";
		if(report.findings.IsEmpty())
			out << "_none_\n";
		else {
			for(const PkgLintFinding& f : report.findings) {
				out << "\n### " << f.severity << " " << f.code << "\n";
				out << "- package: " << (f.atom.IsEmpty() ? String("[none]") : f.atom) << "\n";
				out << "- path: " << (f.path.IsEmpty() ? String("[none]") : f.path) << "\n";
				out << "- message: " << f.message << "\n";
				if(!f.fingerprint.IsEmpty())
					out << "- fingerprint: `" << f.fingerprint << "`\n";
				if(!f.baseline_state.IsEmpty())
					out << "- baseline: " << f.baseline_state << "\n";
				if(!f.details.IsEmpty()) {
					out << "- details:\n";
					for(const String& d : f.details)
						out << "  - " << d << "\n";
				}
				if(!f.candidates.IsEmpty()) {
					out << "- candidates: " << sFmtList(f.candidates) << "\n";
				}
			}
		}
		out << "\n## Resolved baseline entries\n";
		if(report.resolved.IsEmpty())
			out << "_none_\n";
		else {
			for(const PkgLintFinding& f : report.resolved) {
				out << "\n### resolved " << f.code << "\n";
				out << "- package: " << (f.atom.IsEmpty() ? String("[none]") : f.atom) << "\n";
				out << "- path: " << (f.path.IsEmpty() ? String("[none]") : f.path) << "\n";
				out << "- message: " << f.message << "\n";
				if(!f.fingerprint.IsEmpty())
					out << "- fingerprint: `" << f.fingerprint << "`\n";
			}
		}
		return SaveFile(report.report_path, out);
	}

	return StoreAsJsonFile(report, report.report_path, true);
}

static bool sWriteLintReport(const PkgInvocation& inv, const PkgRepository& repo, const PkgLintSummary& sum, const PkgLintMetrics& metrics)
{
	if(inv.report.IsEmpty())
		return true;

	PkgLintReport report;
	report.command_line = inv.command_line;
	report.timestamp = GetSysTime();
	report.target = inv.target;
	report.provider = inv.provider;
	report.compiler = inv.compiler;
	report.linker = inv.linker;
	report.profile = inv.profile;
	report.repository = inv.repository;
	report.atom = inv.atom;
	report.report_path = inv.report;
	report.format = sLintReportFormat(inv.report);
	report.baseline_path = sum.baseline_path;
	report.ci = inv.ci;
	report.quiet = inv.quiet;
	report.summary = inv.summary;
	report.strict = inv.strict;
	report.use_cache = inv.use_cache;
	report.rebuild_cache = inv.rebuild_cache;
	report.update_baseline = inv.update_baseline;
	report.baseline_loaded = sum.baseline_loaded;
	report.baseline_corrupt = sum.baseline_corrupt;
	report.show_baseline = inv.show_baseline;
	report.fail_on_baseline = inv.fail_on_baseline;
	report.package_count = repo.packages.GetCount();
	report.scanned_count = sum.packages;
	report.skipped_count = metrics.root_skipped + metrics.duplicate_packages;
	report.errors = sum.errors;
	report.warnings = sum.warnings;
	report.info = sum.info;
	report.ok = sum.ok;
	report.new_errors = sum.new_errors;
	report.new_warnings = sum.new_warnings;
	report.baseline_known = sum.baseline_known;
	report.baseline_new = sum.baseline_new;
	report.baseline_resolved = sum.baseline_resolved;
	report.baseline_entries = sum.baseline_entries.GetCount();
	report.cache_loaded_entries = repo.cache_loaded_entries;
	report.cache_reused_entries = repo.cache_reused_entries;
	report.cache_reparsed_entries = repo.cache_reparsed_entries;
	report.cache_stale_entries = repo.cache_stale_entries;
	report.cache_used = repo.cache_used;
	for(const PkgLintFinding& f : sum.findings) {
		PkgLintFinding& dst = report.findings.Add();
		sCopyLintFinding(dst, f);
	}
	for(const PkgLintFinding& f : sum.baseline_resolved_findings) {
		PkgLintFinding& dst = report.resolved.Add();
		sCopyLintFinding(dst, f);
	}
	report.categories = sLintSortedCategories(sum);
	return sWriteLintReportFile(report);
}

static bool sStoreLintBaselineFile(const String& path, const PkgLintSummary& sum, const PkgRepository& repo)
{
	if(path.IsEmpty())
		return false;

	PkgLintBaseline baseline;
	baseline.schema = 1;
	baseline.root = repo.root;
	baseline.timestamp = GetSysTime();
	Vector<PkgLintFinding> entries;
	Index<String> seen;
	for(const PkgLintFinding& f : sum.findings) {
		if(f.severity == "ok")
			continue;
		String key = f.fingerprint.IsEmpty() ? sLintFingerprint(f) : f.fingerprint;
		if(FindIndex(seen, key) >= 0)
			continue;
		seen.Add(key);
		PkgLintFinding& dst = entries.Add();
		sCopyLintFinding(dst, f);
		if(dst.fingerprint.IsEmpty())
			dst.fingerprint = key;
	}
	Sort(entries, [](const PkgLintFinding& a, const PkgLintFinding& b) {
		if(a.code != b.code) return a.code < b.code;
		if(a.atom != b.atom) return a.atom < b.atom;
		if(a.path != b.path) return a.path < b.path;
		if(a.message != b.message) return a.message < b.message;
		return a.fingerprint < b.fingerprint;
	});
	for(const PkgLintFinding& f : entries) {
		PkgLintFinding& dst = baseline.entries.Add();
		sCopyLintFinding(dst, f);
		dst.baseline_state.Clear();
	}
	return sStoreLintBaseline(path, baseline);
}

static void sLintPackageMetadata(const PkgRepository& repo, const PkgPackage& p, PkgLintSummary& sum, bool color, const PkgPlan *plan, Index<String>& seen_paths, PkgLintMetrics& metrics, bool summary)
{
	String pkg_atom = p.atom.IsEmpty() ? p.name : p.atom;
	String key = ToLower(UnixPath(NormalizePath(p.path)));
	if(FindIndex(seen_paths, key) >= 0)
	{
		metrics.duplicate_packages++;
		return;
	}
	seen_paths.Add(key);
	sum.packages++;

	if(!sum.quiet)
		Cout() << p.name << " <" << p.path << ">\n";

	if(p.name.IsEmpty() || p.path.IsEmpty()) {
		sLintLine(sum, "error", "PKG-LINT-MISSING-METADATA", "empty package name or path", color, pkg_atom, p.name, p.path);
	}
	if(p.description.IsEmpty())
		sLintLine(sum, "info", "PKG-LINT-MISSING-DESCRIPTION", "description missing", color, pkg_atom, p.name, p.path);

	Index<String> source_seen;
	for(const String& src : p.source_files)
		if(source_seen.Find(src) < 0)
			source_seen.Add(src);
		else {
			sLintLine(sum, "warn", "PKG-LINT-DUPLICATE-SOURCE", "duplicate source file entry: " + src, color, pkg_atom, p.name, p.path);
		}

	Index<String> uses_seen;
	for(const String& use : p.uses)
		if(uses_seen.Find(use) < 0)
			uses_seen.Add(use);
		else {
			sLintLine(sum, "warn", "PKG-LINT-DUPLICATE-USES", "duplicate uses entry: " + use, color, pkg_atom, p.name, p.path);
		}

	if(p.path.Find("ide/Core") >= 0 || p.path.Find("uppsrc/pkg") >= 0 || p.name == "pkg") {
		static const char * const gui_deps[] = {
			"CtrlLib", "CodeEditor", "MainCtrl", "CtrlCore", "Designer", "Browse", "Shell"
		};
		Vector<String> leakage;
		for(const String& use : p.uses)
			for(const char *dep : gui_deps)
				if(use == dep) {
					if(FindIndex(leakage, use) < 0)
						leakage.Add(use);
				}
		if(!leakage.IsEmpty()) {
			sLintLine(sum, "warn", "PKG-LINT-HEADLESS-GUI-DEP", "headless package depends on GUI-facing packages", color, pkg_atom, p.name, p.path);
			sLintSubline(sum, "deps: ", sFmtList(leakage));
		}
	}

	Index<String> accepts_seen;
	for(const String& a : p.accepts)
		if(accepts_seen.Find(a) < 0)
			accepts_seen.Add(a);
		else {
			sLintLine(sum, "warn", "PKG-LINT-DUPLICATE-ACCEPTFLAG", "duplicate acceptflags entry: " + a, color, pkg_atom, p.name, p.path);
		}

	Vector<String> dot_accepts;
	for(const String& a : p.accepts)
		if(a.StartsWith("."))
			dot_accepts.Add(a);
	if(!dot_accepts.IsEmpty()) {
		sLintLine(sum, "warn", "PKG-LINT-DOTTED-ACCEPTFLAG", "acceptflags should use bare names, not dot-prefixed forms", color, pkg_atom, p.name, p.path);
		sLintSubline(sum, "flags: ", sFmtList(dot_accepts));
	}

	Vector<PkgAuditHit> hits;
	for(const String& src : p.source_files)
		sAuditScanFile(src, hits), metrics.source_scan_count++;
	Vector<PkgAuditHit> relevant_hits;
	Vector<String> used = sAuditRelevantFlags(hits, relevant_hits);
	Sort(used, [](const String& a, const String& b) { return a < b; });
	Vector<String> accepted;
	for(const String& s : p.accepts)
		if(FindIndex(accepted, s) < 0)
			accepted.Add(s);
	Sort(accepted, [](const String& a, const String& b) { return a < b; });

	Vector<String> missing;
	Index<String> accepted_set;
	for(const String& s : accepted)
		accepted_set.Add(s);
	for(const String& s : used)
		if(accepted_set.Find(s) < 0)
			missing.Add(s);

	Vector<String> extra;
	Index<String> used_set;
	for(const String& s : used)
		used_set.Add(s);
	for(const String& s : accepted)
		if(used_set.Find(s) < 0)
			extra.Add(s);

	if(used.IsEmpty()) {
		sLintLine(sum, "ok", "PKG-LINT-GLOBAL-FLAGS", "only global/system flags", color, pkg_atom, p.name, p.path);
	}
	else if(missing.IsEmpty()) {
		sLintLine(sum, "ok", "PKG-LINT-ACCEPTFLAGS-OK", "acceptflags cover scoped source flags", color, pkg_atom, p.name, p.path);
	}
	else {
		sLintLine(sum, "warn", "PKG-LINT-MISSING-ACCEPTFLAG", "source uses flags missing from acceptflags", color, pkg_atom, p.name, p.path);
		if(!summary) {
			for(const String& flag : missing) {
				PkgAuditHit best;
				bool found = false;
				for(const PkgAuditHit& hit : relevant_hits) {
					if(hit.flag == flag) {
						best = hit;
						found = true;
						break;
					}
				}
					if(found)
					sLintSubline(sum, "source: ", sRepoRelativePath(repo, best.path) + ":" + AsString(best.line));
				sLintSubline(sum, "hint: ", String("add `acceptflags ") + flag + ";`");
			}
		}
	}

	if(!extra.IsEmpty()) {
			sLintLine(sum, "info", "PKG-LINT-UNUSED-ACCEPTFLAG", "acceptflags not seen in source", color, pkg_atom, p.name, p.path);
		if(!summary)
			sLintSubline(sum, "flags: ", sFmtList(extra));
	}

	if(!p.mainconfig.IsEmpty()) {
		sLintLine(sum, "info", "PKG-LINT-MAINCONFIG", "mainconfig entries present", color, pkg_atom, p.name, p.path);
		if(!summary)
			sLintSubline(sum, "mainconfig: ", sFmtList(p.mainconfig));
	}

	if(p.manifest.present) {
		if(!p.manifest.errors.IsEmpty()) {
			sLintLine(sum, "error", "PKG-LINT-UPPKG-INVALID", "package-local .uppkg manifest has errors", color, pkg_atom, p.name, p.path);
			if(!summary) {
				for(const String& err : p.manifest.errors)
					sLintSubline(sum, "error: ", err);
				for(const String& warn : p.manifest.warnings)
					sLintSubline(sum, "warning: ", warn);
			}
		}
		else if(!p.manifest.warnings.IsEmpty()) {
			sLintLine(sum, "warn", "PKG-LINT-UPPKG-WARNINGS", "package-local .uppkg manifest has warnings", color, pkg_atom, p.name, p.path);
			if(!summary)
				for(const String& warn : p.manifest.warnings)
					sLintSubline(sum, "warning: ", warn);
		}
		else {
			sLintLine(sum, "ok", "PKG-LINT-UPPKG-OK", "package-local .uppkg manifest loaded", color, pkg_atom, p.name, p.path);
		}
	}

	if(plan && !plan->target_masked.IsEmpty()) {
		Vector<String> masked;
		Index<String> masked_set;
		for(const String& s : plan->target_masked)
			masked_set.Add(s);
		for(const String& s : used)
			if(masked_set.Find(s) >= 0)
				masked.Add(s);
		if(!masked.IsEmpty()) {
			sLintLine(sum, "warn", "PKG-LINT-TARGET-MASKED-USE", "source uses flags masked by the target", color, pkg_atom, p.name, p.path);
			if(!summary)
				sLintSubline(sum, "flags: ", sFmtList(masked));
		}
	}

	if(plan) {
		for(const PkgProviderResolution& r : plan->provider_plan.resolutions) {
			if(!r.selected)
				continue;
			if(r.external_package.IsEmpty()) {
				sLintLine(sum, "warn", "PKG-LINT-UNKNOWN-PROVIDER", "provider requirement unresolved: " + r.capability, color, pkg_atom, p.name, p.path);
				if(!summary)
					sLintSubline(sum, "reason: ", r.probe_reason.IsEmpty() ? String("no external package selected") : r.probe_reason);
				continue;
			}
			if(!repo.Resolve(r.external_package).pkg) {
				sLintLine(sum, "error", "PKG-LINT-PROVIDER-MISSING", "provider package missing: " + r.external_package, color, pkg_atom, p.name, p.path);
				if(!summary)
					sLintSubline(sum, "capability: ", r.capability);
			}
		}
	}
}

static void sLintPrintGraphIssues(const PkgPlan& plan, PkgLintSummary& sum, bool color, bool summary)
{
	Index<String> seen;
	for(const PkgResolveIssue& issue : plan.graph.issues) {
		String key = issue.kind + "|" + issue.from + "|" + issue.atom + "|" + issue.reason;
		if(seen.Find(key) >= 0)
			continue;
		seen.Add(key);

		String level = "warn";
		if(issue.kind == "missing" || issue.kind == "missing-set" ||
		   issue.kind == "ambiguous" || issue.kind == "cycle")
			level = "error";

		String code;
		String message;
		if(issue.kind == "missing") {
			message = "missing dependency: " + issue.atom;
			code = "PKG-LINT-MISSING-DEPENDENCY";
		}
		else if(issue.kind == "missing-set") {
			message = "missing set member: " + issue.atom;
			code = "PKG-LINT-MISSING-SET-MEMBER";
		}
		else if(issue.kind == "missing-provider") {
			message = "unresolved provider requirement: " + issue.atom;
			code = "PKG-LINT-UNKNOWN-PROVIDER";
		}
		else if(issue.kind == "ambiguous") {
			message = "ambiguous dependency: " + issue.atom;
			code = "PKG-LINT-AMBIGUOUS-DEPENDENCY";
		}
		else if(issue.kind == "cycle") {
			message = "dependency cycle: " + issue.reason;
			code = "PKG-LINT-DEPENDENCY-CYCLE";
		}
		else {
			message = issue.kind + ": " + issue.atom;
			code = "PKG-LINT-DEPENDENCY";
		}

		sLintLine(sum, level, code, message, color, issue.atom, issue.atom, issue.from);
		if(!summary && !issue.from.IsEmpty())
			sLintSubline(sum, "from: ", issue.from);
		if(!summary && !issue.reason.IsEmpty())
			sLintSubline(sum, "reason: ", issue.reason);
		if(!summary && !issue.candidates.IsEmpty())
			sLintSubline(sum, "candidates: ", sFmtList(issue.candidates));
	}
}

static Vector<String> sLintRootQueries(const PkgInvocation& inv, const PkgRepository& repo)
{
	Vector<String> roots;
	auto add_root = [&](const String& s) {
		if(!s.IsEmpty() && FindIndex(roots, s) < 0)
			roots.Add(s);
	};

	if(inv.all || inv.atom.IsEmpty()) {
		for(const PkgPackage& p : repo.packages)
			add_root(p.path);
		return pick(roots);
	}

	if(inv.atom == "@world" || inv.atom == "world") {
		Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
		for(const String& s : world)
			add_root(s);
		return pick(roots);
	}
	if(inv.atom == "@system" || inv.atom == "system") {
		Vector<String> system = sLoadSet(repo.paths, "system", repo.paths.system_set);
		for(const String& s : system)
			add_root(s);
		return pick(roots);
	}
	if(inv.atom == "@toolchain" || inv.atom == "toolchain") {
		Vector<String> toolchain = sLoadSet(repo.paths, "toolchain", repo.paths.toolchain_set);
		for(const String& s : toolchain)
			add_root(s);
		return pick(roots);
	}

	add_root(inv.atom);
	return pick(roots);
}

static int sLintCommand(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color, double command_seconds)
{
	TimeStop phase;
	PkgLintSummary sum;
	PkgLintMetrics metrics;
	bool check = inv.check || inv.ci;
	Index<String> seen_paths;
	Vector<String> roots = sLintRootQueries(inv, repo);
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	sum.quiet = inv.summary || check || inv.quiet;
	sum.ci = check;
	sum.show_baseline = inv.show_baseline;
	sum.fail_on_baseline = inv.fail_on_baseline;
	sum.update_baseline = inv.update_baseline;
	sum.baseline_path = inv.baseline;
	if(sum.baseline_path.IsEmpty() && inv.update_baseline)
		sum.baseline_path = sLintBaselineDefaultPath(repo.paths);

	if(!sum.baseline_path.IsEmpty()) {
		PkgLintBaseline baseline;
		bool corrupt = false;
		sum.baseline_present = FileExists(sum.baseline_path);
		if(sLoadLintBaseline(sum.baseline_path, baseline, corrupt)) {
			sum.baseline_loaded = true;
			sum.baseline_entries.Clear();
			for(const PkgLintFinding& src : baseline.entries) {
				PkgLintFinding& entry = sum.baseline_entries.Add();
				sCopyLintFinding(entry, src);
				if(entry.fingerprint.IsEmpty())
					entry.fingerprint = sLintFingerprint(entry);
				if(entry.baseline_state.IsEmpty())
					entry.baseline_state = "baseline";
				if(entry.fingerprint.IsEmpty())
					continue;
				if(sum.baseline_index.Find(entry.fingerprint) < 0)
					sum.baseline_index.Add(entry.fingerprint, sum.baseline_entries.GetCount() - 1);
			}
		}
		else if(corrupt) {
			Cerr() << "pkg: lint baseline is corrupt: " << sum.baseline_path << "\n";
			sFlushConsole();
			return 1;
		}
	}

	for(const String& warn : policy.warnings)
		sLintLine(sum, "warn", "PKG-LINT-CONFIG-WARNING", warn, color, Null, Null, Null);
	for(const String& err : policy.errors)
		sLintLine(sum, "error", "PKG-LINT-CONFIG-ERROR", err, color, Null, Null, Null);

	if(!sum.quiet) {
		Cout() << "Linting package metadata";
		if(inv.all)
			Cout() << " for all packages";
		if(check)
			Cout() << " [check]";
		Cout() << "...\n";
		Cout() << "Indexing packages... done, " << repo.packages.GetCount() << " packages\n\n";
		if(!sum.baseline_path.IsEmpty()) {
			Cout() << "Baseline: " << sum.baseline_path;
			if(sum.update_baseline)
				Cout() << " [update]";
			else if(sum.baseline_loaded)
				Cout() << " [loaded]";
			else if(sum.baseline_present)
				Cout() << " [missing or unreadable]";
			Cout() << "\n\n";
		}
	}
	if(inv.ci)
		Cerr() << "pkg: lint --ci is experimental; use --check for local batch lint policy.\n";
	if(roots.IsEmpty()) {
		sLintLine(sum, "info", "PKG-LINT-NO-PACKAGES", "no packages selected", color);
		sLintFinalizeBaseline(sum);
		Cout() << "\nSummary:\n";
		Cout() << "  0 packages, 0 error, 0 warn, 0 info, 0 ok\n";
		if(!sum.baseline_path.IsEmpty()) {
			Cout() << "  baseline entries: " << sum.baseline_entries.GetCount() << "\n";
			Cout() << "  baseline known: " << sum.baseline_known << "\n";
			Cout() << "  baseline new: " << sum.baseline_new << "\n";
			Cout() << "  baseline resolved: " << sum.baseline_resolved << "\n";
		}
		if(sum.quiet)
			sLintPrintTopCategories(sum, color);
		if(inv.debug_timing) {
			metrics.root_count = sum.packages;
			metrics.analysis_seconds = phase.Seconds();
			sLintPrintTiming(repo, metrics, command_seconds);
		}
		if(inv.update_baseline && !sStoreLintBaselineFile(sum.baseline_path, sum, repo)) {
			Cerr() << "failed to write lint baseline: " << sum.baseline_path << "\n";
			return 1;
		}
		bool report_ok = sWriteLintReport(inv, repo, sum, metrics);
		if(!report_ok) {
			Cerr() << "failed to write lint report: " << inv.report << "\n";
		}
		return sLintExitCode(inv, sum, report_ok);
	}

	if(inv.limit > 0 && roots.GetCount() > inv.limit) {
		metrics.root_skipped += roots.GetCount() - inv.limit;
		roots.SetCount(inv.limit);
	}
	metrics.root_count = roots.GetCount();

	for(int i = 0; i < roots.GetCount(); i++) {
		const String& root = roots[i];
		PkgLookupResult root_lookup = repo.Resolve(root);
		if(root_lookup.pkg) {
			String root_key = ToLower(UnixPath(NormalizePath(root_lookup.pkg->path)));
			if(FindIndex(seen_paths, root_key) >= 0)
			{
				metrics.root_skipped++;
				continue;
			}
		}

		PkgInvocation rootinv;
		rootinv.command = PKG_CMD_PLAN;
		rootinv.atom = root;
		rootinv.target = inv.target;
		rootinv.provider = inv.provider;
		rootinv.compiler = inv.compiler;
		rootinv.linker = inv.linker;
		rootinv.profile = inv.profile;
		rootinv.repository = inv.repository;
		rootinv.vcpkg_root = inv.vcpkg_root;
		rootinv.vcpkg_triplet = inv.vcpkg_triplet;
		rootinv.emscripten_profile = inv.emscripten_profile;
		rootinv.jobs = inv.jobs;
		rootinv.verbose = inv.verbose;
		rootinv.update = inv.update;
		rootinv.deep = inv.deep;
		rootinv.newuse = inv.newuse;
		rootinv.changed_use = inv.changed_use;
		rootinv.keep_going = inv.keep_going;
		rootinv.skip_first = inv.skip_first;
		rootinv.probe = inv.probe;
		rootinv.pretend = true;
		rootinv.ask = false;
		rootinv.resume = inv.resume;
		rootinv.oneshot = inv.oneshot;
		rootinv.plan = true;
		rootinv.strict = inv.strict;
		for(const String& s : inv.use_args)
			rootinv.use_args.Add(s);
		for(const String& s : inv.extra)
			rootinv.extra.Add(s);
		rootinv.staged = inv.staged;
		PkgPlan plan = sBuildPlan(rootinv, repo, state, policy, color);

		String title = root;
		if(!plan.atom.IsEmpty())
			title = plan.atom;
		if(!sum.quiet && (inv.summary || inv.all))
			Cout() << "[" << (i + 1) << "/" << roots.GetCount() << "] " << title << "\n";
		else if(!sum.quiet)
			Cout() << title << "\n";
		if(plan.graph.nodes.IsEmpty())
			sLintLine(sum, "error", "PKG-LINT-MISSING-METADATA", "package metadata not found", color, root_lookup.query, root_lookup.query, root_lookup.path);

		if(!(inv.nodeps && root_lookup.pkg))
			sLintPrintGraphIssues(plan, sum, color, inv.summary);

		if(inv.nodeps) {
			if(root_lookup.pkg)
				sLintPackageMetadata(repo, *root_lookup.pkg, sum, color, &plan, seen_paths, metrics, inv.summary);
		}
		else {
			Index<String> local_seen;
			for(const PkgGraphNode& node : plan.graph.nodes) {
				if(!node.resolved || node.path.IsEmpty())
					continue;
				if(FindIndex(local_seen, node.path) >= 0)
					continue;
				local_seen.Add(node.path);
				const PkgPackage* pkg = repo.Resolve(node.path).pkg;
				if(!pkg)
					continue;
				sLintPackageMetadata(repo, *pkg, sum, color, &plan, seen_paths, metrics, inv.summary);
			}
		}
		if(!sum.quiet)
			Cout() << "\n";
	}
	sLintFinalizeBaseline(sum);

	Cout() << "Summary:\n";
	Cout() << "  packages scanned: " << sum.packages << "\n";
	Cout() << "  errors: " << sum.errors << "\n";
	Cout() << "  warnings: " << sum.warnings << "\n";
	Cout() << "  info: " << sum.info << "\n";
	Cout() << "  ok: " << sum.ok << "\n";
	if(!sum.baseline_path.IsEmpty()) {
		Cout() << "  baseline entries: " << sum.baseline_entries.GetCount() << "\n";
		Cout() << "  baseline known: " << sum.baseline_known << "\n";
		Cout() << "  baseline new: " << sum.baseline_new << "\n";
		Cout() << "  baseline resolved: " << sum.baseline_resolved << "\n";
	}
	if(sum.quiet)
		sLintPrintTopCategories(sum, color);
	if(inv.debug_timing) {
		metrics.root_count = sum.packages;
		metrics.analysis_seconds = phase.Seconds();
		sLintPrintTiming(repo, metrics, command_seconds);
	}
	Cout() << "\n";

	if(inv.update_baseline && !sStoreLintBaselineFile(sum.baseline_path, sum, repo)) {
		Cerr() << "failed to write lint baseline: " << sum.baseline_path << "\n";
		return 1;
	}

	bool report_ok = sWriteLintReport(inv, repo, sum, metrics);
	if(!report_ok) {
		Cerr() << "failed to write lint report: " << inv.report << "\n";
	}
	return sLintExitCode(inv, sum, report_ok);
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

static Vector<String> sReadSetFile(const String& path)
{
	Vector<String> v;
	String txt = LoadFile(path);
	Vector<String> lines = Split(txt, '\n');
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.IsEmpty() || line.StartsWith("#"))
			continue;
		v.Add(line);
	}
	return pick(v);
}

static Vector<String> sBuiltInSet(const String& name)
{
	Vector<String> v;
	if(name == "system") {
		v.Add("build");
		v.Add("umk");
		v.Add("pkg");
	}
	else if(name == "toolchain") {
		v.Add("Core");
		v.Add("pkg");
	}
	else if(name == "world") {
		v.Add("pkg");
	}
	return pick(v);
}

static Vector<String> sLoadSet(const PkgConfigPaths& paths, const String& name, const String& path)
{
	Vector<String> v = sReadSetFile(path);
	if(v.IsEmpty())
		v = sBuiltInSet(name);
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
	Vector<String> world = sLoadSet(paths, "world", paths.world);
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
			else if(a == "--doctor") inv.doctor = true, inv.command = PKG_CMD_DOCTOR;
			else if(a == "--metadata") inv.metadata = true, inv.command = PKG_CMD_METADATA;
			else if(a == "--metadata-cache") inv.metadata_cache = true, inv.command = PKG_CMD_METADATA_CACHE;
			else if(a == "--list-sets") inv.list_sets = true, inv.command = PKG_CMD_LIST_SETS;
			else if(a == "--audit-acceptflags") inv.command = PKG_CMD_AUDIT_ACCEPTFLAGS;
			else if(a == "--lint") inv.command = PKG_CMD_LINT;
			else if(a == "--selftest") inv.command = PKG_CMD_SELFTEST;
			else if(a == "--targets") inv.targets = true, inv.command = PKG_CMD_TARGETS;
			else if(a == "--providers") inv.providers = true, inv.command = PKG_CMD_PROVIDERS;
			else if(a == "--bins") inv.bins = true, inv.command = PKG_CMD_BINS;
			else if(a == "--depclean") inv.depclean = true, inv.command = PKG_CMD_DEPCLEAN;
			else if(a == "--all") inv.all = true;
			else if(a == "--brief") inv.brief = true;
			else if(a == "--pretend") inv.pretend = true;
			else if(a == "--ask") inv.ask = true;
			else if(a == "--verbose") inv.verbose = true;
			else if(a == "--update") inv.update = true;
			else if(a == "--deep") inv.deep = true;
			else if(a == "--newuse") inv.newuse = true;
			else if(a == "--changed-use") inv.changed_use = true;
			else if(a == "--strict") inv.strict = true;
			else if(a == "--nodeps") inv.nodeps = true;
			else if(a == "--summary") inv.summary = true;
			else if(a == "--quiet") inv.quiet = true;
			else if(a == "--check") inv.check = true;
			else if(a == "--ci") inv.ci = true;
			else if(a == "--baseline" || a.StartsWith("--baseline=")) {
				inv.baseline = a == "--baseline" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--update-baseline" || a.StartsWith("--update-baseline=")) {
				inv.update_baseline = true;
				inv.baseline = a == "--update-baseline" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--show-baseline") inv.show_baseline = true;
			else if(a == "--hide-baseline") inv.show_baseline = false;
			else if(a == "--fail-on-baseline") inv.fail_on_baseline = true;
			else if(a == "--report" || a.StartsWith("--report=")) {
				inv.report = a == "--report" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--use-cache") inv.use_cache = true;
			else if(a == "--no-cache") inv.use_cache = false;
			else if(a == "--rebuild") inv.rebuild_cache = true;
			else if(a == "--keep-going") inv.keep_going = true;
			else if(a == "--skipfirst") inv.skip_first = true;
			else if(a == "--resume") inv.resume = true;
			else if(a == "--oneshot") inv.oneshot = true;
			else if(a == "--plan") inv.plan = true;
			else if(a == "--install") inv.install = true;
			else if(a == "--probe") inv.probe = true;
			else if(a == "--staged") inv.staged = true;
			else if(a == "--patch") inv.audit_patch = true;
			else if(a == "--debug-timing") inv.debug_timing = true;
			else if(a == "--limit" || a.StartsWith("--limit=")) {
				String v = a == "--limit" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				inv.limit = ScanInt(v);
			}
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
				inv.target_explicit = true;
			}
			else if(a == "--profile" || a.StartsWith("--profile=")) {
				inv.profile = a == "--profile" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				inv.target_explicit = true;
			}
			else if(a == "--root" || a.StartsWith("--root=")) {
				inv.root = a == "--root" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--sysroot" || a.StartsWith("--sysroot=")) {
				inv.sysroot = a == "--sysroot" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
			}
			else if(a == "--provider" || a.StartsWith("--provider=")) {
				inv.provider = a == "--provider" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				inv.provider_explicit = true;
			}
			else if(a == "--jobs" || a.StartsWith("--jobs=")) {
				String v = a == "--jobs" ? (i + 1 < args.GetCount() ? args[++i] : String()) : sOptValue(a);
				inv.jobs = ScanInt(v);
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
				case 'U': inv.changed_use = true; break;
				case 'p': inv.pretend = true; inv.plan = true; break;
				case 's': inv.command = PKG_CMD_SEARCH; break;
				case 'j': {
					String v = a.Mid(j + 1);
					inv.jobs = ScanInt(v);
					j = a.GetCount();
					break;
				}
				case 'l': {
					String v = a.Mid(j + 1);
					inv.limit = ScanInt(v);
					j = a.GetCount();
					break;
				}
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
			else if(cmd == "doctor") inv.command = PKG_CMD_DOCTOR;
			else if(cmd == "metadata") inv.command = PKG_CMD_METADATA;
			else if(cmd == "metadata-cache") inv.command = PKG_CMD_METADATA_CACHE;
			else if(cmd == "list-sets") inv.command = PKG_CMD_LIST_SETS;
			else if(cmd == "targets") inv.command = PKG_CMD_TARGETS;
			else if(cmd == "providers") inv.command = PKG_CMD_PROVIDERS;
			else if(cmd == "bins") inv.command = PKG_CMD_BINS;
			else if(cmd == "clean") inv.command = PKG_CMD_CLEAN;
			else if(cmd == "depclean") inv.command = PKG_CMD_DEPCLEAN;
			else if(cmd == "explain-use") inv.command = PKG_CMD_EXPLAIN_USE;
			else if(cmd == "explain-target") inv.command = PKG_CMD_EXPLAIN_TARGET;
			else if(cmd == "deps") inv.command = PKG_CMD_DEPS;
			else if(cmd == "lint") inv.command = PKG_CMD_LINT;
			else if(cmd == "selftest") inv.command = PKG_CMD_SELFTEST;
			else if(cmd == "target") inv.command = PKG_CMD_TARGET;
			else if(cmd == "eselect") inv.command = PKG_CMD_ESELECT;
			else if(cmd == "audit-acceptflags") inv.command = PKG_CMD_AUDIT_ACCEPTFLAGS;
			else if(cmd == "resume") inv.command = PKG_CMD_RESUME;
			else if(cmd == "search") inv.command = PKG_CMD_SEARCH;
			else inv.command = PKG_CMD_PLAN;
		}
		else if(inv.list_sets)
			inv.command = PKG_CMD_LIST_SETS;
		else if(inv.targets)
			inv.command = PKG_CMD_TARGETS;
		else if(inv.metadata)
			inv.command = PKG_CMD_METADATA;
		else if(inv.info)
			inv.command = PKG_CMD_INFO;
		else if(inv.doctor)
			inv.command = PKG_CMD_DOCTOR;
		else if(inv.search)
			inv.command = PKG_CMD_SEARCH;
		else if(inv.providers)
			inv.command = PKG_CMD_PROVIDERS;
		else if(inv.resume)
			inv.command = PKG_CMD_RESUME;
		else if(inv.all || inv.strict || inv.nodeps || inv.summary || inv.quiet || inv.check || inv.ci || inv.debug_timing || inv.limit > 0 || !inv.report.IsEmpty() || !inv.baseline.IsEmpty() || inv.update_baseline || !inv.use_cache || inv.rebuild_cache)
			inv.command = PKG_CMD_LINT;
		else if(inv.ask || inv.verbose || inv.update || inv.deep || inv.newuse || inv.changed_use || inv.pretend || inv.install || inv.depclean)
			inv.command = PKG_CMD_PLAN;
		else
			inv.command = PKG_CMD_HELP;
	}

	if(positional.GetCount()) {
		int start = 0;
		if(positional[0] == "help" || positional[0] == "version" || positional[0] == "info" || positional[0] == "doctor" ||
		   positional[0] == "metadata" || positional[0] == "list-sets" || positional[0] == "targets" || positional[0] == "providers" || positional[0] == "bins" || positional[0] == "clean" || positional[0] == "depclean" || positional[0] == "explain-use" || positional[0] == "explain-target" || positional[0] == "deps" || positional[0] == "lint" || positional[0] == "selftest" ||
		   positional[0] == "target" || positional[0] == "eselect" || positional[0] == "audit-acceptflags" ||
		   positional[0] == "resume" || positional[0] == "search" || positional[0] == "metadata-cache")
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
				inv.value = rest[2];
			for(int i = 3; i < rest.GetCount(); i++)
				inv.extra.Add(rest[i]);
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
		else if(inv.command == PKG_CMD_DOCTOR) {
			inv.subcommand = rest.GetCount() ? rest[0] : "all";
			for(int i = 1; i < rest.GetCount(); i++)
				inv.extra.Add(rest[i]);
		}
		else if(inv.command == PKG_CMD_SELFTEST) {
			inv.subcommand = rest.GetCount() ? rest[0] : "quick";
			for(int i = 1; i < rest.GetCount(); i++)
				inv.extra.Add(rest[i]);
		}
		else if(inv.command == PKG_CMD_LINT) {
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
		else if(inv.command == PKG_CMD_BINS || inv.command == PKG_CMD_CLEAN || inv.command == PKG_CMD_DEPCLEAN) {
			if(rest.GetCount())
				inv.atom = rest[0];
			for(int i = 1; i < rest.GetCount(); i++)
				inv.extra.Add(rest[i]);
		}
		else if(inv.command == PKG_CMD_PROVIDERS) {
			if(rest.GetCount())
				inv.provider_query = rest[0];
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

static String sFmtProviderPreference(const PkgProviderPreference& p)
{
	String s;
	if(!p.capability.IsEmpty())
		s << p.capability << " -> ";
	s << (p.provider_id.IsEmpty() ? String("[any]") : p.provider_id);
	if(!p.reason.IsEmpty())
		s << " (" << p.reason << ")";
	return s;
}

static String sFmtProviderPreferences(const Vector<PkgProviderPreference>& v)
{
	Vector<String> out;
	for(const PkgProviderPreference& p : v)
		out.Add(sFmtProviderPreference(p));
	return sFmtList(out);
}

static String sFmtUppFlagList(const Vector<PkgUppFlag>& v)
{
	Vector<String> out;
	for(const PkgUppFlag& f : v)
		out.Add(f.scope == PKG_UPP_ACCEPTED ? "." + f.name : f.scope == PKG_UPP_MAIN_ONLY ? "!" + f.name : f.name);
	return sFmtList(out);
}

static void sFlushConsole()
{
	Cout().Flush();
	Cerr().Flush();
}

static String sResolveEselectTarget(const PkgInvocation& inv, const PkgEselectState& st);
static void sPrintTargetInfo(const String& name);

static void sPrintEselectModules(const PkgInvocation& inv)
{
	Cout() << "Usage: pkg eselect <global options> <module name> <module options>\n\n";
	Cout() << "Global options:\n";
	Cout() << "  --brief\n";
	Cout() << "  --colour=<yes|no|auto>\n";
	Cout() << "  --root=<path>\n";
	Cout() << "  --sysroot=<path>\n";
	Cout() << "  --profile=<name>\n\n";
	Cout() << "Built-in modules:\n";
	for(const char *m : sEselectBuiltins)
		Cout() << "  " << m << "\n";
	Cout() << "\nExtra modules:\n";
	for(const char *m : sEselectModules)
		Cout() << "  " << m << "\n";
	Cout() << "\nRepository root: " << sEselectRoot(inv) << "\n";
	Cout() << "Sysroot: " << (inv.sysroot.IsEmpty() ? String("[none]") : inv.sysroot) << "\n";
}

static void sPrintEselectValueList(const PkgInvocation& inv, const PkgRepository& repo, const String& module)
{
	PkgEselectState st = sLoadEselectState(inv);
	if(module == "compiler") {
		Cout() << "Available compiler selectors:\n";
		if(inv.brief)
			Cout() << "  auto clang gcc msvc emcc\n";
		else
			Cout() << "  auto\n  clang\n  gcc\n  msvc\n  emcc\n";
		return;
	}
	if(module == "linker") {
		Cout() << "Available linker selectors:\n";
		if(inv.brief)
			Cout() << "  auto ld lld link emcc\n";
		else
			Cout() << "  auto\n  ld\n  lld\n  link\n  emcc\n";
		return;
	}
	if(module == "target") {
		Vector<PkgTargetProfile> targets = sTargets();
		String active_target = sResolveEselectTarget(inv, st);
		for(const PkgTargetProfile& t : targets) {
			String mark = t.name == active_target ? String(" *") : String();
			if(inv.brief)
				Cout() << t.name << mark << "\n";
			else
				Cout() << t.name << mark << " [" << t.thread_model << "] - " << t.summary << "\n";
		}
		return;
	}
	if(module == "provider") {
		Cout() << "Provider policies:\n";
		Cout() << "  auto\n  portable\n  system\n  vcpkg\n";
		Cout() << "\nProvider catalog:\n";
		Cout() << "  virtual/sqlite\n  virtual/ssl\n  virtual/sdl2\n  virtual/opengl\n  virtual/gui-runtime\n";
		Cout() << "\nBuilt-in providers:\n";
		Cout() << "  upp-plugin-sqlite3\n  system-sqlite\n  vcpkg-sqlite3\n  upp-plugin-ssl\n  system-ssl\n  system-sdl2\n  vcpkg-sdl2\n  msys2-sdl2\n  emscripten-sdl2\n  system-opengl\n  webgl\n  virtualgui-sdl2gl\n";
		return;
	}
	if(module == "profile") {
		Cout() << "Available profiles:\n";
		for(const PkgTargetProfile& t : sTargets())
			Cout() << "  " << t.name << "\n";
		return;
	}
	if(module == "vcpkg") {
		Cout() << "Vcpkg configuration:\n";
		Cout() << "  root: " << (st.vcpkg_root.IsEmpty() ? String("[none]") : st.vcpkg_root) << "\n";
		Cout() << "  triplet: " << (st.vcpkg_triplet.IsEmpty() ? String("[none]") : st.vcpkg_triplet) << "\n";
		Cout() << "\nKnown triplets:\n";
		Cout() << "  auto\n  x64-windows\n  x64-windows-static\n  x64-linux\n  x64-osx\n";
		return;
	}
	if(module == "emscripten") {
		Cout() << "Emscripten configuration:\n";
		Cout() << "  profile: " << (st.emscripten_profile.IsEmpty() ? String("[none]") : st.emscripten_profile) << "\n";
		Cout() << "\nKnown profiles:\n";
		Cout() << "  auto\n  sdk-3.1\n  sdk-3.1.61\n  upstream\n";
		return;
	}
	if(module == "repository") {
		Cout() << "Repository preference: " << (st.repository.IsEmpty() ? String("[none]") : st.repository) << "\n";
		Cout() << "\nKnown repositories:\n";
		if(repo.nests.IsEmpty()) {
			Cout() << "  uppsrc\n  reference\n  stdsrc\n";
		}
		else {
			for(const String& nest : repo.nests)
				Cout() << "  " << nest << "\n";
		}
		return;
	}
	if(module == "news") {
		Cout() << "No news items.\n";
		return;
	}
	Cout() << "Module " << module << " is recognized but has no selector list yet.\n";
}

static void sPrintEselectValueShow(const PkgInvocation& inv, const PkgRepository& repo, const String& module)
{
	(void)repo;
	PkgEselectState st = sLoadEselectState(inv);
	const String *slot = sEselectSlot(st, module);
	if(module == "target") {
		String target = sResolveEselectTarget(inv, st);
		Cout() << "target: " << target << "\n";
		sPrintTargetInfo(target);
		return;
	}
	if(module == "compiler") {
		Cout() << "compiler: " << (st.compiler.IsEmpty() ? String("[none]") : st.compiler) << "\n";
		Cout() << "selected target: " << sResolveEselectTarget(inv, st) << "\n";
		return;
	}
	if(module == "linker") {
		Cout() << "linker: " << (st.linker.IsEmpty() ? String("[none]") : st.linker) << "\n";
		return;
	}
	if(module == "provider") {
		Cout() << "provider: " << (st.provider.IsEmpty() ? String("[none]") : st.provider) << "\n";
		return;
	}
	if(module == "profile") {
		Cout() << "profile: " << (st.profile.IsEmpty() ? String("[none]") : st.profile) << "\n";
		return;
	}
	if(module == "repository") {
		Cout() << "repository: " << (st.repository.IsEmpty() ? String("[none]") : st.repository) << "\n";
		return;
	}
	if(module == "vcpkg") {
		Cout() << "vcpkg root: " << (st.vcpkg_root.IsEmpty() ? String("[none]") : st.vcpkg_root) << "\n";
		Cout() << "vcpkg triplet: " << (st.vcpkg_triplet.IsEmpty() ? String("[none]") : st.vcpkg_triplet) << "\n";
		return;
	}
	if(module == "emscripten") {
		Cout() << "emscripten profile: " << (st.emscripten_profile.IsEmpty() ? String("[none]") : st.emscripten_profile) << "\n";
		return;
	}
	if(!slot || slot->IsEmpty())
		Cout() << module << ": [none]\n";
	else
		Cout() << module << ": " << *slot << "\n";
}

static void sPrintEselectValueSet(const PkgInvocation& inv, const String& module)
{
	PkgEselectState st = sLoadEselectState(inv);
	String *slot = sEselectSlot(st, module);
	String value = inv.value;
	if(module == "vcpkg") {
		String key = ToLower(TrimBoth(inv.value));
		if(key == "root") {
			st.vcpkg_root = inv.extra.GetCount() ? inv.extra[0] : String();
			sStoreEselectState(inv, st);
			Cout() << "vcpkg root set to " << (st.vcpkg_root.IsEmpty() ? String("[none]") : st.vcpkg_root) << "\n";
			return;
		}
		if(key == "triplet") {
			st.vcpkg_triplet = inv.extra.GetCount() ? inv.extra[0] : String();
			sStoreEselectState(inv, st);
			Cout() << "vcpkg triplet set to " << (st.vcpkg_triplet.IsEmpty() ? String("[none]") : st.vcpkg_triplet) << "\n";
			return;
		}
		if(value.IsEmpty() && inv.extra.GetCount())
			value = inv.extra[0];
		st.vcpkg_triplet = value;
		sStoreEselectState(inv, st);
		Cout() << "vcpkg triplet set to " << value << "\n";
		return;
	}
	if(module == "emscripten") {
		if(value.IsEmpty() && inv.extra.GetCount())
			value = inv.extra[0];
		st.emscripten_profile = value;
		sStoreEselectState(inv, st);
		Cout() << "emscripten profile set to " << value << "\n";
		return;
	}
	if(module == "news") {
		Cout() << "news is read-only\n";
		return;
	}
	if(!slot) {
		Cout() << "module not yet backed by a stored selection: " << module << "\n";
		Cout() << "stored repository-local selections currently cover compiler, linker, target, provider, profile, repository, vcpkg, and emscripten\n";
		return;
	}
	*slot = value;
	sStoreEselectState(inv, st);
	Cout() << module << " set to " << value << "\n";
}

static const String *sEselectSlot(const PkgEselectState& st, const String& module)
{
	if(module == "compiler") return &st.compiler;
	if(module == "linker") return &st.linker;
	if(module == "target") return &st.target;
	if(module == "provider") return &st.provider;
	if(module == "profile") return &st.profile;
	if(module == "repository") return &st.repository;
	return nullptr;
}

static String *sEselectSlot(PkgEselectState& st, const String& module)
{
	if(module == "compiler") return &st.compiler;
	if(module == "linker") return &st.linker;
	if(module == "target") return &st.target;
	if(module == "provider") return &st.provider;
	if(module == "profile") return &st.profile;
	if(module == "repository") return &st.repository;
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
		<< "  -U, --changed-use    rebuild only if effective USE or projection changed\n"
		<< "      --strict         treat warnings as failures for lint\n"
		<< "      --check          local lint check mode; concise and non-interactive\n"
		<< "      --baseline PATH  load a lint baseline from PATH\n"
		<< "      --update-baseline PATH\n"
		<< "                       refresh PATH with the current lint findings\n"
		<< "      --show-baseline  mark known and new findings in terminal output\n"
		<< "      --hide-baseline  suppress known/new markers in terminal output\n"
		<< "      --fail-on-baseline\n"
		<< "                       fail CI even when a finding is already baseline-known\n"
		<< "      --nodeps         lint only the selected package metadata, not its dependencies\n"
		<< "      --summary        reduce lint output to one-line package summaries\n"
		<< "      --quiet          print only the final lint summary and categories\n"
		<< "      --ci             deprecated alias for --check (local lint only)\n"
		<< "      --debug-timing   print lint timing and cache counters\n"
		<< "      --limit N        limit lint --all to the first N packages\n"
		<< "      --report PATH    write a JSON or Markdown lint or selftest report\n"
		<< "      --use-cache      prefer the persistent package metadata cache\n"
		<< "      --no-cache       ignore the persistent package metadata cache\n"
		<< "      --rebuild        rebuild the persistent package metadata cache\n"
		<< "  -p, --pretend        print a plan without changing anything\n"
		<< "      --install        execute the planned build after checks\n"
		<< "      --keep-going     continue after independent build failures\n"
		<< "      --skipfirst      resume from the step after the failed step\n"
		<< "      --jobs N         set build parallelism\n"
		<< "      --color <y|n|auto>\n"
		<< "      --colour <yes|no|auto>\n\n"
		<< "Commands:\n"
		<< "  --help, help\n"
		<< "  --version, version\n"
		<< "  --info, info\n"
		<< "  --doctor, doctor [env|state|shell|cache|config|providers [--probe]]\n"
		<< "  --selftest, selftest [quick|doctor|cache|plan|lint|all] [--report PATH]\n"
		<< "  --metadata-cache, metadata-cache [--rebuild]\n"
		<< "  --metadata, metadata\n"
		<< "  --list-sets\n"
		<< "  --lint, lint [atom|@world|--all] [--strict] [--nodeps] [--summary] [--quiet] [--check] [--ci] [--debug-timing] [--limit N] [--baseline PATH] [--update-baseline PATH] [--report PATH]\n"
		<< "  --audit-acceptflags [atom] [--patch]\n"
		<< "  --targets\n"
		<< "  targets\n"
		<< "  --providers [capability] [--probe]\n"
		<< "  --bins, bins\n"
		<< "  clean <atom> [--all] [--pretend|--ask]\n"
		<< "  --depclean [--pretend|--ask]\n"
		<< "  --provider <portable|system|...>\n"
		<< "  -s, --search <query>\n"
		<< "  deps <atom> [USE flags...] --plan\n"
		<< "  explain-use <atom> [USE flags...]\n"
		<< "  explain-target <name>\n"
		<< "  target list|info <name>|explain <name>|set <name>\n"
		<< "  eselect ...\n"
		<< "  resume [--pretend] [--skipfirst] [--keep-going]\n"
		<< "  audit-acceptflags [atom] [--patch]  global/platform flags are skipped\n"
		<< "  selftest [quick|doctor|cache|plan|lint|all] [--report PATH]\n"
		<< "  -avuDN @world\n\n"
		<< "Recognized sets: @world, @system, @toolchain\n"
		<< "Cleanup: use pkg clean or pkg --depclean for safe artifact removal.\n";
	(void)color;
}

static void sPrintVersion()
{
	Cout() << "pkg 0.1\n";
}

static void sPrintSets(const PkgRepository& repo)
{
	Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
	Vector<String> system = sLoadSet(repo.paths, "system", repo.paths.system_set);
	Vector<String> toolchain = sLoadSet(repo.paths, "toolchain", repo.paths.toolchain_set);
	Cout() << "Built-in sets:\n";
	Cout() << "  @world      [" << world.GetCount() << "] " << sFmtList(world) << "\n";
	Cout() << "  @system     [" << system.GetCount() << "] " << sFmtList(system) << "\n";
	Cout() << "  @toolchain  [" << toolchain.GetCount() << "] " << sFmtList(toolchain) << "\n";
	Cout() << "\nSet sources:\n";
	Cout() << "  world: " << (FileExists(repo.paths.world) ? repo.paths.world : String("[built-in default]")) << "\n";
	Cout() << "  system: " << (FileExists(repo.paths.system_set) ? repo.paths.system_set : String("[built-in default]")) << "\n";
	Cout() << "  toolchain: " << (FileExists(repo.paths.toolchain_set) ? repo.paths.toolchain_set : String("[built-in default]")) << "\n";
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
	use.conflicts.Clear();
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
	if(pkg && pkg->manifest.present && pkg->manifest.ok) {
		for(const String& s : pkg->manifest.use_default)
			if(declared.Find(s) < 0)
				declared.Add(s);
		for(const String& s : pkg->manifest.use_forced)
			if(declared.Find(s) < 0)
				declared.Add(s);
		for(const String& s : pkg->manifest.use_masked)
			if(declared.Find(s) < 0)
				declared.Add(s);
	}
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
	for(int i = 0; i < tp.default_use.GetCount(); i++)
		sAddUnique(use.defaults, tp.default_use[i]);
	if(pkg && pkg->manifest.present && pkg->manifest.ok) {
		for(const String& s : pkg->manifest.use_forced)
			sAddUnique(use.forced, s);
		for(const String& s : pkg->manifest.use_masked)
			sAddUnique(use.masked, s);
		for(const String& s : pkg->manifest.use_default)
			sAddUnique(use.defaults, s);
	}

	sTokenizeUseArgs(use.requested, use.selected, use.disabled);
	for(const String& s : use.forced)
		sAddUnique(use.selected, s);
	for(const String& s : use.masked)
		sAddUnique(use.disabled, s);
	for(const String& s : use.forced)
		if(sIsSelected(use.disabled, s))
			sAddUnique(use.conflicts, s);
	for(const String& s : use.selected)
		if(sIsSelected(use.masked, s))
			sAddUnique(use.conflicts, s);

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

static void sExplainUse(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy)
{
	PkgLookupResult lookup = inv.atom.IsEmpty() ? PkgLookupResult() : repo.Resolve(inv.atom);
	const PkgPackage *pkg = lookup.pkg;
	if(lookup.ambiguous) {
		Cout() << "Ambiguous package: " << inv.atom << "\n";
		for(const PkgPackage* c : lookup.candidates)
			if(c)
				Cout() << "  " << sPackageLookupLabel(*c) << "\n";
		return;
	}
	PkgInvocation eff = sEffectiveInvocation(inv, repo, policy, pkg);
	if(pkg && sPolicyPackageMasked(policy, repo, pkg)) {
		Cout() << "Use policy for " << (inv.atom.IsEmpty() ? String("[unknown atom]") : inv.atom) << "\n";
		Cout() << "Target: " << sTargetNameText(eff.target) << "\n";
		Cout() << "Local policy: package is masked\n";
		return;
	}
	PkgUseModel use;
	sBuildUseModel(use, pkg, eff.use_args, eff.target);
	PkgUppProjection upp;
	sProjectUpp(upp, use);
	const PkgTargetProfile& tp = sDefaultTargetProfile(eff.target);

	Cout() << "Use policy for " << (inv.atom.IsEmpty() ? String("[unknown atom]") : inv.atom) << "\n";
	Cout() << "Target: " << sTargetNameText(eff.target) << "\n";
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

static String sCompilerDefaultTarget(const String& compiler)
{
	String c = ToLower(TrimBoth(compiler));
	if(c == "clang")
		return "native-clang";
	if(c == "gcc")
		return "native-gcc";
	if(c == "msvc")
		return "native-msvc";
	if(c == "emcc")
		return "wasm-browser";
	if(c == "mingw")
		return "windows-mingw";
	return String();
}

static String sResolveEselectTarget(const PkgInvocation& inv, const PkgEselectState& st)
{
	if(!inv.target.IsEmpty())
		return inv.target;
	if(!inv.profile.IsEmpty())
		return inv.profile;
	if(!st.target.IsEmpty())
		return st.target;
	if(!st.profile.IsEmpty())
		return st.profile;
	String from_compiler = sCompilerDefaultTarget(!inv.compiler.IsEmpty() ? inv.compiler : st.compiler);
	if(!from_compiler.IsEmpty())
		return from_compiler;
	return "native";
}

static void sApplyEselectDefaults(PkgInvocation& inv, const PkgEselectState& st)
{
	if(inv.compiler.IsEmpty())
		inv.compiler = st.compiler;
	if(inv.linker.IsEmpty())
		inv.linker = st.linker;
	if(inv.provider.IsEmpty())
		inv.provider = st.provider;
	if(inv.profile.IsEmpty())
		inv.profile = st.profile;
	if(inv.repository.IsEmpty())
		inv.repository = st.repository;
	if(inv.vcpkg_root.IsEmpty())
		inv.vcpkg_root = st.vcpkg_root;
	if(inv.vcpkg_triplet.IsEmpty())
		inv.vcpkg_triplet = st.vcpkg_triplet;
	if(inv.emscripten_profile.IsEmpty())
		inv.emscripten_profile = st.emscripten_profile;
	if(inv.target.IsEmpty())
		inv.target = sResolveEselectTarget(inv, st);
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
	Cout() << "default_use: " << sFmtList(t->default_use) << "\n";
	Cout() << "forced_use: " << sFmtList(t->forced_use) << "\n";
	Cout() << "masked_use: " << sFmtList(t->masked_use) << "\n";
	Cout() << "provider_preferences: " << sFmtProviderPreferences(t->provider_preferences) << "\n";
	Cout() << "upp_add: " << sFmtUppFlagList(t->upp_add) << "\n";
	Cout() << "notes: " << sFmtList(t->notes) << "\n";
	Cout() << "warnings: " << sFmtList(t->warnings) << "\n";
	Cout() << "summary: " << t->summary << "\n";
}

static void sListTargets()
{
	Vector<PkgTargetProfile> targets = sTargets();
	for(const PkgTargetProfile& t : targets) {
		Cout() << t.name << " [" << t.thread_model << "] - " << t.summary << "\n";
		Cout() << "  arch: " << (t.architecture.IsEmpty() ? String("[none]") : t.architecture)
		     << ", platform: " << (t.target_platform.IsEmpty() ? String("[none]") : t.target_platform)
		     << ", runtime: " << (t.runtime_environment.IsEmpty() ? String("[none]") : t.runtime_environment)
		     << ", toolchain: " << (t.toolchain.IsEmpty() ? String("[none]") : t.toolchain) << "\n";
		if(!t.default_use.IsEmpty() || !t.forced_use.IsEmpty() || !t.masked_use.IsEmpty())
			Cout() << "  USE: default=" << sFmtList(t.default_use) << ", forced=" << sFmtList(t.forced_use)
			     << ", masked=" << sFmtList(t.masked_use) << "\n";
		if(!t.provider_preferences.IsEmpty())
			Cout() << "  providers: " << sFmtProviderPreferences(t.provider_preferences) << "\n";
		if(!t.upp_add.IsEmpty())
			Cout() << "  UPP: " << sFmtUppFlagList(t.upp_add) << "\n";
	}
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
	Cout() << "Default USE: " << sFmtList(t->default_use) << "\n";
	Cout() << "Forced USE: " << sFmtList(t->forced_use) << "\n";
	Cout() << "Masked USE: " << sFmtList(t->masked_use) << "\n";
	Cout() << "Provider preferences: " << sFmtProviderPreferences(t->provider_preferences) << "\n";
	Cout() << "UPP additions: " << sFmtUppFlagList(t->upp_add) << "\n";
	Cout() << "Notes: " << sFmtList(t->notes) << "\n";
	Cout() << "Warnings: " << sFmtList(t->warnings) << "\n";
	Cout() << "Summary: " << t->summary << "\n";
}

static const PkgProviderResolution* sFindProviderResolution(const PkgProviderPlan& plan, const String& capability)
{
	for(const PkgProviderResolution& r : plan.resolutions)
		if(r.capability == capability)
			return &r;
	return nullptr;
}

static void sPrintDeps(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy)
{
	PkgLookupResult lookup = inv.atom.IsEmpty() ? PkgLookupResult() : repo.Resolve(inv.atom);
	const PkgPackage *pkg = lookup.pkg;
	if(lookup.ambiguous) {
		Cout() << "Ambiguous package: " << inv.atom << "\n";
		for(const PkgPackage* c : lookup.candidates)
			if(c)
				Cout() << "  " << sPackageLookupLabel(*c) << "\n";
		return;
	}
	PkgInvocation eff = sEffectiveInvocation(inv, repo, policy, pkg);
	if(pkg && sPolicyPackageMasked(policy, repo, pkg)) {
		Cout() << "Use policy for " << inv.atom << "\n";
		Cout() << "Target: " << sTargetNameText(eff.target) << "\n";
		Cout() << "Local policy: package is masked\n";
		return;
	}
	PkgUseModel use;
	PkgUppProjection upp;
	sBuildUseModel(use, pkg, eff.use_args, eff.target);
	sProjectUpp(upp, use);
	const PkgTargetProfile& tp = sDefaultTargetProfile(eff.target);
	Vector<String> virtuals;
	if(sIsSelected(use.effective, "sqlite"))
		virtuals.Add("virtual/sqlite");
	bool gui_runtime = sIsSelected(use.effective, "st") || sIsSelected(use.effective, "gtk") || sIsSelected(use.effective, "virtualgui");
	bool browser_gui = tp.runtime_environment == "browser" || tp.toolchain == "emcc" || sIsSelected(use.effective, "virtualgui");
	if(gui_runtime)
		virtuals.Add("virtual/gui-runtime");
	if(browser_gui) {
		virtuals.Add("virtual/sdl2");
		virtuals.Add("virtual/opengl");
	}
	PkgProviderPlan provider_plan;
	const Vector<PkgProviderPreference> *extra_prefs = pkg && pkg->manifest.present && pkg->manifest.ok ? &pkg->manifest.provider_preferences : nullptr;
	sBuildProviderPlan(provider_plan, repo, policy, &tp, eff.target, virtuals, eff.provider, eff, extra_prefs);
	Cout() << "Dependencies for " << inv.atom << "\n";
	Cout() << "Target: " << sTargetNameText(eff.target) << " (" << tp.thread_model << ")\n";
	Cout() << "Virtual requirements:\n";
	if(virtuals.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& v : virtuals)
			Cout() << "  " << v << "\n";

	Cout() << "\nProvider resolution:\n";
	if(provider_plan.resolutions.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgProviderResolution& r : provider_plan.resolutions) {
			Cout() << "  " << r.capability << " -> " << (r.provider_id.IsEmpty() ? String("[none]") : r.provider_id)
			     << " [" << r.probe_status << "]\n";
			if(!r.external_package.IsEmpty())
				Cout() << "    package: " << r.external_package << "\n";
			if(!r.uses_add.IsEmpty())
				Cout() << "    U++ uses: " << sFmtList(r.uses_add) << "\n";
			if(!r.upp_add.IsEmpty()) {
				Vector<String> flags;
				for(const PkgUppFlag& f : r.upp_add)
					flags.Add(sFormatUppFlag(f));
				Cout() << "    UPP flags: " << sFmtList(flags) << "\n";
			}
		}

	Cout() << "\nU++ uses additions:\n";
	if(provider_plan.uses_additions.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const String& p : provider_plan.uses_additions)
			Cout() << "  " << p << "\n";

	Cout() << "\nU++ flag additions:\n";
	if(provider_plan.upp_additions.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgUppFlag& flag : provider_plan.upp_additions)
			Cout() << "  " << sFormatUppFlag(flag) << "\n";

	bool need_system = false;
	for(const PkgProviderResolution& r : provider_plan.resolutions)
		if(r.provider_kind == "system")
			need_system = true;
	Cout() << "\nSystem installation required: " << (need_system ? "yes" : "no") << "\n";
}

static String sProviderStatusColor(const String& status)
{
	if(status == "available")
		return "32;1";
	if(status == "missing" || status == "probe_error")
		return "33;1";
	if(status == "manual")
		return "36;1";
	if(status == "probe_not_run")
		return "90";
	return "36";
}

static void sPrintProvidersCommand(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color)
{
	String target = inv.target.IsEmpty() ? String("native") : inv.target;
	String query = inv.provider_query;
	if(!query.IsEmpty() && !query.StartsWith("virtual/"))
		query = String("virtual/") + query;
	if(query.IsEmpty()) {
		Cout() << "Provider catalog:\n";
		Vector<PkgVirtualCapability> caps = sProviderCapabilities();
		for(const PkgVirtualCapability& cap : caps) {
			Cout() << "  " << cap.capability << " - " << cap.description << "\n";
			Vector<PkgProvider> candidates = sProvidersFor(cap.capability, repo, inv, inv.probe, target);
			for(const PkgProvider& p : candidates) {
				Cout() << "    " << p.id << " [" << p.kind << "; "
				     << sAnsi(sProviderStatusColor(p.probe_status), p.probe_status, color) << "]";
				if(!p.provider.IsEmpty())
					Cout() << " -> " << p.provider;
				if(!p.probe_reason.IsEmpty())
					Cout() << " - " << p.probe_reason;
				if(!p.probe_version.IsEmpty())
					Cout() << " (" << p.probe_version << ")";
				Cout() << "\n";
				if(!p.probe_command.IsEmpty())
					Cout() << "      command: " << p.probe_command << "\n";
				if(!p.probe_path.IsEmpty())
					Cout() << "      path: " << p.probe_path << "\n";
			}
		}
		Cout() << "\nSelection rules:\n";
		Cout() << "  explicit provider preference wins if available\n";
		Cout() << "  target-specific provider preferences come next\n";
		Cout() << "  otherwise prefer upp-plugin/bundled providers\n";
		Cout() << "  then target/platform providers\n";
		Cout() << "  missing/manual providers are reported, not installed\n";
		Cout() << "  use --probe for read-only availability checks\n";
		return;
	}

	Vector<String> virtuals;
	virtuals.Add(query);
	PkgProviderPlan plan;
	sBuildProviderPlan(plan, repo, policy, &sDefaultTargetProfile(target), target, virtuals, inv.provider, inv, nullptr);
	const PkgProviderResolution* res = sFindProviderResolution(plan, query);
	if(!res) {
		Cout() << "Capability: " << query << "\n";
		Cout() << "  [no provider available]\n";
		return;
	}

	Cout() << "Capability: " << res->capability << "\n";
	Cout() << "Selected provider: " << (res->provider_id.IsEmpty() ? String("[none]") : res->provider_id)
	     << " [" << sAnsi(sProviderStatusColor(res->probe_status), res->probe_status, color) << "]\n";
	if(!res->external_package.IsEmpty())
		Cout() << "External package: " << res->external_package << "\n";
	if(!res->probe_reason.IsEmpty())
		Cout() << "Probe reason: " << res->probe_reason << "\n";
	if(!res->probe_command.IsEmpty())
		Cout() << "Probe command: " << res->probe_command << "\n";
	if(!res->probe_path.IsEmpty())
		Cout() << "Probe path: " << res->probe_path << "\n";
	if(!res->probe_version.IsEmpty())
		Cout() << "Probe version: " << res->probe_version << "\n";
	if(!res->uses_add.IsEmpty())
		Cout() << "U++ uses: " << sFmtList(res->uses_add) << "\n";
	if(!res->upp_add.IsEmpty()) {
		Vector<String> flags;
		for(const PkgUppFlag& f : res->upp_add)
			flags.Add(sFormatUppFlag(f));
		Cout() << "U++ flags: " << sFmtList(flags) << "\n";
	}
	Cout() << "\nCandidates:\n";
	Vector<PkgProvider> candidates = sProvidersFor(query, repo, inv, inv.probe, target);
	if(candidates.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgProvider& p : candidates) {
			Cout() << "  " << p.id << " [" << p.kind << "; "
			     << sAnsi(sProviderStatusColor(p.probe_status), p.probe_status, color) << "]";
			if(!p.provider.IsEmpty())
				Cout() << " -> " << p.provider;
			if(!p.details.IsEmpty())
				Cout() << " - " << p.details;
			if(!p.probe_reason.IsEmpty())
				Cout() << " | " << p.probe_reason;
			Cout() << "\n";
			if(!p.probe_command.IsEmpty())
				Cout() << "    probe command: " << p.probe_command << "\n";
			if(!p.probe_path.IsEmpty())
				Cout() << "    probe path: " << p.probe_path << "\n";
			if(!p.probe_version.IsEmpty())
				Cout() << "    probe version: " << p.probe_version << "\n";
			if(!p.uses_add.IsEmpty())
				Cout() << "    U++ uses: " << sFmtList(p.uses_add) << "\n";
			if(!p.upp_add.IsEmpty()) {
				Vector<String> flags;
				for(const PkgUppFlag& f : p.upp_add)
					flags.Add(sFormatUppFlag(f));
				Cout() << "    UPP flags: " << sFmtList(flags) << "\n";
			}
		}
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

static void sPrintMetadata(const PkgRepository& repo, const PkgInvocation& inv)
{
	Cout() << "Repository root: " << repo.paths.root << "\n";
	Cout() << "Package count: " << repo.packages.GetCount() << "\n";
	Cout() << "Nest count: " << repo.nests.GetCount() << "\n";
	Cout() << "State file: " << repo.paths.state << "\n";
	Cout() << "World file: " << repo.paths.world << "\n";
	Cout() << "Package.use: " << repo.paths.package_use << "\n";
	Cout() << "Package.provider: " << repo.paths.package_provider << "\n";
	Cout() << "Package.target: " << repo.paths.package_target << "\n";
	if(!inv.atom.IsEmpty()) {
		Cout() << "\nPackage metadata for: " << inv.atom << "\n";
		PkgLookupResult lookup = repo.Resolve(inv.atom);
		if(lookup.ambiguous) {
			Cout() << "Status: ambiguous\n";
			Cout() << "Candidates:\n";
			for(const PkgPackage* c : lookup.candidates)
				if(c)
					Cout() << "  " << sPackageLookupLabel(*c) << "\n";
			return;
		}
		const PkgPackage* p = lookup.pkg;
		if(!p) {
			Cout() << "Status: not found\n";
			return;
		}
		Cout() << "Status: found\n";
		Cout() << "Name: " << p->name << "\n";
		Cout() << "Nest: " << p->nest << "\n";
		Cout() << "Path: " << p->path << "\n";
		Cout() << "Description: " << (p->description.IsEmpty() ? String("[none]") : p->description) << "\n";
		Cout() << "Acceptflags: " << sFmtList(p->accepts) << "\n";
		Cout() << "Uses: " << sFmtList(p->uses) << "\n";
		Cout() << "Mainconfig: " << sFmtList(p->mainconfig) << "\n";
		Cout() << "Uppkg: " << (p->manifest.present ? p->manifest.path : String("[none]")) << "\n";
		Cout() << "Uppkg status: " << (p->manifest.present ? (p->manifest.ok ? "parsed" : "invalid") : "missing") << "\n";
		if(p->manifest.present) {
			Cout() << "Uppkg target: " << (p->manifest.target.IsEmpty() ? String("[none]") : p->manifest.target) << "\n";
			Cout() << "Uppkg provider: " << (p->manifest.provider.IsEmpty() ? String("[none]") : p->manifest.provider) << "\n";
			Cout() << "Uppkg use default: " << sFmtList(p->manifest.use_default) << "\n";
			Cout() << "Uppkg use forced: " << sFmtList(p->manifest.use_forced) << "\n";
			Cout() << "Uppkg use masked: " << sFmtList(p->manifest.use_masked) << "\n";
		}
	}
}

static void sPrintMetadataCache(const PkgRepository& repo, const PkgInvocation& inv)
{
	Cout() << "Metadata cache:\n";
	Cout() << "  path: " << repo.paths.metadata_cache << "\n";
	Cout() << "  enabled: " << (repo.use_cache ? "yes" : "no") << "\n";
	Cout() << "  rebuild requested: " << (repo.rebuild_cache ? "yes" : "no") << "\n";
	Cout() << "  exists: " << (FileExists(repo.paths.metadata_cache) ? "yes" : "no") << "\n";
	Cout() << "  schema: " << repo.cache_schema << "\n";
	Cout() << "  loaded entries: " << repo.cache_loaded_entries << "\n";
	Cout() << "  reused entries: " << repo.cache_reused_entries << "\n";
	Cout() << "  reparsed entries: " << repo.cache_reparsed_entries << "\n";
	Cout() << "  stale entries: " << repo.cache_stale_entries << "\n";
	Cout() << "  corrupt: " << (repo.cache_corrupt ? "yes" : "no") << "\n";
	Cout() << "  ok: " << (repo.cache_ok ? "yes" : "no") << "\n";
	if(inv.rebuild_cache)
		Cout() << "  status: rebuilt or refreshed during discovery\n";
}

static void sPrintInfo(const PkgRepository& repo, const PkgInvocation& inv, const PkgLocalPolicy& policy)
{
	Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	Cout() << "Repository root: " << repo.paths.root << "\n";
	Cout() << "State file: " << repo.paths.state << "\n";
	Cout() << "World file: " << repo.paths.world << "\n";
	Cout() << "Local config root: " << repo.paths.ai_dir << "\n";
	Cout() << "Package.use: " << repo.paths.package_use << "\n";
	Cout() << "Package.provider: " << repo.paths.package_provider << "\n";
	Cout() << "Package.target: " << repo.paths.package_target << "\n";
	Cout() << "Package.mask: " << repo.paths.package_mask << "\n";
	Cout() << "Package.force: " << repo.paths.package_force << "\n";
	Cout() << "Make.conf: " << repo.paths.make_conf << "\n";
	Cout() << "Eselect file: " << repo.paths.eselect << "\n";
	Cout() << "World entries: " << world.GetCount() << "\n";
	PkgLookupResult lookup = inv.atom.IsEmpty() ? PkgLookupResult() : repo.Resolve(inv.atom);
	PkgInvocation eff = sEffectiveInvocation(inv, repo, policy, lookup.pkg);
	String active_target = !eff.target.IsEmpty() ? eff.target : (state.target.IsEmpty() ? String("native") : state.target);
	const PkgTargetProfile& active_profile = sDefaultTargetProfile(active_target);
	Cout() << "Active target: " << active_target << "\n";
	Cout() << "Active target thread model: " << active_profile.thread_model << "\n";
	Cout() << "Active target reason: " << sTargetThreadReason(active_profile) << "\n";
	Cout() << "Active target compiler: " << active_profile.compiler << "\n";
	Cout() << "Active target toolchain: " << active_profile.toolchain << "\n";
	Cout() << "Selected target: " << (eff.target.IsEmpty() ? String("[none]") : eff.target) << "\n";
	Cout() << "Selected profile: " << (eff.profile.IsEmpty() ? String("[none]") : eff.profile) << "\n";
	Cout() << "Selected repository: " << (eff.repository.IsEmpty() ? String("[none]") : eff.repository) << "\n";
	Cout() << "Active toolchain: " << (state.toolchain.IsEmpty() ? String("[none]") : state.toolchain) << "\n";
	Cout() << "Selected compiler: " << (eff.compiler.IsEmpty() ? String("[none]") : eff.compiler) << "\n";
	Cout() << "Selected linker: " << (eff.linker.IsEmpty() ? String("[none]") : eff.linker) << "\n";
	Cout() << "Selected provider: " << (eff.provider.IsEmpty() ? String("[none]") : eff.provider) << "\n";
	Cout() << "Selected vcpkg root: " << (eff.vcpkg_root.IsEmpty() ? String("[none]") : eff.vcpkg_root) << "\n";
	Cout() << "Selected vcpkg triplet: " << (eff.vcpkg_triplet.IsEmpty() ? String("[none]") : eff.vcpkg_triplet) << "\n";
	Cout() << "Selected emscripten profile: " << (eff.emscripten_profile.IsEmpty() ? String("[none]") : eff.emscripten_profile) << "\n";
	Cout() << "Selected USE: " << sFormatUseList(sMergeUseArgs(Split(policy.make_use, CharFilterWhitespace), eff.use_args)) << "\n";
	Cout() << "Doctor: run `pkg doctor` for environment diagnostics.\n";
	Cout() << "Lint check: use `pkg lint --check` for local batch policy; `--ci` is a compatibility alias.\n";
	Cout() << "Selftest: run `pkg selftest` for local smoke checks.\n";
	Cout() << "Artifacts: run `pkg bins` to inspect recorded build outputs.\n";
	Cout() << "Cleanup: run `pkg clean` or `pkg --depclean` for conservative removal.\n";
}

struct PkgDoctorSummary {
	int ok = 0;
	int warn = 0;
	int info = 0;
	int error = 0;
};

struct PkgSelftestCheck : Moveable<PkgSelftestCheck> {
	String name;
	String status;
	String message;
	double seconds = 0;
	Vector<String> details;

	void Jsonize(JsonIO& jio)
	{
		jio("name", name)("status", status)("message", message)("seconds", seconds);
		jio("details", details);
	}
};

struct PkgSelftestRuntimePath : Moveable<PkgSelftestRuntimePath> {
	bool windows = false;
	bool ok = false;
	String status;
	String message;
	Vector<String> expected;
	Vector<String> missing;

	void Jsonize(JsonIO& jio)
	{
		jio("windows", windows)("ok", ok)("status", status)("message", message);
		jio("expected", expected)("missing", missing);
	}
};

struct PkgSelftestReport : Moveable<PkgSelftestReport> {
	String command_line;
	String report_path;
	String format;
	String group;
	String note;
	String exit_basis;
	Time timestamp;
	double total_seconds = 0;
	int ok = 0;
	int warn = 0;
	int info = 0;
	int error = 0;
	bool strict = false;
	PkgSelftestRuntimePath runtime_path;
	Vector<PkgSelftestCheck> checks;

	void Jsonize(JsonIO& jio)
	{
		jio("command_line", command_line)("report_path", report_path)("format", format);
		jio("group", group)("note", note)("exit_basis", exit_basis)("timestamp", timestamp);
		jio("total_seconds", total_seconds);
		jio("ok", ok)("warn", warn)("info", info)("error", error);
		jio("strict", strict);
		jio("runtime_path", runtime_path);
		jio("checks", checks);
	}
};

static void sDoctorLine(PkgDoctorSummary& sum, const char *level, const String& message, bool color)
{
	String tag = String("[") + level + "]";
	String code = "36";
	if(level == String("ok")) {
		sum.ok++;
		code = "32;1";
	}
	else if(level == String("warn")) {
		sum.warn++;
		code = "33;1";
	}
	else if(level == String("info")) {
		sum.info++;
		code = "36";
	}
	else if(level == String("error")) {
		sum.error++;
		code = "31;1";
	}
	Cout() << "  " << sAnsi(code, tag, color) << "  " << message << "\n";
}

static void sDoctorSubline(const String& label, const String& value)
{
	Cout() << "          " << label << value << "\n";
}

static int sSelftestAddCheck(PkgSelftestReport& report, const String& name, const String& status, const String& message, double seconds)
{
	PkgSelftestCheck& check = report.checks.Add();
	check.name = name;
	check.status = status;
	check.message = message;
	check.seconds = seconds;
	if(status == "ok")
		report.ok++;
	else if(status == "warn")
		report.warn++;
	else if(status == "info")
		report.info++;
	else if(status == "error")
		report.error++;
	else
		report.info++;
	return report.checks.GetCount() - 1;
}

static void sSelftestAddDetail(PkgSelftestReport& report, int check_index, const String& detail)
{
	if(check_index >= 0 && check_index < report.checks.GetCount())
		report.checks[check_index].details.Add(detail);
}

static bool sDoctorPathSuffixPresent(const String& path, const String& suffix)
{
	String needle = ToLower(UnixPath(suffix));
	Vector<String> entries = Split(path, ';');
	for(const String& entry : entries) {
		String e = ToLower(UnixPath(entry));
		if(e.EndsWith(needle))
			return true;
	}
	return false;
}

static PkgSelftestRuntimePath sDiagnoseRuntimePath()
{
	PkgSelftestRuntimePath rt;
#ifdef flagWIN32
	rt.windows = true;
	String path = GetEnv("PATH");
	rt.expected.Add("...\\bin\\clang\\bin");
	if(!sDoctorPathSuffixPresent(path, "bin\\clang\\bin"))
		rt.missing.Add("...\\bin\\clang\\bin");
	rt.expected.Add("...\\bin\\clang\\x86_64-w64-mingw32\\bin");
	if(!sDoctorPathSuffixPresent(path, "bin\\clang\\x86_64-w64-mingw32\\bin"))
		rt.missing.Add("...\\bin\\clang\\x86_64-w64-mingw32\\bin");
	rt.ok = rt.missing.IsEmpty();
	rt.status = rt.windows ? (rt.ok ? "ok" : "warn") : "info";
	rt.message = rt.ok ? "clang runtime DLL paths appear to be present in PATH"
	                   : "clang runtime DLL path may be missing";
#else
	rt.ok = true;
	rt.status = "info";
	rt.message = "Windows clang runtime DLL path check skipped on this platform";
#endif
	return rt;
}

static void sPrintDoctorEnvironment(PkgDoctorSummary& sum, bool color)
{
	Cout() << "Environment:\n";
	sDoctorLine(sum, "ok", "executable found: bin\\pkg.exe", color);
	PkgSelftestRuntimePath rt = sDiagnoseRuntimePath();
	if(rt.windows) {
		if(rt.ok) {
			sDoctorLine(sum, "ok", rt.message, color);
		}
		else {
			sDoctorLine(sum, "warn", rt.message, color);
			for(const String& m : rt.missing)
				sDoctorSubline("expected: ", m);
			sDoctorSubline("symptom: ", "libc++.dll or libunwind.dll loader error");
		}
	}
	else
		sDoctorLine(sum, "info", rt.message, color);
}

static void sPrintDoctorShell(PkgDoctorSummary& sum, bool color)
{
	Cout() << "Shell:\n";
#ifdef flagWIN32
	if(!GetEnv("PSModulePath").IsEmpty()) {
		sDoctorLine(sum, "warn", "PowerShell treats @world specially", color);
		sDoctorSubline("use: ", "cmd /c \"bin\\pkg.exe -pv @world\"");
		sDoctorSubline("or: ", "quote or escape @world before passing it to pkg");
	}
	else {
		sDoctorLine(sum, "info", "Windows shells may need @world quoted or escaped", color);
		sDoctorSubline("use: ", "cmd /c \"bin\\pkg.exe -pv @world\"");
		sDoctorSubline("or: ", "quote or escape @world before passing it to pkg");
	}
#else
	sDoctorLine(sum, "info", "Shell-specific @world handling is Windows-centric", color);
#endif
}

static void sPrintDoctorState(PkgDoctorSummary& sum, const PkgConfigPaths& paths, bool color)
{
	Cout() << "State:\n";
	PkgState state;
	bool state_exists = FileExists(paths.state);
	bool state_ok = state_exists && LoadFromJsonFile(state, paths.state);
	if(!state_exists)
		sDoctorLine(sum, "warn", paths.state + " missing", color);
	else if(!state_ok)
		sDoctorLine(sum, "error", paths.state + " exists but failed to parse", color);
	else {
		sDoctorLine(sum, "ok", paths.state + " parsed", color);
		if(!state.target.IsEmpty())
			sDoctorSubline("target: ", state.target);
		if(!state.toolchain.IsEmpty())
			sDoctorSubline("toolchain: ", state.toolchain);
		int artifacts = 0;
		int failed = 0;
		for(const PkgStateRecord& rec : state.records) {
			if(!(rec.success || rec.build_status == "built" || rec.build_status == "failed"))
				continue;
			artifacts++;
			if(!(rec.success || rec.build_status == "built"))
				failed++;
		}
		sDoctorSubline("artifacts: ", AsString(artifacts));
		if(failed > 0)
			sDoctorSubline("failed artifacts: ", AsString(failed));
		sDoctorSubline("cleanup: ", "pkg clean / pkg --depclean");
	}

	PkgEselectState eselect;
	bool eselect_exists = FileExists(paths.eselect);
	bool eselect_ok = eselect_exists && LoadFromJsonFile(eselect, paths.eselect);
	if(!eselect_exists)
		sDoctorLine(sum, "warn", paths.eselect + " missing", color);
	else if(!eselect_ok)
		sDoctorLine(sum, "error", paths.eselect + " exists but failed to parse", color);
	else {
		sDoctorLine(sum, "ok", paths.eselect + " parsed", color);
		if(!eselect.target.IsEmpty())
			sDoctorSubline("target: ", eselect.target);
		if(!eselect.compiler.IsEmpty())
			sDoctorSubline("compiler: ", eselect.compiler);
		if(!eselect.provider.IsEmpty())
			sDoctorSubline("provider: ", eselect.provider);
		if(!eselect.vcpkg_root.IsEmpty())
			sDoctorSubline("vcpkg root: ", eselect.vcpkg_root);
		if(!eselect.vcpkg_triplet.IsEmpty())
			sDoctorSubline("vcpkg triplet: ", eselect.vcpkg_triplet);
		if(!eselect.emscripten_profile.IsEmpty())
			sDoctorSubline("emscripten profile: ", eselect.emscripten_profile);
	}
}

static void sPrintDoctorCache(PkgDoctorSummary& sum, const PkgRepository& repo, bool color)
{
	Cout() << "Metadata cache:\n";
	if(!FileExists(repo.paths.metadata_cache)) {
		sDoctorLine(sum, "warn", repo.paths.metadata_cache + " missing", color);
		return;
	}
	PkgMetadataCache cache;
	if(!LoadFromJsonFile(cache, repo.paths.metadata_cache)) {
		sDoctorLine(sum, "error", repo.paths.metadata_cache + " exists but failed to parse", color);
		return;
	}
	sDoctorLine(sum, "ok", repo.paths.metadata_cache + " parsed", color);
	sDoctorSubline("schema: ", AsString(cache.schema));
	sDoctorSubline("entries: ", AsString(cache.entries.GetCount()));
	if(!cache.root.IsEmpty())
		sDoctorSubline("root: ", cache.root);
	if(cache.saved_at.IsValid())
		sDoctorSubline("saved_at: ", Format(cache.saved_at));
	sDoctorSubline("reused entries: ", AsString(repo.cache_reused_entries));
	sDoctorSubline("reparsed entries: ", AsString(repo.cache_reparsed_entries));
	sDoctorSubline("stale entries: ", AsString(repo.cache_stale_entries));
	if(repo.cache_corrupt)
		sDoctorSubline("warning: ", "previous cache load failed and was rebuilt");
}

static void sPrintDoctorConfig(PkgDoctorSummary& sum, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color)
{
	Cout() << "Config:\n";
	auto show = [&](const String& path) {
		if(path.IsEmpty())
			return;
		if(FileExists(path))
			sDoctorLine(sum, "ok", path + " parsed", color);
		else
			sDoctorLine(sum, "info", path + " missing", color);
	};
	show(policy.make_conf_path);
	show(policy.package_use_path);
	show(policy.package_provider_path);
	show(policy.package_target_path);
	show(policy.package_mask_path);
	show(policy.package_force_path);
	Cout() << "  local root: " << repo.paths.ai_dir << "\n";
	Cout() << "  effective defaults:\n";
	if(!policy.make_use.IsEmpty())
		Cout() << "    USE=" << policy.make_use << "\n";
	if(!policy.make_target.IsEmpty())
		Cout() << "    TARGET=" << policy.make_target << "\n";
	if(!policy.make_provider.IsEmpty())
		Cout() << "    PROVIDER=" << policy.make_provider << "\n";
	if(!policy.make_compiler.IsEmpty())
		Cout() << "    COMPILER=" << policy.make_compiler << "\n";
	if(!policy.make_linker.IsEmpty())
		Cout() << "    LINKER=" << policy.make_linker << "\n";
	if(!policy.make_profile.IsEmpty())
		Cout() << "    PROFILE=" << policy.make_profile << "\n";
	if(!policy.make_repository.IsEmpty())
		Cout() << "    REPOSITORY=" << policy.make_repository << "\n";
	if(!policy.make_vcpkg_root.IsEmpty())
		Cout() << "    VCPKG_ROOT=" << policy.make_vcpkg_root << "\n";
	if(!policy.make_vcpkg_triplet.IsEmpty())
		Cout() << "    VCPKG_TRIPLET=" << policy.make_vcpkg_triplet << "\n";
	if(!policy.make_emscripten_profile.IsEmpty())
		Cout() << "    EMSCRIPTEN_PROFILE=" << policy.make_emscripten_profile << "\n";
	if(policy.make_jobs > 0)
		Cout() << "    JOBS=" << policy.make_jobs << "\n";
	if(!policy.warnings.IsEmpty()) {
		sDoctorLine(sum, "warn", "config warnings present", color);
		for(const String& warn : policy.warnings)
			sDoctorSubline("warning: ", warn);
	}
	if(!policy.errors.IsEmpty()) {
		sDoctorLine(sum, "error", "config errors present", color);
		for(const String& err : policy.errors)
			sDoctorSubline("error: ", err);
	}

	Vector<const PkgPackage*> manifests;
	for(const PkgPackage& p : repo.packages)
		if(p.manifest.present)
			manifests.Add(&p);
	if(manifests.IsEmpty()) {
		sDoctorLine(sum, "info", "package-local .uppkg manifests: none found", color);
	}
	else {
		sDoctorLine(sum, "ok", "package-local .uppkg manifests found", color);
		for(const PkgPackage* p : manifests) {
			String status = p->manifest.ok ? (p->manifest.warnings.IsEmpty() ? "ok" : "warn") : "error";
			Cout() << "  [" << status << "] " << p->name << " <" << p->manifest.path << ">\n";
			if(!p->manifest.target.IsEmpty())
				sDoctorSubline("target: ", p->manifest.target);
			if(!p->manifest.provider.IsEmpty())
				sDoctorSubline("provider: ", p->manifest.provider);
			if(!p->manifest.use_default.IsEmpty())
				sDoctorSubline("use default: ", sFmtList(p->manifest.use_default));
			if(!p->manifest.use_forced.IsEmpty())
				sDoctorSubline("use forced: ", sFmtList(p->manifest.use_forced));
			if(!p->manifest.use_masked.IsEmpty())
				sDoctorSubline("use masked: ", sFmtList(p->manifest.use_masked));
			if(!p->manifest.provider_preferences.IsEmpty())
				sDoctorSubline("provider prefs: ", sFmtProviderPreferences(p->manifest.provider_preferences));
			for(const String& warn : p->manifest.warnings)
				sDoctorSubline("warning: ", warn);
			for(const String& err : p->manifest.errors)
				sDoctorSubline("error: ", err);
		}
	}
}

static int sProviderDoctorLevel(const String& status)
{
	if(status == "available")
		return 0;
	if(status == "missing" || status == "probe_error")
		return 1;
	return 2;
}

static void sPrintDoctorProviders(PkgDoctorSummary& sum, const PkgInvocation& inv, const PkgRepository& repo, bool color)
{
	Cout() << "Providers:\n";
	if(!inv.probe) {
		sDoctorLine(sum, "info", "provider probing is plan-only/placeholder in this milestone", color);
		sDoctorLine(sum, "info", "toolchain probing is also plan-only/placeholder", color);
		sDoctorLine(sum, "info", "no package managers were invoked", color);
		return;
	}

	String target = inv.target.IsEmpty() ? String("native") : inv.target;
	for(const PkgVirtualCapability& cap : sProviderCapabilities()) {
		Cout() << "  " << cap.capability << ":\n";
		Vector<PkgProvider> candidates = sProvidersFor(cap.capability, repo, inv, true, target);
		for(const PkgProvider& p : candidates) {
			String msg = p.id + " " + p.probe_status;
			if(!p.probe_reason.IsEmpty())
				msg << ": " << p.probe_reason;
			else if(!p.details.IsEmpty())
				msg << ": " << p.details;
			sDoctorLine(sum, sProviderDoctorLevel(p.probe_status) == 0 ? "ok" : sProviderDoctorLevel(p.probe_status) == 1 ? "warn" : "info", msg, color);
			if(!p.probe_command.IsEmpty())
				sDoctorSubline("command: ", p.probe_command);
			if(!p.probe_path.IsEmpty())
				sDoctorSubline("path: ", p.probe_path);
			if(!p.probe_version.IsEmpty())
				sDoctorSubline("version: ", p.probe_version);
		}
	}
	sDoctorLine(sum, "info", "no install commands were run", color);
}

static void sPrintDoctorCommand(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color)
{
	PkgDoctorSummary sum;
	String section = ToLower(TrimBoth(inv.subcommand));
	PkgConfigPaths paths = repo.paths;

	Cout() << "pkg doctor\n\n";
	if(section.IsEmpty() || section == "all" || section == "env")
		sPrintDoctorEnvironment(sum, color);
	if(section.IsEmpty() || section == "all" || section == "shell")
		sPrintDoctorShell(sum, color);
	if(section.IsEmpty() || section == "all" || section == "state")
		sPrintDoctorState(sum, paths, color);
	if(section.IsEmpty() || section == "all" || section == "cache")
		sPrintDoctorCache(sum, repo, color);
	if(section.IsEmpty() || section == "all" || section == "config")
		sPrintDoctorConfig(sum, repo, policy, color);
	if(section.IsEmpty() || section == "all" || section == "providers")
		sPrintDoctorProviders(sum, inv, repo, color);

	if(!section.IsEmpty() && section != "all" && section != "env" && section != "shell" && section != "state" && section != "cache" && section != "config" && section != "providers") {
		sDoctorLine(sum, "warn", "unknown doctor subcommand: " + inv.subcommand, color);
		Cout() << "  available sections: env shell state cache config providers all\n";
	}

	Cout() << "\nSummary:\n";
	Cout() << "  " << sum.ok << " ok, " << sum.warn << " warn, " << sum.info << " info, " << sum.error << " error\n";
}

static bool sPlanHasBlockingRows(const PkgPlan& plan, String& reason);

static String sSelftestReportFormat(const String& path)
{
	return sLintReportFormat(path);
}

static bool sWriteSelftestReportFile(const PkgSelftestReport& report)
{
	if(report.report_path.IsEmpty())
		return true;
	RealizeDirectory(GetFileFolder(report.report_path));
	String format = sSelftestReportFormat(report.report_path);
	if(format == "markdown") {
		String out;
		out << "# pkg selftest report\n\n";
		out << "- note: local selftest, not project CI\n";
		out << "- command_line: `" << report.command_line << "`\n";
		out << "- timestamp: " << AsString(report.timestamp) << "\n";
		out << "- report_path: `" << report.report_path << "`\n";
		out << "- format: markdown\n";
		out << "- group: " << (report.group.IsEmpty() ? String("[none]") : report.group) << "\n";
		out << "- exit_basis: " << (report.exit_basis.IsEmpty() ? String("[none]") : report.exit_basis) << "\n";
		out << "- strict: " << (report.strict ? "yes" : "no") << "\n";
		out << "- total_seconds: " << FormatDouble(report.total_seconds, 3) << "\n";
		out << "- ok: " << report.ok << "\n";
		out << "- warn: " << report.warn << "\n";
		out << "- info: " << report.info << "\n";
		out << "- error: " << report.error << "\n";
		out << "- runtime_path: " << (report.runtime_path.status.IsEmpty() ? String("info") : report.runtime_path.status) << "\n";
		out << "- runtime_path_message: " << report.runtime_path.message << "\n";
		if(!report.runtime_path.expected.IsEmpty())
			out << "- runtime_path_expected: " << sFmtList(report.runtime_path.expected) << "\n";
		if(!report.runtime_path.missing.IsEmpty())
			out << "- runtime_path_missing: " << sFmtList(report.runtime_path.missing) << "\n";
		out << "\n## Checks\n";
		if(report.checks.IsEmpty())
			out << "_none_\n";
		else {
			for(const PkgSelftestCheck& c : report.checks) {
				out << "\n### " << c.status << " " << c.name << "\n";
				out << "- message: " << c.message << "\n";
				out << "- seconds: " << FormatDouble(c.seconds, 3) << "\n";
				if(c.details.IsEmpty())
					out << "- details: [none]\n";
				else {
					out << "- details:\n";
					for(const String& d : c.details)
						out << "  - " << d << "\n";
				}
			}
		}
		return SaveFile(report.report_path, out);
	}

	return StoreAsJsonFile(report, report.report_path, true);
}

static void sSelftestVersionHelp(PkgDoctorSummary& sum, PkgSelftestReport& report, bool color)
{
	int idx = sSelftestAddCheck(report, "version command path", "ok", "version command path", 0.0);
	sDoctorLine(sum, "ok", "version command path", color);
	sSelftestAddDetail(report, idx, "version command should exit cleanly");
	idx = sSelftestAddCheck(report, "help command path", "ok", "help command path", 0.0);
	sDoctorLine(sum, "ok", "help command path", color);
	sSelftestAddDetail(report, idx, "help text should remain available without side effects");
}

static void sSelftestDoctor(PkgDoctorSummary& sum, PkgSelftestReport& report, bool color)
{
	PkgSelftestRuntimePath rt = sDiagnoseRuntimePath();
	report.runtime_path.windows = rt.windows;
	report.runtime_path.ok = rt.ok;
	report.runtime_path.status = rt.status;
	report.runtime_path.message = rt.message;
	Swap(report.runtime_path.expected, rt.expected);
	Swap(report.runtime_path.missing, rt.missing);
	if(rt.windows) {
		if(rt.ok) {
			int idx = sSelftestAddCheck(report, "doctor environment", "ok", rt.message, 0.0);
			sDoctorLine(sum, "ok", "doctor environment", color);
			sSelftestAddDetail(report, idx, "clang runtime DLL paths appear to be present in PATH");
		}
		else {
			int idx = sSelftestAddCheck(report, "doctor environment", "warn", rt.message, 0.0);
			sDoctorLine(sum, "warn", "doctor environment: clang runtime DLL path may be missing", color);
			for(const String& m : rt.missing) {
				sDoctorSubline("expected: ", m);
				sSelftestAddDetail(report, idx, "expected: " + m);
			}
			sDoctorSubline("symptom: ", "libc++.dll or libunwind.dll loader error");
			sSelftestAddDetail(report, idx, "symptom: libc++.dll or libunwind.dll loader error");
		}
	}
	else {
		int idx = sSelftestAddCheck(report, "doctor environment", "info", rt.message, 0.0);
		sDoctorLine(sum, "info", "doctor environment is Windows-specific", color);
		sSelftestAddDetail(report, idx, rt.message);
	}

	if(!GetEnv("PSModulePath").IsEmpty()) {
		int idx = sSelftestAddCheck(report, "doctor shell", "warn", "PowerShell treats @world specially", 0.0);
		sDoctorLine(sum, "warn", "doctor shell: PowerShell treats @world specially", color);
		sDoctorSubline("use: ", "cmd /c \"bin\\pkg.exe -pv @world\"");
		sDoctorSubline("or: ", "quote or escape @world before passing it to pkg");
		sSelftestAddDetail(report, idx, "use: cmd /c \"bin\\pkg.exe -pv @world\"");
		sSelftestAddDetail(report, idx, "or: quote or escape @world before passing it to pkg");
	}
	else {
		int idx = sSelftestAddCheck(report, "doctor shell", "info", "Windows shells may need @world quoted or escaped", 0.0);
		sDoctorLine(sum, "info", "doctor shell: Windows shells may need @world quoted or escaped", color);
		sDoctorSubline("use: ", "cmd /c \"bin\\pkg.exe -pv @world\"");
		sDoctorSubline("or: ", "quote or escape @world before passing it to pkg");
		sSelftestAddDetail(report, idx, "use: cmd /c \"bin\\pkg.exe -pv @world\"");
		sSelftestAddDetail(report, idx, "or: quote or escape @world before passing it to pkg");
	}
}

static void sSelftestCache(PkgDoctorSummary& sum, PkgSelftestReport& report, const PkgRepository& repo, bool color)
{
	String msg;
	String status;
	if(repo.cache_corrupt) {
		status = "warn";
		msg = "doctor cache: cache is corrupt but recoverable";
	}
	else if(repo.cache_ok) {
		status = "ok";
		msg = "doctor cache: cache is readable";
	}
	else {
		status = "warn";
		msg = "doctor cache: cache is missing or not reusable";
	}
	int idx = sSelftestAddCheck(report, "doctor cache", status, msg, 0.0);
	sDoctorLine(sum, status, msg, color);
	if(repo.cache_corrupt)
		sSelftestAddDetail(report, idx, "cache is corrupt but recoverable");
	else if(repo.cache_ok)
		sSelftestAddDetail(report, idx, "cache is readable");
	else
		sSelftestAddDetail(report, idx, "cache is missing or not reusable");

	status = repo.cache_ok ? "ok" : "warn";
	msg = "metadata cache";
	idx = sSelftestAddCheck(report, "metadata cache", status, msg, 0.0);
	sDoctorLine(sum, status, msg, color);
	sSelftestAddDetail(report, idx, repo.cache_ok ? "cache available for reuse" : "cache unavailable or disabled");
}

static void sSelftestSearch(PkgDoctorSummary& sum, PkgSelftestReport& report, const PkgRepository& repo, bool color, bool verbose)
{
	TimeStop ts;
	Vector<const PkgPackage*> matches = repo.Search("pkg");
	if(matches.GetCount()) {
		int idx = sSelftestAddCheck(report, "search pkg", "ok", "search pkg", ts.Seconds());
		sDoctorLine(sum, "ok", "search pkg", color);
		if(verbose)
			sDoctorSubline("matches: ", AsString(matches.GetCount()));
		sSelftestAddDetail(report, idx, "matches: " + AsString(matches.GetCount()));
	}
	else {
		int idx = sSelftestAddCheck(report, "search pkg", "error", "search pkg returned no results", ts.Seconds());
		sDoctorLine(sum, "error", "search pkg returned no results", color);
		sSelftestAddDetail(report, idx, "search pkg returned no results");
	}
}

static void sSelftestPlan(PkgDoctorSummary& sum, PkgSelftestReport& report, const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color, bool verbose)
{
	TimeStop ts;
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);

	PkgInvocation planinv;
	sCopyInvocation(planinv, inv);
	planinv.command = PKG_CMD_PLAN;
	planinv.atom = "pkg";
	planinv.pretend = true;
	planinv.plan = true;
	planinv.ask = false;
	planinv.verbose = verbose;
	PkgPlan plan = sBuildPlan(planinv, repo, state, policy, color);
	String reason;
	String status;
	String message;
	if(plan.graph.nodes.IsEmpty()) {
		status = "error";
		message = "plan pkg: no packages selected";
	}
	else if(sPlanHasBlockingRows(plan, reason)) {
		status = "warn";
		message = "plan pkg: blockers present";
	}
	else {
		status = "ok";
		message = "plan pkg";
	}
	int idx = sSelftestAddCheck(report, "plan pkg", status, message, ts.Seconds());
	sDoctorLine(sum, status, message, color);
	if(status == "warn" && verbose)
		sDoctorSubline("reason: ", reason);
	else if(status == "ok" && verbose)
		sDoctorSubline("graph: ", AsString(plan.graph.nodes.GetCount()) + " nodes");
	if(verbose && !reason.IsEmpty())
		sSelftestAddDetail(report, idx, String("reason: ") + reason);
	else if(verbose)
		sSelftestAddDetail(report, idx, String("graph: ") + AsString(plan.graph.nodes.GetCount()) + " nodes");
	else if(!reason.IsEmpty())
		sSelftestAddDetail(report, idx, reason);

	PkgInvocation miss;
	sCopyInvocation(miss, planinv);
	miss.atom = "app/MyApp";
	PkgLookupResult miss_lookup = repo.Resolve(miss.atom);
	PkgPlan miss_plan = sBuildPlan(miss, repo, state, policy, color);
	if(!miss_lookup.pkg || sPlanHasBlockingRows(miss_plan, reason)) {
		int midx = sSelftestAddCheck(report, "missing package guard", "ok", "missing package guard", 0.0);
		sDoctorLine(sum, "ok", "missing package guard", color);
		if(verbose)
			sDoctorSubline("atom: ", miss.atom);
		sSelftestAddDetail(report, midx, "atom: " + miss.atom);
	}
	else {
		int midx = sSelftestAddCheck(report, "missing package guard", "warn", "missing package guard: package unexpectedly buildable", 0.0);
		sDoctorLine(sum, "warn", "missing package guard: package unexpectedly buildable", color);
		sSelftestAddDetail(report, midx, "package unexpectedly buildable");
	}
}

static void sSelftestProviders(PkgDoctorSummary& sum, PkgSelftestReport& report, const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color, bool verbose)
{
	TimeStop ts;
	PkgInvocation provinv;
	sCopyInvocation(provinv, inv);
	provinv.command = PKG_CMD_PROVIDERS;
	provinv.probe = true;
	const PkgTargetProfile& tp = sDefaultTargetProfile(Nvl(inv.target, String("native")));
	Vector<String> virtuals;
	virtuals.Add("virtual/sqlite");
	virtuals.Add("virtual/gui-runtime");
	PkgProviderPlan pp;
	sBuildProviderPlan(pp, repo, policy, &tp, Nvl(inv.target, String("native")), virtuals, inv.provider, provinv, nullptr);

	int available = 0;
	int missing = 0;
	int manual = 0;
	int unknown = 0;
	for(const PkgProviderResolution& r : pp.resolutions) {
		String s = ToLower(r.probe_status);
		if(s == "available")
			available++;
		else if(s == "missing")
			missing++;
		else if(s == "manual")
			manual++;
		else
			unknown++;
	}
	String status = pp.resolutions.IsEmpty() ? "warn" : "info";
	String message = pp.resolutions.IsEmpty() ? "provider probes are read-only but no catalog entries were resolved"
	                                           : "provider probes are read-only";
	int idx = sSelftestAddCheck(report, "provider probes", status, message, ts.Seconds());
	sDoctorLine(sum, status, message, color);
	sSelftestAddDetail(report, idx, String("available: ") + AsString(available));
	sSelftestAddDetail(report, idx, String("missing: ") + AsString(missing));
	sSelftestAddDetail(report, idx, String("manual: ") + AsString(manual));
	sSelftestAddDetail(report, idx, String("unknown: ") + AsString(unknown));
	if(verbose) {
		sDoctorSubline("available: ", AsString(available));
		sDoctorSubline("missing: ", AsString(missing));
		sDoctorSubline("manual: ", AsString(manual));
		sDoctorSubline("unknown: ", AsString(unknown));
	}
}

static void sSelftestLint(PkgDoctorSummary& sum, PkgSelftestReport& report, const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color, bool verbose)
{
	TimeStop ts;
	String report_path = ForceExt(GetTempFileName("pkg-selftest-lint"), ".json");
	PkgInvocation lint;
	sCopyInvocation(lint, inv);
	lint.command = PKG_CMD_LINT;
	lint.atom = String();
	lint.all = true;
	lint.summary = true;
	lint.quiet = true;
	lint.check = false;
	lint.ci = false;
	lint.limit = lint.limit > 0 ? min(lint.limit, 25) : 25;
	lint.use_cache = true;
	lint.baseline.Clear();
	lint.update_baseline = false;
	lint.report = report_path;
	int rc = sLintCommand(lint, repo, policy, color, 0.0);
	PkgLintReport report_lint;
	bool ok = FileExists(report_path) && LoadFromJsonFile(report_lint, report_path);
	DeleteFile(report_path);
	if(!ok) {
		int idx = sSelftestAddCheck(report, "lint quick", "error", "lint quick: unable to read temporary report", ts.Seconds());
		sDoctorLine(sum, "error", "lint quick: unable to read temporary report", color);
		sSelftestAddDetail(report, idx, "temporary lint report could not be read");
		return;
	}
	String msg;
	msg << "lint quick: " << report_lint.errors << " errors, " << report_lint.warnings << " warnings";
	String status = report_lint.errors > 0 || report_lint.warnings > 0 ? "warn" : "ok";
	int idx = sSelftestAddCheck(report, "lint quick", status, msg, ts.Seconds());
	sDoctorLine(sum, status, msg, color);
	sSelftestAddDetail(report, idx, "baseline: " + (report_lint.baseline_path.IsEmpty() ? String("[none]") : report_lint.baseline_path));
	sSelftestAddDetail(report, idx, String("cache used: ") + (report_lint.cache_used ? "yes" : "no"));
	if(report_lint.errors == 0 && report_lint.warnings == 0 && rc != 0) {
		int eidx = sSelftestAddCheck(report, "lint quick exit code", "error", "lint quick returned a nonzero exit code despite zero findings", 0.0);
		sDoctorLine(sum, "error", "lint quick returned a nonzero exit code despite zero findings", color);
		sSelftestAddDetail(report, eidx, "expected exit 0 for zero findings");
	}
}

static bool sWriteSelftestReport(const PkgSelftestReport& report)
{
	bool ok = sWriteSelftestReportFile(report);
	if(!ok)
		Cerr() << "failed to write selftest report: " << report.report_path << "\n";
	return ok;
}

static int sSelftestCommand(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color)
{
	PkgDoctorSummary sum;
	PkgSelftestReport report;
	TimeStop total;
	report.command_line = inv.command_line;
	report.timestamp = GetSysTime();
	report.report_path = inv.report;
	report.format = sSelftestReportFormat(inv.report);
	report.note = "local selftest, not project CI";
	report.strict = inv.strict;
	String group = ToLower(TrimBoth(inv.subcommand));
	if(group.IsEmpty())
		group = "quick";
	report.group = group;

	Cout() << "pkg selftest\n\n";
	if(group == "quick" || group == "all") {
		sSelftestVersionHelp(sum, report, color);
		sSelftestDoctor(sum, report, color);
		sSelftestCache(sum, report, repo, color);
		sSelftestSearch(sum, report, repo, color, inv.verbose);
		sSelftestPlan(sum, report, inv, repo, policy, color, inv.verbose);
		sSelftestProviders(sum, report, inv, repo, policy, color, inv.verbose);
		sSelftestLint(sum, report, inv, repo, policy, color, inv.verbose);
	}
	else if(group == "doctor")
		sSelftestDoctor(sum, report, color);
	else if(group == "cache")
		sSelftestCache(sum, report, repo, color);
	else if(group == "plan")
		sSelftestPlan(sum, report, inv, repo, policy, color, inv.verbose);
	else if(group == "lint")
		sSelftestLint(sum, report, inv, repo, policy, color, inv.verbose);
	else {
		sDoctorLine(sum, "error", "unknown selftest group: " + inv.subcommand, color);
		Cout() << "  available groups: quick doctor cache plan lint all\n";
		sSelftestAddCheck(report, "selftest group", "error", "unknown selftest group: " + inv.subcommand, 0.0);
	}

	Cout() << "\nSummary:\n";
	Cout() << "  " << sum.ok << " ok, " << sum.warn << " warn, " << sum.error << " error\n";
	report.ok = sum.ok;
	report.warn = sum.warn;
	report.info = sum.info;
	report.error = sum.error;
	report.exit_basis = inv.strict ? "warnings and errors" : "errors only";
	report.total_seconds = total.Seconds();
	if(!inv.report.IsEmpty())
		sWriteSelftestReport(report);
	return sum.error > 0 || (inv.strict && sum.warn > 0) ? 1 : 0;
}

static void sPrintResumeState(const PkgTransaction& tx)
{
	if(tx.steps.IsEmpty()) {
		Cout() << "No saved transaction found.\n";
		return;
	}
	if(tx.result != "failed") {
		Cout() << "No saved failed transaction found.\n";
		Cout() << "Last transaction result: " << (tx.result.IsEmpty() ? String("[none]") : tx.result) << "\n";
		return;
	}

	Cout() << "Last failed transaction:\n";
	Cout() << "  command_line: " << (tx.command_line.IsEmpty() ? String("[none]") : tx.command_line) << "\n";
	Cout() << "  target: " << (tx.target.IsEmpty() ? String("[none]") : tx.target) << "\n";
	Cout() << "  compiler: " << (tx.compiler.IsEmpty() ? String("[none]") : tx.compiler) << "\n";
	Cout() << "  linker: " << (tx.linker.IsEmpty() ? String("[none]") : tx.linker) << "\n";
	Cout() << "  toolchain: " << (tx.toolchain.IsEmpty() ? String("[none]") : tx.toolchain) << "\n";
	Cout() << "  build_method: " << (tx.build_method.IsEmpty() ? String("[none]") : tx.build_method) << "\n";
	Cout() << "  jobs: " << tx.jobs << "\n";
	Cout() << "  staged: " << (tx.staged ? String("yes") : String("no")) << "\n";
	if(!tx.staged_runner.IsEmpty())
		Cout() << "  staged_runner: " << tx.staged_runner << "\n";
	Cout() << "  requested_atoms: " << sFmtList(tx.requested_atoms) << "\n";
	Cout() << "  completed_steps: " << sFmtList(tx.completed_steps) << "\n";
	Cout() << "  result: " << tx.result << "\n";
	Cout() << "  failed_index: " << tx.failed_index << "\n";
	if(!tx.failed_step.atom.IsEmpty()) {
		Cout() << "  failed_step: " << tx.failed_step.atom << "\n";
		Cout() << "    path: " << (tx.failed_step.path.IsEmpty() ? String("[none]") : tx.failed_step.path) << "\n";
		Cout() << "    command: " << (tx.failed_step.command.IsEmpty() ? String("[none]") : tx.failed_step.command) << "\n";
		Cout() << "    exit_code: " << tx.failed_step.exit_code << "\n";
		Cout() << "    reason: " << (tx.failed_step.reason.IsEmpty() ? String("[none]") : tx.failed_step.reason) << "\n";
	}
	Cout() << "  timestamp: " << Format(tx.timestamp) << "\n";
	if(!tx.resume_data.IsEmpty())
		Cout() << "  resume_data: " << sFmtList(tx.resume_data) << "\n";
}

static String sBuildArtifactPath(const String& root, const PkgBuildStep& step);
static void sWriteState(const PkgRepository& repo, const PkgPlan& plan, const PkgTransaction& tx, const String& build_status = "built")
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	state.target = sTargetNameText(plan.target);
	if(state.toolchain.IsEmpty())
		state.toolchain = sDefaultTargetProfile(plan.target).toolchain;
	if(tx.steps.IsEmpty())
		return;

	RealizeDirectory(repo.paths.ai_dir);
	for(const PkgBuildStep& step : tx.steps) {
		if(!step.executed && !step.completed && !step.failed)
			continue;
		PkgStateRecord& rec = state.records.Add();
		rec.atom = step.atom;
		rec.target = state.target;
		rec.toolchain = state.toolchain;
		rec.build_status = step.completed && !step.failed ? "built" : step.failed ? "failed" : build_status;
		rec.artifact_path = Nvl(step.output_path, sBuildArtifactPath(repo.root, step));
		rec.build_method = tx.build_method;
		rec.command_line = step.command;
		rec.output_path = rec.artifact_path;
		rec.staged = tx.staged;
		rec.success = step.completed && !step.failed;
		rec.owned = true;
		rec.timestamp = GetSysTime();
		for(const String& s : plan.use.requested)
			rec.selected_use.Add(s);
		for(const String& s : plan.use.declared)
			rec.declared_use.Add(s);
		for(const String& s : plan.use.effective)
			rec.effective_use.Add(s);
		for(const String& s : sPlanUppFlags(plan.upp))
			rec.effective_uppflags.Add(s);
		for(const String& s : plan.providers)
			rec.providers.Add(s);
		if(const PkgPackage* p = repo.Resolve(step.atom).pkg)
			for(const String& s : p->accepts)
				rec.accepted_flags.Add(s);
	}
	StoreAsJsonFile(state, repo.paths.state, true);
}

struct PkgBuildMethod : Moveable<PkgBuildMethod> {
	String name;
	String path;
	String builder;
};

static String sBuildExePath(const String& root)
{
#ifdef flagWIN32
	return AppendFileName(root, "bin/build.exe");
#else
	return AppendFileName(root, "bin/build");
#endif
}

static Vector<PkgBuildMethod> sListBuildMethods(const String& buildexe)
{
	Vector<PkgBuildMethod> methods;
	String out;
	Vector<String> args;
	args.Add("--list-methods");
	if(Sys(buildexe, args, out) < 0 || out.IsEmpty())
		return methods;
	Vector<String> lines = Split(out, '\n');
	for(String line : lines) {
		line = TrimBoth(line);
		if(line.IsEmpty() || !line.StartsWith("["))
			continue;
		int colon = line.Find(':');
		int builder = line.Find("[builder:");
		if(colon < 0 || builder < 0)
			continue;
		int close = line.Find(']', 1);
		PkgBuildMethod& m = methods.Add();
		if(close >= 0 && close < colon)
			m.name = TrimBoth(line.Mid(close + 1, colon - close - 1));
		else
			m.name = TrimBoth(line.Mid(0, colon));
		m.path = TrimBoth(builder > colon ? line.Mid(colon + 1, builder - colon - 1) : line.Mid(colon + 1));
		int bclose = line.ReverseFind(']');
		if(builder >= 0 && bclose > builder)
			m.builder = TrimBoth(line.Mid(builder + 9, bclose - (builder + 9)));
	}
	return methods;
}

static int sBuildMethodScore(const PkgBuildMethod& m, const String& compiler, const String& target)
{
	String c = ToLower(TrimBoth(compiler));
	String n = ToLower(m.name + " " + m.path + " " + m.builder);
	int score = 100;
	if(c == "clang") {
		if(n.Find("clang") >= 0)
			score = 0;
		else
			score = 1000;
	}
	else if(c == "msvc") {
		if(n.Find("msvs") >= 0 || n.Find("msc") >= 0)
			score = 0;
		else
			score = 1000;
	}
	else if(c == "gcc") {
		if(n.Find("gcc") >= 0)
			score = 0;
		else
			score = 1000;
	}
	else if(c == "emcc") {
		if(n.Find("emcc") >= 0 || n.Find("emscripten") >= 0)
			score = 0;
		else
			score = 1000;
	}
	else if(!c.IsEmpty() && c != "auto") {
		if(n.Find(c) >= 0)
			score = 0;
		else
			score = 800;
	}
	if(target == "wasm-browser" || target == "wasm-node") {
		if(n.Find("emcc") >= 0 || n.Find("emscripten") >= 0)
			score -= 50;
	}
	return score;
}

static String sSelectBuildMethod(const PkgRepository& repo, const PkgPlan& plan, const PkgInvocation& inv)
{
	String compiler = !inv.compiler.IsEmpty() ? inv.compiler : sDefaultTargetProfile(plan.target).compiler;
	Vector<PkgBuildMethod> methods = sListBuildMethods(sBuildExePath(repo.root));
	if(methods.IsEmpty())
		return compiler.IsEmpty() ? String() : compiler;
	int best = -1;
	int best_score = INT_MAX;
	for(int i = 0; i < methods.GetCount(); i++) {
		int score = sBuildMethodScore(methods[i], compiler, plan.target);
		if(best < 0 || score < best_score) {
			best = i;
			best_score = score;
		}
	}
	if(best < 0)
		return compiler.IsEmpty() ? String() : compiler;
	return methods[best].path.IsEmpty() ? methods[best].name : methods[best].path;
}

static bool sIsBuildableStep(const PkgGraphNode& node)
{
	if(node.missing || node.ambiguous || node.cycle || node.blocker)
		return false;
	if(node.requested || node.set_member)
		return true;
	return node.provider_added && node.provider_kind == "upp-plugin";
}

static bool sRunBuildCommand(const String& buildexe, const String& method, int jobs, const String& target, const String& cd, String& output)
{
	LocalProcess p;
	p.NoConvertCharset();
	Vector<String> args;
	if(!method.IsEmpty()) {
		args.Add("-m");
		args.Add(method);
	}
	if(jobs > 0) {
		args.Add("-j" + AsString(jobs));
	}
	args.Add(target);
	if(!p.Start2(buildexe, args, nullptr, cd))
		return false;

	output.Clear();
	String so, se;
	while(p.IsRunning()) {
		bool activity = false;
		while(p.Read2(so, se)) {
			if(!so.IsEmpty()) {
				output.Cat(so);
				Cout() << so;
				activity = true;
			}
			if(!se.IsEmpty()) {
				output.Cat(se);
				Cerr() << se;
				activity = true;
			}
			if(so.IsEmpty() && se.IsEmpty())
				break;
		}
		if(!activity)
			Sleep(10);
	}
	while(p.Read2(so, se)) {
		if(!so.IsEmpty()) {
			output.Cat(so);
			Cout() << so;
		}
		if(!se.IsEmpty()) {
			output.Cat(se);
			Cerr() << se;
		}
	}
	return p.GetExitCode() == 0;
}

static String sBuildArtifactPath(const String& root, const PkgBuildStep& step)
{
	String name = step.atom;
	if(name.IsEmpty())
		name = step.path;
	if(name.IsEmpty())
		return String();
	if(name.EndsWith(".upp") || name.EndsWith(".xupp"))
		name = GetFileTitle(name);
	else if(name.Find('/') >= 0 || name.Find('\\') >= 0)
		name = GetFileTitle(name);
	if(name.IsEmpty())
		return String();
	return AppendFileName(AppendFileName(root, "bin"), GetFileTitle(name) + GetExeExt());
}

static bool sWouldOverwriteRunningExecutable(const String& root, const PkgBuildStep& step, String& reason)
{
#ifdef flagWIN32
	String running = NormalizePath(GetExeFilePath());
	if(running.IsEmpty() || step.path.IsEmpty())
		return false;
	String out = NormalizePath(sBuildArtifactPath(root, step));
	if(out.IsEmpty())
		return false;
	if(ToLower(running) == ToLower(out)) {
		reason = "Refusing to rebuild running executable " + out + " on Windows; use a copied runner or staged build output.";
		return true;
	}
#endif
	(void)step;
	return false;
}

static const PkgGraphNode* sFindGraphNode(const PkgGraph& graph, const String& key);

static Vector<PkgBuildStep> sTransactionSteps(const PkgRepository& repo, const PkgPlan& plan)
{
	Vector<PkgBuildStep> steps;
	for(const String& key : plan.graph.order) {
		const PkgGraphNode *node = sFindGraphNode(plan.graph, key);
		if(!node || !sIsBuildableStep(*node))
			continue;
		PkgBuildStep& step = steps.Add();
		step.atom = node->atom.IsEmpty() ? node->path : node->atom;
		step.path = node->path;
		step.output_path = sBuildArtifactPath(repo.root, step);
		step.reason = node->reason;
		step.requested = node->requested;
		step.provider_added = node->provider_added;
		step.set_member = node->set_member;
		step.root = node->requested || node->set_member || node->provider_added;
	}
	return steps;
}

static String sStageRunnerPath()
{
	return ForceExt(GetTempFileName("pkg-run"), GetExeExt());
}

static String sStageCleanupCommand(const String& path)
{
	String cmd;
	cmd << "ping 127.0.0.1 -n 2 >nul & del /f /q \"" << path << "\"";
	return cmd;
}

static void sScheduleStageCleanup(const String& path)
{
#ifdef flagWIN32
	if(path.IsEmpty() || !FileExists(path))
		return;
	LocalProcess p;
	p.NoConvertCharset();
	Vector<String> args;
	args.Add("/c");
	args.Add(sStageCleanupCommand(path));
	if(p.Start2("cmd", args))
		p.Detach();
#else
	(void)path;
#endif
}

static bool sLaunchStagedRunner(const PkgInvocation& inv, const PkgRepository& repo, const PkgPlan& plan, const String& buildexe, String& staged_runner, String& reason)
{
#ifdef flagWIN32
	staged_runner = sStageRunnerPath();
	if(staged_runner.IsEmpty()) {
		reason = "unable to create staged runner path";
		return false;
	}
	if(!FileCopy(GetExeFilePath(), staged_runner)) {
		reason = "unable to copy current executable to staged runner: " + staged_runner;
		return false;
	}
	Vector<String> args;
	for(const String& s : inv.argv)
		args.Add(s);
	args.Add("--staged");
	LocalProcess p;
	p.NoConvertCharset();
	if(!p.Start2(staged_runner, args, nullptr, repo.root)) {
		reason = "unable to launch staged runner: " + staged_runner;
		DeleteFile(staged_runner);
		return false;
	}
	p.Detach();
	Cout() << "[staged] copied runner: " << staged_runner << "\n";
	Cout() << "[staged] delegated command: " << staged_runner << " " << sJoin(args) << "\n";
	Cout() << "[staged] cleanup will be attempted after the runner exits.\n";
	(void)buildexe;
	(void)plan;
	return true;
#else
	(void)inv; (void)repo; (void)plan; (void)buildexe; (void)staged_runner; (void)reason;
	return false;
#endif
}

static bool sPlanHasBlockingRows(const PkgPlan& plan, String& reason)
{
	for(const PkgPlanItem& item : plan.items) {
		if(item.blocker || item.ambiguous || item.status == 'F') {
			String r = item.reason;
			if(r.IsEmpty())
				r = sPlanStatusText(item.status);
			reason = item.atom;
			if(!r.IsEmpty())
				reason << ": " << r;
			return true;
		}
	}
	for(const PkgResolveIssue& issue : plan.graph.issues) {
		if(issue.kind == "missing" || issue.kind == "missing-provider" || issue.kind == "ambiguous" || issue.kind == "cycle") {
			reason = issue.atom;
			if(!issue.reason.IsEmpty())
				reason << ": " << issue.reason;
			return true;
		}
	}
	return false;
}

static String sBuildTransactionPath(const PkgRepository& repo);
static void sStoreTransaction(const PkgRepository& repo, const PkgTransaction& tx);
static bool sLoadTransaction(const PkgRepository& repo, PkgTransaction& tx);

static void sTransactionTail(PkgTransaction& tx, const PkgBuildStep& step, bool ok)
{
	tx.completed_steps.Clear();
	for(const PkgBuildStep& s : tx.steps)
		if(s.completed)
			tx.completed_steps.Add(s.atom);
	tx.resume_data.Clear();
	for(const String& s : tx.completed_steps)
		tx.resume_data.Add(s);
	if(!ok)
		tx.failed_step = step;
}

static PkgExecutionResult sExecuteTransaction(const PkgInvocation& inv, const PkgRepository& repo, const PkgPlan& plan, bool color, bool resume_mode)
{
	PkgExecutionResult result;
	PkgTransaction tx;
	tx.command_line = inv.command_line;
	tx.target = sTargetNameText(plan.target);
	tx.provider = inv.provider;
	tx.compiler = inv.compiler;
	tx.linker = inv.linker;
	tx.toolchain = sDefaultTargetProfile(plan.target).toolchain;
	tx.jobs = inv.jobs > 0 ? inv.jobs : max(1, CPU_Cores() + 2);
	tx.pretend = inv.pretend;
	tx.ask = inv.ask;
	tx.resume = resume_mode;
	tx.staged = inv.staged;
	tx.staged_runner = inv.staged ? GetExeFilePath() : String();
	tx.keep_going = inv.keep_going;
	tx.skip_first = inv.skip_first;
	tx.build_method = sSelectBuildMethod(repo, plan, inv);
	tx.timestamp = GetSysTime();
	tx.requested_atoms.Add(plan.atom);
	tx.steps = sTransactionSteps(repo, plan);

	if(resume_mode) {
		PkgTransaction prev;
		if(!sLoadTransaction(repo, prev) || prev.result != "failed" || prev.steps.IsEmpty()) {
			Cout() << "No saved failed transaction found.\n";
			result.ok = true;
			result.executed = false;
			return result;
		}
		tx.command_line = prev.command_line;
		tx.target = prev.target;
		tx.provider = prev.provider;
		tx.compiler = prev.compiler;
		tx.linker = prev.linker;
		tx.toolchain = prev.toolchain;
		tx.build_method = prev.build_method;
		tx.result = prev.result;
		tx.jobs = prev.jobs;
		tx.failed_index = prev.failed_index;
		tx.pretend = prev.pretend;
		tx.ask = prev.ask;
		tx.resume = prev.resume;
		tx.keep_going = prev.keep_going;
		tx.skip_first = prev.skip_first;
		tx.requested_atoms.Clear();
		for(const String& s : prev.requested_atoms)
			tx.requested_atoms.Add(s);
		tx.completed_steps.Clear();
		for(const String& s : prev.completed_steps)
			tx.completed_steps.Add(s);
		tx.resume_data.Clear();
		for(const String& s : prev.resume_data)
			tx.resume_data.Add(s);
		tx.failed_step = prev.failed_step;
		tx.steps.Clear();
		for(const PkgBuildStep& s : prev.steps)
			tx.steps.Add(s);
		tx.timestamp = prev.timestamp;
		tx.resume = true;
		tx.pretend = inv.pretend;
		tx.ask = inv.ask;
		tx.keep_going = inv.keep_going;
		tx.skip_first = inv.skip_first;
		tx.staged = prev.staged;
		tx.staged_runner = prev.staged_runner;
		tx.timestamp = GetSysTime();
		result.resumed = true;
		tx.command_line = inv.command_line;
		tx.result = "resuming";
	}
	if(tx.steps.IsEmpty()) {
		String blocker_reason;
		if(!resume_mode && sPlanHasBlockingRows(plan, blocker_reason)) {
			Cout() << sAnsi("31;1", "[blocked]", color) << " build refused: " << blocker_reason << "\n";
			tx.result = "blocked";
			sTransactionTail(tx, PkgBuildStep(), false);
			sStoreTransaction(repo, tx);
			result.ok = false;
			result.executed = false;
			result.message = blocker_reason;
			return result;
		}
		Cout() << "No buildable steps selected.\n";
		tx.result = "nothing-to-do";
		sStoreTransaction(repo, tx);
		result.ok = true;
		result.executed = false;
		return result;
	}

	if(inv.pretend) {
		Cout() << "Executing build plan...\n";
		Cout() << "  build method: " << (tx.build_method.IsEmpty() ? String("[none]") : tx.build_method) << "\n";
		Cout() << "  jobs: " << tx.jobs << "\n";
		Cout() << "  steps: " << tx.steps.GetCount() << "\n";
		for(int i = 0; i < tx.steps.GetCount(); i++) {
			const PkgBuildStep& s = tx.steps[i];
			Cout() << "  [" << (i + 1) << "/" << tx.steps.GetCount() << "] " << s.atom << "\n";
		}
		tx.result = "pretend";
		sStoreTransaction(repo, tx);
		result.ok = true;
		result.executed = false;
		result.message = "pretend";
		return result;
	}

	Cout() << "Executing build plan...\n";
	Cout() << "  build method: " << (tx.build_method.IsEmpty() ? String("[none]") : tx.build_method) << "\n";
	Cout() << "  jobs: " << tx.jobs << "\n";
	Cout() << "  steps: " << tx.steps.GetCount() << "\n\n";

	String buildexe = sBuildExePath(repo.root);
	if(!FileExists(buildexe)) {
		Cout() << sAnsi("31;1", "[failed]", color) << " build executable not found: " << buildexe << "\n";
		tx.result = "failed";
		tx.failed_index = 0;
		if(!tx.steps.IsEmpty())
			tx.failed_step = tx.steps[0];
		sTransactionTail(tx, tx.failed_step, false);
		sStoreTransaction(repo, tx);
		result.ok = false;
		result.executed = false;
		result.failed_index = 0;
		result.failed_atom = tx.failed_step.atom;
		result.failed_step = tx.failed_step;
		result.message = "build executable missing";
		return result;
	}

	for(PkgBuildStep& step : tx.steps)
		step.output_path = Nvl(step.output_path, sBuildArtifactPath(repo.root, step));

	int start = 0;
	if(tx.resume && tx.skip_first)
		start = min(tx.failed_index + 1, tx.steps.GetCount());
	else if(tx.resume && tx.failed_index >= 0)
		start = min(tx.failed_index, tx.steps.GetCount());
	else if(tx.skip_first)
		start = 1;

	if(start >= tx.steps.GetCount()) {
		Cout() << "Nothing left to resume.\n";
		tx.result = "nothing-to-do";
		sStoreTransaction(repo, tx);
		result.ok = true;
		result.executed = false;
		return result;
	}

	int first_failure = -1;
	for(int i = start; i < tx.steps.GetCount(); i++) {
		PkgBuildStep& step = tx.steps[i];
		String step_target = step.path.IsEmpty() ? step.atom : step.path;
		String self_reason;
		if(sWouldOverwriteRunningExecutable(repo.root, step, self_reason)) {
			if(!inv.staged && sLaunchStagedRunner(inv, repo, plan, buildexe, tx.staged_runner, self_reason)) {
				tx.staged = true;
				tx.result = "staged";
				tx.failed_index = -1;
				tx.failed_step = PkgBuildStep();
				sTransactionTail(tx, PkgBuildStep(), true);
				sStoreTransaction(repo, tx);
				result.ok = true;
				result.executed = false;
				result.staged = true;
				result.staged_runner = tx.staged_runner;
				result.message = "staged";
				return result;
			}
			Cout() << sAnsi("31;1", "[refused]", color) << " " << self_reason << "\n";
			tx.result = "refused";
			tx.failed_index = i;
			tx.failed_step = step;
			sTransactionTail(tx, step, false);
			sStoreTransaction(repo, tx);
			result.ok = false;
			result.executed = false;
			result.failed_index = i;
			result.failed_atom = step.atom;
			result.failed_step = step;
			result.message = self_reason;
			return result;
		}
		Cout() << ">>> Building (" << (i - start + 1) << " of " << (tx.steps.GetCount() - start) << ") " << step.atom << "\n";
		Vector<String> cmd_args;
		if(!tx.build_method.IsEmpty()) {
			cmd_args.Add("-m");
			cmd_args.Add(tx.build_method);
		}
		if(tx.jobs > 0)
			cmd_args.Add("-j" + AsString(tx.jobs));
		cmd_args.Add(step_target);
		step.command = buildexe + " " + sJoin(cmd_args);
		step.build_method = tx.build_method;
		step.executed = true;
		step.result = "running";
		String cmd_output;
		bool ok = sRunBuildCommand(buildexe, tx.build_method, tx.jobs, step_target, repo.root, cmd_output);
		step.exit_code = ok ? 0 : 1;
		step.completed = ok;
		step.failed = !ok;
		step.staged = tx.staged;
		step.result = ok ? "success" : "failed";
		if(ok) {
			tx.completed_steps.Add(step.atom);
			Cout() << sAnsi("32;1", "[ok]", color) << " " << step.atom << "\n";
		}
		else {
			if(first_failure < 0) {
				first_failure = i;
				tx.failed_index = i;
				tx.failed_step = step;
			}
			Cout() << sAnsi("31;1", "[failed]", color) << " " << step.atom << "\n";
			if(!inv.keep_going)
				break;
		}
		sTransactionTail(tx, tx.failed_step, ok);
		tx.result = first_failure >= 0 ? "failed" : "running";
		sStoreTransaction(repo, tx);
		Cout() << "\n";
	}

	if(first_failure >= 0) {
		tx.result = "failed";
		tx.failed_index = first_failure;
		sTransactionTail(tx, tx.failed_step, false);
		sStoreTransaction(repo, tx);
		if(!resume_mode)
			sWriteState(repo, plan, tx, "failed");
		if(tx.staged)
			sScheduleStageCleanup(tx.staged_runner.IsEmpty() ? GetExeFilePath() : tx.staged_runner);
		Cout() << "Saved resume state at: " << sBuildTransactionPath(repo) << "\n";
		result.ok = false;
		result.executed = true;
		result.failed_index = first_failure;
		result.failed_atom = tx.failed_step.atom;
		result.failed_step = tx.failed_step;
		result.message = "build failed";
		result.completed_steps.Clear();
		for(const String& s : tx.completed_steps)
			result.completed_steps.Add(s);
		result.resume_data.Clear();
		for(const String& s : tx.resume_data)
			result.resume_data.Add(s);
		return result;
	}

	tx.result = "success";
	sTransactionTail(tx, PkgBuildStep(), true);
	sStoreTransaction(repo, tx);
	if(!resume_mode)
		sWriteState(repo, plan, tx, "built");
	if(tx.staged)
		sScheduleStageCleanup(tx.staged_runner.IsEmpty() ? GetExeFilePath() : tx.staged_runner);
	Cout() << "Build plan completed successfully.\n";
	result.ok = true;
	result.executed = true;
	result.message = "success";
	result.completed_steps.Clear();
	for(const String& s : tx.completed_steps)
		result.completed_steps.Add(s);
	result.resume_data.Clear();
	for(const String& s : tx.resume_data)
		result.resume_data.Add(s);
	return result;
}

static String sBuildTransactionPath(const PkgRepository& repo)
{
	return repo.paths.transaction;
}

static void sStoreTransaction(const PkgRepository& repo, const PkgTransaction& tx)
{
	RealizeDirectory(GetFileFolder(sBuildTransactionPath(repo)));
	StoreAsJsonFile(tx, sBuildTransactionPath(repo), true);
}

static bool sLoadTransaction(const PkgRepository& repo, PkgTransaction& tx)
{
	String path = sBuildTransactionPath(repo);
	if(!FileExists(path))
		return false;
	LoadFromJsonFile(tx, path);
	return true;
}

static bool sRecordMatchesQuery(const PkgStateRecord& rec, const String& query)
{
	String q = TrimBoth(query);
	if(q.IsEmpty())
		return true;
	String h = ToLower(rec.atom + " " + rec.target + " " + rec.toolchain + " " + rec.build_method + " " + rec.artifact_path + " " + rec.output_path + " " + rec.command_line);
	return h.Find(ToLower(q)) >= 0;
}

static String sCleanupPath(const PkgStateRecord& rec)
{
	String path = Nvl(rec.output_path, rec.artifact_path);
	return NormalizePath(path);
}

static String sCurrentExecutablePath()
{
	return NormalizePath(GetExeFilePath());
}

static String sBinArtifactRoot(const PkgRepository& repo)
{
	return NormalizePath(AppendFileName(repo.paths.root, "bin"));
}

static bool sPathIsOwnedArtifact(const PkgRepository& repo, const PkgStateRecord& rec)
{
	String path = sCleanupPath(rec);
	if(path.IsEmpty())
		return false;
	String root = sBinArtifactRoot(repo);
	if(root.IsEmpty())
		return false;
	if(!path.StartsWith(root))
		return false;
	return true;
}

static bool sCleanupIsCurrentExecutable(const PkgRepository& repo, const PkgStateRecord& rec)
{
	String path = sCleanupPath(rec);
	String current = sCurrentExecutablePath();
	if(path.IsEmpty() || current.IsEmpty())
		return false;
	return ToLower(path) == ToLower(current);
}

static bool sCleanupIsProtectedTool(const PkgStateRecord& rec)
{
	String path = sCleanupPath(rec);
	if(path.IsEmpty())
		return false;
	String title = ToLower(GetFileTitle(path));
	return title == "build" || title == "build.exe" || title == "pkg" || title == "pkg.exe" || title == "umk" || title == "umk.exe";
}

static void sRemoveStateRecordPaths(PkgState& state, const Vector<String>& removed)
{
	if(removed.IsEmpty())
		return;
	for(int i = state.records.GetCount() - 1; i >= 0; --i) {
		String path = sCleanupPath(state.records[i]);
		if(!path.IsEmpty() && FindIndex(removed, path) >= 0)
			state.records.Remove(i);
	}
}

struct PkgCleanupEntry : Moveable<PkgCleanupEntry> {
	const PkgStateRecord *rec = nullptr;
	String path;
	String reason;
	bool removable = false;
	bool current = false;
	bool owned = false;
};

static String sCleanupRemoveTag(const PkgCleanupEntry& e)
{
	if(e.current)
		return "[keep         ]";
	if(e.removable)
		return "[clean  R     ]";
	return "[keep         ]";
}

static Vector<PkgCleanupEntry> sCollectCleanupEntries(const PkgRepository& repo, const PkgState& state, const String& query, bool all, const Vector<String>& live_paths, bool depclean)
{
	Vector<PkgCleanupEntry> entries;
	Index<String> seen;
	for(int i = state.records.GetCount() - 1; i >= 0; --i) {
		const PkgStateRecord& rec = state.records[i];
		String path = sCleanupPath(rec);
		if(!path.IsEmpty() && seen.Find(path) >= 0)
			continue;
		if(!path.IsEmpty())
			seen.Add(path);
		PkgCleanupEntry& e = entries.Add();
		e.rec = &rec;
		e.path = path;
		e.current = sCleanupIsCurrentExecutable(repo, rec);
		e.owned = sPathIsOwnedArtifact(repo, rec);
		if(!rec.success && rec.build_status != "built") {
			e.reason = "not a successful built artifact";
			continue;
		}
		if(!e.owned) {
			e.reason = "artifact is not owned by pkg";
			continue;
		}
		if(e.path.IsEmpty()) {
			e.reason = "no recorded artifact path";
			continue;
		}
		if(!FileExists(e.path)) {
			e.reason = "artifact file is missing";
			continue;
		}
		if(e.current) {
			e.reason = "current executable";
			continue;
		}
		if(sCleanupIsProtectedTool(rec)) {
			e.reason = "protected tool artifact";
			continue;
		}
		if(!all && !sRecordMatchesQuery(rec, query)) {
			e.reason = "does not match query";
			continue;
		}
		if(depclean && live_paths.IsEmpty()) {
			e.reason = "dependency graph unavailable";
			continue;
		}
		if(depclean && !live_paths.IsEmpty() && FindIndex(live_paths, e.path) >= 0) {
			e.reason = "kept by current dependency graph";
			continue;
		}
		e.removable = true;
		e.reason = depclean ? "stale artifact" : "recorded artifact";
	}
	return pick(entries);
}

static void sPrintCleanupEntries(const Vector<PkgCleanupEntry>& entries, bool color, bool depclean)
{
	int remove = 0;
	int keep = 0;
	Cout() << (depclean ? "Dependency cleanup plan:\n\n" : "Cleanup plan:\n\n");
	for(const PkgCleanupEntry& e : entries) {
		String tag = e.removable ? sCleanupRemoveTag(e) : "[keep         ]";
		String code = e.removable ? "33;1" : "36";
		if(e.current)
			code = "32;1";
		Cout() << "  " << sAnsi(code, tag, color) << " " << (e.rec ? e.rec->atom : String("[none]")) << "\n";
		Cout() << "       path: " << (e.path.IsEmpty() ? String("[none]") : e.path) << "\n";
		Cout() << "       reason: " << e.reason << "\n";
		if(e.removable)
			remove++;
		else
			keep++;
	}
	Cout() << "\nTotal: " << remove << " remove, " << keep << " keep, 0 errors\n";
}

static bool sApplyCleanupEntries(const PkgRepository& repo, Vector<PkgCleanupEntry>& entries, bool color, bool ask, bool dry_run, bool depclean)
{
	(void)depclean;
	Vector<String> removed_paths;
	bool any_removed = false;
	for(const PkgCleanupEntry& e : entries) {
		if(!e.removable)
			continue;
		if(dry_run)
			continue;
		if(!ask)
			continue;
		Cout() << "  " << sAnsi("33;1", "[remove]", color) << " " << (e.rec ? e.rec->atom : String("[none]")) << "\n";
		Cout() << "       path: " << (e.path.IsEmpty() ? String("[none]") : e.path) << "\n";
		Cout() << "       action: confirm required\n";
		if(!sPromptYesNo()) {
			Cout() << "       skipped\n\n";
			continue;
		}
		if(DeleteFile(e.path)) {
			Cout() << "       removed\n\n";
			removed_paths.Add(e.path);
			any_removed = true;
		}
		else {
			Cout() << "       remove failed\n\n";
		}
	}
	if(any_removed) {
		PkgState state;
		LoadFromJsonFile(state, repo.paths.state);
		sRemoveStateRecordPaths(state, removed_paths);
		RealizeDirectory(repo.paths.ai_dir);
		StoreAsJsonFile(state, repo.paths.state, true);
	}
	return any_removed;
}

static PkgPlan sBuildPlan(const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgLocalPolicy& policy, bool color);

static void sPrintBins(const PkgInvocation& inv, const PkgRepository& repo, bool color)
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	Vector<const PkgStateRecord*> bins;
	Index<String> seen;
	for(int i = state.records.GetCount() - 1; i >= 0; --i) {
		const PkgStateRecord& rec = state.records[i];
		if(rec.build_status == "planned")
			continue;
		if(!sRecordMatchesQuery(rec, inv.atom))
			continue;
		String key = rec.atom + "\n" + rec.target + "\n" + rec.build_method + "\n" + Nvl(rec.output_path, rec.artifact_path);
		if(seen.Find(key) >= 0)
			continue;
		seen.Add(key);
		bins.Add(&rec);
	}
	Sort(bins, [](const PkgStateRecord* a, const PkgStateRecord* b) {
		if(a->atom != b->atom)
			return a->atom < b->atom;
		if(a->target != b->target)
			return a->target < b->target;
		return a->timestamp > b->timestamp;
	});

	Cout() << "Known build artifacts:\n\n";
	if(bins.IsEmpty()) {
		Cout() << "  No build artifacts recorded yet.\n";
		return;
	}
	for(const PkgStateRecord* rec : bins) {
		if(!rec)
			continue;
		String status = rec->success || rec->build_status == "built" ? "ok" : rec->build_status == "failed" ? "warn" : "info";
		String code = status == "ok" ? "32;1" : status == "warn" ? "33;1" : "36";
		Cout() << "  " << sAnsi(code, String("[") + status + "]", color) << " " << rec->atom << "\n";
		Cout() << "       target: " << (rec->target.IsEmpty() ? String("[none]") : rec->target) << "\n";
		Cout() << "       method: " << (rec->build_method.IsEmpty() ? String("[none]") : rec->build_method) << "\n";
		Cout() << "       path: " << Nvl(rec->output_path, rec->artifact_path) << "\n";
		if(!rec->command_line.IsEmpty())
			Cout() << "       command: " << rec->command_line << "\n";
		if(rec->staged)
			Cout() << "       staged: yes\n";
		if(!rec->build_status.IsEmpty())
			Cout() << "       status: " << rec->build_status << "\n";
		if(!rec->selected_use.IsEmpty() || !rec->effective_use.IsEmpty()) {
			const Vector<String>& use = rec->effective_use.IsEmpty() ? rec->selected_use : rec->effective_use;
			Cout() << "       USE: " << sFmtList(use) << "\n";
		}
		if(!rec->providers.IsEmpty())
			Cout() << "       providers: " << sFmtList(rec->providers) << "\n";
		Cout() << "\n";
	}
}

static bool sIsCleanupQueryAll(const PkgInvocation& inv)
{
	return inv.all;
}

static Vector<String> sDepcleanLivePaths(const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgPlan& plan)
{
	Vector<String> live;
	Index<String> seen;
	auto add_path = [&](const String& path) {
		String p = NormalizePath(path);
		if(p.IsEmpty())
			return;
		if(seen.Find(p) >= 0)
			return;
		seen.Add(p);
		live.Add(p);
	};

	for(const String& s : sLoadSet(repo.paths, "world", repo.paths.world))
		add_path(repo.Resolve(s).path);
	for(const String& s : sLoadSet(repo.paths, "system", repo.paths.system_set))
		add_path(repo.Resolve(s).path);
	for(const String& s : sLoadSet(repo.paths, "toolchain", repo.paths.toolchain_set))
		add_path(repo.Resolve(s).path);
	for(const PkgGraphNode& node : plan.graph.nodes) {
		if(!node.resolved)
			continue;
		add_path(node.path.IsEmpty() ? node.atom : node.path);
	}
	(void)inv;
	(void)state;
	return pick(live);
}

static void sCleanupArtifacts(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color, bool depclean)
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	PkgPlan plan;
	Vector<String> live_paths;
	if(depclean) {
		PkgInvocation depinv;
		depinv.command = PKG_CMD_PLAN;
		depinv.atom = "@world";
		depinv.target = inv.target;
		depinv.provider = inv.provider;
		depinv.compiler = inv.compiler;
		depinv.linker = inv.linker;
		depinv.profile = inv.profile;
		depinv.root = inv.root;
		depinv.sysroot = inv.sysroot;
		for(const String& s : inv.use_args)
			depinv.use_args.Add(s);
		depinv.ask = false;
		depinv.pretend = true;
		depinv.verbose = false;
		depinv.install = false;
		depinv.depclean = false;
		depinv.update = inv.update;
		depinv.deep = inv.deep;
		depinv.newuse = inv.newuse;
		depinv.changed_use = inv.changed_use;
		plan = sBuildPlan(depinv, repo, state, policy, color);
		live_paths = sDepcleanLivePaths(inv, repo, state, plan);
	}

	bool query_all = sIsCleanupQueryAll(inv);
	bool has_query = !TrimBoth(inv.atom).IsEmpty();
	if(!depclean && !query_all && !has_query) {
		Cout() << "Specify a package name or --all.\n";
		return;
	}

	Vector<PkgCleanupEntry> entries = sCollectCleanupEntries(repo, state, inv.atom, depclean ? true : query_all, live_paths, depclean);
	if(entries.IsEmpty()) {
		Cout() << "No recorded artifacts match: " << (depclean ? String("[depclean]") : (query_all ? String("[all]") : inv.atom)) << "\n";
		return;
	}

	sPrintCleanupEntries(entries, color, depclean);
	if(inv.pretend)
		return;

	if(!inv.ask) {
		Cout() << "\nUse --ask to delete the removable artifacts.\n";
		return;
	}

	if(!sApplyCleanupEntries(repo, entries, color, true, false, depclean))
		Cout() << "\nNo artifacts were removed.\n";
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
	item.atom = pkg ? pkg->name : atom;
	item.path = pkg ? pkg->path : String();
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
		for(const String& s : pkg->accepts)
			item.accepts.Add(s);
		for(const String& s : pkg->uses)
			item.uses.Add(s);
		for(const String& s : pkg->mainconfig)
			if(!s.IsEmpty())
				item.mainconfig.Add(s);
	}
}

static int sGraphFindNodeIndex(const PkgGraph& graph, const String& key)
{
	for(int i = 0; i < graph.nodes.GetCount(); i++)
		if(graph.nodes[i].key == key)
			return i;
	return -1;
}

static PkgGraphNode& sGraphEnsureNode(PkgGraph& graph, const String& key)
{
	int i = sGraphFindNodeIndex(graph, key);
	if(i >= 0)
		return graph.nodes[i];
	PkgGraphNode& node = graph.nodes.Add();
	node.key = key;
	return node;
}

static void sGraphAddIssue(PkgGraph& graph, const String& kind, const String& from, const String& atom, const String& reason, const Vector<String>& candidates = Vector<String>())
{
	PkgResolveIssue& issue = graph.issues.Add();
	issue.kind = kind;
	issue.from = from;
	issue.atom = atom;
	issue.reason = reason;
	Index<String> seen;
	for(const String& s : candidates)
		if(seen.Find(s) < 0) {
			seen.Add(s);
			issue.candidates.Add(s);
		}
}

static void sGraphAddEdge(PkgGraph& graph, const String& from, const String& to, const String& kind, const String& reason, bool missing = false)
{
	PkgGraphEdge& edge = graph.edges.Add();
	edge.from = from;
	edge.to = to;
	edge.kind = kind;
	edge.reason = reason;
	edge.missing = missing;
}

static String sGraphMissingKey(const String& atom)
{
	return "missing:" + ToLower(TrimBoth(atom));
}

static String sGraphAmbiguousKey(const String& atom)
{
	return "ambiguous:" + ToLower(TrimBoth(atom));
}

static String sGraphCycleKey(const String& from, const String& atom)
{
	return "cycle:" + ToLower(TrimBoth(from)) + "->" + ToLower(TrimBoth(atom));
}

static void sGraphPopulateResolvedNode(PkgGraphNode& node, const PkgPackage& pkg, const String& reason, const String& inclusion, int depth, bool requested, bool provider_added, bool set_member)
{
	if(node.key.IsEmpty())
		node.key = pkg.path;
	node.atom = pkg.name;
	node.path = pkg.path;
	if(node.inclusion.IsEmpty() || requested)
		node.inclusion = inclusion;
	if(node.reason.IsEmpty() || requested)
		node.reason = reason;
	node.repository = pkg.nest;
	node.description = pkg.description;
	if(node.depth == 0 || depth < node.depth)
		node.depth = depth;
	node.resolved = true;
	node.provider_added = node.provider_added || provider_added;
	node.set_member = node.set_member || set_member;
	node.requested = node.requested || requested;
	if(node.accepts.IsEmpty())
		for(const String& s : pkg.accepts)
			node.accepts.Add(s);
	if(node.uses.IsEmpty())
		for(const String& s : pkg.uses)
			node.uses.Add(s);
	if(node.mainconfig.IsEmpty())
		for(const String& s : pkg.mainconfig)
			if(!s.IsEmpty())
				node.mainconfig.Add(s);
}

static void sGraphAppendOrder(PkgGraph& graph, const String& key)
{
	if(FindIndex(graph.order, key) < 0)
		graph.order.Add(key);
}

static const PkgPackage *sResolveExactPackage(const PkgRepository& repo, const String& atom)
{
	PkgLookupResult lookup = repo.Resolve(atom);
	if(lookup.pkg)
		return lookup.pkg;
	String trimmed = TrimBoth(atom);
	if(trimmed.IsEmpty() || !lookup.ambiguous)
		return nullptr;
	String lower = ToLower(trimmed);
	const PkgPackage *exact = nullptr;
	for(const PkgPackage* c : lookup.candidates) {
		if(!c)
			continue;
		String name = ToLower(c->name);
		String path = ToLower(c->path);
		String title = ToLower(GetFileTitle(c->path));
		if(name == lower || title == lower || path == lower || sPackageResolvePath(c->path) == sPackageResolvePath(trimmed)) {
			if(exact)
				return nullptr;
			exact = c;
		}
	}
	return exact;
}

static void sGraphMarkPackageNode(PkgGraph& graph, const String& key, char status, const String& atom, const String& reason, int depth, bool requested, bool provider_added, bool set_member)
{
	PkgGraphNode& node = sGraphEnsureNode(graph, key);
	node.key = key;
	node.atom = atom;
	node.status = status;
	node.depth = depth;
	node.reason = reason;
	node.blocker = status == 'B';
	node.missing = status == 'F';
	node.ambiguous = false;
	node.cycle = false;
	node.resolved = false;
	node.requested = requested;
	node.provider_added = provider_added;
	node.set_member = set_member;
}

static String sResolveGraphAtom(PkgGraph& graph, const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgPlan& plan, const PkgProviderPlan& provider_plan, const PkgLocalPolicy& policy,
                                const String& atom, const String& reason, const String& inclusion, int depth, bool requested, bool provider_added, bool set_member, Vector<String>& stack);
static bool sShouldSkipPlannedAtom(const PkgInvocation& inv, const PkgPlan& plan, const PkgRepository& repo, const PkgState& state, const String& atom);

static String sResolveGraphSet(PkgGraph& graph, const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgPlan& plan, const PkgProviderPlan& provider_plan, const PkgLocalPolicy& policy,
                              const String& set_name, const String& path, const String& reason, const String& inclusion, int depth, bool requested, Vector<String>& stack)
{
	(void)inv;
	(void)state;
	(void)inclusion;
	Vector<String> set = sLoadSet(repo.paths, set_name, path);
	if(set.IsEmpty()) {
		String key = sGraphMissingKey(set_name);
		sGraphMarkPackageNode(graph, key, 'F', set_name, reason.IsEmpty() ? String(set_name) + " set is empty" : reason, depth, requested, false, true);
		PkgGraphNode& node = sGraphEnsureNode(graph, key);
		node.missing = true;
		node.blocker = true;
		node.reason = reason.IsEmpty() ? String(set_name) + " set is empty" : reason;
		sGraphAddIssue(graph, "missing-set", Null, set_name, node.reason);
		sGraphAppendOrder(graph, key);
		return key;
	}
	for(const String& s : set)
		sResolveGraphAtom(graph, inv, repo, state, plan, provider_plan, policy, s, reason.IsEmpty() ? String("member of @") + set_name : reason, "set member", depth, requested, false, true, stack);
	return Null;
}

static String sResolveGraphAtom(PkgGraph& graph, const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgPlan& plan, const PkgProviderPlan& provider_plan, const PkgLocalPolicy& policy,
                                const String& atom, const String& reason, const String& inclusion, int depth, bool requested, bool provider_added, bool set_member, Vector<String>& stack)
{
	(void)inv;
	(void)state;
	String trimmed = TrimBoth(atom);
	if(trimmed.IsEmpty())
		return Null;

	if(trimmed == "@world" || trimmed == "world")
		return sResolveGraphSet(graph, inv, repo, state, plan, provider_plan, policy, "world", repo.paths.world,
		                        String(), inclusion, depth, requested, stack);
	if(trimmed == "@system" || trimmed == "system")
		return sResolveGraphSet(graph, inv, repo, state, plan, provider_plan, policy, "system", repo.paths.system_set,
		                        String(), inclusion, depth, requested, stack);
	if(trimmed == "@toolchain" || trimmed == "toolchain")
		return sResolveGraphSet(graph, inv, repo, state, plan, provider_plan, policy, "toolchain", repo.paths.toolchain_set,
		                        String(), inclusion, depth, requested, stack);

	if(trimmed.StartsWith("virtual/")) {
		const PkgProviderResolution *res = sFindProviderResolution(provider_plan, trimmed);
		if(!res || res->provider_id.IsEmpty() || res->external_package.IsEmpty() || res->provider_kind == "manual" || res->probe_status == "missing") {
			String key = sGraphMissingKey(trimmed);
			sGraphMarkPackageNode(graph, key, 'F', trimmed, reason.IsEmpty() ? String("no provider available") : reason, depth, requested, false, false);
			PkgGraphNode& node = sGraphEnsureNode(graph, key);
			node.missing = true;
			node.blocker = true;
			node.provider = res ? res->provider_id : String("manual");
			node.provider_kind = res ? res->provider_kind : String("manual");
			node.provider_package = res ? res->external_package : String();
			node.provider_status = res ? res->probe_status : String("missing");
			node.provider_command = res ? res->probe_command : String();
			node.provider_path = res ? res->probe_path : String();
			node.provider_version = res ? res->probe_version : String();
			node.provider_reason = res ? res->probe_reason : String("no provider available");
			node.reason = node.provider_reason;
			sGraphAddIssue(graph, "missing-provider", Null, trimmed, node.reason);
			sGraphAppendOrder(graph, key);
			return key;
		}
		return sResolveGraphAtom(graph, inv, repo, state, plan, provider_plan, policy, res->external_package,
		                         reason.IsEmpty() ? String("provider for ") + trimmed : reason,
		                         "provider", depth, requested, true, set_member, stack);
	}

	PkgLookupResult lookup = repo.Resolve(trimmed);
	if(lookup.ambiguous) {
		String key = sGraphAmbiguousKey(trimmed);
		sGraphMarkPackageNode(graph, key, 'F', trimmed, reason.IsEmpty() ? String("ambiguous package name") : reason, depth, requested, provider_added, set_member);
		PkgGraphNode& node = sGraphEnsureNode(graph, key);
		node.ambiguous = true;
		for(const PkgPackage* c : lookup.candidates)
			if(c)
				node.candidates.Add(sPackageLookupLabel(*c));
		sGraphAddIssue(graph, "ambiguous", Null, trimmed, node.reason, node.candidates);
		sGraphAppendOrder(graph, key);
		return key;
	}

	const PkgPackage *pkg = lookup.pkg;
	if(!pkg) {
		String key = sGraphMissingKey(trimmed);
		sGraphMarkPackageNode(graph, key, 'F', trimmed, reason.IsEmpty() ? String("package metadata not found") : reason, depth, requested, provider_added, set_member);
		PkgGraphNode& node = sGraphEnsureNode(graph, key);
		node.missing = true;
		node.blocker = true;
		sGraphAddIssue(graph, "missing", Null, trimmed, node.reason);
		sGraphAppendOrder(graph, key);
		return key;
	}
	if(sPolicyPackageMasked(policy, repo, pkg)) {
		String masked_key = sGraphMissingKey(trimmed);
		sGraphMarkPackageNode(graph, masked_key, 'F', pkg->name, reason.IsEmpty() ? String("masked by local policy") : reason, depth, requested, provider_added, set_member);
		PkgGraphNode& masked = sGraphEnsureNode(graph, masked_key);
		masked.missing = true;
		masked.blocker = true;
		masked.reason = "masked by local policy";
		sGraphAddIssue(graph, "masked", Null, trimmed, masked.reason);
		sGraphAppendOrder(graph, masked_key);
		return masked_key;
	}

	String key = lookup.path.IsEmpty() ? pkg->path : lookup.path;
	int done = FindIndex(graph.order, key);
	if(done >= 0)
		return key;
	if(FindIndex(stack, key) >= 0) {
		String cycle_key = sGraphCycleKey(stack.Top(), key);
		sGraphMarkPackageNode(graph, cycle_key, 'B', pkg->name, String("cycle detected: ") + stack.Top() + " -> " + pkg->name, depth, requested, provider_added, set_member);
		PkgGraphNode& cycle = sGraphEnsureNode(graph, cycle_key);
		cycle.cycle = true;
		cycle.blocker = true;
		cycle.reason = String("cycle detected: ") + stack.Top() + " -> " + pkg->name;
		sGraphAddIssue(graph, "cycle", stack.Top(), pkg->name, cycle.reason);
		sGraphAppendOrder(graph, cycle_key);
		return cycle_key;
	}

	PkgGraphNode& node = sGraphEnsureNode(graph, key);
	sGraphPopulateResolvedNode(node, *pkg, reason.IsEmpty() ? String("new package") : reason, inclusion, depth, requested, provider_added, set_member);
	if(sShouldSkipPlannedAtom(inv, plan, repo, state, lookup.path.IsEmpty() ? pkg->name : lookup.path))
		return key;
	stack.Add(key);
	for(const String& dep : pkg->uses) {
		String dep_reason = String("dependency of ") + pkg->name;
		String dep_key = sResolveGraphAtom(graph, inv, repo, state, plan, provider_plan, policy, dep, dep_reason, "dependency", depth + 1, false, false, false, stack);
		if(!dep_key.IsEmpty()) {
			String kind = dep.StartsWith("virtual/") ? "provider" : "dependency";
			sGraphAddEdge(graph, key, dep_key, kind, dep_reason, dep_key.StartsWith("missing:") || dep_key.StartsWith("ambiguous:") || dep_key.StartsWith("cycle:"));
			node.deps.Add(dep_key);
		}
	}
	stack.Remove(stack.GetCount() - 1);
	node.resolved = true;
	sGraphAppendOrder(graph, key);
	return key;
}

static void sGraphBuild(PkgGraph& graph, const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgPlan& plan, const PkgProviderPlan& provider_plan, const PkgLocalPolicy& policy, const Vector<String>& roots)
{
	graph.nodes.Clear();
	graph.edges.Clear();
	graph.issues.Clear();
	graph.order.Clear();
	graph.nodes.Reserve(max(64, repo.packages.GetCount()));
	graph.edges.Reserve(max(128, repo.packages.GetCount() * 4));
	graph.issues.Reserve(max(64, repo.packages.GetCount() / 8 + 1));
	Vector<String> stack;
	String root_reason = "requested";
	if(inv.atom == "@world" || inv.atom == "world" || (inv.atom.IsEmpty() && inv.command == PKG_CMD_PLAN && inv.ask))
		root_reason = "member of @world";
	else if(inv.atom == "@system" || inv.atom == "system")
		root_reason = "member of @system";
	else if(inv.atom == "@toolchain" || inv.atom == "toolchain")
		root_reason = "member of @toolchain";

	for(const PkgProviderResolution& r : provider_plan.resolutions) {
		if(!r.selected || r.external_package.IsEmpty())
			continue;
		String reason = String("provider for ") + r.capability;
		sResolveGraphAtom(graph, inv, repo, state, plan, provider_plan, policy, r.external_package, reason, "provider", 0, false, true, false, stack);
	}

	for(const String& root : roots) {
		String reason = root_reason;
		if(!inv.atom.IsEmpty() && root == inv.atom)
			reason = "requested";
		sResolveGraphAtom(graph, inv, repo, state, plan, provider_plan, policy, root, reason, "requested", 0, true, false, false, stack);
	}
}

static const PkgGraphNode* sFindGraphNode(const PkgGraph& graph, const String& key)
{
	int i = sGraphFindNodeIndex(graph, key);
	return i >= 0 ? &graph.nodes[i] : nullptr;
}

static void sGraphToPlan(PkgPlan& plan, const PkgGraph& graph, const PkgRepository& repo, const PkgState& state, const PkgInvocation& inv)
{
	for(const String& key : graph.order) {
		const PkgGraphNode *node = sFindGraphNode(graph, key);
		if(!node)
			continue;
		const PkgPackage *pkg = nullptr;
		if(node->resolved && !node->path.IsEmpty())
			pkg = repo.Resolve(node->path).pkg;
		if(!pkg && node->resolved)
			pkg = repo.Resolve(node->atom).pkg;
		if(node->resolved && sShouldSkipPlannedAtom(inv, plan, repo, state, node->path.IsEmpty() ? node->atom : node->path))
			continue;
		char status = node->status;
		if(node->resolved) {
			status = sFindStateRecord(state, node->atom) ? (plan.update ? 'U' : 'R') : 'N';
			if(status == 'R' && plan.newuse) {
				const PkgStateRecord *rec = sFindStateRecord(state, node->atom);
				if(rec && sRecordMatchesNewUse(*rec, plan, pkg))
					continue;
			}
			if(status == 'R' && plan.changed_use) {
				const PkgStateRecord *rec = sFindStateRecord(state, node->atom);
				if(rec && sRecordMatchesChangedUse(*rec, plan))
					continue;
			}
		}
		char base_status = status;
		if(node->blocker)
			base_status = 'B';
		else if(node->missing || node->ambiguous || node->cycle)
			base_status = 'F';
		int depth = node->requested ? node->depth : (node->depth > 0 ? node->depth : 1);
		sPlanAddItem(plan, base_status, node->atom, pkg, node->reason, node->provider, node->blocker, node->resolved, depth);
		PkgPlanItem& item = plan.items.Top();
		if(!node->path.IsEmpty())
			item.path = node->path;
		if(!node->repository.IsEmpty())
			item.repository = node->repository;
		if(!node->description.IsEmpty())
			item.description = node->description;
		if(!node->accepts.IsEmpty()) {
			item.accepts.Clear();
			for(const String& s : node->accepts)
				item.accepts.Add(s);
		}
		if(!node->uses.IsEmpty()) {
			item.uses.Clear();
			for(const String& s : node->uses)
				item.uses.Add(s);
		}
		if(!node->mainconfig.IsEmpty()) {
			item.mainconfig.Clear();
			for(const String& s : node->mainconfig)
				item.mainconfig.Add(s);
		}
		if(!node->provider.IsEmpty())
			item.provider = node->provider;
		if(!node->provider_kind.IsEmpty())
			item.provider_kind = node->provider_kind;
		if(!node->provider_package.IsEmpty())
			item.provider_package = node->provider_package;
		if(!node->provider_status.IsEmpty())
			item.provider_status = node->provider_status;
		if(!node->provider_command.IsEmpty())
			item.provider_command = node->provider_command;
		if(!node->provider_path.IsEmpty())
			item.provider_path = node->provider_path;
		if(!node->provider_version.IsEmpty())
			item.provider_version = node->provider_version;
		if(!node->provider_reason.IsEmpty())
			item.provider_reason = node->provider_reason;
		item.ambiguous = node->ambiguous;
		item.blocker = node->blocker;
		item.resolved = node->resolved;
		if(!node->candidates.IsEmpty()) {
			item.candidates.Clear();
			for(const String& s : node->candidates)
				item.candidates.Add(s);
		}
	}
}

static bool sShouldSkipPlannedAtom(const PkgInvocation& inv, const PkgPlan& plan, const PkgRepository& repo, const PkgState& state, const String& atom)
{
	PkgLookupResult lookup = repo.Resolve(atom);
	String key = lookup.pkg ? lookup.pkg->name : atom;
	const PkgStateRecord* rec = sFindStateRecord(state, key);
	if(!rec)
		return false;
	const PkgPackage* pkg = lookup.pkg;
	if(inv.newuse)
		return sRecordMatchesNewUse(*rec, plan, pkg);
	if(inv.changed_use)
		return sRecordMatchesChangedUse(*rec, plan);
	return false;
}

static void sPlanWalkAtom(PkgPlan& plan, const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const String& atom, const String& reason, Index<String>& seen, int depth)
{
	if(atom == "@world" || atom == "world") {
		Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
		if(world.IsEmpty()) {
			sPlanAddItem(plan, 'F', atom, nullptr, reason.IsEmpty() ? String("world file is empty") : reason, String(), false, false, depth);
			return;
		}
		for(const String& w : world)
			sPlanWalkAtom(plan, inv, repo, state, w, "member of @world", seen, depth);
		return;
	}

	if(atom == "@system" || atom == "system" || atom == "@toolchain" || atom == "toolchain") {
		sPlanAddItem(plan, 'N', atom, nullptr, reason.IsEmpty() ? String("synthetic set") : reason, String(), false, false, depth);
		return;
	}

	if(atom.StartsWith("virtual/")) {
		const PkgProviderResolution* res = sFindProviderResolution(plan.provider_plan, atom);
		if(!res) {
			sPlanAddItem(plan, 'F', atom, nullptr, reason.IsEmpty() ? String("no provider available") : reason, "manual", true, false, depth);
			PkgPlanItem& item = plan.items.Top();
			item.provider_kind = "manual";
			item.provider_status = "missing";
			return;
		}
		const PkgPackage* provider_pkg = res->external_package.IsEmpty() ? nullptr : repo.Resolve(res->external_package).pkg;
		char status = res->provider_kind == "manual" || res->probe_status == "missing" || res->external_package.IsEmpty() ? 'F' : 'N';
		sPlanAddItem(plan, status, atom, provider_pkg, reason.IsEmpty() ? String("provider for ") + atom : reason,
		             res->provider_id, false, provider_pkg != nullptr, depth);
		PkgPlanItem& item = plan.items.Top();
		item.provider_kind = res->provider_kind;
		item.provider_package = res->external_package;
		item.provider_status = res->probe_status;
		item.provider_command = res->probe_command;
		item.provider_path = res->probe_path;
		item.provider_version = res->probe_version;
			item.provider_reason = res->probe_reason;
		if(provider_pkg && plan.deep)
			sPlanWalkAtom(plan, inv, repo, state, provider_pkg->atom, String("provider for ") + atom, seen, depth + 1);
		return;
	}

	PkgLookupResult lookup = repo.Resolve(atom);
	const PkgPackage* pkg = lookup.pkg;
	String key = pkg ? ToLower(pkg->name) : ToLower(TrimBoth(atom));
	if(key.IsEmpty())
		return;

	if(lookup.ambiguous) {
		sPlanAddItem(plan, 'F', atom, nullptr, reason.IsEmpty() ? String("ambiguous package name") : reason, String(), false, false, depth);
		PkgPlanItem& item = plan.items.Top();
		item.ambiguous = true;
		for(const PkgPackage* c : lookup.candidates)
			if(c)
				item.candidates.Add(sPackageLookupLabel(*c));
		return;
	}

	if(!pkg) {
		String cap = atom.StartsWith("virtual/") ? atom : String("virtual/") + atom;
		const PkgProviderResolution* res = sFindProviderResolution(plan.provider_plan, cap);
		if(res) {
			const PkgPackage* provider_pkg = res->external_package.IsEmpty() ? nullptr : repo.Resolve(res->external_package).pkg;
			char status = res->provider_kind == "manual" || res->probe_status == "missing" || res->external_package.IsEmpty() ? 'F' : 'N';
			sPlanAddItem(plan, status, cap, provider_pkg, reason.IsEmpty() ? String("provider for ") + atom : reason,
			             res->provider_id, false, provider_pkg != nullptr, depth);
			PkgPlanItem& item = plan.items.Top();
			item.provider_kind = res->provider_kind;
			item.provider_package = res->external_package;
			item.provider_status = res->probe_status;
			item.provider_command = res->probe_command;
			item.provider_path = res->probe_path;
			item.provider_version = res->probe_version;
			item.provider_reason = res->probe_reason;
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

	if(sShouldSkipPlannedAtom(inv, plan, repo, state, atom))
		return;

	char status = sFindStateRecord(state, atom) ? (plan.update ? 'U' : 'R') : 'N';
	String msg = reason;
	if(msg.IsEmpty())
		msg = status == 'N' ? "new package" : (status == 'U' ? "update requested" : "rebuild requested");
	sPlanAddItem(plan, status, atom, pkg, msg, String(), false, false, depth);

	if(!plan.deep)
		return;

	for(const String& dep : pkg->uses) {
		String dep_reason = String("dependency of ") + atom;
		sPlanWalkAtom(plan, inv, repo, state, dep, dep_reason, seen, depth + 1);
	}
}

static PkgPlan sBuildPlan(const PkgInvocation& inv, const PkgRepository& repo, const PkgState& state, const PkgLocalPolicy& policy, bool color)
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
	plan.changed_use = inv.changed_use;
	PkgLookupResult root_lookup = inv.atom.IsEmpty() ? PkgLookupResult() : repo.Resolve(inv.atom);
	const PkgPackage *root_pkg = root_lookup.pkg ? root_lookup.pkg : sResolveExactPackage(repo, inv.atom);
	PkgInvocation eff = sEffectiveInvocation(inv, repo, policy, root_pkg);
	if(root_pkg && root_pkg->manifest.present && root_pkg->manifest.ok && !inv.target_explicit && !root_pkg->manifest.target.IsEmpty()) {
		bool package_target_override = false;
		for(const PkgPolicyLine& line : policy.package_target)
			if(sPolicyRuleMatches(repo, root_pkg, line.atom) && !line.values.IsEmpty()) {
				package_target_override = true;
				break;
			}
		if(!package_target_override)
			eff.target = root_pkg->manifest.target;
	}
	if(root_pkg && root_pkg->manifest.present && root_pkg->manifest.ok && !inv.provider_explicit && !root_pkg->manifest.provider.IsEmpty()) {
		bool package_provider_override = false;
		for(const PkgPolicyLine& line : policy.package_provider)
			if(sPolicyRuleMatches(repo, root_pkg, line.atom) && !line.values.IsEmpty()) {
				package_provider_override = true;
				break;
			}
		if(!package_provider_override)
			eff.provider = root_pkg->manifest.provider;
	}
	plan.target = eff.target;
	const PkgTargetProfile& tp = sDefaultTargetProfile(eff.target);
	sBuildUseModel(plan.use, root_pkg, eff.use_args, eff.target);
	sProjectUpp(plan.upp, plan.use);
	sApplyUppAdditions(plan.upp, tp.upp_add);
	plan.selected_use.Clear();
	plan.disabled_use.Clear();
	plan.defaulted_use.Clear();
	plan.effective_use.Clear();
	plan.target_forced.Clear();
	plan.target_masked.Clear();
	plan.uppflags.Clear();
	plan.warnings.Clear();
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
	for(const String& s : plan.use.conflicts) {
		String msg;
		if(FindIndex(plan.target_masked, s) >= 0)
			msg = String("masked by target: ") + s;
		else if(FindIndex(plan.target_forced, s) >= 0)
			msg = String("target-forced flag disabled: ") + s;
		else
			msg = String("target USE conflict: ") + s;
		if(FindIndex(plan.warnings, msg) < 0)
			plan.warnings.Add(msg);
	}
	if(sIsSelected(plan.effective_use, "sqlite") && FindIndex(plan.virtuals, "virtual/sqlite") < 0)
		plan.virtuals.Add("virtual/sqlite");
	bool gui_runtime = sIsSelected(plan.effective_use, "st") || sIsSelected(plan.effective_use, "gtk") || sIsSelected(plan.effective_use, "virtualgui");
	bool browser_gui = tp.runtime_environment == "browser" || tp.toolchain == "emcc" || sIsSelected(plan.effective_use, "virtualgui");
	if(gui_runtime && FindIndex(plan.virtuals, "virtual/gui-runtime") < 0)
		plan.virtuals.Add("virtual/gui-runtime");
	if(browser_gui) {
		if(FindIndex(plan.virtuals, "virtual/sdl2") < 0)
			plan.virtuals.Add("virtual/sdl2");
		if(FindIndex(plan.virtuals, "virtual/opengl") < 0)
			plan.virtuals.Add("virtual/opengl");
	}
	const Vector<PkgProviderPreference> *extra_prefs = root_pkg && root_pkg->manifest.present && root_pkg->manifest.ok ? &root_pkg->manifest.provider_preferences : nullptr;
	sBuildProviderPlan(plan.provider_plan, repo, policy, &tp, plan.target, plan.virtuals, eff.provider, eff, extra_prefs);
	for(const PkgProviderResolution& r : plan.provider_plan.resolutions)
		if(!r.external_package.IsEmpty() && FindIndex(plan.providers, r.external_package) < 0)
			plan.providers.Add(r.external_package);
	for(const String& s : plan.provider_plan.warnings)
		if(FindIndex(plan.warnings, s) < 0)
			plan.warnings.Add(s);
	sApplyUppAdditions(plan.upp, plan.provider_plan.upp_additions);
	for(const String& s : Split(sFormatUppProjection(plan.upp), CharFilterWhitespace))
		plan.uppflags.Add(s);
	for(const String& s : tp.warnings)
		if(FindIndex(plan.warnings, s) < 0)
			plan.warnings.Add(s);
	for(const String& s : tp.notes)
		if(FindIndex(plan.warnings, s) < 0)
			plan.warnings.Add(s);

	Vector<String> roots;
	if(!inv.atom.IsEmpty()) {
		if(inv.atom == "@world" || inv.atom == "world") {
			Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
			for(const String& s : world)
				roots.Add(s);
		}
		else if(inv.atom == "@system" || inv.atom == "system") {
			Vector<String> system = sLoadSet(repo.paths, "system", repo.paths.system_set);
			for(const String& s : system)
				roots.Add(s);
		}
		else if(inv.atom == "@toolchain" || inv.atom == "toolchain") {
			Vector<String> toolchain = sLoadSet(repo.paths, "toolchain", repo.paths.toolchain_set);
			for(const String& s : toolchain)
				roots.Add(s);
		}
		else
			roots.Add(inv.atom);
	}
	else if(inv.command == PKG_CMD_PLAN && inv.ask) {
		Vector<String> world = sLoadSet(repo.paths, "world", repo.paths.world);
		for(const String& s : world)
			roots.Add(s);
	}

	sGraphBuild(plan.graph, inv, repo, state, plan, plan.provider_plan, policy, roots);
	sGraphToPlan(plan, plan.graph, repo, state, inv);

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
		Cout() << "  " << sAnsi("34", "provider:", plan.color) << ' ' << item.provider
		     << (item.provider_kind.IsEmpty() ? String() : String(" [") + item.provider_kind + "]")
		     << (item.provider_status.IsEmpty() ? String() : String(" [") + item.provider_status + "]") << "\n";
	if(!item.provider_command.IsEmpty())
		Cout() << "  " << sAnsi("90", "probe command:", plan.color) << ' ' << item.provider_command << "\n";
	if(!item.provider_path.IsEmpty())
		Cout() << "  " << sAnsi("90", "probe path:", plan.color) << ' ' << item.provider_path << "\n";
	if(!item.provider_version.IsEmpty())
		Cout() << "  " << sAnsi("90", "probe version:", plan.color) << ' ' << item.provider_version << "\n";
	if(!item.provider_reason.IsEmpty())
		Cout() << "  " << sAnsi("90", "probe reason:", plan.color) << ' ' << item.provider_reason << "\n";
	if(!item.provider_package.IsEmpty())
		Cout() << "  " << sAnsi("34", "package:", plan.color) << ' ' << item.provider_package << "\n";
	if(!item.reason.IsEmpty())
		Cout() << "  " << sAnsi("90", "reason:", plan.color) << ' ' << item.reason << "\n";
	if(!item.repository.IsEmpty())
		Cout() << "  " << sAnsi("90", "repository:", plan.color) << ' ' << item.repository << "\n";
	if(!item.description.IsEmpty())
		Cout() << "  " << sAnsi("90", "description:", plan.color) << ' ' << item.description << "\n";
	if(!item.path.IsEmpty())
		Cout() << "  " << sAnsi("90", "path:", plan.color) << ' ' << item.path << "\n";
	if(plan.verbose) {
		Cout() << "  " << sAnsi("90", "declared USE:", plan.color) << ' ' << sFormatUseList(plan.use.declared) << "\n";
		Cout() << "  " << sAnsi("90", "selected USE:", plan.color) << ' ' << sFormatUseList(plan.use.selected) << "\n";
		Cout() << "  " << sAnsi("90", "default USE:", plan.color) << ' ' << sFormatUseList(plan.use.defaults) << "\n";
		Cout() << "  " << sAnsi("90", "forced USE:", plan.color) << ' ' << sFormatUseList(plan.use.forced) << "\n";
		Cout() << "  " << sAnsi("90", "masked USE:", plan.color) << ' ' << sFormatUseList(plan.use.masked) << "\n";
		Cout() << "  " << sAnsi("90", "effective USE:", plan.color) << ' ' << sFormatUseList(plan.use.effective) << "\n";
		if(!item.accepts.IsEmpty())
			Cout() << "  " << sAnsi("90", "acceptflags:", plan.color) << ' ' << sFmtList(item.accepts) << "\n";
		if(!item.uses.IsEmpty())
			Cout() << "  " << sAnsi("90", "uses:", plan.color) << ' ' << sFmtList(item.uses) << "\n";
		if(!item.mainconfig.IsEmpty())
			Cout() << "  " << sAnsi("90", "mainconfig:", plan.color) << ' ' << sFmtList(item.mainconfig) << "\n";
		if(!plan.use.transitions.IsEmpty()) {
			Cout() << "  " << sAnsi("90", "USE transitions:", plan.color) << '\n';
			for(const PkgUseTransition& tr : plan.use.transitions)
				Cout() << "    " << tr.marker << tr.flag << " (" << tr.reason << ")\n";
		}
		Cout() << "  " << sAnsi("90", "UPP global:", plan.color) << ' ' << sFormatUseList(plan.upp.global) << "\n";
		Cout() << "  " << sAnsi("90", "UPP accepted:", plan.color) << ' ' << sFormatUseList(plan.upp.accepted) << "\n";
		Cout() << "  " << sAnsi("90", "UPP main-only:", plan.color) << ' ' << sFormatUseList(plan.upp.main_only) << "\n";
		if(item.ambiguous && !item.candidates.IsEmpty())
			Cout() << "  " << sAnsi("90", "candidates:", plan.color) << ' ' << sFmtList(item.candidates) << "\n";
	}
	else if(item.ambiguous && !item.candidates.IsEmpty()) {
		Cout() << "  " << sAnsi("90", "candidates:", plan.color) << ' ' << sFmtList(item.candidates) << "\n";
	}
	Cout() << "\n";
}

static PkgPlan sPrintPlan(const PkgInvocation& inv, const PkgRepository& repo, const PkgLocalPolicy& policy, bool color)
{
	PkgState state;
	LoadFromJsonFile(state, repo.paths.state);
	PkgPlan plan = sBuildPlan(inv, repo, state, policy, color);
	const PkgTargetProfile& tp = sDefaultTargetProfile(plan.target);

	Cout() << "These are the packages that would be built, in order:\n\n";
	Cout() << "Calculating dependencies... done!\n";
	Cout() << "Dependency resolution took " << FormatDouble(plan.dependency_seconds, 2) << " s (backtrack: "
	     << plan.backtrack << '/' << plan.backtrack_limit << ").\n\n";
	Cout() << "Dependency graph: " << plan.graph.nodes.GetCount() << " nodes, " << plan.graph.edges.GetCount() << " edges.\n\n";

	if(plan.verbose) {
		Cout() << "Target:\n";
		Cout() << "  name: " << sTargetNameText(plan.target) << "\n";
		Cout() << "  arch: " << (tp.architecture.IsEmpty() ? String("[none]") : tp.architecture) << "\n";
		Cout() << "  platform: " << (tp.target_platform.IsEmpty() ? String("[none]") : tp.target_platform) << "\n";
		Cout() << "  runtime: " << (tp.runtime_environment.IsEmpty() ? String("[none]") : tp.runtime_environment) << "\n";
		Cout() << "  toolchain: " << (tp.toolchain.IsEmpty() ? String("[none]") : tp.toolchain) << "\n";
		Cout() << "  thread model: " << tp.thread_model << "\n";
		Cout() << "  forced USE: " << sFmtList(tp.forced_use) << "\n";
		Cout() << "  masked USE: " << sFmtList(tp.masked_use) << "\n";
		Cout() << "  default USE: " << sFmtList(tp.default_use) << "\n";
		Cout() << "  provider preferences: " << sFmtProviderPreferences(tp.provider_preferences) << "\n";
		Cout() << "  UPP additions: " << sFmtUppFlagList(tp.upp_add) << "\n";
		if(!tp.warnings.IsEmpty())
			Cout() << "  warnings: " << sFmtList(tp.warnings) << "\n";
		if(!tp.notes.IsEmpty())
			Cout() << "  notes: " << sFmtList(tp.notes) << "\n";
		Cout() << "\n";
	}

	if(plan.items.IsEmpty())
		Cout() << "[ no packages selected ]\n\n";
	else
		for(const PkgPlanItem& item : plan.items)
			sPrintPlanItem(plan, item);

	Cout() << "USE: " << sFormatUseModel(plan.use) << "\n";
	Cout() << "Effective USE: " << sFmtList(plan.effective_use) << "\n";
	Cout() << "TARGET: " << sTargetNameText(plan.target) << "\n";
	Cout() << "UPPFLAGS: " << sFormatUppProjection(plan.upp) << "\n";
	if(!plan.warnings.IsEmpty())
		Cout() << "Warnings: " << sFmtList(plan.warnings) << "\n";
	Cout() << "Providers:\n";
	if(plan.provider_plan.resolutions.IsEmpty())
		Cout() << "  [none]\n";
	else
		for(const PkgProviderResolution& r : plan.provider_plan.resolutions) {
			Cout() << "  " << r.capability << " -> " << (r.provider_id.IsEmpty() ? String("[none]") : r.provider_id)
			     << " [" << r.probe_status << "]";
			if(!r.external_package.IsEmpty())
				Cout() << " -> " << r.external_package;
			if(!r.probe_reason.IsEmpty())
				Cout() << " - " << r.probe_reason;
			if(!r.probe_version.IsEmpty())
				Cout() << " (" << r.probe_version << ")";
			Cout() << "\n";
			if(!r.probe_command.IsEmpty())
				Cout() << "    probe command: " << r.probe_command << "\n";
			if(!r.probe_path.IsEmpty())
				Cout() << "    probe path: " << r.probe_path << "\n";
		}
	if(!plan.provider_plan.uses_additions.IsEmpty()) {
		Cout() << "Provider U++ uses: " << sFmtList(plan.provider_plan.uses_additions) << "\n";
	}
	if(!plan.provider_plan.upp_additions.IsEmpty()) {
		Vector<String> flags;
		for(const PkgUppFlag& f : plan.provider_plan.upp_additions)
			flags.Add(sFormatUppFlag(f));
		Cout() << "Provider UPP flags: " << sFmtList(flags) << "\n";
	}
	if(!plan.providers.IsEmpty())
		Cout() << "Providers: " << sFmtList(plan.providers) << "\n";
	if(!plan.virtuals.IsEmpty())
		Cout() << "Virtual requirements: " << sFmtList(plan.virtuals) << "\n";
	if(!plan.provider_plan.warnings.IsEmpty())
		Cout() << "Provider warnings: " << sFmtList(plan.provider_plan.warnings) << "\n";
	Cout() << "Total packages: " << plan.items.GetCount() << "\n";

	return plan;
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

static void sPrintEselectCommand(const PkgInvocation& inv, const PkgRepository& repo)
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
	if(inv.module == "news" && (inv.subcommand.IsEmpty() || inv.subcommand == "list" || inv.subcommand == "show")) {
		Cout() << "No news items.\n";
		return;
	}
	if(inv.subcommand == "list") {
		sPrintEselectValueList(inv, repo, inv.module);
		return;
	}
	if(inv.subcommand == "show") {
		sPrintEselectValueShow(inv, repo, inv.module);
		return;
	}
	if(inv.subcommand == "set") {
		if(inv.module == "target") {
			PkgInvocation t;
			t.subcommand = "set";
			t.target = inv.value;
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

static void sAuditAcceptFlags(const PkgInvocation& inv, const PkgRepository& repo, bool color)
{
	Vector<const PkgPackage*> targets;
	if(inv.atom.IsEmpty()) {
		for(const PkgPackage& p : repo.packages)
			targets.Add(&p);
	}
	else {
		PkgLookupResult lookup = repo.Resolve(inv.atom);
		if(lookup.ambiguous) {
			Cout() << sAnsi("33;1", "[warn]", color) << " ambiguous package: " << inv.atom << "\n";
			for(const PkgPackage* c : lookup.candidates)
				if(c)
					Cout() << "  " << sPackageLookupLabel(*c) << "\n";
			return;
		}
		const PkgPackage* p = lookup.pkg;
		if(!p) {
			Cout() << sAnsi("33;1", "[warn]", color) << " unknown package: " << inv.atom << "\n";
			return;
		}
		targets.Add(p);
	}
	Sort(targets, [](const PkgPackage* a, const PkgPackage* b) { return a->name < b->name; });

	int scanned = 0;
	int warnings = 0;
	int ok = 0;
	int skipped = 0;
	Cout() << "Acceptflags audit:\n\n";
	for(int i = 0; i < targets.GetCount(); i++) {
		const PkgPackage& p = *targets[i];
		++scanned;
		sAuditPrintPackage(repo, p, color, inv.audit_patch, warnings, ok, skipped);
		if(i + 1 < targets.GetCount())
			Cout() << "\n";
	}
	Cout() << "\nSummary:\n";
	Cout() << "  packages scanned: " << scanned << "\n";
	Cout() << "  warnings: " << warnings << "\n";
	Cout() << "  ok: " << ok << "\n";
	Cout() << "  skipped: " << skipped << "\n";
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
	inv.command_line = sJoin(args);
	inv.argv.Clear();
	for(const String& s : args)
		inv.argv.Add(s);
	bool color = sUseColor(inv);
	(void)color;
	TimeStop command_total;

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
	                 inv.command == PKG_CMD_PROVIDERS || inv.command == PKG_CMD_SEARCH || inv.command == PKG_CMD_EXPLAIN_USE ||
	                 inv.command == PKG_CMD_EXPLAIN_TARGET || inv.command == PKG_CMD_DEPS || inv.command == PKG_CMD_PLAN ||
	                 inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS || inv.command == PKG_CMD_ESELECT ||
	                 inv.command == PKG_CMD_RESUME || inv.command == PKG_CMD_SELFTEST ||
	                 inv.command == PKG_CMD_TARGET || inv.command == PKG_CMD_DOCTOR || inv.command == PKG_CMD_LINT ||
	                 inv.command == PKG_CMD_METADATA_CACHE ||
	                 inv.command == PKG_CMD_BINS || inv.command == PKG_CMD_CLEAN || inv.command == PKG_CMD_DEPCLEAN;
	PkgRepository repo;
	repo.use_cache = inv.use_cache;
	repo.rebuild_cache = inv.rebuild_cache;
	if(need_repo)
		repo.Discover();
	PkgLocalPolicy policy;
	if(need_repo)
		policy = sLoadLocalPolicy(repo.paths, repo);

	if(inv.command == PKG_CMD_LIST_SETS) {
		sPrintSets(repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_TARGETS) {
		sListTargets();
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_BINS) {
		sPrintBins(inv, repo, color);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_CLEAN) {
		sCleanupArtifacts(inv, repo, policy, color, false);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_DEPCLEAN) {
		sCleanupArtifacts(inv, repo, policy, color, true);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_INFO) {
		sPrintInfo(repo, inv, policy);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_DOCTOR) {
		sPrintDoctorCommand(inv, repo, policy, color);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_SELFTEST) {
		int rc = sSelftestCommand(inv, repo, policy, color);
		sFlushConsole();
		return rc;
	}
	if(inv.command == PKG_CMD_LINT) {
		int rc = sLintCommand(inv, repo, policy, color, command_total.Seconds());
		sFlushConsole();
		return rc;
	}
	if(inv.command == PKG_CMD_PROVIDERS) {
		sPrintProvidersCommand(inv, repo, policy, color);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_METADATA) {
		sPrintMetadata(repo, inv);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_METADATA_CACHE) {
		sPrintMetadataCache(repo, inv);
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
		sPrintEselectCommand(inv, repo);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_EXPLAIN_USE) {
		sExplainUse(inv, repo, policy);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_EXPLAIN_TARGET) {
		sExplainTarget(inv.target);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_DEPS) {
		sPrintDeps(inv, repo, policy);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_PLAN) {
		PkgPlan plan = sPrintPlan(inv, repo, policy, color);
		if(inv.pretend) {
			sFlushConsole();
			return 0;
		}
		if(inv.ask) {
			if(!sPromptYesNo()) {
				Cout() << "Aborted.\n";
				sFlushConsole();
				return 0;
			}
			Cout() << "Continuing.\n";
		}
		if(inv.ask || inv.install) {
			PkgExecutionResult exec = sExecuteTransaction(inv, repo, plan, color, false);
			if(!exec.ok) {
				sFlushConsole();
				return 1;
			}
		}
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_AUDIT_ACCEPTFLAGS) {
		sAuditAcceptFlags(inv, repo, color);
		sFlushConsole();
		return 0;
	}
	if(inv.command == PKG_CMD_RESUME) {
		PkgTransaction tx;
		if(!sLoadTransaction(repo, tx) || tx.result != "failed" || tx.steps.IsEmpty()) {
			Cout() << "No saved failed transaction found.\n";
			sFlushConsole();
			return 0;
		}
		sPrintResumeState(tx);
		if(inv.pretend) {
			Cout() << "pretend mode active\n";
			sFlushConsole();
			return 0;
		}
		if(inv.ask) {
			if(!sPromptYesNo()) {
				Cout() << "Aborted.\n";
				sFlushConsole();
				return 0;
			}
			Cout() << "Continuing.\n";
		}
		PkgPlan plan;
		plan.target = tx.target;
		plan.atom = tx.requested_atoms.IsEmpty() ? String() : tx.requested_atoms[0];
		{
			PkgExecutionResult exec = sExecuteTransaction(inv, repo, plan, color, true);
			if(!exec.ok) {
				sFlushConsole();
				return 1;
			}
		}
		sFlushConsole();
		return 0;
	}

	Cerr() << "not implemented yet\n";
	sFlushConsole();
	return 1;
}

} // namespace Upp

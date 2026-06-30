#ifndef _pkg_Pkg_h_
#define _pkg_Pkg_h_

#include <ide/Core/Core.h>

NAMESPACE_UPP

enum PkgColorMode {
	PKG_COLOR_AUTO,
	PKG_COLOR_YES,
	PKG_COLOR_NO,
};

enum PkgCommandKind {
	PKG_CMD_NONE,
	PKG_CMD_HELP,
	PKG_CMD_VERSION,
	PKG_CMD_INFO,
	PKG_CMD_DOCTOR,
	PKG_CMD_METADATA,
	PKG_CMD_LIST_SETS,
	PKG_CMD_TARGETS,
	PKG_CMD_PROVIDERS,
	PKG_CMD_SEARCH,
	PKG_CMD_PLAN,
	PKG_CMD_DEPS,
	PKG_CMD_EXPLAIN_USE,
	PKG_CMD_EXPLAIN_TARGET,
	PKG_CMD_TARGET,
	PKG_CMD_ESELECT,
	PKG_CMD_AUDIT_ACCEPTFLAGS,
	PKG_CMD_RESUME,
};

struct PkgUseMap : Moveable<PkgUseMap> {
	String upp_flag;
	String scope;
	String thread_model;

	void Jsonize(JsonIO& jio);
};

struct PkgUsePolicy : Moveable<PkgUsePolicy> {
	String name;
	String description;
	bool default_on = false;
	Vector<PkgUseMap> maps_to;

	void Jsonize(JsonIO& jio);
};

enum PkgUppScope {
	PKG_UPP_GLOBAL,
	PKG_UPP_ACCEPTED,
	PKG_UPP_MAIN_ONLY,
};

struct PkgUseTransition : Moveable<PkgUseTransition> {
	String flag;
	String marker;
	String reason;
};

struct PkgUseModel : Moveable<PkgUseModel> {
	Vector<String> requested;
	Vector<String> declared;
	Vector<String> defaults;
	Vector<String> forced;
	Vector<String> masked;
	Vector<String> conflicts;
	Vector<String> selected;
	Vector<String> disabled;
	Vector<String> effective;
	Vector<PkgUseTransition> transitions;
};

struct PkgUppFlag : Moveable<PkgUppFlag> {
	String name;
	int scope = PKG_UPP_GLOBAL;
	String reason;

	void Jsonize(JsonIO& jio);
};

struct PkgUppProjection : Moveable<PkgUppProjection> {
	Vector<PkgUppFlag> flags;
	Vector<String> global;
	Vector<String> accepted;
	Vector<String> main_only;
	Vector<String> transitions;
};

struct PkgProviderPreference : Moveable<PkgProviderPreference> {
	String capability;
	String provider_id;
	String reason;
	int priority = 0;

	void Jsonize(JsonIO& jio);
};

struct PkgTargetProfile : Moveable<PkgTargetProfile> {
	String name;
	String host_platform;
	String build_platform;
	String target_platform;
	String runtime_environment;
	String architecture;
	String toolchain;
	String sysroot;
	String thread_model;
	String compiler;
	String linker;
	String sdk;
	String summary;
	Vector<String> default_use;
	Vector<String> forced_use;
	Vector<String> masked_use;
	Vector<PkgProviderPreference> provider_preferences;
	Vector<PkgUppFlag> upp_add;
	Vector<String> notes;
	Vector<String> warnings;

	void Jsonize(JsonIO& jio);
};

struct PkgProvider : Moveable<PkgProvider> {
	String capability;
	String id;
	String kind;
	String provider;
	String details;
	int priority = 0;
	bool system_install = false;
	Vector<String> targets;
	Vector<String> uses_add;
	Vector<PkgUppFlag> upp_add;
	String probe_status;
	String probe_command;
	String probe_path;
	String probe_version;
	String probe_reason;
	bool manual = false;
};

struct PkgVirtualCapability : Moveable<PkgVirtualCapability> {
	String capability;
	String description;
	Vector<String> provider_ids;
};

struct PkgProviderResolution : Moveable<PkgProviderResolution> {
	String capability;
	String provider_id;
	String provider_kind;
	String provider;
	String external_package;
	String details;
	String probe_status;
	String probe_command;
	String probe_path;
	String probe_version;
	String probe_reason;
	int priority = 0;
	bool selected = false;
	bool manual = false;
	Vector<String> targets;
	Vector<String> uses_add;
	Vector<PkgUppFlag> upp_add;
};

struct PkgProviderPlan : Moveable<PkgProviderPlan> {
	Vector<PkgVirtualCapability> capabilities;
	Vector<PkgProviderResolution> resolutions;
	Vector<String> uses_additions;
	Vector<PkgUppFlag> upp_additions;
	Vector<String> warnings;
};

struct PkgPackage : Moveable<PkgPackage> {
	String atom;
	String name;
	String nest;
	String path;
	String dir;
	String description;
	Vector<String> accepts;
	Vector<String> uses;
	Vector<String> mainconfig;
	Vector<String> source_files;
	Time mtime;
};

struct PkgLookupResult : Moveable<PkgLookupResult> {
	String query;
	String canonical;
	String path;
	const PkgPackage *pkg = nullptr;
	Vector<const PkgPackage*> candidates;
	bool direct_path = false;
	bool ambiguous = false;
};

struct PkgStateRecord : Moveable<PkgStateRecord> {
	String atom;
	String target;
	String toolchain;
	String build_status;
	String artifact_path;
	Vector<String> selected_use;
	Vector<String> declared_use;
	Vector<String> effective_use;
	Vector<String> effective_uppflags;
	Vector<String> accepted_flags;
	Vector<String> providers;
	Time timestamp;

	void Jsonize(JsonIO& jio);
};

struct PkgState : Moveable<PkgState> {
	Vector<PkgStateRecord> records;
	String target;
	String toolchain;

	void Jsonize(JsonIO& jio);
};

struct PkgEselectState : Moveable<PkgEselectState> {
	String compiler;
	String linker;
	String target;
	String provider;
	String profile;
	String repository;
	String vcpkg_root;
	String vcpkg_triplet;
	String emscripten_profile;

	void Jsonize(JsonIO& jio);
};

struct PkgConfigPaths {
	String root;
	String ai_dir;
	String sets_dir;
	String system_set;
	String toolchain_set;
	String world;
	String package_use;
	String package_provider;
	String package_target;
	String state;
	String eselect;
};

struct PkgPlanItem : Moveable<PkgPlanItem> {
	char status = 'N';
	String atom;
	String path;
	String use;
	String target;
	String uppflags;
	String provider;
	String provider_kind;
	String provider_package;
	String provider_status;
	String provider_command;
	String provider_path;
	String provider_version;
	String provider_reason;
	String reason;
	String repository;
	String description;
	Vector<String> accepts;
	Vector<String> uses;
	Vector<String> mainconfig;
	Vector<String> candidates;
	bool ambiguous = false;
	bool interactive = false;
	bool blocker = false;
	bool resolved = false;
};

struct PkgPlan : Moveable<PkgPlan> {
	String atom;
	String target;
	bool color = false;
	bool ask = false;
	bool verbose = false;
	bool pretend = false;
	bool update = false;
	bool deep = false;
	bool newuse = false;
	bool changed_use = false;
	PkgUseModel use;
	PkgUppProjection upp;
	Vector<String> selected_use;
	Vector<String> disabled_use;
	Vector<String> defaulted_use;
	Vector<String> effective_use;
	Vector<String> target_forced;
	Vector<String> target_masked;
	Vector<String> uppflags;
	Vector<String> providers;
	Vector<String> virtuals;
	Vector<String> warnings;
	PkgProviderPlan provider_plan;
	Vector<PkgPlanItem> items;
	int backtrack = 0;
	int backtrack_limit = 20;
	double dependency_seconds = 0.0;
};

struct PkgRepository {
	String root;
	PkgConfigPaths paths;
	Vector<PkgPackage> packages;
	Vector<String> nests;

	void Discover();
	PkgLookupResult Resolve(const String& atom) const;
	const PkgPackage* Find(const String& atom) const;
	Vector<const PkgPackage*> Search(const String& query) const;
	bool HasPackage(const String& atom) const;
};

struct PkgInvocation {
	PkgCommandKind command = PKG_CMD_NONE;
	PkgColorMode color = PKG_COLOR_AUTO;
	String atom;
	String query;
	String provider_query;
	String target;
	String provider;
	String compiler;
	String linker;
	String profile;
	String repository;
	String vcpkg_root;
	String vcpkg_triplet;
	String emscripten_profile;
	String value;
	String module;
	String subcommand;
	String root;
	String sysroot;
	Vector<String> use_args;
	Vector<String> extra;
	bool ask = false;
	bool verbose = false;
	bool update = false;
	bool deep = false;
	bool newuse = false;
	bool changed_use = false;
	bool probe = false;
	bool pretend = false;
	bool resume = false;
	bool oneshot = false;
	bool plan = false;
	bool metadata = false;
	bool brief = false;
	bool list_sets = false;
	bool targets = false;
	bool providers = false;
	bool info = false;
	bool doctor = false;
	bool search = false;
	bool install = false;
	bool audit_patch = false;
};

bool ParsePkgArgs(const Vector<String>& args, PkgInvocation& inv, String& error);
int  RunPkg(const Vector<String>& args);

PkgConfigPaths FindPkgConfigPaths(const String& root);
String PkgRepoRoot();
bool   IsGlobalAuditFlag(const String& flag);

// Lightweight CLI skeleton kept for plan/package batching.
struct CliOptions {
	bool ask = false;
	bool pretend = false;
	bool verbose = false;
	bool update = false;
	bool deep = false;
	bool newuse = false;
	bool changed_use = false;

	String color = "auto";
	int jobs = 0;
	String target;
	String provider;

	bool help = false;
	bool version = false;
	bool info = false;
	bool depclean = false;
	bool list_sets = false;
	bool search = false;
	bool sync = false;
	bool metadata = false;
	bool audit_acceptflags = false;
	bool eselect = false;

	String search_query;
	Vector<String> atoms;
	Vector<String> eselect_args;
};

void PrintHelp();
void PrintVersion();
void PrintInfo();
bool ParseCommandLine(const Vector<String>& args, CliOptions& opts, String& error);

END_UPP_NAMESPACE

#endif

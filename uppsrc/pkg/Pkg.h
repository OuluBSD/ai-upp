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
	PKG_CMD_METADATA_CACHE,
	PKG_CMD_LIST_SETS,
	PKG_CMD_TARGETS,
	PKG_CMD_PROVIDERS,
	PKG_CMD_SEARCH,
	PKG_CMD_LINT,
	PKG_CMD_PLAN,
	PKG_CMD_DEPS,
	PKG_CMD_EXPLAIN_USE,
	PKG_CMD_EXPLAIN_TARGET,
	PKG_CMD_BINS,
	PKG_CMD_CLEAN,
	PKG_CMD_DEPCLEAN,
	PKG_CMD_TARGET,
	PKG_CMD_ESELECT,
	PKG_CMD_AUDIT_ACCEPTFLAGS,
	PKG_CMD_RESUME,
	PKG_CMD_SELFTEST,
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

struct PkgUppkgManifest : Moveable<PkgUppkgManifest> {
	String path;
	bool present = false;
	bool ok = false;
	String target;
	String provider;
	Vector<String> use_default;
	Vector<String> use_forced;
	Vector<String> use_masked;
	Vector<PkgProviderPreference> provider_preferences;
	Vector<String> notes;
	Vector<String> warnings;
	Vector<String> errors;
	Vector<String> unknown_keys;

	void Jsonize(JsonIO& jio);
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
	PkgUppkgManifest manifest;
	Time mtime;

	void Jsonize(JsonIO& jio);
};

struct PkgMetadataCacheEntry : Moveable<PkgMetadataCacheEntry> {
	String path;
	String manifest_path;
	int64 size = 0;
	Time mtime;
	int64 manifest_size = 0;
	Time manifest_mtime;
	PkgPackage pkg;

	void Jsonize(JsonIO& jio);
};

struct PkgMetadataCache : Moveable<PkgMetadataCache> {
	int schema = 2;
	String root;
	Vector<PkgMetadataCacheEntry> entries;
	Time saved_at;

	void Jsonize(JsonIO& jio);
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

struct PkgResolveCacheEntry : Moveable<PkgResolveCacheEntry> {
	String canonical;
	String path;
	Vector<int> candidates;
	int pkg_index = -1;
	bool direct_path = false;
	bool ambiguous = false;
};

struct PkgGraphEdge : Moveable<PkgGraphEdge> {
	String from;
	String to;
	String kind;
	String reason;
	bool missing = false;
};

struct PkgResolveIssue : Moveable<PkgResolveIssue> {
	String kind;
	String from;
	String atom;
	String reason;
	Vector<String> candidates;
};

struct PkgGraphNode : Moveable<PkgGraphNode> {
	String key;
	String atom;
	String path;
	String inclusion;
	String reason;
	String repository;
	String description;
	String provider;
	String provider_kind;
	String provider_package;
	String provider_status;
	String provider_command;
	String provider_path;
	String provider_version;
	String provider_reason;
	char status = 'N';
	int depth = 0;
	bool resolved = false;
	bool missing = false;
	bool ambiguous = false;
	bool cycle = false;
	bool provider_added = false;
	bool set_member = false;
	bool requested = false;
	bool blocker = false;
	Vector<String> accepts;
	Vector<String> uses;
	Vector<String> mainconfig;
	Vector<String> candidates;
	Vector<String> deps;
};

struct PkgGraph : Moveable<PkgGraph> {
	Vector<PkgGraphNode> nodes;
	Vector<PkgGraphEdge> edges;
	Vector<PkgResolveIssue> issues;
	Vector<String> order;
};

struct PkgResolveResult : Moveable<PkgResolveResult> {
	PkgGraph graph;
	Vector<String> roots;
};

struct PkgStateRecord : Moveable<PkgStateRecord> {
	String atom;
	String target;
	String toolchain;
	String build_status;
	String artifact_path;
	String build_method;
	String command_line;
	String output_path;
	Vector<String> selected_use;
	Vector<String> declared_use;
	Vector<String> effective_use;
	Vector<String> effective_uppflags;
	Vector<String> accepted_flags;
	Vector<String> providers;
	bool staged = false;
	bool success = false;
	bool owned = true;
	Time timestamp;

	void Jsonize(JsonIO& jio);
};

struct PkgState : Moveable<PkgState> {
	Vector<PkgStateRecord> records;
	String target;
	String toolchain;

	void Jsonize(JsonIO& jio);
};

struct PkgBuildStep : Moveable<PkgBuildStep> {
	String atom;
	String path;
	String output_path;
	String build_method;
	String reason;
	String command;
	String result;
	int exit_code = -1;
	bool requested = false;
	bool provider_added = false;
	bool set_member = false;
	bool root = false;
	bool executed = false;
	bool completed = false;
	bool skipped = false;
	bool failed = false;
	bool staged = false;

	void Jsonize(JsonIO& jio);
};

struct PkgExecutionResult : Moveable<PkgExecutionResult> {
	bool ok = false;
	bool executed = false;
	bool resumed = false;
	bool staged = false;
	int failed_index = -1;
	String failed_atom;
	String message;
	String staged_runner;
	Vector<String> completed_steps;
	Vector<String> resume_data;
	PkgBuildStep failed_step;
};

struct PkgTransaction : Moveable<PkgTransaction> {
	String command_line;
	String target;
	String provider;
	String compiler;
	String linker;
	String toolchain;
	String build_method;
	String result;
	int jobs = 0;
	int failed_index = -1;
	bool pretend = false;
	bool ask = false;
	bool resume = false;
	bool staged = false;
	bool keep_going = false;
	bool skip_first = false;
	String staged_runner;
	Vector<String> requested_atoms;
	Vector<String> completed_steps;
	Vector<String> resume_data;
	PkgBuildStep failed_step;
	Vector<PkgBuildStep> steps;
	Time timestamp;

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
	String cache_dir;
	String metadata_cache;
	String sets_dir;
	String make_conf;
	String system_set;
	String toolchain_set;
	String world;
	String package_use;
	String package_provider;
	String package_target;
	String package_mask;
	String package_force;
	String state;
	String eselect;
	String transaction;
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
	int depth = 0;
	bool requested = false;
	bool provider_added = false;
	bool set_member = false;
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
	PkgGraph graph;
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
	VectorMap<String, Vector<int> > resolve_atom_index;
	VectorMap<String, Vector<int> > resolve_name_index;
	VectorMap<String, Vector<int> > resolve_path_index;
	VectorMap<String, Vector<int> > resolve_qualified_index;
	mutable VectorMap<String, PkgResolveCacheEntry> resolve_cache;
	int discover_paths = 0;
	int loaded_packages = 0;
	int cache_loaded_entries = 0;
	int cache_reused_entries = 0;
	int cache_reparsed_entries = 0;
	int cache_stale_entries = 0;
	int cache_schema = 0;
	bool cache_ok = false;
	bool cache_corrupt = false;
	bool cache_used = false;
	bool use_cache = true;
	bool rebuild_cache = false;
	mutable int resolve_calls = 0;
	mutable int resolve_cache_hits = 0;
	mutable int resolve_index_hits = 0;
	mutable int search_calls = 0;
	mutable int find_calls = 0;
	double discover_seconds = 0.0;

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
	Vector<String> argv;
	String root;
	String sysroot;
	String command_line;
	Vector<String> use_args;
	Vector<String> extra;
	int jobs = 0;
	bool target_explicit = false;
	bool provider_explicit = false;
	bool ask = false;
	bool verbose = false;
	bool update = false;
	bool deep = false;
	bool newuse = false;
	bool changed_use = false;
	bool strict = false;
	bool nodeps = false;
	bool summary = false;
	bool quiet = false;
	bool check = false;
	bool ci = false;
	bool keep_going = false;
	bool skip_first = false;
	bool probe = false;
	bool debug_timing = false;
	bool pretend = false;
	bool resume = false;
	bool oneshot = false;
	bool plan = false;
	bool metadata = false;
	bool metadata_cache = false;
	bool brief = false;
	bool list_sets = false;
	bool targets = false;
	bool providers = false;
	bool info = false;
	bool doctor = false;
	bool search = false;
	bool install = false;
	bool audit_patch = false;
	bool bins = false;
	bool clean = false;
	bool depclean = false;
	bool all = false;
	bool staged = false;
	String report;
	int limit = 0;
	bool use_cache = true;
	bool rebuild_cache = false;
	String baseline;
	bool update_baseline = false;
	bool show_baseline = true;
	bool fail_on_baseline = false;
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

#ifndef _ide_Core_CoreBase_h_
#define _ide_Core_CoreBase_h_
#include <Core/Core.h>
#include <Esc/Esc.h>
#include <plugin/bz2/bz2.h>
#include <plugin/lz4/lz4.h>
#include <plugin/lzma/lzma.h>
#include <plugin/zstd/zstd.h>
#include "Logger.h"
#include "Host.h"

namespace Upp {

int CharFilterCid(int c);
int    ReadLNG(CParser& p);
String MakeLNG(int lang);
bool   OldLang();
String        PrintTime(int msecs);
inline String GetPrintTime(dword time0) { return PrintTime(msecs() - time0); }
bool   SaveChangedFile(const char *path, String data, bool delete_empty = false);
bool IsDoc(String s);
void CopyFile(const String& dst, const String& src, bool brc = false);
void CopyFolder(const char *_dst, const char *_src, Index<String>& used, bool all, bool brc = false);
class Workspace;
struct Ide;
class  Ctrl;
class  Image;
String CacheDir();
String CacheFile(const char *name);
void   ReduceCache();
void   ReduceCache(int mb_limit);
void   ReduceCacheFolder(const char *path, int64 max_total);
class PPInfo {
public:
	enum { AUTO, APPROVED, PROHIBITED };
	struct PPFile : Moveable<PPFile> {
		int                           scan_serial = 0;
		VectorMap<String, String>     flags; // "#if... flagXXXX", key - flagXXX, value - comment
		VectorMap<String, String>     all_defines; // #define ..., 1 - speculative
		VectorMap<String, String>     defines[2]; // #define ..., 1 - speculative
		Index<String>                 includes[2]; // 1 - speculative includes (in #if conditionals)
		Index<String>                 define_includes[2]; // #define LAYOUTFILE
		bool                          guarded = false; // has include guards
		int                           blitz = 0; // AUTO, APPROVED, PROHIBITED
		Time                          time = Null; // file time
		bool                          dirty = true; // need to be rechecked for change (filetime)
		void Dirty()                          { dirty = true; time = Null; }
		void Parse(Stream& in);
		void Serialize(Stream& s);
	};
	struct Dir : Moveable<Dir> {
		Index<String>           subdirs;
		VectorMap<String, Time> files;
		void Load(const String& dir);
	};
	ArrayMap<String, PPFile>                   files;
	Vector<String>                             includes; // include dirs
	int                                        includes_base_count; // for trimming out additional includes
	VectorMap<String, String>                  inc_cache; // cache for FindIncludeFile
	VectorMap<String, Dir>                     dir_cache; // cache for GetFileTime, FileExists, DirExists
	String                                     current_dir; // CurrentDirectory for NormalizePath
	VectorMap<String, String>                  normalize_path_cache; // cache for NormalizePath
	static std::atomic<int>                    scan_serial;
	VectorMap<String, Index<String> >          depends; // externally forced dependecies
	PPFile& File(const String& path);
public:
	static void           RescanAll()                                           { scan_serial++; }
	Event<const String&, const String&> WhenBlitzBlock;
	Time                  GetFileTime(const String& path);
	bool                  FileExists(const String& path)                        { return !IsNull(GetFileTime(path)); }
	String                NormalizePath(const String& path, const String& curr_dir);
	String                NormalizePath(const String& path)                     { return NormalizePath(path, current_dir); }
	String                FindIncludeFile(const char *s, const String& filedir, const Vector<String>& incdirs);
	String                FindIncludeFile(const char *s, const String& filedir);
	void                  SetIncludes(Vector<String>&& includes);
	void                  SetIncludes(const String& includes);
	void                  BaseIncludes();
	void                  AddInclude(const String& include);
	void                  Depends(const String& path, Index<String>& used);
	void                  Dirty(const String& path);
	void                  Dirty();
	const Index<String>&  GetIncludes(const String& path)                       { return File(path).includes[0]; }
	void                  AddDependency(const String& file, const String& dep);
	Time                  GatherDependencies(const String& path, VectorMap<String, Time>& result,
	                                         ArrayMap<String, Index<String>>& define_includes,
	                                         bool speculative, const String& include, Vector<String>& chain, bool& found);
	Time                  GatherDependencies(const String& path, VectorMap<String, Time>& result,
	                                         ArrayMap<String, Index<String>>& define_includes, bool speculative);
	Time                  GetTime(const String& path, VectorMap<String, Time> *ret_result = NULL);
	bool                  BlitzApproved(const String& path);
	void                  ClearDependencies();
	const VectorMap<String, String>& GetFileDefines(const String& path);
	PPInfo();
};
struct Assembly : Moveable<Assembly> {
	String name;
	String upp;
	bool operator==(const Assembly& b) const { return name == b.name && upp == b.upp; }
};
struct OptItem : Moveable<OptItem> {
	String text;
	String when;
	bool   valid;
	OptItem() { valid = true; }
};
struct CustomStep : Moveable<CustomStep> {
	String when;
	String ext;
	String command;
	String output;
	String AsString() const;
	void   Load(CParser& p);
	String GetExt() const;
	bool   MatchExt(const char *fn) const;
};
struct Package {
	struct Config : Moveable<Config> {
		String param;
		String name;
	};
	String          description;
	Array<OptItem>  uses;
	Array<OptItem>  target;
	Array<OptItem>  library;
	Array<OptItem>  static_library;
	Array<OptItem>  option;
	Array<OptItem>  link;
	Array<OptItem>  include;
	Array<OptItem>  flag;
	Array<OptItem>  pkg_config;
	Vector<String>  accepts;
	Vector<Config>  config;
	Array<CustomStep> custom;
	int             charset;
	bool            noblitz;
	bool            nowarnings;
	bool            bold;
	bool            italic;
	Color           ink;
	int             spellcheck_comments;
	int             tabsize;
	bool            cr;
	String          path;
	String          dir;
	Time            time;
	struct File : Moveable<File> {
		String name;
		String options;
		Array<OptItem> depends;
		String highlight;
		String charset_str;
		String flags;
		String patch;
		bool   readonly;
		bool   separator;
		bool   pch;
		bool   nopch;
		bool   noblitz;
		int    charset;
		int    tabsize;
		int    font;
		int    spellcheck_comments;
		Array<OptItem> option;
		File() { readonly = separator = pch = nopch = noblitz = false; charset = 0; tabsize = font = spellcheck_comments = Null; ink = Null; }
		File(const String& name) : name(name) { readonly = separator = pch = nopch = noblitz = false; charset = 0; tabsize = font = spellcheck_comments = Null; ink = Null; }
		int GetCount() const { return name.GetCount(); }
		String ToString() const { return name; }
		Color ink;
	};
	Array<File>     file;
	int            GetCount() const                { return file.GetCount(); }
	const File&    operator[](int i) const         { return file[i]; }
	File&          operator[](int i)              { return file[i]; }
	bool            Load(const char *path);
	bool            Save(const char *path) const;
	void            Reset();
	void            Option(bool& option, const char *name);
	static void     SetPackageResolver(bool (*Resolve)(const String& error, const String& path, int line));
	Package();
};
struct Nest : Moveable<Nest> {
	VectorMap<String, String> var;
	Vector<String> packages;
	String         name;
	String         path;
	mutable VectorMap<String, String> package_cache;
	bool           hub_loaded;
	bool           Load(const char *path);
	bool           Save(const char *path);
	String         GetPackagePath(const String& package) const;
	String         PackageDirectory(const String& name);
	String         PackageDirectory0(const String& name);
	void           InvalidatePackageCache();
	String         Get(const String& id);
	void           Set(const String& id, const String& val);
	Nest() { hub_loaded = false; }
};
Nest& MainNest();
String PackageDirectory(const String& name);
String GetPackagePathNest(const String& path);
String GetPathNest(const String& path);
String GetMethodName(const String& method);
String GetMethodFile(const String& method);
VectorMap<String, String> GetMethodVars(const String& method);
void                      SaveMethodVars(const String& method, const VectorMap<String, String>& var);
String BlitzBaseFile();
void   ResetBlitz();
struct Builder {
	Host   *host;
	String  method;
	String  outdir;
	static VectorMap<String, int> tmpfilei;
	virtual String GetTargetExt() const = 0;
	virtual void   AddFlags(Index<String>& cfg) = 0;
	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	                            String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int targetmode) = 0;
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap) = 0;
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout) = 0;
	void    ChDir(const String& path);
	String  GetPathQ(const String& path) const;
	Vector<Host::FileInfo> GetFileInfo(const Vector<String>& path) const;
	virtual String         GetBuildInfoPath() const { return Null; }
	virtual String         CompilerName() const { return Null; }
	virtual bool           IsInternalCompiler() const { return false; }
	Host::FileInfo GetFileInfo(const String& path) const;
	Time    GetFileTime(const String& path) const;
	static VectorMap<String, String> cmdx_cache;
	VectorMap<String, Time> dependencies;
	String  onefile;
	Time    HdependFileTime(const String& path);
	String  CmdX(const char *s);
	int     Execute(const char *cmdline);
	int     Execute(const char *cl, Stream& out);
	String  GetHostPath(const String& path);
	String  GetHostPathQ(const String& path);
	Builder() : host(nullptr) {}
	virtual ~Builder() {}
	Index<String>          config;
	String                 version;
	virtual void           AddMakeFile(struct MakeFile& makefile, String package,
		const Vector<String>& all_uses, const Vector<String>& all_libraries,
		const Index<String>& common_config, bool exporting) {}
	virtual void           AddCCJ(struct MakeFile& mfinfo, String package, const Index<String>& common_config, bool exporting, bool last_ws) {}
	virtual void           SaveBuildInfo(const String& package) {}
	virtual void           CleanPackage(const String& package, const String& outdir) {}
	virtual void           AfterClean() {}
	String                 compiler;
	Vector<String>         include;
	Vector<String>         libpath;
	String                 cpp_options;
	String                 c_options;
	String                 debug_options;
	String                 release_options;
	String                 debug_link;
	String                 release_link;
	String                 common_link;
	String                 debug_cuda;
	String                 release_cuda;
	String                 script;
	VectorMap<String, String> MacroMap;
	Vector<String>         macdef;
	Vector<String>         CINC;
	Vector<String> GetAllAccepts(int pk) const;
	Index<String>          pkg_config;
	bool                   allow_pch;
	bool                   main_conf;
	String                 target;
	String                 mainpackage;
	int                    start_time;
};
typedef Builder *(*BuilderCreateFn)();
VectorMap<String, BuilderCreateFn>& BuilderMap();
void RegisterBuilder(const char *name, BuilderCreateFn create);
class Workspace {
public:
	Vector<String> GetAllAccepts(int pk) const;
public:
	struct P : Moveable<P> {
		String  package;
		String  config;
		Package pkg;
		void Load(const String& name);
	};
	Vector<P>      package;
	Vector<int>    use_order;
public:
	int            GetCount() const                { return package.GetCount(); }
	const String&  operator[](int i) const        { return package[i].package; }
	const String&  GetConfig(int i) const         { return package[i].config; }
	const Package& GetPackage(int i) const        { return package[i].pkg; }
	Package&       GetPackage(int i)              { return package[i].pkg; }
	void           Scan(const char *mainpackage);
	void           Scan(const char *mainpackage, const char *mainconfig);
	void           Scan(const char *prjname, const Vector<String>& flag);
	void           Clear();
	void           AddLoad(const String& name);
	void           AddUses(Package& p, const Vector<String>& flag);
	void           Dump();
	Workspace() {}
};
struct IdeContext {
	virtual bool             IsVerbose() const = 0;
	virtual void             PutConsole(const char *s) = 0;
	virtual void             PutVerbose(const char *s) = 0;
	virtual void             PutLinking() = 0;
	virtual void             PutLinkingEnd(bool ok) = 0;
	virtual const Workspace& IdeWorkspace() const = 0;
	virtual bool             IdeIsBuilding() const = 0;
	virtual String           IdeGetOneFile() const = 0;
	virtual int              ConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false) = 0;
	virtual int              ConsoleExecute(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false) = 0;
	virtual int              ConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert) = 0;
	virtual int              ConsoleAllocSlot() = 0;
	virtual bool             ConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) = 0;
	virtual bool             ConsoleRun(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) = 0;
	virtual void             ConsoleFlush() = 0;
	virtual void             ConsoleBeginGroup(String group) = 0;
	virtual void             ConsoleEndGroup() = 0;
	virtual bool             ConsoleWait() = 0;
	virtual bool             ConsoleWait(int slot) = 0;
	virtual bool             ConsoleGetError() = 0;
	virtual bool             ConsoleIsRunning() = 0;
	virtual void             ConsoleKill(int slot) = 0;
	virtual void             ConsoleKill() = 0;
	virtual void             ConsoleClear() = 0;
	virtual void             ConsoleOnFinish(Event<>  cb) = 0;
	virtual void             IdeProcessEvents() = 0;
	virtual void             IdeSetBottom(Ctrl& ctrl) = 0;
	virtual void             IdeHideBottom() = 0;
	virtual void             IdeRemoveBottom(Ctrl& ctrl) = 0;
	virtual void             IdeSetRight(Ctrl& ctrl) = 0;
	virtual void             IdeRemoveRight(Ctrl& ctrl) = 0;
	virtual void             IdeHideRight() = 0;
	virtual void             IdeSetFocus() = 0;
	virtual void             IdeSetMain(const String& package) = 0;
	virtual void             IdeEditFile(const String& path) = 0;
	virtual void             IdeSetDebugPos(const String& fn, int line, const Image& img, int i) = 0;
	virtual void             IdeHideDebugPos() = 0;
	virtual String           IdeGetCurrentMainPackage() = 0;
	virtual int              IdeGetHydraThreads() = 0;
	virtual String           GetMethodName(const String& method) = 0;
	virtual void             IdeSetBar() = 0;
	virtual ~IdeContext() {}
};
void SetTheIdeContext(IdeContext *context);
IdeContext *TheIdeContext();
class BinObjInfo {
public:
	struct Block {
		String ident;
		String script;
		String file;
		bool   compress;
		int    encoding;
		int    flags;
		int    index;
		int    scriptline;
		int    length;
		enum { ENC_ZIP, ENC_BZ2, ENC_LZ4, ENC_LZMA, ENC_ZSTD };
		enum { FLG_ARRAY = 1, FLG_MASK = 2 };
		void   Compress(String& data);
	};
	ArrayMap<String, ArrayMap<int, Block>> blocks;
	void Parse(CParser& binscript, String base_dir);
	BinObjInfo();
};
String CleanupId(const char *s);
String CleanupPretty(const String& signature);
int FindLastId(const String& s, const String& id);
struct ItemTextPart : Moveable<ItemTextPart> {
	int pos;
	int len;
	int type;
	int ii;
	int pari;
};
Vector<ItemTextPart> ParsePretty(const String& name, const String& signature, int *fn_info = NULL);
struct MakeBuild;
struct MakeFile;
struct IdeMacro {
	String menu;
	String submenu;
	String name;
	dword  hotkey;
	EscValue code;
	IdeMacro();
};
Array<IdeMacro>& UscMacros();

} // namespace Upp

#endif

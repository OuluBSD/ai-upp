#include "Vfs.h"
#include <AICore2/AICore.h>
#include <ide/ide.h>

bool IsStruct(int kind);
bool IsFunction(int kind);

NAMESPACE_UPP

extern VfsValue*         (*IdeMetaEnvironment_FindDeclaration)(const VfsValue& n);
extern Vector<VfsValue*> (*IdeMetaEnvironment_FindDeclarationsDeep)(const VfsValue& n);
extern bool (*IsStructPtr)(int kind);
extern bool (*IsFunctionPtr)(int kind);
extern bool (*IsStructKindPtr)(const VfsValue& n);
extern String (*VfsValue_GetBasesString)(const VfsValue& v);


String GetBasesString(const VfsValue& v)
{
	String s;
	Vector<const VfsValue*> bases = v.AstFindAllShallow(CXCursor_CXXBaseSpecifier);
	for(const VfsValue* n : bases) {
		if(!s.IsEmpty())
			s.Cat(", ");
		s << n->id;
		const AstValue* a = *n;
		if (a)
			s << " (" << a->type << ")";
	}
	return s;
}


INITBLOCK {
	IdeMetaEnvironment_FindDeclarationsDeep  = &IdeMetaEnvironment::FindDeclarationsDeepStatic;
	IdeMetaEnvironment_FindDeclaration       = &IdeMetaEnvironment::FindDeclarationStatic;
	IsStructPtr = &::IsStruct;
	IsFunctionPtr = &::IsFunction;
	IsStructKindPtr = &IsStructKind;
	VfsValue_GetBasesString = &GetBasesString;
	FindNodeEnvPtr = &FindNodeEnv;
	IdeVfsFillDatasetPtrsPtr = &IdeVfsFillDatasetPtrs;
}

String GetAiPathCandidate(const String& includes_, String dir);


bool IsStructKind(const VfsValue& n)
{
	const AstValue* a = n;
	if (!a)
		return false;
	return	a->kind == CXCursor_StructDecl &&
			a->kind == CXCursor_ClassDecl &&
			a->kind == CXCursor_ClassTemplate &&
			a->kind == CXCursor_ClassTemplatePartialSpecialization;
}

IdeMetaEnvironment::IdeMetaEnvironment() : env(MetaEnv()) {
	
}


bool MakeRelativePath(const String& includes_, const String& dir, String& best_ai_dir,
                      String& best_rel_dir)
{
	bool found = false;
	Vector<String> ai_dirs = GetAiDirsRaw();
	if(!ai_dirs.IsEmpty()) {
		int def_cand_parts = INT_MAX;
		String ai_dir = ai_dirs.Top();
		String includes = includes_;
		MergeWith(includes, ";", GetClangInternalIncludes());
		for(const String& s : Split(includes, ';')) {
#ifdef PLATFORM_WIN32 // we need to ignore internal VC++ headers
			static VectorMap<String, bool> use;
			int q = use.Find(s);
			if(q < 0) {
				q = use.GetCount();
				use.Add(s, !FileExists(AppendFileName(s, "vcruntime.h")));
			}
			if(use[q])
#endif
			{
				if(dir.Find(s) == 0) {
					String rel_dir = dir.Mid(s.GetCount());
					int cand_parts = Split(rel_dir, DIR_SEPS).GetCount();
					// Prefer the shortest directory
					if(cand_parts < def_cand_parts) {
						best_ai_dir = ai_dir;
						best_rel_dir = rel_dir;
						found = true;
					}
				}
			}
		}
	}
	return found;
}

void Assign(VfsValue& mn, VfsValue* owner, const ClangNode& n)
{
	mn.owner = owner;
	int c = n.sub.GetCount();
	mn.sub.SetCount(c);
	for(int i = 0; i < c; i++)
		UPP::Assign(mn.sub[i], &mn, n.sub[i]);
	mn.id = n.id;
	mn.type_hash = n.type_hash;
	AstValue& to = mn;
	to.kind = n.kind;
	to.type = n.type;
	to.begin = n.begin;
	to.end = n.end;
	to.filepos_hash = n.filepos_hash;
	to.is_ref = n.is_ref;
	to.is_definition = n.is_definition;
}


void Store(IdeMetaEnvironment& ienv, String& includes, const String& path, ClangNode& cn)
{
	auto& env = ienv.env;
	ClangTypeResolver ctr;
	if(!ctr.Process(cn)) {
		LOG("MetaEnvironment::Store: error: clang type resolving failed: " + ctr.GetError());
		return;
	}
	if(!env.MergeResolver(ctr)) {
		LOG("MetaEnvironment::Store: error: merging resolver failed");
		return;
	}

	cn.TranslateTypeHash(ctr.GetTypeTranslation());

	// LOG(n.GetTreeString());
	VfsSrcFile& file = ienv.ResolveFile(includes, path);
	VfsSrcPkg& pkg = *file.pkg;
	VfsValue n;
	int file_id = pkg.GetFileId(path);
	ASSERT(file_id >= 0);
	UPP::Assign(n, 0, cn);
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file_id);
	n.RealizeSerial();
	if(!env.MergeValue(env.root, n, MERGEMODE_OVERWRITE_OLD))
		return;

	pkg.Store(false);
}

void UpdateWorkspace(IdeMetaEnvironment& ienv, Workspace& wspc) {
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		String pkg_name = wspc.package.GetKey(i);
		auto& pkg = wspc.package[i];
		String dir = GetFileDirectory(pkg.path);
		VfsSrcPkg& mpkg = ienv.GetAddPkg(dir);
		for (auto& file : pkg.file) {
			if (file.separator) continue;
			String filename = file;
			String ext = GetFileExt(filename);
			String path = AppendFileName(dir, filename);
			if (EcsIndexer::AcceptExt(ext))
				EcsIndexer::RunPath(path);
		}
	}
}





void ClearTempCheck(int src, int id);
int CreateTempCheck(int src);

VfsValue& VfsSrcFile::GetTemp() {
	return *temp;
}

VfsValue& VfsSrcFile::CreateTemp(int dbg_src) {
	ASSERT(temp.IsEmpty());
	temp_id = CreateTempCheck(dbg_src);
	VfsValue& n = temp.Create();
	AstValue& a = n;
	a.kind = Cursor_Namespace;
	return n;
}

void VfsSrcFile::ClearTemp(int dbg_src) {
	ClearTempCheck(dbg_src, temp_id);
	temp.Clear();
	temp_id = -1;
}

void VfsSrcFile::FixAstValue()
{
	// This is a hotfix after moving clang related data to AstValue in VfsValue::value
	// Nothing makes sense completely yet
	if (temp) {
		AstValue& a = *temp;
		if (a.kind < 0)
			a.kind = Cursor_Namespace;
	}
}

void VfsSrcFile::Visit(Vis& v)
{
	if (v.IsLoading()) {
		if (!temp.IsEmpty())
			ClearTemp(2); // TODO this is very unoptimized clear
		CreateTemp(1);
	}
	else if (v.IsStoring())
		UpdateStoring();
	v.Ver(1)
	(1)	("highest_seen_serial", (int64&)highest_seen_serial)
	    ("seen_types", (VectorMap<int64,String>&)seen_types)
	    ("root", *temp, VISIT_NODE)
	    ;
	if (!v.IsHashing())
		v("hash", saved_hash);
	if (v.IsLoading()) {
		/*if (saved_hash.GetCount()) {
			LOG(saved_hash);
		}*/
		FixAstValue();
		UpdateLoading();
	}
}

String VfsSrcFile::UpdateStoring()
{
	ASSERT_(managed_file, "Trying to jsonize non-managed file");
	if (temp.IsEmpty()) return String();
	VfsValue& file_nodes = *temp;
	
	int file_id = GetId();
	ASSERT(file_id >= 0 && pkg->id >= 0);
	if (keep_file_ids)
		file_nodes.SetPkgDeep(pkg->id);
	else
		file_nodes.SetPkgFileDeep(pkg->id, file_id);
	// LOG(file_nodes.GetTreeString());
	
	String old_hash = saved_hash;
	bool total_hash_diffs = false;
	saved_hash = IntStr64(file_nodes.AstGetSourceHash(&total_hash_diffs));
	
	/*if (1) {
		LOG(file_nodes.GetSourceHashDump());
	}*/
	
	ASSERT(!saved_hash.IsEmpty());
	ASSERT(!full_path.IsEmpty());
	RefreshSeenTypes();
	
	return old_hash; // return old hash for reverting
}

#if 0
String VfsSrcFile::StoreJson()
{
	String old_hash = UpdateStoring();
	String json = VisitToJson(*this);
	
	saved_hash = old_hash;
	
	ClearTemp(3);
	return json;
}
#endif

bool VfsSrcFile::Store(bool forced)
{
	ASSERT_(managed_file, "Trying to overwrite non-managed file");
	ASSERT(temp);
	VfsValue& file_nodes = *temp;
	
	int file_id = GetId();
	ASSERT(file_id >= 0 && pkg->id >= 0);
	if (keep_file_ids)
		file_nodes.SetPkgDeep(pkg->id);
	else
		file_nodes.SetPkgFileDeep(pkg->id, file_id);
	// LOG(file_nodes.GetTreeString());
	
	// Optional REFRENCES.md file for Codex AI
	bool is_ref_file = GlobalCreateReferencesFile();
	String ref_file;
	if (is_ref_file) {
		ref_file = AppendFileName(GetFileDirectory(full_path), "REFERENCES.md");
		if (!FileExists(ref_file)) {
			forced = true;
		}
	}
	
	bool total_hash_diffs = false;
	String hash = IntStr64(file_nodes.AstGetSourceHash(&total_hash_diffs));
	if(!forced) {
		if(saved_hash == hash && !total_hash_diffs) {
			ClearTemp(11);
			return true;
		}
		if(saved_hash == hash && total_hash_diffs) {
			TODO;
		}
		else if(total_hash_diffs) {
			TODO;
		}
	}
	saved_hash = hash;
	ASSERT(!saved_hash.IsEmpty());
	ASSERT(!full_path.IsEmpty());
	RefreshSeenTypes();
	
	lock.Enter();
	RealizeDirectory(GetFileDirectory(full_path));
	bool succ = true;
	
	if (IsDirTree()) {
		VersionControlSystem vcs;
		vcs.Initialize(full_path, true);
		Vis vis(vcs);
		this->Visit(vis);
		vcs.Close();
	}
	else if (IsBinary()) {
		FileOut s(full_path);
		Vis vis(s);
		this->Visit(vis);
		s.Close();
	}
	else {
		succ = VisitToJsonFile(*this, full_path);
	}
	
	if (is_ref_file) {
		StoreSimpleReferences(ref_file);
	}
	
	lock.Leave();
	
	ClearTemp(4);
	return succ;
}

bool VfsSrcFile::Load()
{
	if (!temp.IsEmpty()) {
		// Wait few seconds
		TimeStop ts;
		while (!temp.IsEmpty() && ts.Seconds() < 5.0)
			Sleep(100);
		if (!temp.IsEmpty())
			return false;
		//DLOG("temp was freed");
	}
	lock.Enter();
	saved_hash.Clear();
	
	#ifdef flagDEBUG
	bool temp_is_empty = temp.IsEmpty();
	if (!temp_is_empty) {
		auto* ptr = &*temp;
		LOG("Temp was not cleared: id=" << temp_id);
		Panic("Temporary VfsValue was not cleared previously");
		ASSERT(0);
	}
	#endif
	CreateTemp(2);
	
	ASSERT(this->full_path.GetCount());
	bool succ = true;
	if (IsDirTree()) {
		VersionControlSystem vcs;
		vcs.Initialize(full_path, false);
		Vis vis(vcs);
		this->Visit(vis);
		vcs.Close();
	}
	else if (IsBinary()) {
		FileIn s(full_path);
		Vis vis(s);
		if(s.GetSize() > 0)
			this->Visit(vis);
		s.Close();
	}
	else {
		succ = VisitFromJsonFile(*this, full_path);
	}
	
	lock.Leave();
	return succ && !saved_hash.IsEmpty();
}

bool VfsSrcFile::StoreSimpleReferences(const String& ref_file) {
	
	// Load Markdown
	
	// Mark all in the Markdown to be deleted, unless revived
	
	
	return true;
}

void VfsSrcFile::UpdateLoading() {
	int file_id = GetId();
	ASSERT(file_id >= 0 && pkg->id >= 0);
	temp->SetPkgDeep(pkg->id);

	OnSeenTypes();
	OnSerialCounter();
	temp->RealizeSerial();
	lock.Leave();
	
// see SRC_TXT_HEADER_ENABLE
	#ifndef flagV1
	if (GetFileExt(full_path) == ".db-src") {
		SrcTxtHeader* src = dynamic_cast<SrcTxtHeader*>(&*temp->ext);
		ASSERT(src);
		DatasetIndex().GetAdd(full_path) = src;
	}
	#endif
}

bool VfsSrcFile::LoadJson(String json) {
	lock.Enter();
	saved_hash.Clear();
	
	ASSERT_(!temp, "Temporary VfsValue was not cleared previously");
	CreateTemp(3);
	VisitFromJson(*this, json);
	
	lock.Leave();
	return !saved_hash.IsEmpty();
}

void VfsSrcFile::OnSerialCounter()
{
	MetaEnvironment& env = MetaEnv();
	env.serial_counter =
		max(env.serial_counter,
		this->highest_seen_serial);
}

void VfsSrcFile::OnSeenTypes()
{
	MetaEnvironment& env = MetaEnv();
	for(auto it : ~seen_types)
		env.types.GetAdd(it.key).seen_type = it.value;
}

void VfsSrcFile::RefreshSeenTypes()
{
	ASSERT(temp);
	VfsValue& file_nodes = *temp;
	MetaEnvironment& env = MetaEnv();
	Index<hash_t> type_hashes;
	file_nodes.GetTypeHashes(type_hashes);
	seen_types.Clear();
	for(hash_t h : type_hashes) {
		int i = env.types.Find(h);
		if(i >= 0)
			seen_types.Add(h, env.types[i].seen_type);
	}
	highest_seen_serial = env.CurrentSerial();
}

void VfsSrcFile::MakeTempFromEnv(bool all_files) {
	IdeMetaEnvironment& ienv = IdeMetaEnv();
	MetaEnvironment& env = ienv.env;
	CreateTemp(4);
	#ifdef flagDEBUG
	//LOG(env.root.GetTreeString());
	env.root.DeepChk();
	#endif
	int file_id = GetId();
	ASSERT(file_id >= 0);
	if (all_files)
		ienv.SplitValue(env.root, *temp, pkg->id);
	else
		ienv.SplitValue(env.root, *temp, pkg->id, file_id);
}

int VfsSrcFile::GetId() const {
	if (!pkg)
		return -1;
	String rel_path = pkg->GetRelativePath(full_path);
	int file_id = pkg->rel_files.Find(rel_path);
	ASSERT(file_id >= 0);
	return file_id;
}















void VfsSrcPkg::Init()
{
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	VfsSrcFile& file = GetAddFile(path);
	file.KeepFileIndices();
	file.ManageFile();
	int file_id = file.GetId();
	ASSERT(file_id >= 0 && file.rel_path.GetCount());
}

bool VfsSrcPkg::Store(bool forced)
{
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	IdeMetaEnvironment& env = IdeMetaEnv();
	VfsSrcFile& file = GetAddFile(path);
	VfsSrcPkg& pkg = *file.pkg;
	VfsValue& file_nodes = file.CreateTemp(5);
	int file_id = file.GetId();
	ASSERT(file_id >= 0 && pkg.id >= 0);
	env.SplitValue(env.env.root, file_nodes, pkg.id);
	return file.Store(forced);
}


#if 0

void VfsSrcPkg::PostSave()
{
	if(!post_saving) {
		post_saving = true;
		PostCallback(THISBACK1(Save, false));
	}
}

void VfsSrcPkg::Clear() { files.Clear(); }

void VfsSrcPkg::SetPath(String bin_path, String upp_dir)
{
	this->bin_path = bin_path;
	// dir = GetFileDirectory(path);
	this->upp_dir = upp_dir;
}

void VfsSrcPkg::Load()
{
	Clear();
	if(FileExists(path)) {
		//lock.Enter(); // useless here
		LoadFromJsonFile(*this, path);
		//lock.Leave();
		if (saved_hash.IsEmpty())
			saved_hash = GetHashSha1();
	}
}

void VfsSrcPkg::Save(bool forced)
{
	post_saving = false;
	if(!path.IsEmpty()) {
		// The hash value must be both 32-bit and 64-bit compatible,
		// so use the sha1 hasher instead of the fast hasher
		String current_sha1 = GetHashSha1();
		
		//lock.Enter(); // useless here
		if (current_sha1 != saved_hash || forced) {
			saved_hash = current_sha1;
			RealizeDirectory(GetFileDirectory(path));
			StoreAsJsonFile(*this, path, true);
		}
		//lock.Leave();
	}
}

thread_local static bool aionfile_getting_sha1;

String VfsSrcPkg::GetHashSha1() {
	Sha1Stream s;
	aionfile_getting_sha1 = true;
	s % *this;
	aionfile_getting_sha1 = false;
	return s.FinishString();
}

void VfsSrcPkg::Serialize(Stream& s)
{
	{
		Mutex::Lock ml(lock);
		
		byte version = 1;
		s % version;
		
		// DO NOT READ "saved_hash" HERE AS THIS Serialize FUNCTION IS USED FOR GETTING THE HASH
		//// s % saved_hash; <-- NO!
		
		if (version >= 1)
			s % files;
	}
	
	if (!aionfile_getting_sha1 && s.IsLoading() && saved_hash.IsEmpty())
		saved_hash = GetHashSha1();
}

void VfsSrcPkg::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	json
		("saved_hash", saved_hash) // it's fine here
		("files", files)
		;
}

void VfsSrcPkg::operator=(const VfsSrcPkg& f) {
	Mutex::Lock ml(lock);
	files <<= f.files;
	saved_hash = f.saved_hash;
	path = f.path;
	dir = f.dir;
}
#endif

String VfsSrcPkg::GetTitle() const {
	if (title.IsEmpty()) {
		if (dir.Right(1) == DIR_SEPS)
			title = GetFileTitle(dir.Left(dir.GetCount()-1));
		else
			title = GetFileTitle(dir);
	}
	return title;
}

String VfsSrcPkg::GetRelativePath(const String& path) const
{
	String dir = GetDirectory();
	int i = path.Find(dir);
	if(i == 0) {
		String s = path.Mid(dir.GetCount());
		if(s.GetCount() && s[0] == DIR_SEP)
			s = s.Mid(1);
		return s;
	}
	else
		TODO; // return NormalizePath(path, dir);
	return String();
}

String VfsSrcPkg::GetFullPath(const String& rel_path) const
{
	String dir = GetDirectory();
	return AppendFileName(dir, rel_path);
}

String VfsSrcPkg::GetFullPath(int file_i) const
{
	return AppendFileName(dir, rel_files[file_i]);
}

void VfsSrcPkg::Visit(Vis& v) {
	v.Ver(1)
	(1)	("src_files", src_files, VISIT_VECTOR)
		("rel_files", rel_files)
		;
}

#if 0
VfsSrcFile& VfsSrcPkg::RealizePath(const String& includes, const String& path)
{
	String rel_file = NormalizePath(path, dir);
	int a = rel_file.Find(dir);
	if(a == 0)
		rel_file = rel_file.Mid(dir.GetCount());
	else {
		String def_dir = GetFileDirectory(path);
		Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
		if (!upp_dirs.IsEmpty()) {
			String& upp_dir = upp_dirs[0];
			ASSERT(path.Find(upp_dir) == 0);
			rel_file = path.Mid(upp_dir.GetCount());
		}
		else {
			String ai_dir, rel_dir;
			if (MakeRelativePath(includes, def_dir, ai_dir, rel_dir)) {
				String filename = GetFileName(path);
				rel_file = AppendFileName(rel_dir, filename);
			}
		}
	}
#ifdef flagWIN32
	rel_file.Replace(DIR_SEPS, "/");
#endif
	if (rel_file.GetCount() && rel_file[0] == '/')
		rel_file = rel_file.Mid(1);
	
	Mutex::Lock ml(lock);
	int i = files.Find(rel_file);
	if(i >= 0) {
		VfsSrcFile& o = files[i];
		return o;
	}
	VfsSrcFile& o = files.Add(rel_file);
	return o;
}
#endif

VfsSrcFile& VfsSrcPkg::GetMetaFile() {
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	return GetAddFile(path);
}

int VfsSrcPkg::GetAddRelPath(const String& rel_path)
{
	int i = rel_files.FindAdd(rel_path);
	return i;
}

VfsSrcFile& VfsSrcPkg::GetAddFile(const String& full_path)
{
	String dir = GetDirectory();
	String rel_path = GetRelativePath(full_path);
	int file_id = GetAddRelPath(rel_path);
	String src_rel_path = GetRelSrcPath(rel_path);
	for (VfsSrcFile& f : src_files) {
		if (f.rel_path == src_rel_path) {
			int file_id = f.GetId();
			ASSERT(file_id >= 0);
			return f;
		}
	}
	VfsSrcFile& f = src_files.Add();
	f.pkg = this;
	f.rel_path = src_rel_path;
	f.full_path = AppendFileName(dir, src_rel_path);
	f.env_file = GetFileExt(f.full_path) == ".env";
	ASSERT(f.full_path.Right(2) != ".d");
	return f;
}

int VfsSrcPkg::FindFile(String path) const {
	String rel_path = GetRelativePath(path);
	return rel_files.Find(rel_path);
}

int VfsSrcPkg::GetFileId(const String& path) const {
	String rel_path = GetRelativePath(path);
	return rel_files.Find(rel_path);
}

String IdeMetaEnvironment::ResolveVfsSrcPkgPath(const String& includes, String path,
                                              String& ret_upp_dir)
{
	String def_dir = GetFileDirectory(path);
	Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
	for(String& upp_dir : upp_dirs) {
		ret_upp_dir = upp_dir;
		return GetAiPathCandidate(includes, upp_dir);
	}
	ret_upp_dir = def_dir;
	return GetAiPathCandidate(includes, def_dir);
}

VfsSrcFile& IdeMetaEnvironment::ResolveFile(const String& includes, const String& path)
{
	String upp_dir;
	String pkg_path = ResolveVfsSrcPkgPath(includes, path, upp_dir);
	env.lock.EnterWrite();
	VfsSrcPkg& pkg = GetAddPkg(upp_dir);
	ASSERT(pkg.id >= 0);
	VfsSrcFile& file = pkg.GetAddFile(pkg_path);
	int file_id = file.GetId();
	ASSERT(file_id >= 0);
	env.lock.LeaveWrite();
	return file;
}

int IdeMetaEnvironment::FindPkg(String dir) const {
	ASSERT(dir.Find(META_FILENAME) < 0); // NOT HERE
	ASSERT(DirectoryExists(dir));
	if (dir.Right(1) != DIR_SEPS)
		dir.Cat(DIR_SEPS);
	int i = 0;
	for (const auto& pkg : pkgs) {
		if (pkg.dir == dir)
			return i;
		i++;
	}
	return -1;
}

VfsSrcPkg& IdeMetaEnvironment::GetAddPkg(String dir) {
	ASSERT(dir.Find(".bin") < 0); // NOT HERE
	ASSERT(DirectoryExists(dir));
	if (dir.Right(1) != DIR_SEPS)
		dir.Cat(DIR_SEPS);
	for (auto& pkg : pkgs) {
		if (pkg.dir == dir)
			return pkg;
	}
	int id = pkgs.GetCount();
	auto& pkg = pkgs.Add();
	pkg.dir = dir;
	pkg.id = id;
	pkg.Init();
	return pkg;
}

Vector<VfsValue*> IdeMetaEnvironment::FindAllEnvs() {
	Vector<VfsValue*> v;
	for (const VfsSrcPkg& pkg : pkgs) {
		for (const VfsSrcFile& file : pkg.src_files) {
			if (file.env_file) {
				int file_id = file.GetId();
				VfsValue& n = RealizeFileNode(pkg.id, file_id, AsTypeHash<PkgEnv>());
				v << &n;
			}
		}
	}
	return v;
}

VfsValue* IdeMetaEnvironment::FindNodeEnv(Entity& n)
{
	String ctx = n.Data("ctx");
	if (ctx.IsEmpty())
		return 0;
	for (const VfsSrcPkg& pkg : pkgs) {
		for (const VfsSrcFile& file : pkg.src_files) {
			if (file.env_file) {
				int file_id = file.GetId();
				VfsValue& fn = RealizeFileNode(pkg.id, file_id, AsTypeHash<PkgEnv>());
				for (VfsValue& s : fn.sub)
					if (s.type_hash == AsTypeHash<Context>() && s.id == ctx)
						return &s;
			}
		}
	}
	return 0;
}

// see SRC_TXT_HEADER_ENABLE
VfsValue* IdeMetaEnvironment::LoadDatabaseSourceVisit(VfsSrcFile& file, String path, Vis& v) {
	if (!file.pkg)
		return 0;
	int file_id = file.GetId();
	VfsValue& filenode = RealizeFileNode(file.pkg->id, file_id, AsTypeHash<SrcTxtHeader>());
	if (!filenode.ext) {
		One<SrcTxtHeader> ext;
		ext.Create(filenode);
		ext->filepath = path;
		ext->Visit(v);
		DatasetIndex().GetAdd(path) = &*ext;
		filenode.serial = env.NewSerial();
		filenode.ext = ext.Detach();
		filenode.type_hash = filenode.ext->GetTypeHash();
		ASSERT(&filenode.ext->val == &filenode);
	}
	if (filenode.id.IsEmpty())
		filenode.id = GetFileTitle(path);
	return &filenode;
}

bool IdeMetaEnvironment::LoadFileRoot(const String& includes, const String& path, bool manage_file)
{
	VfsSrcFile& file = ResolveFile(includes, path);
	file.ManageFile(manage_file);
	
// see SRC_TXT_HEADER_ENABLE
	// Hotfix for old .db-src file
	if (GetFileExt(path) == ".db-src") {
		String json = LoadFile(path);
		Value jv = ParseJSON(json);
		JsonIO j(jv);
		Vis vis(j);
		if (LoadDatabaseSourceVisit(file, path, vis)) {
			file.ClearTemp(5);
			return true;
		}
	}
	
	file.lock.Enter();
	if(file.Load()) {
		OnLoadFile(file);
		file.ClearTemp(6);
		file.lock.Leave();
		return true;
	}
	file.ClearTemp(7);
	file.lock.Leave();
	return false;
}

bool IdeMetaEnvironment::LoadFileRootVisit(const String& includes, const String& path, Vis& v, bool manage_file, VfsValue*& file_node) {
	file_node = 0;
	
	VfsSrcFile& file = ResolveFile(includes, path);
	int file_id = file.GetId();
	file.ManageFile(manage_file);
	
	// Hotfix for old .db-src file
	String ext = GetFileExt(path);
	if (ext == ".db-src") {
		VfsValue* fn = LoadDatabaseSourceVisit(file, path, v);
		if (fn) {
			file_node = fn;
			return true;
		}
	}
	else if (ext == ".ecs") {
		VfsValue& fn = RealizeFileNode(file.pkg->id, file_id, 0);
		if (fn.id.IsEmpty())
			fn.id = GetFileTitle(path);
		file_node = &fn;
	}
	else if (ext == ".env") {
		VfsValue& fn = RealizeFileNode(file.pkg->id, file_id, AsTypeHash<PkgEnv>());
		if (fn.id.IsEmpty())
			fn.id = GetFileTitle(path);
		file_node = &fn;
	}
	else {
		TODO;
	}
	
	ASSERT(v.IsLoading());
	file.Visit(v);
	if (!v.IsError()) {
		OnLoadFile(file);
		file.ClearTemp(8);
		return true;
	}
	file.ClearTemp(9);
	return false;
}

VfsSrcFile& IdeMetaEnvironment::Load(const String& includes, const String& path)
{
	VfsSrcFile& file = ResolveFile(includes, path);
	if(file.Load()) {
		OnLoadFile(file);
	}
	file.ClearTemp(10);
	return file;
}

void IdeMetaEnvironment::OnLoadFile(VfsSrcFile& file)
{
	VfsValue& file_nodes = file.GetTemp();
	file_nodes.SetTempDeep();
	if (!file.saved_hash.IsEmpty()) {
		if (!(file.saved_hash == IntStr64(file_nodes.AstGetSourceHash()))) {
			DUMP(file.saved_hash.GetCount());
			LOG(file.saved_hash);
			LOG(IntStr64(file_nodes.AstGetSourceHash()));
			LOG(file_nodes.GetSourceHashDump());
		}
		ASSERT(file.saved_hash == IntStr64(file_nodes.AstGetSourceHash()));
	}
	//file_nodes.SetPkgDeep(pkg.id);
	//LOG(file_nodes.GetTreeString());
	env.MergeValue(env.root, file_nodes, MERGEMODE_OVERWRITE_OLD);
	// LOG(root.GetTreeString());

#if DEBUG_METANODE_DTOR
	Vector<VfsValue*> comments;
	root.FindAllDeep(METAKIND_COMMENT, comments);
	for(auto* c : comments)
		c->trace_kill = true;
#endif
}

VfsValue& IdeMetaEnvironment::RealizeFileNode(int pkg, int file, hash_t type_hash) {
	for (auto& s : env.root.sub) {
		if (s.pkg == pkg && s.file == file && s.type_hash == type_hash) {
			if (!s.serial)
				s.serial = env.NewSerial();
			return s;
		}
	}
	VfsValue& n = env.root.sub.Add();
	n.owner = &env.root;
	auto& p = pkgs[pkg];
	n.id = file >= 0 && file < p.rel_files.GetCount() ? GetFileTitle(p.rel_files[file]) : String();
	n.serial = env.NewSerial();
	n.file = file;
	n.pkg = pkg;
	AstValue& a = n;
	a.kind = CXCursor_Namespace;
	if (type_hash)
		n.CreateExt(type_hash);
	return n;
}

void IdeMetaEnvironment::SplitValue(VfsValue& root, VfsValueSubset& other, int pkg_id)
{
	root.PointPkgTo(other, pkg_id);
}

void IdeMetaEnvironment::SplitValue(VfsValue& root, VfsValueSubset& other, int pkg_id, int file_id)
{
	root.PointPkgTo(other, pkg_id, file_id);
}

void IdeMetaEnvironment::SplitValue(const VfsValue& root, VfsValue& other, int pkg_id)
{
	root.CopyPkgTo(other, pkg_id);
}

void IdeMetaEnvironment::SplitValue(const VfsValue& root, VfsValue& other, int pkg_id, int file_id)
{
	root.CopyPkgTo(other, pkg_id, file_id);
}

String IdeMetaEnvironment::GetFilepath(int pkg_id, int file_id) const
{
	const auto& pkg = this->pkgs[pkg_id];
	String path = pkg.GetFullPath(file_id);
	return path;
}

VfsValue* IdeMetaEnvironment::FindDeclarationStatic(const VfsValue& n) {
	return IdeMetaEnv().FindDeclaration(n);
}

Vector<VfsValue*> IdeMetaEnvironment::FindDeclarationsDeepStatic(const VfsValue& n) {
	return IdeMetaEnv().FindDeclarationsDeep(n);
}

VfsValue* IdeMetaEnvironment::FindDeclaration(const VfsValue& n)
{
	auto& env = MetaEnv();
	const AstValue* b = n;
	if (!b)
		return 0;
	if(!b->filepos_hash)
		return 0;
	int i = env.filepos.Find(b->filepos_hash);
	if(i < 0)
		return 0;
	const auto& vec = env.filepos[i].hash_nodes;
	for(const auto& ptr : vec) {
		if(!ptr)
			continue;
		VfsValue& p = *ptr;
		const AstValue* a1 = p;
		if(a1 && a1->is_definition /* || p.IsClassTemplateDefinition()*/)
			return &p;
	}
	return 0;
}

Vector<VfsValue*> IdeMetaEnvironment::FindDeclarationsDeep(const VfsValue& n)
{
	const AstValue* a = n;
	Vector<VfsValue*> v;
	if(a && a->kind == CXCursor_CXXBaseSpecifier) {
		for(const auto& s : n.sub) {
			VfsValue* d = FindDeclaration(s);
			if(d)
				v.Add(d);
		}
		return v;
	}
	else
		TODO;
	return v;
}

bool IsMergeable(CXCursorKind kind)
{
	return IsMergeable((int)kind);
}

IdeMetaEnvironment& IdeMetaEnv()
{
	static IdeMetaEnvironment env;
	return env;
}

VfsValue* FindNodeEnv(Entity& n) {
	return IdeMetaEnv().FindNodeEnv(n);
}

VfsValue* IdeVfsFillDatasetPtrs(DatasetPtrs& p, hash_t type_hash) {
	if (type_hash == AsTypeHash<SrcTxtHeader>()) {
		for (auto db : ~DatasetIndex()) {
			VfsValueExt& ext = *db.value;
			if (ext.val.type_hash == type_hash)
				return &ext.val;
		}
	}
	else TODO;
	return 0;
}


END_UPP_NAMESPACE

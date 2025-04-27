#include "Meta.h"
#include <AICore/AICore.h>

NAMESPACE_UPP

String GetAiPathCandidate(const String& includes_, String dir);

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

void Assign(MetaNode& mn, MetaNode* owner, const ClangNode& n)
{
	mn.owner = owner;
	int c = n.sub.GetCount();
	mn.sub.SetCount(c);
	for(int i = 0; i < c; i++)
		UPP::Assign(mn.sub[i], &mn, n.sub[i]);
	mn.kind = n.kind;
	mn.id = n.id;
	mn.type = n.type;
	mn.type_hash = n.type_hash;
	mn.begin = n.begin;
	mn.end = n.end;
	mn.filepos_hash = n.filepos_hash;
	mn.is_ref = n.is_ref;
	mn.is_definition = n.is_definition;
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
	MetaSrcFile& file = ienv.ResolveFile(includes, path);
	MetaSrcPkg& pkg = *file.pkg;
	MetaNode n;
	UPP::Assign(n, 0, cn);
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file.id);
	n.RealizeSerial();
	if(!env.MergeNode(env.root, n, MERGEMODE_OVERWRITE_OLD))
		return;

	pkg.Store(false);
}

void UpdateWorkspace(IdeMetaEnvironment& ienv, Workspace& wspc) {
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		String pkg_name = wspc.package.GetKey(i);
		auto& pkg = wspc.package[i];
		String dir = GetFileDirectory(pkg.path);
		MetaSrcPkg& mpkg = ienv.GetAddPkg(dir);
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





void ClearTempCheck(int id);
int CreateTempCheck(int src);

MetaNode& MetaSrcFile::GetTemp() {
	return *temp;
}

MetaNode& MetaSrcFile::CreateTemp(int dbg_src) {
	ASSERT(temp.IsEmpty());
	temp_id = CreateTempCheck(dbg_src);
	return temp.Create();
}

void MetaSrcFile::ClearTemp() {
	ClearTempCheck(temp_id);
	temp.Clear();
	temp_id = -1;
}

void MetaSrcFile::Visit(Vis& v)
{
	if (v.IsLoading()) {
		if (!temp.IsEmpty())
			ClearTemp(); // TODO this is very unoptimized clear
		CreateTemp(1);
	}
	else if (v.IsStoring())
		UpdateStoring();
	v.Ver(1)
	(1)	("id", id)
		("hash", saved_hash)
		("highest_seen_serial", (int64&)highest_seen_serial)
	    ("seen_types", (VectorMap<int64,String>&)seen_types)
	    ("root", *temp, VISIT_NODE)
	    ;
	if (v.IsLoading()) {
		UpdateLoading();
	}
}

String MetaSrcFile::UpdateStoring()
{
	ASSERT_(managed_file, "Trying to jsonize non-managed file");
	if (temp.IsEmpty()) return String();
	MetaNode& file_nodes = *temp;
	
	ASSERT(id >= 0 && pkg->id >= 0);
	if (keep_file_ids)
		file_nodes.SetPkgDeep(pkg->id);
	else
		file_nodes.SetPkgFileDeep(pkg->id, id);
	// LOG(file_nodes.GetTreeString());
	
	String old_hash = saved_hash;
	bool total_hash_diffs = false;
	saved_hash = IntStr64(file_nodes.GetSourceHash(&total_hash_diffs));
	
	ASSERT(!saved_hash.IsEmpty());
	ASSERT(!full_path.IsEmpty());
	RefreshSeenTypes();
	
	return old_hash; // return old hash for reverting
}

#if 0
String MetaSrcFile::StoreJson()
{
	String old_hash = UpdateStoring();
	String json = VisitToJson(*this);
	
	saved_hash = old_hash;
	
	ClearTemp();
	return json;
}
#endif

bool MetaSrcFile::Store(bool forced)
{
	ASSERT_(managed_file, "Trying to overwrite non-managed file");
	ASSERT(temp);
	MetaNode& file_nodes = *temp;
	
	ASSERT(id >= 0 && pkg->id >= 0);
	if (keep_file_ids)
		file_nodes.SetPkgDeep(pkg->id);
	else
		file_nodes.SetPkgFileDeep(pkg->id, id);
	// LOG(file_nodes.GetTreeString());
	
	bool total_hash_diffs = false;
	String hash = IntStr64(file_nodes.GetSourceHash(&total_hash_diffs));
	if(!forced) {
		if(saved_hash == hash && !total_hash_diffs)
			return true;
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
	
	lock.Leave();
	
	ClearTemp();
	return succ;
}

bool MetaSrcFile::Load()
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
		Panic("Temporary MetaNode was not cleared previously");
		ASSERT(0);
	}
	#endif
	CreateTemp(2);
	
	ASSERT(this->full_path.GetCount());
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
		VisitFromJsonFile(*this, full_path);
	}
	
	lock.Leave();
	return !saved_hash.IsEmpty();
}

void MetaSrcFile::UpdateLoading() {
	ASSERT(id >= 0 && pkg->id >= 0);
	temp->SetPkgFileDeep(pkg->id, id);

	OnSeenTypes();
	OnSerialCounter();
	temp->RealizeSerial();
	lock.Leave();
	
// see SRC_TXT_HEADER_ENABLE
	#ifdef flagAI
	if (GetFileExt(full_path) == ".db-src") {
		SrcTxtHeader* src = dynamic_cast<SrcTxtHeader*>(&*temp->ext);
		ASSERT(src);
		DatasetIndex().GetAdd(full_path) = src;
	}
	#endif
}

bool MetaSrcFile::LoadJson(String json) {
	lock.Enter();
	saved_hash.Clear();
	
	ASSERT_(!temp, "Temporary MetaNode was not cleared previously");
	CreateTemp(3);
	VisitFromJson(*this, json);
	
	lock.Leave();
	return !saved_hash.IsEmpty();
}

void MetaSrcFile::OnSerialCounter()
{
	MetaEnvironment& env = MetaEnv();
	env.serial_counter =
		max(env.serial_counter,
		this->highest_seen_serial);
}

void MetaSrcFile::OnSeenTypes()
{
	MetaEnvironment& env = MetaEnv();
	for(auto it : ~seen_types)
		env.types.GetAdd(it.key).seen_type = it.value;
}

void MetaSrcFile::RefreshSeenTypes()
{
	ASSERT(temp);
	MetaNode& file_nodes = *temp;
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

void MetaSrcFile::MakeTempFromEnv(bool all_files) {
	IdeMetaEnvironment& ienv = IdeMetaEnv();
	MetaEnvironment& env = ienv.env;
	CreateTemp(4);
	#ifdef flagDEBUG
	//LOG(env.root.GetTreeString());
	env.root.DeepChk();
	#endif
	if (all_files)
		ienv.SplitNode(env.root, *temp, pkg->id);
	else
		ienv.SplitNode(env.root, *temp, pkg->id, id);
}















void MetaSrcPkg::Init()
{
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	MetaSrcFile& file = GetAddFile(path);
	file.KeepFileIndices();
	file.ManageFile();
	ASSERT(file.id == 0 && file.rel_path.GetCount());
}

bool MetaSrcPkg::Store(bool forced)
{
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	IdeMetaEnvironment& env = IdeMetaEnv();
	MetaSrcFile& file = GetAddFile(path);
	MetaSrcPkg& pkg = *file.pkg;
	MetaNode& file_nodes = file.CreateTemp(5);
	ASSERT(file.id >= 0 && pkg.id >= 0);
	env.SplitNode(env.env.root, file_nodes, pkg.id);
	return file.Store(forced);
}


#if 0

void MetaSrcPkg::PostSave()
{
	if(!post_saving) {
		post_saving = true;
		PostCallback(THISBACK1(Save, false));
	}
}

void MetaSrcPkg::Clear() { files.Clear(); }

void MetaSrcPkg::SetPath(String bin_path, String upp_dir)
{
	this->bin_path = bin_path;
	// dir = GetFileDirectory(path);
	this->upp_dir = upp_dir;
}

void MetaSrcPkg::Load()
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

void MetaSrcPkg::Save(bool forced)
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

String MetaSrcPkg::GetHashSha1() {
	Sha1Stream s;
	aionfile_getting_sha1 = true;
	s % *this;
	aionfile_getting_sha1 = false;
	return s.FinishString();
}

void MetaSrcPkg::Serialize(Stream& s)
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

void MetaSrcPkg::Jsonize(JsonIO& json)
{
	Mutex::Lock ml(lock);
	json
		("saved_hash", saved_hash) // it's fine here
		("files", files)
		;
}

void MetaSrcPkg::operator=(const MetaSrcPkg& f) {
	Mutex::Lock ml(lock);
	files <<= f.files;
	saved_hash = f.saved_hash;
	path = f.path;
	dir = f.dir;
}
#endif

String MetaSrcPkg::GetTitle() const {
	if (title.IsEmpty()) {
		if (dir.Right(1) == DIR_SEPS)
			title = GetFileTitle(dir.Left(dir.GetCount()-1));
		else
			title = GetFileTitle(dir);
	}
	return title;
}

String MetaSrcPkg::GetRelativePath(const String& path) const
{
	String dir = GetDirectory();
	int i = path.Find(dir);
	if(i >= 0) {
		String s = path.Mid(dir.GetCount());
		if(s.GetCount() && s[0] == DIR_SEP)
			s = s.Mid(1);
		return s;
	}
	else
		TODO; // return NormalizePath(path, dir);
	return String();
}

String MetaSrcPkg::GetFullPath(const String& rel_path) const
{
	String dir = GetDirectory();
	return AppendFileName(dir, rel_path);
}

String MetaSrcPkg::GetFullPath(int file_i) const
{
	return files[file_i].full_path;
}

void MetaSrcPkg::Visit(Vis& v) {
	v.Ver(1)
	(1)	("files", files, VISIT_VECTOR);
}

#if 0
MetaSrcFile& MetaSrcPkg::RealizePath(const String& includes, const String& path)
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
		MetaSrcFile& o = files[i];
		return o;
	}
	MetaSrcFile& o = files.Add(rel_file);
	return o;
}
#endif

MetaSrcFile& MetaSrcPkg::GetMetaFile() {
	ASSERT(dir.GetCount());
	String path = AppendFileName(dir, META_FILENAME);
	return GetAddFile(path);
}

MetaSrcFile& MetaSrcPkg::GetAddFile(const String& full_path)
{
	String dir = GetDirectory();
	if (full_path.Find(dir) == 0) {
		String rel_path = GetRelativePath(full_path);
		for (MetaSrcFile& f : files) {
			if (f.rel_path == rel_path) {
				ASSERT(f.id >= 0);
				return f;
			}
		}
		int id = files.GetCount();
		MetaSrcFile& f = files.Add();
		f.id = id;
		f.pkg = this;
		f.rel_path = rel_path;
		f.full_path = full_path;
		f.env_file = GetFileExt(full_path) == ".env";
		ASSERT(f.full_path.Right(2) != ".d");
		return f;
	}
	else {
		for (MetaSrcFile& f : files) {
			if (f.full_path == full_path) {
				ASSERT(f.id >= 0);
				return f;
			}
		}
		int id = files.GetCount();
		MetaSrcFile& f = files.Add();
		f.id = id;
		f.pkg = this;
		f.rel_path = "";
		f.full_path = full_path;
		f.env_file = GetFileExt(full_path) == ".env";
		ASSERT(f.full_path.Right(2) != ".d");
		return f;
	}
}

int MetaSrcPkg::FindFile(String path) const {
	int i = 0;
	for (const auto& file : files) {
		if (file.full_path == path)
			return i;
		i++;
	}
	if (i < 0) {
		path = GetRelativePath(path);
		i = 0;
		for (const auto& file : files) {
			if (file.rel_path == path)
				return i;
			i++;
		}
	}
	return i;
}

String IdeMetaEnvironment::ResolveMetaSrcPkgPath(const String& includes, String path,
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

MetaSrcFile& IdeMetaEnvironment::ResolveFile(const String& includes, const String& path)
{
	String upp_dir;
	String pkg_path = ResolveMetaSrcPkgPath(includes, path, upp_dir);
	env.lock.EnterWrite();
	MetaSrcPkg& pkg = GetAddPkg(upp_dir);
	ASSERT(pkg.id >= 0);
	//String rel_path = pkg.GetRelativePath(path);
	MetaSrcFile& file = pkg.GetAddFile(path);
	ASSERT(file.id >= 0);
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

MetaSrcPkg& IdeMetaEnvironment::GetAddPkg(String dir) {
	ASSERT(dir.Find(".bin") < 0); // NOT HERE
	if (!DirectoryExists(dir)) {
		RealizeDirectory(dir);
	}
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

Vector<MetaNode*> IdeMetaEnvironment::FindAllEnvs() {
	Vector<MetaNode*> v;
	for (const MetaSrcPkg& pkg : pkgs) {
		for (const MetaSrcFile& file : pkg.files) {
			if (file.env_file) {
				MetaNode& n = RealizeFileNode(pkg.id, file.id, METAKIND_PKG_ENV);
				v << &n;
			}
		}
	}
	return v;
}

MetaNode* IdeMetaEnvironment::FindNodeEnv(Entity& n)
{
	String ctx = n.Data("ctx");
	if (ctx.IsEmpty())
		return 0;
	for (const MetaSrcPkg& pkg : pkgs) {
		for (const MetaSrcFile& file : pkg.files) {
			if (file.env_file) {
				MetaNode& fn = RealizeFileNode(pkg.id, file.id, METAKIND_PKG_ENV);
				for (MetaNode& s : fn.sub)
					if (s.kind == METAKIND_CONTEXT && s.id == ctx)
						return &s;
			}
		}
	}
	return 0;
}

// see SRC_TXT_HEADER_ENABLE
MetaNode* IdeMetaEnvironment::LoadDatabaseSourceVisit(MetaSrcFile& file, String path, Vis& v) {
	if (!file.pkg)
		return 0;
	MetaNode& filenode = RealizeFileNode(file.pkg->id, file.id, METAKIND_DATABASE_SOURCE);
	if (!filenode.ext) {
		One<SrcTxtHeader> ext;
		ext.Create(filenode);
		ext->filepath = path;
		ext->Visit(v);
		DatasetIndex().GetAdd(path) = &*ext;
		filenode.serial = env.NewSerial();
		filenode.ext = ext.Detach();
		ASSERT(&filenode.ext->node == &filenode);
	}
	if (filenode.id.IsEmpty())
		filenode.id = GetFileTitle(path);
	return &filenode;
}

bool IdeMetaEnvironment::LoadFileRoot(const String& includes, const String& path, bool manage_file)
{
	MetaSrcFile& file = ResolveFile(includes, path);
	file.ManageFile(manage_file);
	
// see SRC_TXT_HEADER_ENABLE
	// Hotfix for old .db-src file
	if (GetFileExt(path) == ".db-src") {
		String json = LoadFile(path);
		Value jv = ParseJSON(json);
		JsonIO j(jv);
		Vis vis(j);
		if (LoadDatabaseSourceVisit(file, path, vis)) {
			file.ClearTemp();
			return true;
		}
	}
	
	file.lock.Enter();
	if(file.Load()) {
		OnLoadFile(file);
		file.ClearTemp();
		file.lock.Leave();
		return true;
	}
	file.ClearTemp();
	file.lock.Leave();
	return false;
}

bool IdeMetaEnvironment::LoadFileRootVisit(const String& includes, const String& path, Vis& v, bool manage_file, MetaNode*& file_node) {
	file_node = 0;
	
	MetaSrcFile& file = ResolveFile(includes, path);
	file.ManageFile(manage_file);
	
	// Hotfix for old .db-src file
	String ext = GetFileExt(path);
	if (ext == ".db-src") {
		MetaNode* fn = LoadDatabaseSourceVisit(file, path, v);
		if (fn) {
			file_node = fn;
			return true;
		}
	}
	else if (ext == ".ecs") {
		MetaNode& fn = RealizeFileNode(file.pkg->id, file.id, METAKIND_ECS_SPACE);
		if (fn.id.IsEmpty())
			fn.id = GetFileTitle(path);
		file_node = &fn;
	}
	else if (ext == ".env") {
		MetaNode& fn = RealizeFileNode(file.pkg->id, file.id, METAKIND_PKG_ENV);
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
		file.ClearTemp();
		return true;
	}
	file.ClearTemp();
	return false;
}

MetaSrcFile& IdeMetaEnvironment::Load(const String& includes, const String& path)
{
	MetaSrcFile& file = ResolveFile(includes, path);
	if(file.Load()) {
		OnLoadFile(file);
	}
	file.ClearTemp();
	return file;
}

void IdeMetaEnvironment::OnLoadFile(MetaSrcFile& file)
{
	MetaNode& file_nodes = file.GetTemp();
	file_nodes.SetTempDeep();
	ASSERT(file.saved_hash == IntStr64(file_nodes.GetSourceHash()));
	//file_nodes.SetPkgDeep(pkg.id);
	//LOG(file_nodes.GetTreeString());
	env.MergeNode(env.root, file_nodes, MERGEMODE_OVERWRITE_OLD);
	// LOG(root.GetTreeString());

#if DEBUG_METANODE_DTOR
	Vector<MetaNode*> comments;
	root.FindAllDeep(METAKIND_COMMENT, comments);
	for(auto* c : comments)
		c->trace_kill = true;
#endif
}

MetaNode& IdeMetaEnvironment::RealizeFileNode(int pkg, int file, int kind) {
	for (auto& s : env.root.sub) {
		if (s.pkg == pkg && s.file == file && s.kind == kind) {
			if (!s.serial)
				s.serial = env.NewSerial();
			return s;
		}
	}
	MetaNode& n = env.root.sub.Add();
	n.pkg = pkg;
	n.file = file;
	n.kind = kind;
	n.serial = env.NewSerial();
	n.id = pkgs[pkg].files[file].GetTitle();
	return n;
}

void IdeMetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id)
{
	root.PointPkgTo(other, pkg_id);
}

void IdeMetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id)
{
	root.PointPkgTo(other, pkg_id, file_id);
}

void IdeMetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id)
{
	root.CopyPkgTo(other, pkg_id);
}

void IdeMetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id)
{
	root.CopyPkgTo(other, pkg_id, file_id);
}

String IdeMetaEnvironment::GetFilepath(int pkg_id, int file_id) const
{
	const auto& pkg = this->pkgs[pkg_id];
	String path = pkg.GetFullPath(file_id);
	return path;
}

IdeMetaEnvironment& IdeMetaEnv()
{
	static IdeMetaEnvironment env;
	return env;
}

END_UPP_NAMESPACE

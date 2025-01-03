#include <AI/AI.h>
#include <ide/ide.h>
#include <AI/Ctrl/Ctrl.h>

NAMESPACE_UPP

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

String GetAiPathCandidate(const String& includes_, String dir)
{
	Vector<String> ai_dirs = GetAiDirsRaw();
	Vector<String> upp_dirs = GetUppDirs();
	String dummy_cand, def_cand, any_ai_cand, preferred_ai_cand;
	int def_cand_parts = INT_MAX;
	String filename = META_FILENAME;
	dummy_cand = dir + DIR_SEPS + filename;
	
	if(!ai_dirs.IsEmpty()) {
		for(const String& upp_dir : upp_dirs) {
			if(dir.Find(upp_dir) != 0)
				continue;
			String rel_path = dir.Mid(upp_dir.GetCount());
			for(const String& ai_dir : ai_dirs) {
				String ai_dir_cand = AppendFileName(ai_dir, rel_path);
				String path = AppendFileName(ai_dir_cand, filename);
				if(any_ai_cand.IsEmpty())
					any_ai_cand = path;
				if(preferred_ai_cand.IsEmpty()) {
					if (META_EXISTS_FN(path))
						preferred_ai_cand = path;
				}
			}
		}
	}
	if(!preferred_ai_cand.IsEmpty())
		return preferred_ai_cand;
	else if(!any_ai_cand.IsEmpty())
		return any_ai_cand;

	if(!ai_dirs.IsEmpty()) {
		String ai_dir, rel_dir;
		if(MakeRelativePath(includes_, dir, ai_dir, rel_dir)) {
			String abs_dir = AppendFileName(ai_dir, rel_dir);
			def_cand = AppendFileName(abs_dir, filename);
		}
	}

	if(!def_cand.IsEmpty())
		return def_cand;
	else
		return dummy_cand;
}

Vector<String> FindParentUppDirectories(const String& sub_dir)
{
	Vector<String> results;
	Vector<String> parts = Split(sub_dir, DIR_SEPS);
	for(int i = 0; i < parts.GetCount(); i++) {
		int c = parts.GetCount() - i;
		if(!c)
			continue;
		String parent_dir;
		for(int j = 0; j < c; j++) {
// a posix path always begins with the root /
#ifndef flagPOSIX
			if(!parent_dir.IsEmpty())
#endif
				parent_dir << DIR_SEPS;
			parent_dir << parts[j];
		}
		String topname = parts[c - 1];
		String upp_path = parent_dir + DIR_SEPS + topname + ".upp";
		if(!FileExists(upp_path))
			continue;
		results << parent_dir;
	}
	return results;
}

MetaNode& MetaSrcFile::GetTemp() {
	return *temp;
}

MetaNode& MetaSrcFile::CreateTemp() {
	return temp.Create();
}

void MetaSrcFile::ClearTemp() {
	temp.Clear();
}

void MetaSrcFile::Visit(NodeVisitor& vis)
{
	if (vis.IsLoading() || temp.IsEmpty())
		temp.Create();
	else if (vis.IsStoring())
		UpdateStoring();
	vis.Ver(1)
	(1)	("id", id)
		("hash", saved_hash)
		("highest_seen_serial", (int64&)highest_seen_serial)
	    ("seen_types", (VectorMap<int64,String>&)seen_types)
	    ("root", *temp, VISIT_NODE)
	    ;
	if (vis.IsLoading())
		UpdateLoading();
}

String MetaSrcFile::UpdateStoring()
{
	ASSERT_(managed_file, "Trying to jsonize non-managed file");
	ASSERT(temp);
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
	
	temp.Clear();
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
			Panic("TODO");
		}
		else if(total_hash_diffs) {
			Panic("TODO");
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
		NodeVisitor vis(vcs);
		this->Visit(vis);
		vcs.Close();
	}
	else if (IsBinary()) {
		FileOut s(full_path);
		NodeVisitor vis(s);
		this->Visit(vis);
		s.Close();
	}
	else {
		succ = VisitToJsonFile(*this, full_path);
	}
	
	lock.Leave();
	
	temp.Clear();
	return succ;
}

bool MetaSrcFile::Load()
{
	lock.Enter();
	saved_hash.Clear();
	
	ASSERT_(!temp, "Temporary MetaNode was not cleared previously");
	temp.Create();
	ASSERT(this->full_path.GetCount());
	if (IsDirTree()) {
		VersionControlSystem vcs;
		vcs.Initialize(full_path, false);
		NodeVisitor vis(vcs);
		this->Visit(vis);
		vcs.Close();
	}
	else if (IsBinary()) {
		FileIn s(full_path);
		NodeVisitor vis(s);
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
	
	if (GetFileExt(full_path) == ".db-src") {
		SrcTxtHeader* src = dynamic_cast<SrcTxtHeader*>(&*temp->ext);
		ASSERT(src);
		DatasetIndex().GetAdd(full_path) = src;
	}
	
}

bool MetaSrcFile::LoadJson(String json) {
	lock.Enter();
	saved_hash.Clear();
	
	ASSERT_(!temp, "Temporary MetaNode was not cleared previously");
	temp.Create();
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
	MetaEnvironment& env = MetaEnv();
	temp.Create();
	#ifdef flagDEBUG
	//LOG(env.root.GetTreeString());
	env.root.DeepChk();
	#endif
	if (all_files)
		env.SplitNode(env.root, *temp, pkg->id);
	else
		env.SplitNode(env.root, *temp, pkg->id, id);
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
	MetaEnvironment& env = MetaEnv();
	MetaSrcFile& file = GetAddFile(path);
	MetaSrcPkg& pkg = *file.pkg;
	MetaNode& file_nodes = file.CreateTemp();
	ASSERT(file.id >= 0 && pkg.id >= 0);
	env.SplitNode(env.root, file_nodes, pkg.id);
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
		Panic("TODO"); // return NormalizePath(path, dir);
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

void MetaSrcPkg::Visit(NodeVisitor& vis) {
	vis.Ver(1)
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



















String MetaEnvironment::ResolveMetaSrcPkgPath(const String& includes, String path,
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

MetaSrcFile& MetaEnvironment::ResolveFile(const String& includes, const String& path)
{
	String upp_dir;
	String pkg_path = ResolveMetaSrcPkgPath(includes, path, upp_dir);
	lock.EnterWrite();
	MetaSrcPkg& pkg = GetAddPkg(upp_dir);
	ASSERT(pkg.id >= 0);
	//String rel_path = pkg.GetRelativePath(path);
	MetaSrcFile& file = pkg.GetAddFile(path);
	ASSERT(file.id >= 0);
	lock.LeaveWrite();
	return file;
}

int MetaEnvironment::FindPkg(String dir) const {
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

MetaSrcPkg& MetaEnvironment::GetAddPkg(String dir) {
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

#if 0
MetaSrcFile& MetaEnvironment::ResolveFileInfo(const String& includes, String path)
{
	return ResolveFile(includes, path).RealizePath(includes, path);
}
#endif






MetaEnvironment& MetaEnv() { return Single<MetaEnvironment>(); }

MetaEnvironment::MetaEnvironment() {
	root.kind = CXCursor_Namespace;
	root.serial = NewSerial();
}

hash_t MetaEnvironment::NewSerial() {
	serial_lock.Enter();
	hash_t new_hash = ++serial_counter;
	serial_lock.Leave();
	return new_hash;
}

void MetaEnvironment::UpdateWorkspace(Workspace& wspc) {
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		String pkg_name = wspc.package.GetKey(i);
		auto& pkg = wspc.package[i];
		String dir = GetFileDirectory(pkg.path);
		MetaSrcPkg& mpkg = GetAddPkg(dir);
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

Vector<MetaNode*> MetaEnvironment::FindAllEnvs() {
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

MetaNode* MetaEnvironment::FindNodeEnv(Entity& n)
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

MetaNode* MetaEnvironment::LoadDatabaseSourceVisit(MetaSrcFile& file, String path, NodeVisitor& vis) {
	//data.Replace("\r","");
	//if (data.Find("{\n\t\"written\":") >= 0) {
	Panic("TODO");
	
	MetaNode& filenode = RealizeFileNode(file.pkg->id, file.id, METAKIND_DATABASE_SOURCE);
	One<SrcTxtHeader> ext;
	ext.Create(filenode);
	ext->filepath = path;
	ext->Visit(vis);
	DatasetIndex().GetAdd(path) = &*ext;
	filenode.serial = NewSerial();
	filenode.ext = ext.Detach();
	ASSERT(&filenode.ext->node == &filenode);
	filenode.id = GetFileTitle(path);
	return &filenode;
}

bool MetaEnvironment::LoadFileRoot(const String& includes, const String& path, bool manage_file)
{
	MetaSrcFile& file = ResolveFile(includes, path);
	file.ManageFile(manage_file);
	
	// Hotfix for old .db-src file
	if (GetFileExt(path) == ".db-src") {
		String json = LoadFile(path);
		Value jv = ParseJSON(json);
		JsonIO j(jv);
		NodeVisitor vis(j);
		if (LoadDatabaseSourceVisit(file, path, vis))
			return true;
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

bool MetaEnvironment::LoadFileRootVisit(const String& includes, const String& path, NodeVisitor& vis, bool manage_file, MetaNode** file_node) {
	if (file_node) *file_node = 0;
	
	MetaSrcFile& file = ResolveFile(includes, path);
	file.ManageFile(manage_file);
	
	// Hotfix for old .db-src file
	if (GetFileExt(path) == ".db-src") {
		MetaNode* fn = LoadDatabaseSourceVisit(file, path, vis);
		if (fn) {
			if (file_node)
				*file_node = fn;
			return true;
		}
	}
	
	ASSERT(vis.IsLoading());
	file.Visit(vis);
	if (!vis.IsError()) {
		OnLoadFile(file);
		file.ClearTemp();
		return true;
	}
	file.ClearTemp();
	return false;
}

MetaSrcFile& MetaEnvironment::Load(const String& includes, const String& path)
{
	MetaSrcFile& file = ResolveFile(includes, path);
	if(file.Load()) {
		OnLoadFile(file);
	}
	file.ClearTemp();
	return file;
}

void MetaEnvironment::OnLoadFile(MetaSrcFile& file)
{
	MetaNode& file_nodes = file.GetTemp();
	file_nodes.SetTempDeep();
	ASSERT(file.saved_hash == IntStr64(file_nodes.GetSourceHash()));
	//file_nodes.SetPkgDeep(pkg.id);
	//LOG(file_nodes.GetTreeString());
	MergeNode(root, file_nodes, MERGEMODE_OVERWRITE_OLD);
	// LOG(root.GetTreeString());

#if DEBUG_METANODE_DTOR
	Vector<MetaNode*> comments;
	root.FindAllDeep(METAKIND_COMMENT, comments);
	for(auto* c : comments)
		c->trace_kill = true;
#endif
}

void MetaEnvironment::Store(String& includes, const String& path, ClangNode& cn)
{
	ClangTypeResolver ctr;
	if(!ctr.Process(cn)) {
		LOG("MetaEnvironment::Store: error: clang type resolving failed: " + ctr.GetError());
		return;
	}
	if(!MergeResolver(ctr)) {
		LOG("MetaEnvironment::Store: error: merging resolver failed");
		return;
	}

	cn.TranslateTypeHash(ctr.GetTypeTranslation());

	// LOG(n.GetTreeString());
	MetaSrcFile& file = ResolveFile(includes, path);
	MetaSrcPkg& pkg = *file.pkg;
	MetaNode n;
	n.Assign(0, cn);
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file.id);
	n.RealizeSerial();
	if(!MergeNode(root, n, MERGEMODE_OVERWRITE_OLD))
		return;

	pkg.Store(false);
}

bool MetaEnvironment::MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode)
{
	MetaNode& n0 = *scope.Top();
	ASSERT(n0.kind == n1.kind && n0.id == n1.id);
	if(IsMergeable((CXCursorKind)n0.kind)) {
		for(const MetaNode& sub1 : n1.sub) {
			int i = n0.Find(sub1.kind, sub1.id);
			if(i < 0) {
				auto& n = n0.Add(sub1);
				MergeVisitPost(n);
			}
			else {
				MetaNode& sub0 = n0.sub[i];
				scope << &sub0;
				bool succ = MergeVisit(scope, sub1, mode);
				scope.Pop();
				if(!succ)
					return false;
			}
		}
		n0.Chk();
	}
	else {
		return MergeVisitPartMatching(scope, n1, mode);
	}
	return true;
}

bool MetaEnvironment::MergeVisitPartMatching(Vector<MetaNode*>& scope, const MetaNode& n1,
                                             MergeMode mode)
{
	MetaNode& n0 = *scope.Top();
	struct Hashes {
		const MetaNode* n = 0;
		const MetaNode* match = 0;
		bool source_kind;
		hash_t hash;
		bool ready = false;
		dword serial;
		void Set(const MetaNode* mn, bool is_source_kind, hash_t c)
		{
			n = mn;
			hash = c;
			source_kind = is_source_kind;
		}
	};

	if(n0.serial == n1.serial) {
		n0.Chk();
		return true;
	}
	ASSERT(n0.serial && n1.serial);
	const MetaNode& pri = mode == MERGEMODE_OVERWRITE_OLD ? (n0.serial > n1.serial ? n0 : n1)
	                                                      : (n0.serial < n1.serial ? n0 : n1);
	const MetaNode& sec = &pri == &n0 ? n1 : n0;
	
	if (pri.sub.IsEmpty() && sec.sub.IsEmpty()) {
		if (mode == MERGEMODE_OVERWRITE_OLD || mode == MERGEMODE_UPDATE_SUBSET) {
			if (n0.serial < n1.serial)
				n0.CopyFieldsFrom(n1);
			return true;
		}
		else {
			if (n0.serial > n1.serial)
				n0.CopyFieldsFrom(n1, true);
			return true;
		}
	}
	
	Array<Hashes> pri_subs;
	Array<Hashes> sec_subs;
	pri_subs.Reserve(pri.sub.GetCount());
	sec_subs.Reserve(sec.sub.GetCount());
	for(const MetaNode& pri : pri.sub) {
		if(pri.IsSourceKind())
			pri_subs.Add().Set(&pri, true, pri.GetSourceHash());
		else
			pri_subs.Add().Set(&pri, false, pri.GetTotalHash());
	}
	for(const MetaNode& sec : sec.sub) {
		if(sec.IsSourceKind())
			sec_subs.Add().Set(&sec, true, sec.GetSourceHash());
		else
			sec_subs.Add().Set(&sec, false, sec.GetTotalHash());
	}

	// Match hashes starting from the beginning
	int pri_c = pri_subs.GetCount();
	int sec_c = sec_subs.GetCount();
	for(int node_kind_mode = 0; node_kind_mode < 2; node_kind_mode++) {
		bool filter_source_kind = !node_kind_mode;

		while(1) {
			bool found_matches = false;
			for(int i = 0, j = 0; i < pri_c && j < sec_c; i++, j++) {
				auto& pri = pri_subs[i];
				auto& sec = sec_subs[j];
				if(pri.source_kind != filter_source_kind || pri.ready) {
					j--;
					continue;
				}
				else if(sec.source_kind != filter_source_kind || sec.ready) {
					i--;
					continue;
				}
				if(pri.source_kind != filter_source_kind &&
				   sec.source_kind != filter_source_kind)
					continue;
				if(pri.ready && sec.ready)
					continue;

				if(pri.hash == sec.hash) {
					pri.match = sec.n;
					sec.match = pri.n;
					pri.ready = true;
					sec.ready = true;
					found_matches = true;
				}
				else
					break;
			}
			// Match common hashes starting from the end
			for(int i = 0, j = 0; i < pri_c && j < sec_c; i++, j++) {
				int i0 = pri_c - 1 - i;
				int j0 = sec_c - 1 - j;
				auto& pri = pri_subs[i0];
				auto& sec = sec_subs[j0];
				if(pri.source_kind != filter_source_kind || pri.ready) {
					j--;
					continue;
				}
				else if(sec.source_kind != filter_source_kind || sec.ready) {
					i--;
					continue;
				}
				if(pri.source_kind != filter_source_kind &&
				   sec.source_kind != filter_source_kind)
					continue;
				if(pri.ready && sec.ready)
					continue;

				if(pri.hash == sec.hash) {
					pri.match = sec.n;
					sec.match = pri.n;
					pri.ready = true;
					sec.ready = true;
					found_matches = true;
				}
				else
					break;
			}
			if(!found_matches)
				break;
		}
	}

	bool all_pri_match = true;
	for(auto& pri : pri_subs) if(!pri.ready) {
		all_pri_match = false;
		break;
	}
	bool all_sec_match = true;
	bool all_sec_source_match = true;
	for(auto& sec : sec_subs) if(!sec.ready) {
		all_sec_match = false;
		if (sec.source_kind) {
			all_sec_source_match = false;
			break;
		}
	}

	// Copy secondary extras, if all primary matches and all secondary sources matches
	if(all_pri_match && all_sec_source_match && !all_sec_match) {
		while(true) {
			int added_count = 0;
			for(int i = 1; i < sec_subs.GetCount(); i++) {
				auto& sec = sec_subs[i];
				if(sec.ready)
					continue;
				ASSERT(!sec.source_kind);
				auto& seca = sec_subs[i - 1];
				if(!seca.ready)
					continue;
				bool added = false;
				for(int j = 0; j < pri_subs.GetCount(); j++) {
					if(pri_subs[j].match == seca.n) {
						auto& pri = pri_subs.Insert(j + 1);
						pri.match = sec.n;
						pri.ready = true;
						sec.ready = true;
						added = true;
						break;
					}
				}
				if(added)
					added_count++;
			}
			for(int i = sec_subs.GetCount() - 2; i >= 0; i--) {
				auto& sec = sec_subs[i];
				if(sec.ready)
					continue;
				ASSERT(!sec.source_kind);
				auto& seca = sec_subs[i + 1];
				if(!seca.ready)
					continue;
				bool added = false;
				for(int j = 0; j < pri_subs.GetCount(); j++) {
					if(pri_subs[j].match == seca.n) {
						auto& pri = pri_subs.Insert(j);
						pri.match = sec.n;
						pri.ready = true;
						sec.ready = true;
						added = true;
						break;
					}
				}
				if(added)
					added_count++;
			}
			int unready_count = 0;
			for(int i = 0; i < sec_subs.GetCount(); i++)
				if(!sec_subs[i].ready)
					unready_count++;
			if(!unready_count)
				break;
			if(added_count)
				continue;
			for(int i = 0; i < sec_subs.GetCount(); i++) {
				auto& sec = sec_subs[i];
				if(sec.ready)
					continue;
				ASSERT(!sec.source_kind);
				bool added = false;
				if(i > 0) {
					auto& seca = sec_subs[i - 1];
					if(!seca.ready)
						continue;
					for(int j = 0; j < pri_subs.GetCount(); j++) {
						if(pri_subs[j].match == seca.n) {
							auto& pri = pri_subs.Insert(j + 1);
							pri.match = sec.n;
							pri.ready = true;
							sec.ready = true;
							added = true;
							break;
						}
					}
				}
				if(!added && i + 1 < sec_subs.GetCount()) {
					auto& seca = sec_subs[i + 1];
					if(!seca.ready)
						continue;
					for(int j = 0; j < pri_subs.GetCount(); j++) {
						if(pri_subs[j].match == seca.n) {
							auto& pri = pri_subs.Insert(j);
							pri.match = sec.n;
							pri.ready = true;
							sec.ready = true;
							added = true;
							break;
						}
					}
				}
				if(!added && i == 0) {
					auto& pri = pri_subs.Insert(0);
					pri.match = sec.n;
					pri.ready = true;
					sec.ready = true;
					added = true;
				}
				if(!added && i == sec_subs.GetCount() - 1) {
					auto& pri = pri_subs.Add();
					pri.match = sec.n;
					pri.ready = true;
					sec.ready = true;
					added = true;
				}
				if(added)
					added_count++;
			}
			if(!added_count) {
				ASSERT_(0, "unexpected internal error"); // this shouldn't ever happen
				return false;
			}
		}
	}

	hash_t old_serial = n0.serial;
	if(mode == MERGEMODE_KEEP_OLD) {
		if(n0.serial > n1.serial && !n0.IsFieldsSame(n1)) {
			n0.CopyFieldsFrom(n1);
			n0.serial = NewSerial(); // TODO this might be too early to create new serial, if all data is same
		}
	}
	else {
		if(n0.serial < n1.serial && !n0.IsFieldsSame(n1)) {
			n0.CopyFieldsFrom(n1);
			n0.serial = NewSerial(); // TODO this might be too early to create new serial, if all data is same
		}
	}

	if(&pri == &n0) {
		int pos = 0;
		for(int i = 0; i < pri_subs.GetCount(); i++) {
			auto& pri1 = pri_subs[i];
			if(!pri1.n) {
				pri1.n = &n0.sub.Insert(pos);
				if(n0.serial == old_serial)
					n0.serial = NewSerial();
			}
			else {
				ASSERT(pri1.n == &n0.sub[pos]);
			}
			pos++;
		}

		for(auto& pri : pri_subs) {
			if(pri.ready && pri.match) {
				MetaNode& s0 = const_cast<MetaNode&>(*pri.n);
				hash_t old_sub_serial = s0.serial;
				scope.Add(&s0);
				bool succ = MergeVisitPartMatching(scope, *pri.match, mode);
				scope.Remove(scope.GetCount() - 1);
				if(old_sub_serial != s0.serial && n0.serial == old_serial)
					n0.serial = NewSerial();
				if(!succ)
					return false;
			}
		}
	}
	else {
		for(auto& sec : sec_subs) {
			if(sec.ready && sec.match) {
				MetaNode& s0 = const_cast<MetaNode&>(*sec.n);
				hash_t old_sub_serial = s0.serial;
				scope.Add(&s0);
				bool succ = MergeVisitPartMatching(scope, *sec.match, mode);
				scope.Remove(scope.GetCount() - 1);
				if(old_sub_serial != s0.serial && n0.serial == old_serial)
					n0.serial = NewSerial();
				if(!succ)
					return false;
			}
		}
	}
	
	n0.Chk();
	return true;
}

void MetaEnvironment::MergeVisitPost(MetaNode& n)
{
	RefreshNodePtrs(n);
	for(auto& s : n.sub)
		MergeVisitPost(s);
}

bool MetaEnvironment::MergeNode(MetaNode& root, const MetaNode& other, MergeMode mode)
{
	Vector<MetaNode*> scope;
	scope << &root;
	return MergeVisit(scope, other, mode);
}

void MetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id)
{
	root.PointPkgTo(other, pkg_id);
}

void MetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id)
{
	root.PointPkgTo(other, pkg_id, file_id);
}

void MetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id)
{
	root.CopyPkgTo(other, pkg_id);
}

void MetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id)
{
	root.CopyPkgTo(other, pkg_id, file_id);
}

String MetaEnvironment::GetFilepath(int pkg_id, int file_id) const
{
	const auto& pkg = this->pkgs[pkg_id];
	String path = pkg.GetFullPath(file_id);
	return path;
}

MetaNode::~MetaNode()
{
#if DEBUG_METANODE_DTOR
	if(trace_kill)
		Panic("trace-kill");
#endif
}

void MetaNode::PointPkgTo(MetaNodeSubset& other, int pkg_id)
{
	other.n = this;
	for(auto& n0 : sub) {
		if(n0.HasPkgDeep(pkg_id)) {
			MetaNodeSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id);
		}
	}
}

void MetaNode::PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id)
{
	other.n = this;
	for(auto& n0 : sub) {
		if(n0.HasPkgFileDeep(pkg_id, file_id)) {
			MetaNodeSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id, file_id);
		}
	}
}

void MetaNode::CopyPkgTo(MetaNode& other, int pkg_id) const
{
	other.CopyFieldsFrom(*this, true);
	for(const auto& n0 : sub) {
		if(n0.HasPkgDeep(pkg_id)) {
			MetaNode& n1 = other.Add();
			n0.CopyPkgTo(n1, pkg_id);
		}
	}
}

void MetaNode::CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const
{
	other.CopyFieldsFrom(*this, true);
	for(const auto& n0 : sub) {
		if(n0.HasPkgFileDeep(pkg_id, file_id)) {
			MetaNode& n1 = other.Add();
			n0.CopyPkgTo(n1, pkg_id, file_id);
		}
	}
}

bool MetaNode::HasPkgDeep(int pkg_id) const
{
	if(this->pkg == pkg_id)
		return true;
	for(const auto& n : sub)
		if(n.HasPkgDeep(pkg_id))
			return true;
	return false;
}

bool MetaNode::HasPkgFileDeep(int pkg_id, int file_id) const
{
	if(this->pkg == pkg_id && this->file == file_id)
		return true;
	for(const auto& n : sub)
		if(n.HasPkgFileDeep(pkg_id, file_id))
			return true;
	return false;
}

void MetaNode::SetPkgDeep(int pkg_id)
{
	this->pkg = pkg_id;
	for(auto& n : sub)
		n.SetPkgDeep(pkg_id);
}

void MetaNode::SetFileDeep(int file_id)
{
	this->file = file_id;
	for(auto& n : sub)
		n.SetFileDeep(file_id);
}

void MetaNode::SetPkgFileDeep(int pkg_id, int file_id)
{
	this->pkg = pkg_id;
	this->file = file_id;
	for(auto& n : sub)
		n.SetPkgFileDeep(pkg_id, file_id);
}

void MetaNode::SetTempDeep()
{
	only_temporary = true;
	for(auto& n : sub)
		n.SetTempDeep();
}

bool MetaEnvironment::IsMergeable(int kind) { return IsMergeable((CXCursorKind)kind); }

bool MetaEnvironment::IsMergeable(CXCursorKind kind)
{
	switch(kind) {
	// case CXCursor_StructDecl:
	// case CXCursor_ClassDecl:
	case CXCursor_Namespace:
	case CXCursor_LinkageSpec:
		return true;
	default:
		return false;
	}
}

void MetaEnvironment::RefreshNodePtrs(MetaNode& n)
{
	/*if (n.kind == CXCursor_ClassTemplate) {
	    //LOG(n.GetTreeString());
	}*/
	ASSERT(!n.only_temporary);
	if(n.filepos_hash != 0) {
		auto& vec = this->filepos.GetAdd(n.filepos_hash).hash_nodes;
		bool found = false;
		bool found_zero = false;
		for(auto& p : vec) {
			MetaNode* ptr = &*p;
			found_zero = ptr == 0 || found_zero;
			if(ptr == &n) {
				found = true;
				break;
			}
		}
		if(!found)
			vec.Add(&n);
		if(found_zero) {
			Vector<int> rmlist;
			for(int i = 0; i < vec.GetCount(); i++)
				if(&*vec[i] == 0)
					rmlist << i;
			vec.Remove(rmlist);
		}
	}
	if(n.type_hash) {
		auto& vec = this->filepos.GetAdd(n.filepos_hash).hash_nodes;
		bool found = false;
		bool found_zero = false;
		for(auto& p : vec) {
			MetaNode* ptr = &*p;
			found_zero = ptr == 0 || found_zero;
			if(ptr == &n) {
				found = true;
				break;
			}
		}
		if(!found)
			vec.Add(&n);
		if(found_zero) {
			Vector<int> rmlist;
			for(int i = 0; i < vec.GetCount(); i++)
				if(&*vec[i] == 0)
					rmlist << i;
			vec.Remove(rmlist);
		}
	}
	if (!n.serial)
		n.serial = NewSerial();
}

String MetaNode::GetTreeString(int depth) const
{
	String s;
	s.Cat('\t', depth);
	if(1)
		s << IntStr(pkg) << ":" << IntStr(file) << ": ";
	s << GetKindString();
	if(!id.IsEmpty())
		s << ": " << id;
	s << "\n";
	if (ext) {
		s.Cat('\t', depth+1);
		int fac_i = MetaExtFactory::FindKindFactory(kind);
		s << "EXT:";
		if (fac_i >= 0) {
			const auto& fac = MetaExtFactory::List()[fac_i];
			s << " " << fac.name;
			if (!fac.ctrl_name.IsEmpty())
				s << " (" << fac.ctrl_name << ")";
		}
		s << "\n";
	}
	for(auto& n : sub)
		s << n.GetTreeString(depth + 1);
	return s;
}

int MetaNode::Find(int kind, const String& id) const
{
	int i = 0;
	for(const MetaNode& n : sub) {
		if(n.kind == kind && n.id == id)
			return i;
		i++;
	}
	return -1;
}

void MetaNode::Destroy()
{
	if(!owner)
		return;
	int i = 0;
	for(MetaNode& n : owner->sub) {
		if(&n == this) {
			owner->sub.Remove(i);
			break;
		}
		i++;
	}
}

void MetaNode::Assign(MetaNode* owner, const ClangNode& n)
{
	this->owner = owner;
	int c = n.sub.GetCount();
	sub.SetCount(c);
	for(int i = 0; i < c; i++)
		sub[i].Assign(this, n.sub[i]);
	kind = n.kind;
	id = n.id;
	type = n.type;
	type_hash = n.type_hash;
	begin = n.begin;
	end = n.end;
	filepos_hash = n.filepos_hash;
	is_ref = n.is_ref;
	is_definition = n.is_definition;
}

MetaNode& MetaNode::GetAdd(String id, String type, int kind)
{
	for(MetaNode& s : sub)
		if(s.kind == kind && s.id == id && s.type == type)
			return s;
	MetaNode& s = sub.Add();
	s.owner = this;
	s.id = id;
	s.type = type;
	s.kind = kind;
	s.serial = MetaEnv().NewSerial();
	s.file = this->file;
	s.pkg = this->pkg;
	this->serial = MetaEnv().NewSerial();
	if (kind >= METAKIND_EXTENSION_BEGIN && kind <= METAKIND_EXTENSION_END) {
		int i = MetaExtFactory::FindKindFactory(kind);
		if (i >= 0) {
			s.ext = MetaExtFactory::List()[i].new_fn(s);
		}
	}
	return s;
}

MetaNode& MetaNode::Add(const MetaNode& n)
{
	MetaNode& s = sub.Add();
	s.owner = this;
	s.CopySubFrom(n);
	s.CopyFieldsFrom(n);
	s.serial = MetaEnv().NewSerial();
	if (n.pkg < 0 && n.file < 0) {
		s.file = this->file;
		s.pkg = this->pkg;
	}
	this->serial = MetaEnv().NewSerial();
	s.Chk();
	return s;
}

MetaNode& MetaNode::Add(MetaNode* n)
{
	MetaNode& s = sub.Add(n);
	s.owner = this;
	if (s.pkg < 0 && s.file < 0) {
		s.file = this->file;
		s.pkg = this->pkg;
	}
	s.serial = MetaEnv().NewSerial();
	this->serial = MetaEnv().NewSerial();
	s.Chk();
	return s;
}

MetaNode& MetaNode::Add()
{
	MetaNode& s = sub.Add();
	s.owner = this;
	s.file = this->file;
	s.pkg = this->pkg;
	s.serial = MetaEnv().NewSerial();
	this->serial = MetaEnv().NewSerial();
	s.Chk();
	return s;
}

MetaNode& MetaNode::Add(int kind, String id)
{
	MetaNode& s = Add();
	s.kind = kind;
	s.id = id;
	if (kind >= METAKIND_EXTENSION_BEGIN && kind <= METAKIND_EXTENSION_END) {
		int i = MetaExtFactory::FindKindFactory(kind);
		if (i >= 0) {
			s.ext = MetaExtFactory::List()[i].new_fn(s);
		}
	}
	s.Chk();
	return s;
}

void MetaNode::CopyFrom(const MetaNode& n)
{
	CopySubFrom(n);
	CopyFieldsFrom(n);
}

void MetaNode::CopySubFrom(const MetaNode& n)
{
	int c = n.sub.GetCount();
	sub.SetCount(c);
	for(int i = 0; i < c; i++)
		sub[i].Assign(this, n.sub[i]);
}

String MetaNode::GetKindString() const { return GetKindString(kind); }

String MetaNode::GetKindString(int kind)
{
	if(kind >= 0 && kind <= CXCursor_OverloadCandidate)
		return GetCursorKindName((CXCursorKind)kind);
	switch (kind) {
		#define DATASET_ITEM(type, name, kind, group, desc) case kind: return desc;
		DATASET_LIST
		#undef DATASET_ITEM
	default:
		return "Unknown kind: " + IntStr(kind);
	}
}

void MetaNode::FindDifferences(const MetaNode& n, Vector<String>& diffs, int max_diffs) const
{
	bool had_diff = false;
#define CHK_FIELD(x)                                                                           \
	if(x != n.x) {                                                                             \
		had_diff = true;                                                                       \
		diffs.Add("Different " #x ": " + AsString(x) + " vs " + AsString(n.x) + " at " +       \
		          GetKindString() + ", " + id + " (" + type + ")");                            \
	}
	CHK_FIELD(kind);
	CHK_FIELD(id);
	CHK_FIELD(type);
	CHK_FIELD(type_hash);
	CHK_FIELD(begin);
	CHK_FIELD(end);
	CHK_FIELD(filepos_hash);
	CHK_FIELD(file);
	CHK_FIELD(pkg);
	CHK_FIELD(is_ref);
	CHK_FIELD(is_definition);
	CHK_FIELD(sub.GetCount());
	CHK_FIELD(is_disabled);
	CHK_FIELD(serial);
	if(!had_diff)
		for(int i = 0; i < sub.GetCount(); i++) {
			CHK_FIELD(sub[i].kind);
			CHK_FIELD(sub[i].id);
			if(had_diff)
				break;
			sub[i].FindDifferences(n.sub[i], diffs, max_diffs);
			if(diffs.GetCount() >= max_diffs)
				break;
		}
}

bool MetaNode::IsFieldsSame(const MetaNode& n) const
{
	if ((bool)ext != (bool)n.ext) return false;
	bool eq =
	       kind == n.kind && id == n.id && type == n.type && type_hash == n.type_hash &&
	       begin == n.begin && end == n.end && filepos_hash == n.filepos_hash &&
	       file == n.file && pkg == n.pkg && is_ref == n.is_ref &&
	       is_definition == n.is_definition && is_disabled == n.is_disabled &&
	       serial == n.serial;
	if (eq) eq = *ext == *n.ext; // kind is already checked to be the same, and both has ext
	return eq;
}

void MetaNode::CopyFieldsFrom(const MetaNode& n, bool forced_downgrade)
{
	kind = n.kind;
	id = n.id;
	type = n.type;
	type_hash = n.type_hash;
	begin = n.begin;
	end = n.end;
	filepos_hash = n.filepos_hash;
	file = n.file;
	pkg = n.pkg;
	is_ref = n.is_ref;
	is_definition = n.is_definition;
	is_disabled = n.is_disabled;
	if (n.ext) {
		ASSERT(kind >= 0);
		ext = MetaExtFactory::CloneKind(kind, *n.ext, *this);
	} else ext.Clear();
	ASSERT(serial <= n.serial || forced_downgrade);
	serial = n.serial;
	Chk();
}

hash_t MetaNode::GetTotalHash() const
{
	CombineHash ch;
	ch	.Do(kind)
		.Do(id)
		.Do(type)
		.Do(type_hash)
		.Do(begin)
		.Do(end)
		.Do(filepos_hash)
		.Do(file)
		.Do(pkg)
		.Do(is_ref)
		.Do(is_definition)
		.Do(is_disabled)
		.Do(serial);
	if (ext)
		ch.Put(ext->GetHashValue());
	for(const auto& s : sub)
		ch.Put(s.GetTotalHash());
	return ch;
}

void MetaNode::Visit(NodeVisitor& vis) {
	#define Do(x) (#x,x)
	vis.Ver(1)
	(1)	Do(kind)
		Do(id)
		Do(type)
		Do((int64&)type_hash)
		Do(begin)
		Do(end)
		Do((int64&)filepos_hash)
		Do(file)
		//Do(pkg)
		Do(is_ref)
		Do(is_definition)
		Do(is_disabled)
		Do((int64&)serial)
		;
	
	bool has_ext = ext;
	vis("has_ext", has_ext);
	if (has_ext) {
		if (vis.IsLoading()) ext = MetaExtFactory::CreateKind(kind, *this);
		if (ext)
			vis("ext",*ext, VISIT_NODE);
	}
	vis
		("sub", sub, VISIT_VECTOR)
		;
	#undef Do
	if (vis.IsLoading())
		FixParent();
	
	#if 1
	if (vis.IsLoading())
		Chk();
	#endif
}

void MetaNode::DeepChk() {
	#ifdef flagDEBUG
	Chk();
	for (auto& s : sub)
		s.DeepChk();
	#endif
}

void MetaNode::Chk() {
	#ifdef flagDEBUG
	#if 1
	if (kind == METAKIND_ECS_ENTITY && owner) {
		//ASSERT(owner->kind != METAKIND_ECS_SPACE);
		ASSERT(owner->kind != METAKIND_CONTEXT);
		ASSERT(owner->kind != METAKIND_PKG_ENV);
	}
	#endif
	#endif
}

hash_t MetaNode::GetSourceHash(bool* total_hash_diffs) const
{
	if(kind < 0 || kind >= METAKIND_BEGIN)
		return 0;
	CombineHash ch;
	ch.Do(kind).Do(id).Do(type);
	for(const auto& s : sub) {
		if(s.kind < 0 || s.kind >= METAKIND_BEGIN) {
			if(total_hash_diffs)
				*total_hash_diffs = true;
			continue;
		}
		ch.Put(s.GetSourceHash());
	}
	return ch;
}

Vector<MetaNode*> MetaNode::FindAllShallow(int kind)
{
	Vector<MetaNode*> vec;
	for(auto& s : sub)
		if(s.kind == kind)
			vec << &s;
	return vec;
}

void MetaNode::FindAllDeep(int kind, Vector<MetaNode*>& out)
{
	if(this->kind == kind)
		out << this;
	for(auto& s : sub)
		s.FindAllDeep(kind, out);
}

void MetaNode::FindAllDeep(int kind, Vector<const MetaNode*>& out) const
{
	if(this->kind == kind)
		out << this;
	for(const auto& s : sub)
		s.FindAllDeep(kind, out);
}

Vector<const MetaNode*> MetaNode::FindAllShallow(int kind) const
{
	Vector<const MetaNode*> vec;
	for(const auto& s : sub)
		if(s.kind == kind)
			vec << &s;
	return vec;
}

bool MetaNode::IsSourceKind() const { return kind >= 0 && kind < METAKIND_BEGIN; }

bool MetaNode::IsStructKind() const
{
	return kind == CXCursor_StructDecl && kind == CXCursor_ClassDecl &&
	       kind == CXCursor_ClassTemplate &&
	       kind == CXCursor_ClassTemplatePartialSpecialization;
}

int MetaNode::GetRegularCount() const
{
	int c = 0;
	for(const auto& s : sub)
		if(s.kind >= 0 && s.kind < METAKIND_BEGIN)
			c++;
	return c;
}

String MetaNode::GetBasesString() const
{
	String s;
	Vector<const MetaNode*> bases = FindAllShallow(CXCursor_CXXBaseSpecifier);
	for(const MetaNode* n : bases) {
		if(!s.IsEmpty())
			s.Cat(", ");
		s << n->id << " (" << n->type << ")";
	}
	return s;
}

String MetaNode::GetNestString() const
{
	if(owner)
		return owner->id;
	return String();
}

bool MetaNode::OwnerRecursive(const MetaNode& n) const
{
	MetaNode* o = this->owner;
	while(o) {
		if(o == &n)
			return true;
		o = o->owner;
	}
	return false;
}

bool MetaNode::ContainsDeep(const MetaNode& n) const
{
#if 1
	return n.OwnerRecursive(*this) || &n == this;
#else
	if(this == &n)
		return true;
	for(const auto& s : sub)
		if(s.ContainsDeep(n))
			return true;
	return false;
#endif
}

void MetaNode::RemoveAllShallow(int kind)
{
	Vector<int> rmlist;
	int i = 0;
	for(auto& s : sub) {
		if(s.kind == kind)
			rmlist << i;
		i++;
	}
	if(!rmlist.IsEmpty())
		sub.Remove(rmlist);
}

void MetaNode::RemoveAllDeep(int kind)
{
	RemoveAllShallow(kind);
	for(auto& s : sub)
		s.RemoveAllDeep(kind);
}

void MetaNode::GetTypeHashes(Index<hash_t>& type_hashes) const
{
	if(type_hash)
		type_hashes.FindAdd(type_hash);
	for(auto& s : sub)
		s.GetTypeHashes(type_hashes);
}

void MetaNode::RealizeSerial() {
	if (!serial)
		serial = MetaEnv().NewSerial();
	for (auto& s : sub)
		s.RealizeSerial();
}

Vector<Ptr<MetaNodeExt>> MetaNode::GetAllExtensions() {
	Vector<Ptr<MetaNodeExt>> v;
	for (auto& s : sub) {
		if (s.ext) {
			v.Add(&*s.ext);
		}
	}
	return v;
}

String MetaNode::GetPath() const {
	static const int LIMIT = 64;
	const MetaNode* ptrs[LIMIT];
	int i = 1;
	ptrs[0] = this;
	while (i < LIMIT-1) {
		if (ptrs[i-1]->owner) {
			const MetaNode* p = ptrs[i-1]->owner;
			ptrs[i++] = p;
		}
		else break;
	}
	ptrs[i] = 0;
	const MetaNode** iter = ptrs;
	String path;
	while (*iter) {
		path.Cat('/');
		path.Cat((*iter)->id);
		iter++;
	}
	return path;
}



MetaNodeExt& MetaExtCtrl::GetExt() {return *ext;}
MetaNode& MetaExtCtrl::GetNode() {return ext->node;}
String MetaExtCtrl::GetFilePath() const {
	if (owner)
		return owner->GetFilePath();
	else
		return String();
}

/*void MetaEnvironment::Store(const String& includes, const String& path, FileAnnotation& fa)
{
    MetaSrcPkg& af = ResolveFile(includes, path);
    af.Store(includes, path, fa);
}*/

/*void MetaSrcPkg::Store(const String& includes, const String& path, FileAnnotation& fa)
{
    MetaSrcFile& afi = RealizePath(includes, path);
    afi.UpdateLinks(fa);
    Save();
}*/

/*bool MetaNode::IsClassTemplateDefinition() const {
    if (kind == CXCursor_ClassTemplate)
        //for (const MetaNode& s : sub)
        //	if (s.kind == CXCursor_CompoundStmt)
                return true;
    return false;
}*/

MetaNode* MetaEnvironment::FindDeclaration(const MetaNode& n)
{
	if(!n.filepos_hash)
		return 0;
	int i = filepos.Find(n.filepos_hash);
	if(i < 0)
		return 0;
	const auto& vec = filepos[i].hash_nodes;
	for(const auto& ptr : vec) {
		if(!ptr)
			continue;
		MetaNode& p = *ptr;
		if(p.is_definition /* || p.IsClassTemplateDefinition()*/)
			return &p;
	}
	return 0;
}

Vector<MetaNode*> MetaEnvironment::FindDeclarationsDeep(const MetaNode& n)
{
	Vector<MetaNode*> v;
	if(n.kind == CXCursor_CXXBaseSpecifier) {
		for(const auto& s : n.sub) {
			MetaNode* d = FindDeclaration(s);
			if(d)
				v.Add(d);
		}
		return v;
	}
	else
		Panic("TODO");
	return v;
}

MetaNode* MetaEnvironment::FindTypeDeclaration(hash_t type_hash)
{
	if(!type_hash)
		return 0;
	int i = types.Find(type_hash);
	if(i < 0)
		return 0;
	const auto& vec = types[i].hash_nodes;
	for(const auto& ptr : vec) {
		if(!ptr)
			continue;
		MetaNode& p = *ptr;
		if(p.is_definition /* || p.IsClassTemplateDefinition()*/)
			return &p;
	}
	return 0;
}

bool MetaEnvironment::MergeResolver(ClangTypeResolver& ctr)
{
	const VectorMap<hash_t, Index<String>>& scope_paths = ctr.GetScopePaths();
	auto& translation = ctr.GetTypeTranslation();

	for(int i = 0; i < scope_paths.GetCount(); i++) {
		hash_t src_hash = scope_paths.GetKey(i);
		const Index<String>& idx = scope_paths[i];
		String path = idx[0];
		hash_t dst_hash = RealizeTypePath(path);
		translation.GetAdd(src_hash, dst_hash);
	}

	return true;
}

hash_t MetaEnvironment::RealizeTypePath(const String& path)
{
	hash_t h = path.GetHashValue();
	types.GetAdd(h).seen_type = path;
	return h;
}

MetaNode& MetaEnvironment::RealizeFileNode(int pkg, int file, int kind) {
	for (auto& s : root.sub) {
		if (s.pkg == pkg && s.file == file && s.kind == kind) {
			if (!s.serial)
				s.serial = NewSerial();
			return s;
		}
	}
	MetaNode& n = root.sub.Add();
	n.pkg = pkg;
	n.file = file;
	n.kind = kind;
	n.serial = NewSerial();
	n.id = pkgs[pkg].files[file].GetTitle();
	return n;
}















MetaNodeExt* MetaExtFactory::CreateKind(int kind, MetaNode& owner) {
	int i = FindKindFactory(kind);
	if (i < 0) return 0;
	return List()[i].new_fn(owner);
}

MetaNodeExt* MetaExtFactory::CloneKind(int kind, const MetaNodeExt& e, MetaNode& owner) {
	int i = FindKindFactory(kind);
	if (i < 0) return 0;
	MetaNodeExt* n = List()[i].new_fn(owner);
	n->CopyFrom(e);
	return n;
}

MetaNodeExt* MetaExtFactory::Clone(const MetaNodeExt& e, MetaNode& owner) {
	for (Factory& f : List()) {
		if (f.is_fn(e)) {
			MetaNodeExt* n = f.new_fn(owner);
			n->CopyFrom(e);
			return n;
		}
	}
	return 0;
}

int MetaExtFactory::FindKindFactory(int kind) {
	int i = 0;
	for (Factory& f : List()) {if (f.kind == kind) return i; i++;}
	return -1;
}

int MetaExtFactory::FindKindCategory(int k) {
	#define DATASET_ITEM(type, name, kind, group, desc) if (k == kind) return group;
	DATASET_LIST
	#undef DATASET_ITEM
	return -1;
}

void MetaNodeExt::CopyFrom(const MetaNodeExt& e) {
	StringStream s;
	s.SetStoring();
	NodeVisitor read(s);
	const_cast<MetaNodeExt&>(e).Visit(read); // reading
	s.SetLoading();
	s.Seek(0);
	NodeVisitor write(s);
	Visit(write);
}

bool MetaNodeExt::operator==(const MetaNodeExt& e) const {
	return GetHashValue() == e.GetHashValue();
}

hash_t MetaNodeExt::GetHashValue() const {
	NodeVisitor vis(0);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
	return vis.hash;
}

void MetaNodeExt::Serialize(Stream& s){
	NodeVisitor vis(s);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
}

void MetaNodeExt::Jsonize(JsonIO& json){
	NodeVisitor vis(json);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
}

END_UPP_NAMESPACE

#include <AI/AI.h>
#include <ide/ide.h>

NAMESPACE_UPP

#if 0

void MetaSrcPkg::PostSave()
{
	if(!post_saving) {
		post_saving = true;
		PostCallback(THISBACK1(Save, false));
	}
}

void MetaSrcPkg::Clear() { files.Clear(); }

void MetaSrcPkg::SetPath(String path)
{
	this->path = path;
	dir = GetFileDirectory(path);
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

bool MakeRelativePath(const String& includes_, const String& dir, String& best_ai_dir, String& best_rel_dir)
{
	bool found = false;
	Vector<String> ai_dirs = GetAiDirsRaw();
	if (!ai_dirs.IsEmpty()) {
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
				if (dir.Find(s) == 0) {
					String rel_dir = dir.Mid(s.GetCount());
					int cand_parts = Split(rel_dir, DIR_SEPS).GetCount();
					// Prefer the shortest directory
					if (cand_parts < def_cand_parts) {
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
String GetAiPathCandidate(const String& includes_, String dir)
{
	Vector<String> ai_dirs = GetAiDirsRaw();
	Vector<String> upp_dirs = GetUppDirs();
	String dummy_cand, def_cand, any_ai_cand, preferred_ai_cand;
	int def_cand_parts = INT_MAX;
	dummy_cand = dir + DIR_SEPS + "AI.json";
	if (!ai_dirs.IsEmpty()) {
		for (const String& upp_dir : upp_dirs) {
			if (dir.Find(upp_dir) != 0) continue;
			String rel_path = dir.Mid(upp_dir.GetCount());
			for (const String& ai_dir : ai_dirs) {
				String ai_dir_cand = AppendFileName(ai_dir, rel_path);
				String path = AppendFileName(ai_dir_cand, "AI.json");
				if (any_ai_cand.IsEmpty())
					any_ai_cand = path;
				if (preferred_ai_cand.IsEmpty() && FileExists(path))
					preferred_ai_cand = path;
			}
		}
	}
	if (!preferred_ai_cand.IsEmpty())
		return preferred_ai_cand;
	else if (!any_ai_cand.IsEmpty())
		return any_ai_cand;
	
	if (!ai_dirs.IsEmpty()) {
		String ai_dir, rel_dir;
		if (MakeRelativePath(includes_, dir, ai_dir, rel_dir)) {
			String abs_dir = AppendFileName(ai_dir, rel_dir);
			def_cand = AppendFileName(abs_dir, "AI.json");
		}
	}
	
	if (!def_cand.IsEmpty())
		return def_cand;
	else
		return dummy_cand;
}
#if 0
Vector<String> FindParentUppDirectories(const String& sub_dir) {
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
#endif
String MetaEnvironment::ResolveMetaSrcPkgPath(const String& includes, String path)
{
	String def_dir = GetFileDirectory(path);
	Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
	for (String& upp_dir : upp_dirs)
		return GetAiPathCandidate(includes, upp_dir);
	return GetAiPathCandidate(includes, def_dir);
}
#if 0
MetaSrcPkg& MetaEnvironment::ResolveFile(const String& includes, String path)
{
	String aion_path = ResolveMetaSrcPkgPath(includes, path);
	lock.EnterRead();
	int i = files.Find(aion_path);
	if(i >= 0) {
		MetaSrcPkg& f = files[i];
		lock.LeaveRead();
		return f;
	}
	lock.LeaveRead();
	lock.EnterWrite();
	MetaSrcPkg& f = files.Add(aion_path);
	f.SetPath(aion_path);
	f.Load();
	lock.LeaveWrite();
	return f;
}

MetaSrcFile& MetaEnvironment::ResolveFileInfo(const String& includes, String path)
{
	return ResolveFile(includes, path).RealizePath(includes, path);
}

void MetaSrcPkg::Load(const String& includes, const String& path, FileAnnotation& fa)
{
	lock.Enter();
	if(IsEmpty())
		Load();
	lock.Leave();
	MetaSrcFile& afi = RealizePath(includes, path);
	afi.UpdateLinks(fa);
}

#endif

MetaEnvironment& MetaEnv() { return Single<MetaEnvironment>(); }

MetaEnvironment::MetaEnvironment() {
	root.kind = CXCursor_Namespace;
	
	
}

void MetaEnvironment::Load(const String& includes, const String& path, FileAnnotation& fa)
{
	String aion_path = ResolveMetaSrcPkgPath(includes, path);
	
	if (pkgs.Find(aion_path) < 0) {
		MetaNode file_nodes;
		MetaSrcPkg& pkg = pkgs.Add(aion_path);
		if (pkg.Load(aion_path, file_nodes)) {
			int pkg_id = pkgs.Find(aion_path);
			pkg.saved_hash = IntStr64(file_nodes.GetCommonHash());
			MergeNode(root, file_nodes, pkg_id);
		}
	}
}
void MetaEnvironment::Store(String& includes, const String& path, ClangNode& n)
{
	//LOG(n.GetTreeString());
	
	Vector<MetaNode*> scope;
	scope << &root;
	if (!MergeVisit(scope, n))
		return;
	
	String aion_path = ResolveMetaSrcPkgPath(includes, path);
	MetaSrcPkg& pkg = pkgs.GetAdd(aion_path);
	int pkg_id = pkgs.Find(aion_path);
	
	MetaNode file_nodes;
	SplitNode(root, file_nodes, pkg_id);
	
	String hash = IntStr64(file_nodes.GetCommonHash());
	if (pkg.saved_hash != hash) {
		pkg.saved_hash = hash;
		pkg.Store(aion_path, file_nodes);
	}
}

void MetaEnvironment::MergeNode(MetaNode& root, const MetaNode& other, int pkg_id) {
	
}

void MetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id) {
	
}

bool MetaEnvironment::MergeVisit(Vector<MetaNode*>& scope, ClangNode& n1) {
	MetaNode& n0 = *scope.Top();
	
	/*if (n0.kind != n1.kind) {
		LOG("MetaEnvironment::MergeVisit: error: kind mismatch " << n0.kind << " != " << n1.kind);
		return false;
	}
	
	if (n0.id != n1.id) {
		LOG("MetaEnvironment::MergeVisit: error: kind matches, but id mismatches " << n0.id << " != " << n1.id);
		return false;
	}*/
	ASSERT(n0.kind == n1.kind && n0.id == n1.id);
	
	if (IsMergeable((CXCursorKind)n0.kind)) {
		for (ClangNode& sub1 : n1.sub) {
			int i = n0.Find(sub1.kind, sub1.id);
			if (i < 0)
				n0.sub.Add() = sub1;
			else {
				MetaNode& sub0 = n0.sub[i];
				scope << &sub0;
				bool succ = MergeVisit(scope, sub1);
				scope.Pop();
				if (!succ)
					return false;
			}
		}
	}
	else {
		if (!n0.common_hash)
			n0.common_hash = n0.GetCommonHash();
		hash_t n1_common_hash = n1.GetCommonHash();
		if (n0.common_hash != n1_common_hash) {
			// Node changed
			n0 = n1;
			n0.common_hash = n1_common_hash;
		}
	}
	return true;
}

bool MetaEnvironment::IsMergeable(CXCursorKind kind) {
	switch(kind) {
		//case CXCursor_StructDecl:
		//case CXCursor_ClassDecl:
		case CXCursor_Namespace:
		case CXCursor_LinkageSpec:
			return true;
		default:
			return false;
	}
}

String MetaNode::GetTreeString(int depth) const {
	String s;
	s.Cat('\t', depth);
	s << FetchString(clang_getCursorKindSpelling((CXCursorKind)kind));
	if (!id.IsEmpty()) s << ": " << id;
	s << "\n";
	for (auto& n : sub)
		s << n.GetTreeString(depth+1);
	return s;
}

int MetaNode::Find(int kind, const String& id) const {
	int i = 0;
	for (const MetaNode& n : sub) {
		if (n.kind == kind && n.id == id)
			return i;
		i++;
	}
	return -1;
}

void MetaNode::operator=(const ClangNode& n) {
	int c = n.sub.GetCount();
	sub.SetCount(c);
	for(int i = 0; i < c; i++)
		sub[i] = n.sub[i];
	kind = n.kind;
	id = n.id;
	begin = n.begin;
	end = n.end;
	common_hash = 0; // too heavy to update here
}

hash_t MetaNode::GetCommonHash() const {
	CombineHash ch;
	ch.Do(kind).Do(id);
	for (const auto& s : sub)
		ch.Put(s.GetCommonHash());
	return ch;
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

END_UPP_NAMESPACE

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
#endif
void MetaSrcPkg::SetPath(String bin_path, String upp_dir)
{
	this->bin_path = bin_path;
	//dir = GetFileDirectory(path);
	this->upp_dir = upp_dir;
}
#if 0
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
String GetAiPathCandidate(const String& includes_, String dir)
{
	Vector<String> ai_dirs = GetAiDirsRaw();
	Vector<String> upp_dirs = GetUppDirs();
	String dummy_cand, def_cand, any_ai_cand, preferred_ai_cand;
	int def_cand_parts = INT_MAX;
	dummy_cand = dir + DIR_SEPS + "Meta.bin";
	if (!ai_dirs.IsEmpty()) {
		for (const String& upp_dir : upp_dirs) {
			if (dir.Find(upp_dir) != 0) continue;
			String rel_path = dir.Mid(upp_dir.GetCount());
			for (const String& ai_dir : ai_dirs) {
				String ai_dir_cand = AppendFileName(ai_dir, rel_path);
				String path = AppendFileName(ai_dir_cand, "Meta.bin");
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
			def_cand = AppendFileName(abs_dir, "Meta.bin");
		}
	}
	
	if (!def_cand.IsEmpty())
		return def_cand;
	else
		return dummy_cand;
}

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

String MetaEnvironment::ResolveMetaSrcPkgPath(const String& includes, String path, String& ret_upp_dir)
{
	String def_dir = GetFileDirectory(path);
	Vector<String> upp_dirs = FindParentUppDirectories(def_dir);
	for (String& upp_dir : upp_dirs) {
		ret_upp_dir = upp_dir;
		return GetAiPathCandidate(includes, upp_dir);
	}
	ret_upp_dir = def_dir;
	return GetAiPathCandidate(includes, def_dir);
}

MetaSrcPkg& MetaEnvironment::ResolveFile(const String& includes, String path)
{
	String upp_dir;
	String aion_path = ResolveMetaSrcPkgPath(includes, path, upp_dir);
	lock.EnterWrite();
	int pkg_i = pkgs.Find(aion_path);
	MetaSrcPkg& pkg = pkg_i >= 0 ? pkgs[pkg_i] : pkgs.GetAdd(aion_path);
	if (pkg.id < 0) pkg.id = pkg_i >= 0 ? pkg_i : pkgs.GetCount()-1;
	pkg.SetPath(aion_path, upp_dir);
	String rel_path = pkg.GetRelativePath(path);
	pkg.filenames.FindAdd(rel_path);
	lock.LeaveWrite();
	return pkg;
}

String MetaSrcPkg::GetRelativePath(const String& path) const {
	int i = path.Find(upp_dir);
	if (i >= 0) {
		String s = path.Mid(upp_dir.GetCount());
		if (s.GetCount() && s[0] == DIR_SEP)
			s = s.Mid(1);
		return s;
	}
	else
		Panic("TODO"); //return NormalizePath(path, dir);
}

String MetaSrcPkg::GetFullPath(const String& rel_path) const {
	return AppendFileName(upp_dir, rel_path);
}

String MetaSrcPkg::GetFullPath(int file_i) const {
	return AppendFileName(upp_dir, filenames[file_i]);
}

#if 0
MetaSrcFile& MetaEnvironment::ResolveFileInfo(const String& includes, String path)
{
	return ResolveFile(includes, path).RealizePath(includes, path);
}
#endif
bool MetaSrcPkg::Store(MetaNode& file_nodes)
{
	String hash = IntStr64(file_nodes.GetCommonHash());
	if (saved_hash == hash)
		return true;
	saved_hash = hash;
	ASSERT(!saved_hash.IsEmpty());
	ASSERT(!bin_path.IsEmpty());
	lock.Enter();
	RealizeDirectory(GetFileDirectory(bin_path));
	FileOut s(bin_path);
	Serialize(s);
	s % file_nodes;
	s.Close();
	lock.Leave();
	return true;
}

bool MetaSrcPkg::Load(MetaNode& file_nodes)
{
	ASSERT(this->bin_path.GetCount());
	bool succ = false;
	lock.Enter();
	FileIn s(bin_path);
	if (s.GetSize() > 0) {
		Serialize(s);
		s % file_nodes;
		succ = !saved_hash.IsEmpty();
	}
	s.Close();
	lock.Leave();
	return succ;
}

MetaEnvironment& MetaEnv() { return Single<MetaEnvironment>(); }

MetaEnvironment::MetaEnvironment() {
	root.kind = CXCursor_Namespace;
	
	
}

void MetaEnvironment::Load(const String& includes, const String& path)
{
	MetaNode file_nodes;
	MetaSrcPkg& pkg = this->ResolveFile(includes, path);
	if (pkg.Load(file_nodes)) {
		file_nodes.SetTempDeep();
		ASSERT(pkg.saved_hash == IntStr64(file_nodes.GetCommonHash()));
		file_nodes.SetPkgDeep(pkg.id);
		MergeNode(root, file_nodes);
	}
}

void MetaEnvironment::Store(String& includes, const String& path, ClangNode& cn)
{
	//LOG(n.GetTreeString());
	MetaSrcPkg& pkg = ResolveFile(includes, path);
	String rel_path = pkg.GetRelativePath(path);
	int file_id = pkg.filenames.Find(rel_path);
	MetaNode n;
	n = cn;
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file_id);
	if (!MergeNode(root, n))
		return;
	
	MetaNode file_nodes;
	SplitNode(root, file_nodes, pkg.id);
	
	pkg.Store(file_nodes);
}

bool MetaEnvironment:: MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1) {
	MetaNode& n0 = *scope.Top();
	ASSERT(n0.kind == n1.kind && n0.id == n1.id);
	if (IsMergeable((CXCursorKind)n0.kind)) {
		for (const MetaNode& sub1 : n1.sub) {
			int i = n0.Find(sub1.kind, sub1.id);
			if (i < 0) {
				auto& n = n0.sub.Add();
				n = sub1;
				MergeVisitPost(n);
			}
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
			MergeVisitPost(n0);
		}
	}
	return true;
}

void MetaEnvironment::MergeVisitPost(MetaNode& n) {
	RefreshFilePos(n);
	for (auto& s : n.sub)
		MergeVisitPost(s);
}

bool MetaEnvironment::MergeNode(MetaNode& root, const MetaNode& other) {
	Vector<MetaNode*> scope;
	scope << &root;
	return MergeVisit(scope, other);
}

void MetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id) {
	root.PointPkgTo(other, pkg_id);
}

void MetaEnvironment::SplitNode(MetaNode& root, MetaNodeSubset& other, int pkg_id, int file_id) {
	root.PointPkgTo(other, pkg_id, file_id);
}

void MetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id) {
	root.CopyPkgTo(other, pkg_id);
}

void MetaEnvironment::SplitNode(const MetaNode& root, MetaNode& other, int pkg_id, int file_id) {
	root.CopyPkgTo(other, pkg_id, file_id);
}

String MetaEnvironment::GetFilepath(int pkg_id, int file_id) const {
	const auto& pkg = this->pkgs[pkg_id];
	String path = pkg.GetFullPath(file_id);
	return path;
}

void MetaNode::PointPkgTo(MetaNodeSubset& other, int pkg_id) {
	other.n = this;
	for (auto& n0 : sub) {
		if (n0.HasPkgDeep(pkg_id)) {
			MetaNodeSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id);
		}
	}
}

void MetaNode::PointPkgTo(MetaNodeSubset& other, int pkg_id, int file_id) {
	other.n = this;
	for (auto& n0 : sub) {
		if (n0.HasPkgFileDeep(pkg_id, file_id)) {
			MetaNodeSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id, file_id);
		}
	}
}

void MetaNode::CopyPkgTo(MetaNode& other, int pkg_id) const {
	other.CopyFieldsFrom(*this);
	for (const auto& n0 : sub) {
		if (n0.HasPkgDeep(pkg_id)) {
			MetaNode& n1 = other.sub.Add();
			n0.CopyPkgTo(n1, pkg_id);
		}
	}
}

void MetaNode::CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const {
	other.CopyFieldsFrom(*this);
	for (const auto& n0 : sub) {
		if (n0.HasPkgFileDeep(pkg_id, file_id)) {
			MetaNode& n1 = other.sub.Add();
			n0.CopyPkgTo(n1, pkg_id, file_id);
		}
	}
}

bool MetaNode::HasPkgDeep(int pkg_id) const {
	if (this->pkg == pkg_id)
		return true;
	for (const auto& n : sub)
		if (n.HasPkgDeep(pkg_id))
			return true;
	return false;
}

bool MetaNode::HasPkgFileDeep(int pkg_id, int file_id) const {
	if (this->pkg == pkg_id && this->file == file_id)
		return true;
	for (const auto& n : sub)
		if (n.HasPkgFileDeep(pkg_id, file_id))
			return true;
	return false;
}

void MetaNode::SetPkgDeep(int pkg_id) {
	this->pkg = pkg_id;
	for (auto& n : sub)
		n.SetPkgDeep(pkg_id);
}

void MetaNode::SetFileDeep(int file_id) {
	this->file = file_id;
	for (auto& n : sub)
		n.SetFileDeep(file_id);
}

void MetaNode::SetTempDeep() {
	only_temporary = true;
	for (auto& n : sub)
		n.SetTempDeep();
}

bool MetaEnvironment::IsMergeable(int kind) {
	return IsMergeable((CXCursorKind)kind);
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

void MetaEnvironment::RefreshFilePos(MetaNode& n) {
	ASSERT(!n.only_temporary);
	if (n.filepos_hash != 0) {
		auto& vec = this->filepos_nodes.GetAdd(n.filepos_hash);
		bool found = false;
		bool found_zero = false;
		for (auto& p : vec) {
			MetaNode* ptr = &*p;
			found_zero = ptr == 0 || found_zero;
			if (ptr == &n) {
				found = true;
				break;
			}
		}
		if (!found)
			vec.Add(&n);
		if (found_zero) {
			Vector<int> rmlist;
			for(int i = 0; i < vec.GetCount(); i++)
				if (&*vec[i] == 0)
					rmlist << i;
			vec.Remove(rmlist);
		}
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
	type = n.type;
	begin = n.begin;
	end = n.end;
	filepos_hash = n.filepos_hash;
	common_hash = 0; // too heavy to update here
	is_ref = n.is_ref;
}

void MetaNode::CopyFieldsFrom(const MetaNode& n) {
	kind = n.kind;
	id = n.id;
	type = n.type;
	begin = n.begin;
	end = n.end;
	common_hash = n.common_hash;
	filepos_hash = n.filepos_hash;
	file = n.file;
	pkg = n.pkg;
	is_ref = n.is_ref;
}

hash_t MetaNode::GetCommonHash() const {
	CombineHash ch;
	ch.Do(kind).Do(id).Do(type);
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

MetaNode* MetaEnvironment::FindDeclaration(const MetaNode& n) {
	if (!n.filepos_hash) return 0;
	int i = filepos_nodes.Find(n.filepos_hash);
	if (i < 0) return 0;
	const auto& vec = filepos_nodes[i];
	for (const auto& ptr : vec)
		if (ptr && !ptr->is_ref)
			return &*ptr;
	return 0;
}

END_UPP_NAMESPACE

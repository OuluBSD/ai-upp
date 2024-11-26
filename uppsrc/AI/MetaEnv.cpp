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
	return String();
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
bool MetaSrcPkg::Store(MetaNode& file_nodes, bool forced)
{
	bool total_hash_diffs = false;
	String hash = IntStr64(file_nodes.GetCommonHash(&total_hash_diffs));
	if (!forced) {
		if (saved_hash == hash && !total_hash_diffs)
			return true;
		if (saved_hash == hash && total_hash_diffs) {
			Panic("TODO");
		}
		else if (total_hash_diffs) {
			Panic("TODO");
		}
	}
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

MetaSrcPkg& MetaEnvironment::Load(const String& includes, const String& path)
{
	MetaNode file_nodes;
	MetaSrcPkg& pkg = this->ResolveFile(includes, path);
	if (pkg.Load(file_nodes)) {
		LOG(file_nodes.GetTreeString());
		file_nodes.SetTempDeep();
		ASSERT(pkg.saved_hash == IntStr64(file_nodes.GetCommonHash()));
		file_nodes.SetPkgDeep(pkg.id);
		MergeNode(root, file_nodes, MERGEMODE_OVERWRITE_OLD);
		LOG(root.GetTreeString());
		
		#if DEBUG_METANODE_DTOR
		Vector<MetaNode*> comments;
		root.FindAllDeep(METAKIND_COMMENT, comments);
		for (auto* c : comments)
			c->trace_kill = true;
		#endif
	}
	return pkg;
}

void MetaEnvironment::Store(MetaSrcPkg& pkg, bool forced)
{
	MetaNode file_nodes;
	SplitNode(root, file_nodes, pkg.id);
	
	LOG(file_nodes.GetTreeString());
	pkg.Store(file_nodes, forced);
}

void MetaEnvironment::Store(String& includes, const String& path, ClangNode& cn)
{
	//LOG(n.GetTreeString());
	MetaSrcPkg& pkg = ResolveFile(includes, path);
	String rel_path = pkg.GetRelativePath(path);
	int file_id = pkg.filenames.Find(rel_path);
	MetaNode n;
	n.Assign(0, cn);
	n.SetPkgDeep(pkg.id);
	n.SetFileDeep(file_id);
	if (!MergeNode(root, n, MERGEMODE_OVERWRITE_OLD))
		return;
	
	Store(pkg);
}

bool MetaEnvironment:: MergeVisit(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode) {
	MetaNode& n0 = *scope.Top();
	ASSERT(n0.kind == n1.kind && n0.id == n1.id);
	if (IsMergeable((CXCursorKind)n0.kind)) {
		for (const MetaNode& sub1 : n1.sub) {
			int i = n0.Find(sub1.kind, sub1.id);
			if (i < 0) {
				auto& n = n0.Add(sub1);
				MergeVisitPost(n);
			}
			else {
				MetaNode& sub0 = n0.sub[i];
				scope << &sub0;
				bool succ = MergeVisit(scope, sub1, mode);
				scope.Pop();
				if (!succ)
					return false;
			}
		}
	}
	else {
		if (mode == MERGEMODE_BAIL_OUT)
			return false;
		if (MERGEMODE_KEEP_OLD && !n0.sub.IsEmpty() && n1.sub.IsEmpty()) {
			// pass
		}
		else if (MERGEMODE_KEEP_OLD && n0.sub.IsEmpty() && !n1.sub.IsEmpty()) {
			n0.CopySubFrom(n1);
		}
		else if (MERGEMODE_OVERWRITE_OLD && (n0.sub.IsEmpty() || n1.sub.IsEmpty())) {
			n0.CopyFrom(n1);
		}
		else {
			hash_t n0_total_hash = n0.GetTotalHash();
			hash_t n1_total_hash = n1.GetTotalHash();
			if (n0_total_hash != n1_total_hash) {
				Vector<String> find_diffs;
				n0.FindDifferences(n1, find_diffs);
				DUMPC(find_diffs);
				Panic("TODO"); // fix find_diffs hash
				return MergeVisitPartMatching(scope, n1, mode);
			}
		}
	}
	return true;
}

bool MetaEnvironment::MergeVisitPartMatching(Vector<MetaNode*>& scope, const MetaNode& n1, MergeMode mode) {
	MetaNode& n0 = *scope.Top();
	if (mode == MERGEMODE_OVERWRITE_OLD)
		n0.CopyFieldsFrom(n1);
	
	ASSERT(n0.sub.GetCount());
	ASSERT(n0.kind == n1.kind && n0.id == n1.id);
	ASSERT(!IsMergeable((CXCursorKind)n0.kind));
	struct Hashes {
		MetaNode* n = 0;
		MetaNode* match = 0;
		hash_t common_hash;
		bool ready = false;
		//int common_match = -1, insert = -1;
		//int prev = -1, next = -1;
		void Set(MetaNode* mn, hash_t c) {n = mn; common_hash = c;}
	};
	Array<Hashes> n0_subs;
	Array<Hashes> n1_subs;
	n0_subs.Reserve(n0.sub.GetCount());
	n1_subs.Reserve(n1.sub.GetCount());
	for (MetaNode& s0 : n0.sub)
		n0_subs.Add().Set(&s0, s0.GetCommonHash());
	for (const MetaNode& s1 : n1.sub)
		n1_subs.Add().Set(const_cast<MetaNode*>(&s1), s1.GetCommonHash());
	
	// Match common hashes starting from the beginning
	int c = min(n0_subs.GetCount(), n1_subs.GetCount());
	for(int i = 0; i < c; i++) {
		auto& s0 = n0_subs[i];
		auto& s1 = n1_subs[i];
		if (s0.common_hash == s1.common_hash) {
			s0.match = s1.n;
			s1.match = s0.n;
			s0.ready = true;
			s1.ready = true;
		}
		else
			break;
	}
	// Match common hashes starting from the end
	for(int i = 0; i < c; i++) {
		int j0 = n0_subs.GetCount()-1-i;
		int j1 = n1_subs.GetCount()-1-i;
		auto& s0 = n0_subs[j0];
		auto& s1 = n1_subs[j1];
		if (s0.match != 0 || s1.match != 0)
			break;
		if (s0.common_hash == s1.common_hash) {
			s0.match = s1.n;
			s1.match = s0.n;
			s0.ready = true;
			s1.ready = true;
		}
		else
			break;
	}
	
	Vector<int> rmlist;
	for(int i = 0; i < n0_subs.GetCount(); i++) {
		auto& s0 = n0_subs[i];
		if (s0.match != 0) continue;
		for(int j = 0; j < n1_subs.GetCount(); j++) {
			auto& s1 = n1_subs[j];
			if (s1.match != 0) continue;
			if (s0.common_hash == s1.common_hash) {
				s0.match = s1.n;
				s1.match = s0.n;
				s0.ready = true;
				s1.ready = true;
				break;
			}
		}
		if (!s0.match) {
			if (mode == MERGEMODE_BAIL_OUT)
				return false;
			if (mode == MERGEMODE_OVERWRITE_OLD)
				rmlist.Add(i);
			if (mode == MERGEMODE_KEEP_OLD)
				s0.ready = true;
		}
	}
	
	if (mode == MERGEMODE_OVERWRITE_OLD) {
		if (rmlist.GetCount()) {
			n0_subs.Remove(rmlist);
			n0.sub.Remove(rmlist);
			rmlist.Clear();
		}
		for(auto& s0 : n0_subs) {ASSERT(s0.ready);}
	}
	
	if (mode == MERGEMODE_OVERWRITE_OLD && n0.sub.IsEmpty()) {
		ASSERT(n0_subs.IsEmpty());
		for(int i = 0; i < n1_subs.GetCount(); i++) {
			auto& s1 = n1_subs[i];
			auto& s0 = n0_subs.Add();
			s0.match = s1.n;
			s0.ready = true;
			s1.ready = true;
		}
	}
	else if (mode == MERGEMODE_OVERWRITE_OLD && !n0.sub.IsEmpty()) {
		for(int i = 0; i < n0_subs.GetCount(); i++) {ASSERT(n0_subs[i].ready);}
		int insert_lost_pos = -1;
		bool insert_from_begin;
		while (true) {
			int added_count = 0;
			for(int i = 1; i < n1_subs.GetCount(); i++) {
				auto& s1 = n1_subs[i];
				if (s1.ready) continue;
				auto& s1a = n1_subs[i-1];
				if (!s1a.ready) continue;
				bool added = false;
				for(int j = 0; j < n0_subs.GetCount(); j++) {
					if (n0_subs[j].match == s1a.n) {
						auto& s0 = n0_subs.Insert(j+1);
						s0.match = s1.n;
						s0.ready = true;
						s1.ready = true;
						added = true;
						insert_lost_pos = j+2;
						insert_from_begin = true;
						break;
					}
				}
				if (added)
					added_count++;
			}
			for (int i = n1_subs.GetCount()-2; i >= 0; i--) {
				auto& s1 = n1_subs[i];
				if (s1.ready) continue;
				auto& s1a = n1_subs[i+1];
				if (!s1a.ready) continue;
				bool added = false;
				for(int j = 0; j < n0_subs.GetCount(); j++) {
					if (n0_subs[j].match == s1a.n) {
						auto& s0 = n0_subs.Insert(j);
						s0.match = s1.n;
						s0.ready = true;
						s1.ready = true;
						added = true;
						insert_lost_pos = j;
						insert_from_begin = false;
						break;
					}
				}
				if (added)
					added_count++;
			}
			int unready_count = 0;
			for(int i = 0; i < n1_subs.GetCount(); i++)
				if (!n1_subs[i].ready)
					unready_count++;
			if (!unready_count)
				break;
			if (added_count)
				continue;
			if (insert_lost_pos < 0) {
				ASSERT_(0, "unexpected internal error"); // this shouldn't ever happen
				insert_lost_pos = n0_subs.GetCount();
				insert_from_begin = true;
			}
			for(int i = 1; i < n1_subs.GetCount(); i++) {
				auto& s1 = n1_subs[i];
				if (s1.ready) continue;
				auto& s0 = n0_subs.Insert(insert_lost_pos);
				s0.match = s1.n;
				s0.ready = true;
				s1.ready = true;
				if (insert_from_begin)
					insert_lost_pos++;
			}
		}
	}
	
	if (mode == MERGEMODE_BAIL_OUT || mode == MERGEMODE_OVERWRITE_OLD) {
		for(int i = 0; i < n0_subs.GetCount(); i++)
			if (!n0_subs[i].ready)
				return false;
	}
	if (mode == MERGEMODE_BAIL_OUT || mode == MERGEMODE_KEEP_OLD) {
		for(int i = 0; i < n1_subs.GetCount(); i++)
			if (!n1_subs[i].ready)
				return false;
	}
	
	
	Array<MetaNode> new_sub;
	int pos = 0;
	for(int i = 0; i < n0_subs.GetCount(); i++) {
		auto& s0 = n0_subs[i];
		if (!s0.n)
			s0.n = &n0.sub.Insert(pos);
		else {
			ASSERT(s0.n == &n0.sub[pos]);
		}
		auto& n0 = *s0.n;
		auto& n1 = *s0.match;
		if (n0.GetTotalHash() == n1.GetTotalHash()) {
			// pass
		}
		else {
			if (MERGEMODE_KEEP_OLD && !n0.sub.IsEmpty() && n1.sub.IsEmpty()) {
				// pass
			}
			else if (MERGEMODE_KEEP_OLD && n0.sub.IsEmpty() && !n1.sub.IsEmpty()) {
				n0.CopySubFrom(n1);
			}
			else if (MERGEMODE_OVERWRITE_OLD && (n0.sub.IsEmpty() || n1.sub.IsEmpty())) {
				n0.CopyFrom(n1);
			}
			else {
				hash_t n0_total_hash = n0.GetTotalHash();
				hash_t n1_total_hash = n1.GetTotalHash();
				if (n0_total_hash != n1_total_hash) {
					Vector<String> find_diffs;
					n0.FindDifferences(n1, find_diffs);
					DUMPC(find_diffs);
					scope.Add(s0.n);
					if (!MergeVisitPartMatching(scope, *s0.match, mode))
						return false;
					scope.Pop();
				}
			}
		}
		pos++;
	}
	
	return true;
}

void MetaEnvironment::MergeVisitPost(MetaNode& n) {
	RefreshNodePtrs(n);
	for (auto& s : n.sub)
		MergeVisitPost(s);
}

bool MetaEnvironment::MergeNode(MetaNode& root, const MetaNode& other, MergeMode mode) {
	Vector<MetaNode*> scope;
	scope << &root;
	return MergeVisit(scope, other, mode);
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

MetaNode::~MetaNode() {
	#if DEBUG_METANODE_DTOR
	if (trace_kill)
		Panic("trace-kill");
	#endif
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
			MetaNode& n1 = other.Add();
			n0.CopyPkgTo(n1, pkg_id);
		}
	}
}

void MetaNode::CopyPkgTo(MetaNode& other, int pkg_id, int file_id) const {
	other.CopyFieldsFrom(*this);
	for (const auto& n0 : sub) {
		if (n0.HasPkgFileDeep(pkg_id, file_id)) {
			MetaNode& n1 = other.Add();
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

void MetaEnvironment::RefreshNodePtrs(MetaNode& n) {
	/*if (n.kind == CXCursor_ClassTemplate) {
		LOG(n.GetTreeString());
	}*/
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
	if (n.type_hash) {
		auto& vec = this->type_hash_nodes.GetAdd(n.type_hash);
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

void MetaNode::Destroy() {
	if (!owner) return;
	int i = 0;
	for (MetaNode& n : owner->sub) {
		if (&n == this) {
			owner->sub.Remove(i);
			break;
		}
		i++;
	}
}

void MetaNode::Assign(MetaNode* owner, const ClangNode& n) {
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

MetaNode& MetaNode::GetAdd(String id, String type, int kind) {
	for (MetaNode& s : sub)
		if (s.kind == kind && s.id == id && s.type == type)
			return s;
	MetaNode& s = sub.Add();
	s.owner = this;
	s.id = id;
	s.type = type;
	s.kind = kind;
	return s;
}

MetaNode& MetaNode::Add(const MetaNode& n) {
	MetaNode& s = sub.Add();
	s.owner = this;
	s.CopySubFrom(n);
	s.CopyFieldsFrom(n);
	return s;
}

MetaNode& MetaNode::Add(MetaNode* n) {
	MetaNode& s = sub.Add(n);
	s.owner = this;
	return s;
}

MetaNode& MetaNode::Add() {
	MetaNode& s = sub.Add();
	s.owner = this;
	return s;
}

void MetaNode::CopyFrom(const MetaNode& n) {
	CopySubFrom(n);
	CopyFieldsFrom(n);
}

void MetaNode::CopySubFrom(const MetaNode& n) {
	int c = n.sub.GetCount();
	sub.SetCount(c);
	for(int i = 0; i < c; i++)
		sub[i].Assign(this, n.sub[i]);
}

String MetaNode::GetKindString() const {return GetKindString(kind);}

String MetaNode::GetKindString(int kind) {
	if (kind < METAKIND_BEGIN)
		return GetCursorKindName((CXCursorKind)kind);
	else if (kind == METAKIND_COMMENT)
		return "MetaKind: Comment";
	else if (kind == METAKIND_ECS_NODE)
		return "MetaKind: ECS-Node";
	else if (kind == METAKIND_ECS_SPACE)
		return "MetaKind: ECS-Space";
	else
		return "Unknown kind: " + IntStr(kind);
}

void MetaNode::FindDifferences(const MetaNode& n, Vector<String>& diffs, int max_diffs) const {
	bool had_diff = false;
	#define CHK_FIELD(x) if (x != n.x) {had_diff = true; diffs.Add("Different " #x ": " + AsString(x) + " vs " + AsString(n.x) + " at " + GetKindString() + ", " + id + " (" + type + ")");}
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
	if (!had_diff) for(int i = 0; i < sub.GetCount(); i++) {
		CHK_FIELD(sub[i].kind);
		CHK_FIELD(sub[i].id);
		if (had_diff)
			break;
		sub[i].FindDifferences(n.sub[i], diffs, max_diffs);
		if (diffs.GetCount() >= max_diffs)
			break;
	}
}

void MetaNode::CopyFieldsFrom(const MetaNode& n) {
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
}

hash_t MetaNode::GetTotalHash() const {
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
		;
	for (const auto& s : sub)
		ch.Put(s.GetTotalHash());
	return ch;
}

hash_t MetaNode::GetCommonHash(bool* total_hash_diffs) const {
	CombineHash ch;
	ASSERT(kind >= 0 && kind < METAKIND_BEGIN); // otherwise not common kind
	ch.Do(kind).Do(id).Do(type);
	for (const auto& s : sub) {
		if (s.kind < 0 || s.kind >= METAKIND_BEGIN) {
			if (total_hash_diffs)
				*total_hash_diffs = true;
			continue;
		}
		ch.Put(s.GetCommonHash());
	}
	return ch;
}

Vector<MetaNode*> MetaNode::FindAllShallow(int kind) {
	Vector<MetaNode*> vec;
	for (auto& s : sub)
		if (s.kind == kind)
			vec << &s;
	return vec;
}

void MetaNode::FindAllDeep(int kind, Vector<MetaNode*>& out) {
	if (this->kind == kind)
		out << this;
	for (auto& s : sub)
		s.FindAllDeep(kind, out);
}

void MetaNode::FindAllDeep(int kind, Vector<const MetaNode*>& out) const {
	if (this->kind == kind)
		out << this;
	for (const auto& s : sub)
		s.FindAllDeep(kind, out);
}

Vector<const MetaNode*> MetaNode::FindAllShallow(int kind) const {
	Vector<const MetaNode*> vec;
	for (const auto& s : sub)
		if (s.kind == kind)
			vec << &s;
	return vec;
}

bool MetaNode::IsStructKind() const {
	return	kind == CXCursor_StructDecl &&
			kind == CXCursor_ClassDecl &&
			kind == CXCursor_ClassTemplate &&
			kind == CXCursor_ClassTemplatePartialSpecialization;
}

int MetaNode::GetRegularCount() const {
	int c = 0;
	for (const auto& s : sub)
		if (s.kind >= 0 && s.kind < METAKIND_BEGIN)
			c++;
	return c;
}

String MetaNode::GetBasesString() const {
	String s;
	Vector<const MetaNode*> bases = FindAllShallow(CXCursor_CXXBaseSpecifier);
	for (const MetaNode* n : bases) {
		if (!s.IsEmpty()) s.Cat(", ");
		s << n->id << " (" << n->type << ")";
	}
	return s;
}

String MetaNode::GetNestString() const {
	if (owner)
		return owner->id;
	return String();
}

bool MetaNode::OwnerRecursive(const MetaNode& n) const {
	MetaNode* o = this->owner;
	while (o) {
		if (o == &n)
			return true;
		o = o->owner;
	}
	return false;
}

bool MetaNode::ContainsDeep(const MetaNode& n) const {
	#if 1
	return n.OwnerRecursive(*this) || &n == this;
	#else
	if (this == &n)
		return true;
	for (const auto& s : sub)
		if (s.ContainsDeep(n))
			return true;
	return false;
	#endif
}

void MetaNode::RemoveAllShallow(int kind) {
	Vector<int> rmlist;
	int i = 0;
	for (auto& s : sub) {
		if (s.kind == kind)
			rmlist << i;
		i++;
	}
	if (!rmlist.IsEmpty())
		sub.Remove(rmlist);
}

void MetaNode::RemoveAllDeep(int kind) {
	RemoveAllShallow(kind);
	for (auto& s : sub)
		s.RemoveAllDeep(kind);
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

MetaNode* MetaEnvironment::FindDeclaration(const MetaNode& n) {
	if (!n.filepos_hash) return 0;
	int i = filepos_nodes.Find(n.filepos_hash);
	if (i < 0) return 0;
	const auto& vec = filepos_nodes[i];
	for (const auto& ptr : vec) {
		if (!ptr) continue;
		MetaNode& p = *ptr;
		if (p.is_definition/* || p.IsClassTemplateDefinition()*/)
			return &p;
	}
	return 0;
}

Vector<MetaNode*> MetaEnvironment::FindDeclarationsDeep(const MetaNode& n) {
	Vector<MetaNode*> v;
	if (n.kind == CXCursor_CXXBaseSpecifier) {
		for (const auto& s : n.sub) {
			MetaNode* d = FindDeclaration(s);
			if (d) v.Add(d);
		}
		return v;
	}
	else Panic("TODO");
	return v;
}

MetaNode* MetaEnvironment::FindTypeDeclaration(unsigned type_hash) {
	if (!type_hash) return 0;
	int i = type_hash_nodes.Find(type_hash);
	if (i < 0) return 0;
	const auto& vec = type_hash_nodes[i];
	for (const auto& ptr : vec) {
		if (!ptr) continue;
		MetaNode& p = *ptr;
		if (p.is_definition/* || p.IsClassTemplateDefinition()*/)
			return &p;
	}
	return 0;
}


END_UPP_NAMESPACE

#include "Meta.h"
#ifdef flagAI
#include <AICore/AICore.h>
#endif

NAMESPACE_UPP

String (*GetCursorKindNamePtr)(CXCursorKind);

#if 0
String FetchString(CXString cs)
{
	String result = clang_getCString(cs);
	clang_disposeString(cs);
	return result;
}

String GetCursorKindName(CXCursorKind cursorKind)
{
	return FetchString(clang_getCursorKindSpelling(cursorKind));
}
#endif

#define DO_TEMP_CHECK 0

int CreateTempCheck(int src) {
	static int counter = 0;
	#if DO_TEMP_CHECK
	LOG("CreateTemp " << src << ", id=" << counter);
	if (counter == 25) {
		LOG("__BREAK__");
	}
	#endif
	return counter++;
}

void ClearTempCheck(int id) {
	#if DO_TEMP_CHECK
	LOG("ClearTemp id=" << id);
	#endif
}

#undef DO_TEMP_CHECK


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
	
	MountManager& mm = MountManager::System();
	mm.Mount(INTERNAL_ROOT_FILE("prj"), this, "MetaEnvironment");
}

hash_t MetaEnvironment::NewSerial() {
	serial_lock.Enter();
	hash_t new_hash = ++serial_counter;
	serial_lock.Leave();
	return new_hash;
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
	s << "(" << HexStrPtr(this) << ")";
	s << "\n";
	if (ext) {
		s.Cat('\t', depth+1);
		int fac_i = MetaExtFactory::FindKindFactory(kind);
		s << "EXT:";
		if (fac_i >= 0) {
			const auto& fac = MetaExtFactory::List()[fac_i];
			s << " " << ClassPathTop(fac.name);
			if (!fac.ctrl_name.IsEmpty())
				s << " (" << fac.ctrl_name << ")";
		}
		else {
			TypeCls type = ext->GetTypeCls();
			s << " " << type.GetName();
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

int MetaNode::Find(const String& id) const
{
	int i = 0;
	for(const MetaNode& n : sub) {
		if (n.id == id)
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

MetaNode* MetaNode::Detach(MetaNode* n) {
	for(int i = 0; i < sub.GetCount(); i++) {
		if (&sub[i] == n)
			return sub.Detach(i);
	}
	return 0;
}

MetaNode* MetaNode::Detach(int i) {
	if (i >= 0 && i < sub.GetCount())
		return sub.Detach(i);
	return 0;
}

void MetaNode::Remove(MetaNode* n) {
	int i = 0;
	for (auto& s : sub) {
		if (&s == n) {
			sub.Remove(i);
			break;
		}
		i++;
	}
}

void MetaNode::Remove(int i) {
	if (i >= 0 && i < sub.GetCount())
		sub.Remove(i);
}

MetaNode* MetaNode::FindPath(const VfsPath& path) {
	MetaNode* n = this;
	for(const String& part : path.Parts()) {
		int i = n->Find(part);
		if (i < 0)
			return 0;
		n = &n->sub[i];
	}
	return n;
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
	if(kind >= 0 && kind <= CXCursor_OverloadCandidate) {
		if (GetCursorKindNamePtr)
			return GetCursorKindNamePtr((CXCursorKind)kind);
		else
			return "Kind(" + IntStr(kind) + ")";
	}
	switch (kind) {
		#define DATASET_ITEM(type, name, kind, group, desc) case kind: return desc;
		DATASET_LIST
		#undef DATASET_ITEM
	default:
		return IntStr(kind);
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

void MetaNode::Visit(Vis& v) {
	#define Do(x) (#x,x)
	v.Ver(1)
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
	v("has_ext", has_ext);
	if (has_ext) {
		if (v.IsLoading()) ext = MetaExtFactory::CreateKind(kind, *this);
		if (ext)
			v("ext",*ext, VISIT_NODE);
	}
	v
		("sub", sub, VISIT_VECTOR)
		;
	#undef Do
	if (v.IsLoading())
		FixParent();
	
	#if 1
	if (v.IsLoading())
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

bool MetaNode::IsOwnerDeep(MetaNodeExt& n) const {
	const MetaNode* p = this;
	while (p) {
		if (p->ext && (&*p->ext) == &n)
			return true;
		p = p->owner;
	}
	return false;
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

Vector<MetaNode*> MetaNode::FindAll(TypeCls type)
{
	Vector<MetaNode*> vec;
	for(auto& s : sub)
		if(s.ext && s.ext->GetTypeCls() == type)
			vec << &s;
	return vec;
}

Vector<MetaNode*> MetaNode::FindAllShallow(int kind)
{
	Vector<MetaNode*> vec;
	for(auto& s : sub)
		if(s.kind == kind)
			vec << &s;
	return vec;
}

MetaNode* MetaNode::FindDeep(TypeCls type)
{
	if (ext && ext->GetTypeCls() == type)
		return this;
	for (MetaNode& n : sub)
		if (auto r = n.FindDeep(type))
			return r;
	return 0;
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

VfsPath MetaNode::GetPath() const {
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
	if (i == 1) return VfsPath();
	ptrs[i] = 0;
	const MetaNode** iter = ptrs+i-2;
	const MetaNode** end = ptrs-1;
	Vector<Value> path;
	path.Reserve(i);
	while (iter != end) {
		path.Add((*iter)->id);
		iter--;
	}
	return path;
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
		TODO;
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

hash_t MetaEnvironment::RealizeTypePath(const String& path)
{
	hash_t h = path.GetHashValue();
	types.GetAdd(h).seen_type = path;
	return h;
}

bool MetaEnvironment::GetFiles(const VfsPath& rel_path, Vector<VfsItem>& items) {
	// Note: appends to items
	MetaNode* n = &this->root;
	const auto& parts = rel_path.Parts();
	for(int i = 0; i < parts.GetCount(); i++) {
		const String& part = parts[i];
		int j = n->Find(part);
		if (j < 0)
			return false;
		n = &n->sub[j];
	}
	for(int i = 0; i < n->sub.GetCount(); i++) {
		MetaNode& n0 = n->sub[i];
		VfsItem& it = items.Add();
		it.name = n0.id;
		it.type = VFS_DIRECTORY;
		if (!n0.type.IsEmpty())
			it.type_str = n0.type;
		else if (n0.ext)
			it.type_str = n0.ext->GetName();
	}
	return true;
}

VfsItemType MetaEnvironment::CheckItem(const VfsPath& rel_path) {
	MetaNode* n = &this->root;
	const auto& parts = rel_path.Parts();
	for(int i = 0; i < parts.GetCount(); i++) {
		const String& part = parts[i];
		int j = n->Find(part);
		if (j < 0)
			return VFS_NULL;
		n = &n->sub[j];
	}
	return VFS_DIRECTORY;
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
	Vis read(s);
	const_cast<MetaNodeExt&>(e).Visit(read); // reading
	s.SetLoading();
	s.Seek(0);
	Vis write(s);
	Visit(write);
}

bool MetaNodeExt::operator==(const MetaNodeExt& e) const {
	return GetHashValue() == e.GetHashValue();
}

hash_t MetaNodeExt::GetHashValue() const {
	Vis vis(1);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
	return vis.hash;
}

int MetaNodeExt::GetKind() const {
	return node.kind;
}

void MetaNodeExt::Serialize(Stream& s){
	Vis vis(s);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
}

void MetaNodeExt::Jsonize(JsonIO& json){
	Vis vis(json);
	const_cast<MetaNodeExt*>(this)->Visit(vis);
}

END_UPP_NAMESPACE

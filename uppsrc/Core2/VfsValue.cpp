#include "Core.h"

#if 0
#ifdef flagAI
#include <AICore/AICore.h>
#endif
#include <Eon/Eon.h>
#endif

NAMESPACE_UPP

String (*GetCursorKindNamePtr)(int);
String (*VfsValue_GetBasesString)(const VfsValue& v);


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













bool AstValue::IsNullInstance() const {
	return	kind < 0 &&
			type.IsEmpty() &&
			begin == Null &&
			end == Null &&
			filepos_hash == 0 &&
			is_ref == false &&
			is_definition == false &&
			is_disabled == false;
}

void AstValue::Serialize(Stream& s) {
	s / kind / type % begin % end / filepos_hash;
	s % is_ref % is_definition % is_disabled;
}

void AstValue::Xmlize(XmlIO& xml) {
	xml ("kind", kind)
		("type", type)
		("begin", begin)
		("end", end)
		("filepos_hash", (int64&)filepos_hash)
		("is_ref", is_ref)
		("is_definition", is_definition)
		("is_disabled", is_disabled)
		;
}

void AstValue::Jsonize(JsonIO& io) {
	io  ("kind", kind)
		("type", type)
		("begin", begin)
		("end", end)
		("filepos_hash", (int64&)filepos_hash)
		("is_ref", is_ref)
		("is_definition", is_definition)
		("is_disabled", is_disabled)
		;
}

hash_t AstValue::GetHashValue() const {
	CombineHash c;
	c	.Do(kind)
		.Do(type)
		.Do(begin)
		.Do(end)
		.Do(filepos_hash)
		.Do(is_ref)
		.Do(is_definition)
		.Do(is_disabled)
		;
	return c;
}

String AstValue::ToString() const {
	return StoreAsJson(*this);
}

bool AstValue::operator==(const AstValue& v) const {
	return	kind == v.kind &&
			type == v.type &&
			begin == v.begin &&
			end == v.end &&
			filepos_hash == v.filepos_hash &&
			is_ref == v.is_ref &&
			is_definition == v.is_definition &&
			is_disabled == v.is_disabled;
}

int AstValue::Compare(const AstValue& v) const {
	return	kind - v.kind;
}

int AstValue::PolyCompare(const Value& v) const {
	return	kind - (int)v;
}













#if 0
VfsSrcFile& MetaEnvironment::ResolveFileInfo(const String& includes, String path)
{
	return ResolveFile(includes, path).RealizePath(includes, path);
}
#endif






MetaEnvironment& MetaEnv() { return Single<MetaEnvironment>(); }

MetaEnvironment::MetaEnvironment() {
	AstValue& ast = root;
	ast.kind = 22; //CXCursor_Namespace;
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

bool MetaEnvironment::MergeVisit(Vector<VfsValue*>& scope, const VfsValue& n1, MergeMode mode)
{
	VfsValue& n0 = *scope.Top();
	AstValue* a0 = n0;
	const AstValue* a1 = n1;
	ASSERT(a0 && a1 && a0->kind == a1->kind && n0.id == n1.id);
	TODO
	#if 0
	if(a0 && a1 && IsMergeable((CXCursorKind)a0->kind)) {
		for(const VfsValue& sub1 : n1.sub) {
			int i = n0.Find(sub1.kind, sub1.id);
			if(i < 0) {
				auto& n = n0.Add(sub1);
				MergeVisitPost(n);
			}
			else {
				VfsValue& sub0 = n0.sub[i];
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
	#endif
	return true;
}

bool MetaEnvironment::MergeVisitPartMatching(Vector<VfsValue*>& scope, const VfsValue& n1,
                                             MergeMode mode)
{
	VfsValue& n0 = *scope.Top();
	struct Hashes {
		const VfsValue* n = 0;
		const VfsValue* match = 0;
		bool source_kind;
		hash_t hash;
		bool ready = false;
		dword serial;
		void Set(const VfsValue* mn, bool is_source_kind, hash_t c)
		{
			n = mn;
			hash = c;
			source_kind = is_source_kind;
		}
	};
	AstValue* p0 = n0;
	const AstValue* p1 = n1;
	ASSERT(p0 && p1); // unverified (part of cleanup)
	if (!p0 || !p1)
		return false;
	AstValue& a0 = *p0;
	const AstValue& a1 = *p1;

	if(n0.serial == n1.serial) {
		n0.Chk();
		return true;
	}
	ASSERT(n0.serial && n1.serial);
	const VfsValue& pri = mode == MERGEMODE_OVERWRITE_OLD ? (n0.serial > n1.serial ? n0 : n1)
	                                                      : (n0.serial < n1.serial ? n0 : n1);
	const VfsValue& sec = &pri == &n0 ? n1 : n0;
	
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
	for(const VfsValue& pri : pri.sub) {
		if(pri.IsAstValue())
			pri_subs.Add().Set(&pri, true, pri.AstGetSourceHash());
		else
			pri_subs.Add().Set(&pri, false, pri.GetTotalHash());
	}
	for(const VfsValue& sec : sec.sub) {
		if(sec.IsAstValue())
			sec_subs.Add().Set(&sec, true, sec.AstGetSourceHash());
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
				VfsValue& s0 = const_cast<VfsValue&>(*pri.n);
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
				VfsValue& s0 = const_cast<VfsValue&>(*sec.n);
				AstValue* sa0 = s0;
				ASSERT(sa0);
				hash_t old_sub_serial = s0.serial;
				scope.Add(&s0);
				bool succ = MergeVisitPartMatching(scope, *sec.match, mode);
				scope.Remove(scope.GetCount() - 1);
				if(old_sub_serial != n0.serial && n0.serial == old_serial)
					n0.serial = NewSerial();
				if(!succ)
					return false;
			}
		}
	}
	
	n0.Chk();
	return true;
}

void MetaEnvironment::MergeVisitPost(VfsValue& n)
{
	RefreshNodePtrs(n);
	for(auto& s : n.sub)
		MergeVisitPost(s);
}

bool MetaEnvironment::MergeValue(VfsValue& root, const VfsValue& other, MergeMode mode)
{
	Vector<VfsValue*> scope;
	scope << &root;
	return MergeVisit(scope, other, mode);
}

VfsValue::~VfsValue()
{
#if DEBUG_METANODE_DTOR
	if(trace_kill)
		Panic("trace-kill");
#endif
}

String VfsValue::ToString() const {
	if (ext) return ext->ToString();
	return id;
}

void VfsValue::ClearExtDeep() {
	for (auto& s : sub)
		s.ClearExtDeep();
	if (ext)
		ext.Clear();
}

void VfsValue::PointPkgTo(VfsValueSubset& other, int pkg_id)
{
	other.n = this;
	for(auto& n0 : sub) {
		if(n0.HasPkgDeep(pkg_id)) {
			VfsValueSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id);
		}
	}
}

void VfsValue::PointPkgTo(VfsValueSubset& other, int pkg_id, int file_id)
{
	other.n = this;
	for(auto& n0 : sub) {
		if(n0.HasPkgFileDeep(pkg_id, file_id)) {
			VfsValueSubset& n1 = other.sub.Add();
			n0.PointPkgTo(n1, pkg_id, file_id);
		}
	}
}

void VfsValue::CopyPkgTo(VfsValue& other, int pkg_id) const
{
	other.CopyFieldsFrom(*this, true);
	for(const auto& n0 : sub) {
		if(n0.HasPkgDeep(pkg_id)) {
			VfsValue& n1 = other.Add();
			n0.CopyPkgTo(n1, pkg_id);
		}
	}
}

void VfsValue::CopyPkgTo(VfsValue& other, int pkg_id, int file_id) const
{
	other.CopyFieldsFrom(*this, true);
	for(const auto& n0 : sub) {
		if(n0.HasPkgFileDeep(pkg_id, file_id)) {
			VfsValue& n1 = other.Add();
			n0.CopyPkgTo(n1, pkg_id, file_id);
		}
	}
}

bool VfsValue::HasPkgDeep(int pkg_id) const
{
	if(this->pkg == pkg_id)
		return true;
	for(const auto& n : sub)
		if(n.HasPkgDeep(pkg_id))
			return true;
	return false;
}

bool VfsValue::HasPkgFileDeep(int pkg_id, int file_id) const
{
	if(this->pkg == pkg_id && this->file == file_id)
		return true;
	for(const auto& n : sub)
		if(n.HasPkgFileDeep(pkg_id, file_id))
			return true;
	return false;
}

void VfsValue::SetPkgDeep(int pkg_id)
{
	this->pkg = pkg_id;
	for(auto& n : sub)
		n.SetPkgDeep(pkg_id);
}

void VfsValue::SetFileDeep(int file_id)
{
	this->file = file_id;
	for(auto& n : sub)
		n.SetFileDeep(file_id);
}

void VfsValue::SetPkgFileDeep(int pkg_id, int file_id)
{
	this->pkg = pkg_id;
	this->file = file_id;
	for(auto& n : sub)
		n.SetPkgFileDeep(pkg_id, file_id);
}

void VfsValue::SetTempDeep()
{
	only_temporary = true;
	for(auto& n : sub)
		n.SetTempDeep();
}

void MetaEnvironment::RefreshNodePtrs(VfsValue& n)
{
	/*if (n.kind == CXCursor_ClassTemplate) {
	    //LOG(n.GetTreeString());
	}*/
	AstValue* p = n;
	ASSERT(p); // unverified (part of cleanup)
	if (!p)
		return;
	AstValue& a = *p;
	
	ASSERT(!n.only_temporary);
	if(a.filepos_hash != 0) {
		auto& vec = this->filepos.GetAdd(a.filepos_hash).hash_nodes;
		bool found = false;
		bool found_zero = false;
		for(auto& p : vec) {
			VfsValue* ptr = &*p;
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
		auto& vec = this->filepos.GetAdd(a.filepos_hash).hash_nodes;
		bool found = false;
		bool found_zero = false;
		for(auto& p : vec) {
			VfsValue* ptr = &*p;
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

String VfsValue::GetTypeString() const
{
	if (ext)
		return ext->GetTypeCls().GetName();
	const AstValue* a = *this;
	if (a && a->type.GetCount())
		return a->type;
	else if (a && a->kind > 0)
		return AstGetKindString(a->kind);
	if (type_hash) {
		int i = VfsValueExtFactory::FindTypeHashFactory(type_hash);
		if (i >= 0)
			return VfsValueExtFactory::List()[i].name;
	}
	return String();
}

String VfsValue::GetTreeString(int depth) const
{
	String s;
	s.Cat('\t', depth);
	if(1)
		s << IntStr(pkg) << ":" << IntStr(file) << ": ";
	if (!value.IsNull()) {
		s << value.GetTypeName();
		dword type = value.GetType();
		if (type == INT_V || type == INT64_V || type == DOUBLE_V)
			s << "(" << value.ToString() << ")";
		if (type == STRING_V) {
			String vs = value.ToString();
			if (vs.GetCount() > 13)
				vs = vs.Left(10) + "...";
			s << "(" << vs << ")";
		}
	}
	if(!id.IsEmpty())
		s << ": " << id;
	s << "(" << HexStrPtr(this) << ")";
	s << "\n";
	if (ext) {
		ASSERT(type_hash != 0);
		s.Cat('\t', depth+1);
		int fac_i = VfsValueExtFactory::FindTypeHashFactory(type_hash);
		s << "EXT:";
		if (fac_i >= 0) {
			const auto& fac = VfsValueExtFactory::List()[fac_i];
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

int VfsValue::AstFind(int kind, const String& id) const
{
	int i = 0;
	for(const VfsValue& n : sub) {
		if(n.type_hash == type_hash && n.id == id)
			return i;
		i++;
	}
	return -1;
}

int VfsValue::Find(const String& id) const
{
	int i = 0;
	for(const VfsValue& n : sub) {
		if (n.id == id)
			return i;
		i++;
	}
	return -1;
}

void VfsValue::Destroy()
{
	if(!owner)
		return;
	int i = 0;
	for(VfsValue& n : owner->sub) {
		if(&n == this) {
			owner->sub.Remove(i);
			break;
		}
		i++;
	}
}

VfsValue& VfsValue::AstGetAdd(String id, String type, int kind)
{
	for(VfsValue& s : sub) {
		if (s.id != id)
			continue;
		AstValue* a = s;
		if(a && a->kind == kind && a->type == type)
			return s;
	}
	VfsValue& s = sub.Add();
	s.owner = this;
	s.id = id;
	s.serial = MetaEnv().NewSerial();
	s.file = this->file;
	s.pkg = this->pkg;
	
	AstValue& a = s;
	a.type = type;
	a.kind = kind;
	ASSERT(kind > 0 && kind < 1000); // CXCursor range
	
	#if 0
	this->serial = MetaEnv().NewSerial();
	if (kind >= METAKIND_EXTENSION_BEGIN && kind <= METAKIND_EXTENSION_END) {
		int i = VfsValueExtFactory::FindKindFactory(kind);
		if (i >= 0) {
			s.ext = VfsValueExtFactory::List()[i].new_fn(s);
			s.type_hash = s.ext->GetTypeHash();
		}
	}
	#endif
	
	return s;
}

VfsValue& VfsValue::GetAdd(String id, hash_t type_hash) {
	for (auto& s : sub)
		if (s.id == id && s.type_hash == type_hash)
			return s;
	return Add(id, 0);
}

void VfsValue::UninitializeDeep()
{
	for (auto& s : sub)
		s.UninitializeDeep();
	if (ext) {
		ext->UninitializeDeep();
		/*ext->Uninitialize();
		ext->SetInitialized(false);*/
		ASSERT(!ext->IsInitialized());
	}
}

void VfsValue::StopDeep()
{
	for (auto& s : sub)
		s.StopDeep();
	if (ext)
		ext->Stop();
}

void VfsValue::ClearDependenciesDeep()
{
	for (auto& s : sub)
		s.ClearDependenciesDeep();
	if (ext)
		ext->ClearDependencies();
}

VfsValue& VfsValue::Add(const VfsValue& n)
{
	VfsValue& s = sub.Add();
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

VfsValue& VfsValue::Add(VfsValue* n)
{
	VfsValue& s = sub.Add(n);
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

VfsValue& VfsValue::Add(String id, hash_t h)
{
	VfsValue& s = sub.Add();
	s.id = id;
	s.owner = this;
	s.file = this->file;
	s.pkg = this->pkg;
	s.serial = MetaEnv().NewSerial();
	this->serial = MetaEnv().NewSerial();
	s.Chk();
	if (h) {
		s.ext = VfsValueExtFactory::Create(h, s);
		if (s.ext)
			s.type_hash = h;
	}
	return s;
}

VfsValue& VfsValue::AstAdd(int kind, String id)
{
	VfsValue& s = Add();
	s.id = id;
	
	AstValue& a = s;
	a.kind = kind;
	ASSERT(kind > 0 && kind < 1000);
	
	s.Chk();
	return s;
}

VfsValue* VfsValue::Detach(VfsValue* n) {
	for(int i = 0; i < sub.GetCount(); i++) {
		if (&sub[i] == n)
			return sub.Detach(i);
	}
	return 0;
}

VfsValue* VfsValue::Detach(int i) {
	if (i >= 0 && i < sub.GetCount())
		return sub.Detach(i);
	return 0;
}

void VfsValue::Remove(VfsValue* n) {
	int i = 0;
	for (auto& s : sub) {
		if (&s == n) {
			sub.Remove(i);
			break;
		}
		i++;
	}
}

void VfsValue::Remove(int i) {
	if (i >= 0 && i < sub.GetCount())
		sub.Remove(i);
}

VfsValue* VfsValue::FindPath(const VfsPath& path) {
	VfsValue* n = this;
	for(const String& part : path.Parts()) {
		int i = n->Find(part);
		if (i < 0)
			return 0;
		n = &n->sub[i];
	}
	return n;
}

void VfsValue::CopyFrom(const VfsValue& n)
{
	CopySubFrom(n);
	CopyFieldsFrom(n);
}

void VfsValue::CopySubFrom(const VfsValue& n)
{
	int c = n.sub.GetCount();
	sub.SetCount(c);
	for(int i = 0; i < c; i++)
		sub[i].Assign(this, n.sub[i]);
}

String VfsValue::AstGetKindString() const {
	const AstValue* ast = *this;
	ASSERT(ast);
	if (!ast) return "not a AstValue";
	if (GetCursorKindNamePtr)
		return GetCursorKindNamePtr(ast->kind);
	else
		return IntStr(ast->kind);
}

String VfsValue::AstGetKindString(int kind)
{
	if (GetCursorKindNamePtr)
		return GetCursorKindNamePtr(kind);
	else
		return IntStr(kind);
}

bool VfsValue::FindDifferences(const VfsValue& n, Vector<String>& diffs, int max_diffs) const
{
	bool had_diff = false;
	#define CHK_FIELD(x)                                                                           \
	if(x != n.x) {                                                                             \
		had_diff = true;                                                                       \
		diffs.Add("Different " #x ": " + AsString(x) + " vs " + AsString(n.x) + " at " +       \
		          ", " + id + " (" + HexStr64(n.type_hash) + ")");                               \
	}
	CHK_FIELD(id);
	CHK_FIELD(type_hash);
	CHK_FIELD(serial);
	CHK_FIELD(file);
	CHK_FIELD(sub.GetCount());
	CHK_FIELD(pkg);
	const AstValue* a0 = *this;
	const AstValue* a1 = n;
	if (a0 && a1) {
	#define CHK_AST_FIELD(x)                                                                           \
		if(a0->x != a1->x) {                                                                             \
			had_diff = true;                                                                       \
			diffs.Add("Different " #x ": " + AsString(a0->x) + " vs " + AsString(a1->x) + " at " +       \
			          AstGetKindString(a0->kind) + ", " + id + " (" + a0->type + ")");                            \
		}
		CHK_AST_FIELD(kind);
		CHK_AST_FIELD(type);
		CHK_AST_FIELD(begin);
		CHK_AST_FIELD(end);
		CHK_AST_FIELD(filepos_hash);
		CHK_AST_FIELD(is_ref);
		CHK_AST_FIELD(is_definition);
		CHK_AST_FIELD(is_disabled);
	}
	else if (!a0 && !a1) {
		CHK_FIELD(value);
	}
	else {
		had_diff = true;
		diffs.Add("Different value type: " + value.GetTypeName() + " vs " + n.value.GetTypeName());
	}
	
	if(!had_diff) {
		for(int i = 0; i < sub.GetCount(); i++) {
			CHK_FIELD(sub[i].id);
			if(had_diff)
				break;
			had_diff = sub[i].FindDifferences(n.sub[i], diffs, max_diffs);
			if(had_diff || diffs.GetCount() >= max_diffs)
				break;
		}
	}
	#undef CHK_FIELD
	#undef CHK_AST_FIELD
	return had_diff;
}

bool VfsValue::IsFieldsSame(const VfsValue& n) const
{
	if ((bool)ext != (bool)n.ext)
		return false;
	
	if (!( id == n.id && type_hash == n.type_hash &&
	       file == n.file && pkg == n.pkg &&
	       serial == n.serial && value.GetType() == n.value.GetType()))
		return false;
	
	const AstValue* a0 = *this;
	const AstValue* a1 = n;
	if (a0 && a1) {
		auto& a = *a0;
		auto& b = *a1;
	    if(!(	a.kind == b.kind && a.type == b.type &&
				a.begin == b.begin && a.end == b.end && a.filepos_hash == b.filepos_hash &&
				a.is_ref == b.is_ref &&
				a.is_definition == b.is_definition && a.is_disabled == b.is_disabled))
			return false;
	}
	
	if (ext && n.ext && !(*ext == *n.ext)) // kind is already checked to be the same, and both has ext
		return false;
	
	return true;
}

void VfsValue::CopyFieldsFrom(const VfsValue& n, bool forced_downgrade)
{
	id = n.id;
	type_hash = n.type_hash;
	file = n.file;
	pkg = n.pkg;
	
	const AstValue* a1 = n;
	if (a1) {
		AstValue& a = *this;
		const AstValue& b = *a1;
		a.kind = b.kind;
		a.type = b.type;
		a.begin = b.begin;
		a.end = b.end;
		a.filepos_hash = b.filepos_hash;
		a.is_ref = b.is_ref;
		a.is_definition = b.is_definition;
		a.is_disabled = b.is_disabled;
	}
	else
		value = n.value;
	
	if (n.ext) {
		ext = VfsValueExtFactory::Clone(*n.ext, *this);
		type_hash = ext->GetTypeHash();
	}
	else
		ext.Clear();
	
	ASSERT(serial <= n.serial || forced_downgrade);
	serial = n.serial;
	Chk();
}

hash_t VfsValue::GetTotalHash() const
{
	CombineHash ch;
	ch	.Do(id)
		.Do(type_hash)
		.Do(serial)
		.Do(file)
		.Do(pkg) // TODO ensure that the hash is not used across sessions. This is not persistent hash because of this field.
	;
	const AstValue* a0 = *this;
	if (a0) {
		const AstValue& a = *a0;
		ch
			.Do(a.kind)
			.Do(a.type)
			.Do(a.begin)
			.Do(a.end)
			.Do(a.filepos_hash)
			.Do(a.is_ref)
			.Do(a.is_definition)
			.Do(a.is_disabled);
	}
	else
		ch.Do(value);
	
	if (ext)
		ch.Put(ext->GetHashValue());
	
	for(const auto& s : sub)
		ch.Put(s.GetTotalHash());
	
	return ch;
}

void VfsValue::Visit(Vis& v) {
	#define Do(x) (#x,x)
	v.Ver(3);
	if (v.file_ver <= 2) {
		AstValue& a = *this;
		v
		(1)	Do(a.kind)
			Do(id)
			Do(a.type)
			Do((int64&)type_hash)
			Do(a.begin)
			Do(a.end)
			Do((int64&)a.filepos_hash)
			Do(file)
			//Do(pkg)
			Do(a.is_ref)
			Do(a.is_definition)
			Do(a.is_disabled)
			Do((int64&)serial)
		(2)	Do(value)
			;
	}
	else {
		v
		(3)	Do(id)
			Do((int64&)type_hash)
			Do(file)
			//Do(pkg)
			Do((int64&)serial)
			;
		
		bool is_ast = (const AstValue*)*this;
		v	Do(is_ast);
		if (v.IsLoading()) {
			if (is_ast) {
				AstValue& a = *this;
				v
					Do(a.kind)
					Do(a.type)
					Do(a.begin)
					Do(a.end)
					Do((int64&)a.filepos_hash)
					Do(a.is_ref)
					Do(a.is_definition)
					Do(a.is_disabled);
			}
			else
				v	Do(value);
		}
	}
	
	if (type_hash) {
		if (v.IsLoading()) {
			ext = VfsValueExtFactory::Create(type_hash, *this);
			type_hash = ext->GetTypeHash();
			if (ext)
				v("ext",*ext, VISIT_NODE);
			else {
				RLOG("VfsValue::Visit: error: could not load type_hash " + HexStr64(type_hash));
			}
		}
		else {
			if (type_hash) {
				v("ext",*ext, VISIT_NODE);
			}
			else {
				ASSERT(!ext);
			}
		}
	}
	else {
		if (v.IsLoading())
			ext.Clear();
		else {
			ASSERT(!ext);
		}
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

void VfsValue::DeepChk() {
	#ifdef flagDEBUG
	Chk();
	for (auto& s : sub)
		s.DeepChk();
	#endif
}

void VfsValue::Chk() {
	#ifdef flagDEBUG
	if (owner && type_hash && type_hash == AsTypeHash<Entity>()) {
		//ASSERT(owner->kind != METAKIND_ECS_SPACE);
		TODO // add context && pkg-env component
		/*
		ASSERT(owner->type_hash != METAKIND_CONTEXT);
		ASSERT(owner->kind != METAKIND_PKG_ENV);
		*/
	}
	#endif
}

bool VfsValue::IsOwnerDeep(VfsValueExt& n) const {
	const VfsValue* p = this;
	while (p) {
		if (p->ext && (&*p->ext) == &n)
			return true;
		p = p->owner;
	}
	return false;
}

int VfsValue::GetCount() const {
	return sub.GetCount();
}

int VfsValue::GetDepth() const {
	int depth = 0;
	VfsValue* n = owner;
	while (n) {
		depth++;
		n = n->owner;
	}
	return depth;
}

hash_t VfsValue::AstGetSourceHash(bool* total_hash_diffs) const
{
	const AstValue* a = *this;
	if (a->kind < 0 || a->kind >= 1000) {
		if(total_hash_diffs)
			*total_hash_diffs = true;
		return 0;
	}
	CombineHash ch;
	ch.Do(a->kind).Do(id).Do(a->type);
	for(const auto& s : sub) {
		bool b = false;
		hash_t h = s.AstGetSourceHash(&b);
		if (b)
			continue;
		ch.Put(h);
	}
	return ch;
}

Vector<VfsValue*> VfsValue::FindAll(TypeCls type)
{
	Vector<VfsValue*> vec;
	for(auto& s : sub)
		if(s.ext && s.ext->GetTypeCls() == type)
			vec << &s;
	return vec;
}

Vector<VfsValue*> VfsValue::FindTypeAllShallow(hash_t type_hash)
{
	Vector<VfsValue*> vec;
	for(auto& s : sub) {
		if (s.type_hash == type_hash)
			vec << &s;
	}
	return vec;
}

Vector<VfsValue*> VfsValue::FindAllShallow(hash_t type_hash)
{
	Vector<VfsValue*> vec;
	for(auto& s : sub) {
		if (s.type_hash == type_hash)
			vec << &s;
	}
	return vec;
}

Vector<VfsValue*> VfsValue::AstFindAllShallow(int kind)
{
	Vector<VfsValue*> vec;
	for(auto& s : sub) {
		const AstValue* a = s;
		if (a && a->kind == kind)
			vec << &s;
	}
	return vec;
}

VfsValue* VfsValue::FindDeep(TypeCls type)
{
	if (ext && ext->GetTypeCls() == type)
		return this;
	for (VfsValue& n : sub)
		if (auto r = n.FindDeep(type))
			return r;
	return 0;
}

void VfsValue::AstFindAllDeep(int kind, Vector<VfsValue*>& out)
{
	const AstValue* a = *this;
	if(a && a->kind == kind)
		out << this;
	for(auto& s : sub)
		s.AstFindAllDeep(kind, out);
}

void VfsValue::AstFindAllDeep(int kind, Vector<const VfsValue*>& out) const
{
	const AstValue* a = *this;
	if(a && a->kind == kind)
		out << this;
	for(const auto& s : sub)
		s.AstFindAllDeep(kind, out);
}

Vector<const VfsValue*> VfsValue::AstFindAllShallow(int kind) const
{
	Vector<const VfsValue*> vec;
	for(const auto& s : sub) {
		const AstValue* a = s;
		if(a && a->kind == kind)
			vec << &s;
	}
	return vec;
}

bool VfsValue::IsAstValue() const {
	const AstValue* v = *this;
	return v;
}

int VfsValue::GetAstValueCount() const
{
	int c = 0;
	for(const auto& s : sub) {
		const AstValue* a = s;
		if (a)
			c++;
	}
	return c;
}

String VfsValue::GetBasesString() const
{
	if (VfsValue_GetBasesString)
		return VfsValue_GetBasesString(*this);
	else
		return String();
}

String VfsValue::GetNestString() const
{
	if(owner)
		return owner->id;
	return String();
}

bool VfsValue::OwnerRecursive(const VfsValue& n) const
{
	VfsValue* o = this->owner;
	while(o) {
		if(o == &n)
			return true;
		o = o->owner;
	}
	return false;
}

bool VfsValue::ContainsDeep(const VfsValue& n) const
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

void VfsValue::AstRemoveAllShallow(int kind)
{
	Vector<int> rmlist;
	int i = 0;
	for(auto& s : sub) {
		const AstValue* a = s;
		if(a && a->kind == kind)
			rmlist << i;
		i++;
	}
	if(!rmlist.IsEmpty())
		sub.Remove(rmlist);
}

void VfsValue::AstRemoveAllDeep(int kind)
{
	AstRemoveAllShallow(kind);
	for(auto& s : sub)
		s.AstRemoveAllDeep(kind);
}

void VfsValue::GetTypeHashes(Index<hash_t>& type_hashes) const
{
	if(type_hash)
		type_hashes.FindAdd(type_hash);
	for(auto& s : sub)
		s.GetTypeHashes(type_hashes);
}

void VfsValue::RealizeSerial() {
	if (!serial)
		serial = MetaEnv().NewSerial();
	for (auto& s : sub)
		s.RealizeSerial();
}

Vector<Ptr<VfsValueExt>> VfsValue::GetAllExtensions() {
	Vector<Ptr<VfsValueExt>> v;
	for (auto& s : sub) {
		if (s.ext) {
			v.Add(&*s.ext);
		}
	}
	return v;
}

VfsPath VfsValue::GetPath() const {
	static const int LIMIT = 64;
	const VfsValue* ptrs[LIMIT];
	int i = 1;
	ptrs[0] = this;
	while (i < LIMIT-1) {
		if (ptrs[i-1]->owner) {
			const VfsValue* p = ptrs[i-1]->owner;
			ptrs[i++] = p;
		}
		else break;
	}
	if (i == 1) return VfsPath();
	ptrs[i] = 0;
	const VfsValue** iter = ptrs+i-2;
	const VfsValue** end = ptrs-1;
	Vector<Value> path;
	path.Reserve(i);
	while (iter != end) {
		path.Add((*iter)->id);
		iter--;
	}
	return path;
}

VfsValue& VfsValue::operator[](int i) {
	VfsValue& n = sub[i];
	if (n.symbolic_link) {
		Vector<hash_t> visited;
		visited << (hash_t)this;
		VfsValue* l = n.symbolic_link;
		while (l) {
			int i = VectorFind(visited, (hash_t)l);
			if (i >= 0) {
				l = n.symbolic_link; // reset
				break;
			}
			visited << (hash_t)l;
			if (!l->symbolic_link)
				break;
			l = l->symbolic_link;
		}
		return *l;
	}
	return n;
}

VfsValue::operator AstValue& () {
	return RealizeRawValue<AstValue>(value);
}

VfsValue::operator AstValue* () {
	return FindRawValue<AstValue>(value);
}

VfsValue::operator const AstValue* () const {
	return FindRawValue<AstValue>(value);
}



/*void MetaEnvironment::Store(const String& includes, const String& path, FileAnnotation& fa)
{
    VfsSrcPkg& af = ResolveFile(includes, path);
    af.Store(includes, path, fa);
}*/

/*void VfsSrcPkg::Store(const String& includes, const String& path, FileAnnotation& fa)
{
    VfsSrcFile& afi = RealizePath(includes, path);
    afi.UpdateLinks(fa);
    Save();
}*/

/*bool VfsValue::IsClassTemplateDefinition() const {
    if (kind == CXCursor_ClassTemplate)
        //for (const VfsValue& s : sub)
        //	if (s.kind == CXCursor_CompoundStmt)
                return true;
    return false;
}*/

VfsValue* MetaEnvironment::FindTypeDeclaration(hash_t type_hash)
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
		VfsValue& p = *ptr;
		const AstValue* a1 = p;
		if(a1 && a1->is_definition /* || p.IsClassTemplateDefinition()*/)
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
	VfsValue* n = &this->root;
	const auto& parts = rel_path.Parts();
	for(int i = 0; i < parts.GetCount(); i++) {
		const String& part = parts[i];
		int j = n->Find(part);
		if (j < 0)
			return false;
		n = &n->sub[j];
	}
	for(int i = 0; i < n->sub.GetCount(); i++) {
		VfsValue& n0 = n->sub[i];
		VfsItem& it = items.Add();
		it.name = n0.id;
		it.type = VFS_DIRECTORY;
		const AstValue* a0 = n0;
		if (a0 && !a0->type.IsEmpty())
			it.type_str = a0->type;
		else if (n0.ext)
			it.type_str = n0.ext->GetName();
	}
	return true;
}

VfsItemType MetaEnvironment::CheckItem(const VfsPath& rel_path) {
	VfsValue* n = &this->root;
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















VfsValueExt* VfsValueExtFactory::Create(hash_t type_hash, VfsValue& owner) {
	int i = FindTypeHashFactory(type_hash);
	if (i < 0) return 0;
	return List()[i].new_fn(owner);
}

#if 1
VfsValueExt* VfsValueExtFactory::Clone(const VfsValueExt& e, VfsValue& owner) {
	hash_t type_hash = e.GetTypeHash();
	int i = FindTypeHashFactory(type_hash);
	if (i < 0)
		return 0;
	VfsValueExt* n = List()[i].new_fn(owner);
	if (n)
		n->CopyFrom(e);
	return n;
}
#else
VfsValueExt* VfsValueExtFactory::Clone(const VfsValueExt& e, VfsValue& owner) {
	for (Factory& f : List()) {
		if (f.is_fn(e)) {
			VfsValueExt* n = f.new_fn(owner);
			n->CopyFrom(e);
			return n;
		}
	}
	return 0;
}
#endif

int VfsValueExtFactory::FindTypeHashFactory(hash_t type_hash) {
	int i = 0;
	for (Factory& f : List()) {
		if (f.type_hash == type_hash)
			return i;
		i++;
	}
	return -1;
}

int VfsValueExtFactory::FindTypeClsFactory(TypeCls t) {
	int i = 0;
	for (Factory& f : List()) {
		if (f.type_cls == t)
			return i;
		i++;
	}
	return -1;
}

int VfsValueExtFactory::AstFindKindCategory(int k) {
	TODO
	#if 0
	#define DATASET_ITEM(type, name, desc) \
		if (k == kind) \
			return group;
	DATASET_LIST
	#undef DATASET_ITEM
	#endif
	return -1;
}



VfsValueExt::VfsValueExt(VfsValue& n) : val(n) {
	
}

VfsValueExt::~VfsValueExt() {
	UninitializeDeep();
}

String VfsValueExt::GetName() const {return String();}
double VfsValueExt::GetUtility() {ASSERT_(0, "Not implemented"); return 0;}
double VfsValueExt::GetEstimate() {ASSERT_(0, "Not implemented"); return 0;}
double VfsValueExt::GetDistance(VfsValue& dest) {ASSERT_(0, "Not implemented"); return 0;}
bool VfsValueExt::TerminalTest(NodeRoute& prev) {ASSERT_(0, "Not implemented"); return true;}
String VfsValueExt::ToString() const {return String();}
bool VfsValueExt::Initialize(const WorldState& ws) {SetInitialized(true); return true;}
bool VfsValueExt::PostInitialize() {return true;}
void VfsValueExt::Uninitialize() {}
bool VfsValueExt::IsInitialized() const {return is_initialized;}
void VfsValueExt::SetInitialized(bool b) {is_initialized = b;}
String VfsValueExt::GetCategory() {return String();}
void VfsValueExt::Update(double dt) {ASSERT_(0,"unimplemented");}
bool VfsValueExt::Arg(String key, Value value) {ASSERT_(0,"unimplemented"); return false;}
bool VfsValueExt::Start() {return true;}
void VfsValueExt::Stop() {}
void VfsValueExt::UninitializeDeep() {
	if (IsInitialized()) {
		Uninitialize();
		SetInitialized(false);
		ClearDependencies();
	}
}
bool VfsValueExt::AddDependency(VfsValueExt& val) {
	for (auto& d : deps)
		if (d == &val)
			return true;
	deps.Add(&val);
	return true;
}
VfsValueExtPtr VfsValueExt::GetDependency(int i) const {
	return i >= 0 && i < deps.GetCount() ? deps[i] : VfsValueExtPtr();
}
int VfsValueExt::GetDependencyCount() const {
	return deps.GetCount();
}
void VfsValueExt::RemoveDependency(int i) {
	if (i >= 0 && i < deps.GetCount())
		deps.Remove(i);
}
void VfsValueExt::ClearDependencies() {
	deps.Clear();
}
void VfsValueExt::RemoveDependency(const VfsValueExt* e) {
	for(int i = 0; i < deps.GetCount(); i++)
		if (deps[i] == e)
			deps.Remove(i--);
}
String VfsValueExt::GetTreeString(int indent) const {
	String s;
	if (indent)
		s.Cat('\t', indent);
	s += val.id;
	s += ": " + ToString();
	return s;
}
void VfsValueExt::CopyFrom(const VfsValueExt& e) {
	StringStream s;
	s.SetStoring();
	Vis read(s);
	const_cast<VfsValueExt&>(e).Visit(read); // reading
	s.SetLoading();
	s.Seek(0);
	Vis write(s);
	Visit(write);
}

bool VfsValueExt::operator==(const VfsValueExt& e) const {
	return GetHashValue() == e.GetHashValue();
}

hash_t VfsValueExt::GetHashValue() const {
	Vis vis(1);
	const_cast<VfsValueExt*>(this)->Visit(vis);
	return vis.hash;
}

int VfsValueExt::AstGetKind() const {
	const AstValue* a = val;
	return a ? a->kind : 0;
}

void VfsValueExt::Serialize(Stream& s){
	Vis vis(s);
	const_cast<VfsValueExt*>(this)->Visit(vis);
}

void VfsValueExt::Jsonize(JsonIO& json){
	Vis vis(json);
	const_cast<VfsValueExt*>(this)->Visit(vis);
}

END_UPP_NAMESPACE

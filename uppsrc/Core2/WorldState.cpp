#include "Core.h"


NAMESPACE_UPP


WorldState::WorldState() {
	Clear();
}

void WorldState::Clear() {
	values.Clear();
	//using_act.Clear();
	//cur_atom = Null;
	//type = INVALID;
}

int WorldState::GetValueCount() const {
	// this is a bit ankward counting
	int c = 0;
	for (const String& v : values.GetValues())
		c += (v.IsEmpty() ? 0 : 1);
	return c;
}

void WorldState::FindKeys(String key_left, Index<String>& keys) const {
	for(int i = 0; i < values.GetCount(); i++) {
		auto& v = values.GetValue(i);
		if (v.IsNull())
			continue;
		String key = values.GetKey(i);
		if (key.Left(key_left.GetCount()) == key_left)
			keys.FindAdd(key);
	}
}

void WorldState::SetTrue(const String& key) {Set(key, true);}

void WorldState::SetFalse(const String& key) {Set(key, false);}

bool WorldState::IsEmpty() const {return values.GetCount() == 0;}

WorldState& WorldState::operator=(const WorldState& src) {
	values = src.values;
	return *this;
}


hash_t WorldState::GetHashValue() const {
	CombineHash c;
	for(int i = values.GetCount()-1; i >= 0; i--) {
		const Value& o = values.GetValue(i);
		c.Put(o.GetHashValue());
	}
	return c;
}

bool WorldState::Set(const String& key, bool value) {
	values.GetAdd(key) = value;
	return true;
}

bool WorldState::Set(const String& key, String value) {
	values.GetAdd(key) = value;
	return true;
}

bool WorldState::IsTrue(const String& key, bool def) const {
	int i = values.Find(key);
	if (i >= 0)
		return values.GetValue(i);
	return def;
}

bool WorldState::IsFalse(const String& key, bool def) const {
	int i = values.Find(key);
	if (i >= 0)
		return !values.GetValue(i);
	return def;
}

bool WorldState::IsFalse(int idx) const {
	return !values.GetValue(idx);
}

bool WorldState::IsUndefined(const String& key) const {
	TODO
	return false;
}

bool WorldState::IsUndefined(int idx) const {
	TODO
	return false;
}

String WorldState::Get(const String& key, String def) const {
	int i = values.Find(key);
	if (i < 0)
		return def;
	const Value& o = values.GetValue(i);
	return o.ToString();
}

String WorldState::Get(int idx) const {
	const Value& o = values.GetValue(idx);
	return o.ToString();
}

Size WorldState::GetSize(const String& cx, const String& cy, Size def) const {
	String cx_str = Get(cx);
	String cy_str = Get(cy);
	if (cx_str.IsEmpty() || cy_str.IsEmpty())
		return def;
	return Size(StrInt(cx_str), StrInt(cy_str));
}

int WorldState::GetInt(const String& key, int def) const {
	int i = values.Find(key);
	if (i >= 0) {
		const Value& o = values.GetValue(i);
		return o;
	}
	return def;
}

double WorldState::GetDouble(const String& key, double def) const {
	int i = values.Find(key);
	if (i >= 0) {
		const Value& o = values.GetValue(i);
		return o;
	}
	return def;
}

bool WorldState::GetBool(const String& key, bool def) const {
	int i = values.Find(key);
	if (i >= 0) {
		const Value& o = values.GetValue(i);
		return o;
	}
	return def;
}

String WorldState::GetString(const String& key, String def) const {
	int i = values.Find(key);
	if (i >= 0) {
		const Value& o = values.GetValue(i);
		if (!o.Is<String>()) {LOG(AsJSON(o));}
		ASSERT(o.Is<String>());
		return o
		;
	}
	return def;
}

String WorldState::ToString() const {
	String s;
	for(int i = 0; i < values.GetCount(); i++) {
		const Value& vo = values.GetValue(i);
		String v = vo.ToString();
		if (v.IsEmpty()) v = "false";
		String k = values.GetKey(i).ToString();
		if (!s.IsEmpty())
			s << ",";
		s << k << "=" << v;
	}
	return s;
}

String WorldState::GetFullString() const {
	TODO
	return String();
}

bool WorldState::Contains(const WorldState& ws) const {
	TODO
	return false;
}

bool WorldState::Conflicts(const WorldState& ws) const {
	TODO
	return false;
}


int WorldState::Compare(int idx, const WorldState& ws) const {
	TODO
	return 0;
}










BinaryWorldState::BinaryWorldState() {
	
	Clear();
}

BinaryWorldState::BinaryWorldState(const BinaryWorldState& ws) {
	*this = ws;
}

BinaryWorldState::BinaryWorldState(BinaryWorldState&& ws) {
	Swap(mask, ws.mask);
	Swap(atoms, ws.atoms);
}

void BinaryWorldState::Clear() {
	atoms.Clear();
}

bool BinaryWorldState::SetMasked(int index, bool value, bool req_resolve) {
	ASSERT(index >= 0 && index < mask->keys.GetCount());
	if (index < 0) return false;
	if (atoms.GetCount() <= index) {
		atoms.SetCount(index+1);
	}
	auto& atom = atoms[index];
	atom.in_use = true;
	atom.value = value;
	atom.req_resolve = req_resolve;
	return true;
}

bool BinaryWorldState::SetKey(const WorldStateKey& key, bool value, bool req_resolve) {
	ASSERT(mask);
	if (!mask)
		return false;
	#ifdef flagDEBUG
	if (req_resolve && key.GetLength() > 1) {
		int i = mask->session->atoms.Find(key);
		ASSERT(i >= 0);
		//const auto& atom = mask->session->atoms[i];
		//ASSERT(atom.decl_atom_idx >= 0);
	}
	#endif
	int index = mask->FindAdd(key, req_resolve);
	return SetMasked(index, value, req_resolve);
}

bool BinaryWorldState::SetAtomIndex(int atom_idx, bool value, bool req_resolve) {
	ASSERT(atom_idx >= 0 && atom_idx < mask->session->atoms.GetCount());
	WorldStateKey key = mask->session->GetAtomKey(atom_idx);
	return SetKey(key, value, req_resolve);
}

BinaryWorldState& BinaryWorldState::operator = (const BinaryWorldState& src) {
	atoms <<= src.atoms;
	mask = src.mask;
	return *this;
}

bool BinaryWorldState::operator==(const BinaryWorldState& src) const {
	if (mask != src.mask)
		return false;
	int count = min(src.atoms.GetCount(), atoms.GetCount());
	for(int i = count; i < src.atoms.GetCount(); i++)
		if (src.atoms[i].in_use)
			return false;
	for(int i = count; i < atoms.GetCount(); i++)
		if (atoms[i].in_use)
			return false;
	for(int i = 0; i < count; i++) {
		if (src.atoms[i].in_use != atoms[i].in_use ||
			src.atoms[i].value  != atoms[i].value)
			return false;
	}
	return true;
}

bool BinaryWorldState::IsPartialMatch(const BinaryWorldState& src) const {
	if (mask != src.mask)
		return false;
	int count = min(src.atoms.GetCount(), atoms.GetCount());
	for(int i = count; i < src.atoms.GetCount(); i++)
		if (src.atoms[i].in_use)
			return false;
	for(int i = 0; i < count; i++) {
		bool u0 = atoms[i].in_use;
		bool u1 = src.atoms[i].in_use;
		if (!u0 && u1)
			return false;
		if (!u1)
			continue;
		ASSERT(u0 && u1);
		bool v0 = atoms[i].value;
		bool v1 = src.atoms[i].value;
		if (v0 != v1)
			return false;
	}
	return true;
}

hash_t BinaryWorldState::GetHashValue() const {
	CombineHash c;
	int last_i = atoms.GetCount()-1;
	while (last_i >= 0) {
		if (atoms[last_i].in_use)
			break;
		last_i--;
	}
	for(int i = 0; i <= last_i; i++) {
		bool b = atoms[last_i].in_use;
		c.Put(b);
		if (b)
			c.Put(atoms[i].value);
	}
	return c;
}

Value BinaryWorldState::ToValue() const {
	ValueMap map;
	int i = 0;
	for (auto& at : this->atoms) {
		if (at.in_use) {
			if (!at.req_resolve) {
				map.Add(i, (int)at.value);
			}
			else {
				ValueArray arr;
				arr.SetCount(2);
				arr.Set(0, (int)at.value);
				arr.Set(1, (int)at.req_resolve);
				map.Add(i, arr);
			}
		}
		i++;
	}
	return map;
}


bool BinaryWorldState::FromValue(bool use_params, Value v, Event<String> WhenError) {
	ASSERT(mask);
	if (!mask) {Clear(); return false;}
	ValueMap ws = v;
	if (v.Is<ValueMap>()) {
		ws = v;
	}
	else {
		WhenError("unexpected value type " + v.GetTypeName());
		return false;
	}
	
	if (ws.IsEmpty()) {
		WhenError("empty world state map");
		return false;
	}
	
	Value first_key = ws.GetKey(0);
	if (first_key.Is<int>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			int j = ws.GetKey(i);
			Value val = ws.GetValue(i);
			if (val.Is<bool>()) {
				bool v = val;
				SetMasked(j, v, false);
			}
			else if (val.Is<int>()) {
				int v = val;
				SetMasked(j, v, false);
			}
			else if (val.Is<ValueArray>()) {
				ValueArray arr = val;
				if (arr.GetCount() != 2) {WhenError("unexpected ValueArray size: " + IntStr(arr.GetCount())); return false;}
				int v = arr[0];
				int req_resolve = arr[1];
				SetMasked(j, v, req_resolve);
			}
			else {
				WhenError("unexpected type: " + val.GetTypeName());
				return false;
			}
		}
	}
	else if (first_key.Is<String>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			String atom_name = ws.GetKey(i);
			WorldStateKey atom_key;
			if (!use_params && !mask->session->ParseRaw(atom_name, atom_key)) return false;
			if ( use_params && !mask->session->ParseDecl(atom_name, atom_key)) return false;
			auto& session = *mask->session;
			int j = session.atoms.Find(atom_key);
			if (j < 0) {WhenError("the goal atom can't be found: " + atom_name); return false;}
			auto& atom = session.atoms[j];
			Value val = ws.GetValue(i);
			if (val.Is<bool>()) {
				bool v = val;
				atom.goal = v;
				SetKey(atom_key, v, false);
			}
			else if (val.Is<int>()) {
				int v = val;
				atom.goal = v;
				SetKey(atom_key, v, false);
			}
			else if (val.Is<ValueArray>()) {
				ValueArray arr = val;
				if (arr.GetCount() != 2) {WhenError("unexpected ValueArray size: " + IntStr(arr.GetCount())); return false;}
				int v = arr[0];
				atom.goal = v;
				int req_resolve = arr[1];
				SetKey(atom_key, v, req_resolve);
			}
			else {
				WhenError("unexpected type: " + val.GetTypeName());
				return false;
			}
		}
	}
	else {
		LOG(AsJSON(v, true));
		ASSERT(0);
		return false;
	}
	
	return true;
}

String BinaryWorldState::ToString() const {
	String s;
	int i = 0;
	for(auto& a : atoms) {
		if (a.in_use) {
			if (mask && mask->session) {
				int atom_idx = mask->keys[i].atom_idx;
				s << mask->session->GetKeyString(mask->session->atoms[atom_idx].key) << ": ";
			}
			else
				s << i << ": ";
			s << (a.value ? "true" : "false");
			s << "\n";
		}
		i++;
	}
	if (s.IsEmpty())
		s = "<null worldstate>";
	return s;
}

String BinaryWorldState::ToShortInlineString() const {
	String s;
	for(auto& a : atoms)
		s.Cat(a.in_use ? (a.value ? '|' : '_') : '.');
	return s;
}

String BinaryWorldState::ToInlineString() const {
	String s;
	ASSERT(mask && mask->session);
	if (!mask || !mask->session) return "error";
	int i = 0;
	for(auto& a : atoms) {
		if (a.in_use) {
			if (!s.IsEmpty())
				s.Cat(' ');
			const auto& key = mask->keys[i].key;
			bool put_end = false;
			for(int j = 0; j < key.size; j++) {
				int k = key.vector[j];
				if (k < 0) break;
				if (j == 1) {s.Cat('('); put_end = true;}
				String str = mask->session->key_values[k];
				str.Replace(" ", "_");
				if (a.value)
					str = ToUpper(str);
				else
					str = ToLower(str);
				s << str;
			}
			if (put_end) s.Cat(')');
		}
		i++;
	}
	return s;
}

void BinaryWorldState::SetIntersection(BinaryWorldState& a, BinaryWorldState& b) {
	int count = min(a.atoms.GetCount(), b.atoms.GetCount());
	atoms.SetCount(0);
	atoms.SetCount(count);
	auto A = a.atoms.Begin();
	auto B = b.atoms.Begin();
	for (auto& to : atoms) {
		if (A->in_use && B->in_use &&
			A->value != B->value) {
			to.in_use = true;
			to.value = B->value;
		}
		A++; B++;
	}
	mask = b.mask;
}

void BinaryWorldState::SetDifference(BinaryWorldState& a, BinaryWorldState& b) {
	int count = b.atoms.GetCount();
	atoms.SetCount(0);
	atoms.SetCount(count);
	auto A = a.atoms.Begin();
	auto A_end = a.atoms.End();
	auto B = b.atoms.Begin();
	for (auto& to : atoms) {
		bool add = false;
		if (B->in_use) {
			if (A == A_end)
				add = B->value;
			else
				add = A->value != B->value;
		}
		if (add) {
			to.in_use = true;
			to.value = B->value;
		}
		A = A == A_end ? A : A+1;
		B++;
	}
	mask = b.mask;
}

bool BinaryWorldState::IsEmpty() const {
	for (auto& a : atoms)
		if (a.in_use)
			return false;
	return true;
}






int BinaryWorldStateSession::FindAtom(const Key& k) const {
	hash_t h = k.GetHashValue();
	lock.EnterRead();
	int i = atoms.Find(h);
	lock.LeaveRead();
	return i;
}

int BinaryWorldStateSession::FindAddAtom(const Key& k) {
	hash_t h = k.GetHashValue();
	lock.EnterRead();
	int i = atoms.Find(h);
	lock.LeaveRead();
	if (i >= 0)
		return i;
	lock.EnterWrite();
	i = atoms.GetCount();
	auto& atom = atoms.Add(h);
	atom.key = k;
	atom.key_len = k.GetLength();
	atom.goal = false;
	atom.initial = false;
	lock.LeaveWrite();
	return i;
}

BinaryWorldStateSession::Item& BinaryWorldStateSession::GetAddAtom(const Key& k) {
	hash_t h = k.GetHashValue();
	lock.EnterRead();
	int i = atoms.Find(h);
	lock.LeaveRead();
	if (i >= 0)
		return atoms[i];
	lock.EnterWrite();
	i = atoms.GetCount();
	auto& atom = atoms.Add(h);
	atom.key = k;
	atom.key_len = k.GetLength();
	atom.goal = false;
	atom.initial = false;
	lock.LeaveWrite();
	return atom;
}

String BinaryWorldStateSession::GetKeyString(int idx) const {
	String res;
	if (idx < 0 || idx >= key_values.GetCount())
		return res;
	lock.EnterRead();
	res = key_values[idx];
	lock.LeaveRead();
	return res;
}

String BinaryWorldStateSession::GetKeyString(const Key& k) const {
	lock.EnterRead();
	String out;
	int len = 0;
	for(int i = 0; i < k.size; i++) {
		int idx = k.vector[i];
		if (idx < 0)
			break;
		if (i == 1)
			out.Cat('(');
		else if(i > 1)
			out << ", ";
		out.Cat((String)key_values[idx]);
		len++;
	}
	lock.LeaveRead();
	if (len >= 2)
		out.Cat(')');
	return out;
}

WorldStateKey BinaryWorldStateSession::GetAtomKey(int atom_idx) const {
	WorldStateKey ret;
	lock.EnterRead();
	ret = atoms[atom_idx].key;
	lock.LeaveRead();
	return ret;
}

bool BinaryWorldStateSession::ParseRaw(const String& atom_name, Key& atom_key) {
	Key& k = atom_key;
	k[0] = key_values.FindAdd(atom_name);
	if (k[0] < 0) {return false;}
	for(int i = 1; i <= WSKEY_MAX_PARAMS; i++)
		k[i] = -1;
	return true;
}

bool BinaryWorldStateSession::ParseDecl(const String& atom_name, Key& atom_key) {
	Key& k = atom_key;
	int i = atom_name.Find("(");
	if (i < 0) {
		String id = TrimBoth(atom_name);
		k[0] = key_values.FindAdd(id);
	}
	else {
		String id = TrimBoth(atom_name.Left(i));
		k[0] = key_values.FindAdd(id);
		String params = TrimBoth(atom_name.Mid(i));
		CParser p(params);
		try {
			//String id = p.ReadId();
			//k[0] = key_values.FindAdd(id);
			int len = 1;
			if (p.Char('(')) {
				for(int i = 0; i < WSKEY_MAX_PARAMS; i++) {
					if (p.IsChar(')')) break;
					if (i) p.PassChar(',');
					id = p.ReadId();
					k[len++] = key_values.FindAdd(id);
				}
				p.PassChar(')');
			}
			for(int i = len; i < WSKEY_MAX_PARAMS; i++)
				k[i] = -1;
		}
		catch (Exc e) {
			LOG("BinaryWorldStateSession::ParseDecl: error: " << e);
			return false;
		}
	}
	return true;
}

bool BinaryWorldStateSession::ParseCall(const String& atom_name, Key& atom_key) {
	Key& k = atom_key;
	CParser p(atom_name);
	try {
		String id = p.ReadId();
		k[0] = key_values.FindAdd(id);
		int len = 1;
		if (p.Char('(')) {
			for(int i = 0; i < WSKEY_MAX_PARAMS; i++) {
				if (p.IsChar(')')) break;
				if (i) p.PassChar(',');
				Value value;
				if (p.IsString()) {
					value = p.ReadString();
				}
				else if (p.IsId()) {
					value = p.ReadId();
				}
				else if (p.IsInt()) {
					value = p.ReadInt64();
				}
				else if (p.IsDouble()) {
					value = p.ReadDouble();
				}
				
				k[len++] = key_values.FindAdd(value);
			}
			p.PassChar(')');
		}
		for(int i = len; i < WSKEY_MAX_PARAMS; i++)
			k[i] = -1;
	}
	catch (Exc e) {
		LOG("BinaryWorldStateSession::ParseCall: error: " << e);
		return false;
	}
	return true;
}




WorldStateKey::WorldStateKey() {
	for(int i = 0; i < size; i++)
		vector[i] = -1;
}

WorldStateKey::WorldStateKey(const WorldStateKey& key) {
	*this = key;
}

bool WorldStateKey::operator==(const WorldStateKey& k) const {
	bool same = true;
	for(int i = 0; i < size; i++)
		same = same && vector[i] == k.vector[i];
	return same;
}

WorldStateKey::operator hash_t() const {
	CombineHash ch;
	for(int i = 0; i < size; i++)
		ch.Do(vector[i]);
	return ch;
}

bool WorldStateKey::IsEmpty() const {
	return vector[0] == -1;
}

int WorldStateKey::GetLength() const {
	int len = 0;
	for(int i = 0; i < size; i++) {
		if (vector[i] < 0)
			break;
		len++;
	}
	return len;
}






int BinaryWorldStateMask::Find(const Key& key) const {
	int i = 0;
	for (const auto& it : keys) {
		if (it.key == key)
			return i;
		i++;
	}
	return -1;
}

int BinaryWorldStateMask::FindAdd(const Key& key, bool req_resolve) {
	int i = 0;
	for (const auto& it : keys) {
		if (it.key == key)
			return i;
		i++;
	}
	ASSERT(session);
	if (!session) return -1;
	int atom_idx = session->atoms.Find(key);
	ASSERT(atom_idx >= 0);
	if (atom_idx < 0) return -1;
	
	int decl_atom_idx = -1;
	if (req_resolve) {
		int j = 0;
		int len = key.GetLength();
		for (auto& a : session->atoms) {
			if (a.key_len == len &&
				a.key.vector[0] == key.vector[0] &&
				a.decl_atom_idx < 0 &&
				j != atom_idx) {
				decl_atom_idx = j;
				// TODO check multiple "a.decl_atom_idx < 0" atoms
				break;
			}
			j++;
		}
	}
	
	i = keys.GetCount();
	auto& it = keys.Add();
	it.key = key;
	it.atom_idx = atom_idx;
	it.decl_atom_idx = decl_atom_idx;
	ASSERT(decl_atom_idx != atom_idx);
	return i;
}


END_UPP_NAMESPACE

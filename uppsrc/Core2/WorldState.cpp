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
	Swap(atom_values, ws.atom_values);
	Swap(using_atom, ws.using_atom);
}

void BinaryWorldState::Clear() {
	atom_values.Clear();
	using_atom.Clear();
}

bool BinaryWorldState::SetMasked(int index, bool value) {
	ASSERT(index >= 0 && index < mask->keys.GetCount());
	if (index < 0) return false;
	if (using_atom.GetCount() <= index) {
		using_atom.SetCount(index+1, false);
		atom_values.SetCount(index+1, false);
	}
	using_atom[index] = true;
	atom_values[index] = value;
	return true;
}

bool BinaryWorldState::SetKey(const WorldStateKey& key, bool value) {
	ASSERT(mask);
	if (!mask)
		return false;
	int index = VectorFindAdd(mask->keys, key);
	ASSERT(index >= 0);
	if (index < 0) return false;
	return SetMasked(index, value);
}

BinaryWorldState& BinaryWorldState::operator = (const BinaryWorldState& src) {
	atom_values <<= src.atom_values;
	using_atom <<= src.using_atom;
	mask = src.mask;
	return *this;
}

bool BinaryWorldState::operator==(const BinaryWorldState& src) const {
	if (mask != src.mask)
		return false;
	int count = min(src.using_atom.GetCount(), using_atom.GetCount());
	for(int i = count; i < src.using_atom.GetCount(); i++)
		if (src.using_atom[i])
			return false;
	for(int i = count; i < using_atom.GetCount(); i++)
		if (using_atom[i])
			return false;
	for(int i = 0; i < count; i++) {
		if (src.using_atom[i] != using_atom[i] ||
			src.atom_values[i] != atom_values[i])
			return false;
	}
	return true;
}

bool BinaryWorldState::IsPartialMatch(const BinaryWorldState& src) const {
	if (mask != src.mask)
		return false;
	int count = min(src.using_atom.GetCount(), using_atom.GetCount());
	for(int i = count; i < src.using_atom.GetCount(); i++)
		if (src.using_atom[i])
			return false;
	for(int i = 0; i < count; i++) {
		bool u0 = using_atom[i];
		bool u1 = src.using_atom[i];
		if (!u0 && u1)
			return false;
		if (!u1)
			continue;
		ASSERT(u0 && u1);
		bool v0 = atom_values[i];
		bool v1 = src.atom_values[i];
		if (v0 != v1)
			return false;
	}
	return true;
}

hash_t BinaryWorldState::GetHashValue() const {
	CombineHash c;
	int last_i = atom_values.GetCount()-1;
	while (last_i >= 0) {
		if (using_atom[last_i])
			break;
		last_i--;
	}
	for(int i = 0; i <= last_i; i++) {
		bool b = using_atom[last_i];
		c.Put(b);
		if (b)
			c.Put(atom_values[i]);
	}
	return c;
}

Value BinaryWorldState::ToValue() const {
	ValueMap map;
	int i = 0;
	for (bool b : this->using_atom) {
		if (b)
			map.Add(i, (int)this->atom_values[i]);
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
			int v = ws.GetValue(i);
			SetMasked(j, v);
		}
	}
	else if (first_key.Is<String>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			String atom_name = ws.GetKey(i);
			WorldStateKey atom_key;
			if (!mask->ParseKey(use_params, atom_name, atom_key))
				return false;
			Value v = ws.GetValue(i);
			auto& session = *mask->session;
			int j = session.atoms.Find(atom_key);
			if (j < 0) {WhenError("the goal atom can't be found: " + atom_name); return false;}
			auto& atom = session.atoms[j];
			atom.goal = v;
			SetKey(atom_key, v);
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
	for(int i = 0; i < this->using_atom.GetCount(); i++) {
		if (using_atom[i]) {
			if (mask && mask->session) {
				TODO //s << session->atoms.GetKey(i) << ": ";
			}
			else
				s << i << ": ";
			s << (this->atom_values[i] ? "true" : "false");
			s << "\n";
		}
	}
	if (s.IsEmpty())
		s = "<null worldstate>";
	return s;
}

String BinaryWorldState::ToShortInlineString() const {
	String s;
	auto val = atom_values.Begin();
	for(auto& b : this->using_atom) {
		s.Cat(b ? (*val ? '|' : '_') : '.');
		val++;
	}
	return s;
}

String BinaryWorldState::ToInlineString() const {
	String s;
	ASSERT(mask && mask->session);
	if (!mask || !mask->session) return "error";
	for(int i = 0; i < this->using_atom.GetCount(); i++) {
		if (using_atom[i]) {
			if (!s.IsEmpty())
				s.Cat(' ');
			const auto& key = mask->keys[i];
			bool put_end = false;
			for(int j = 0; j < key.size; j++) {
				int k = key.vector[j];
				if (k < 0) break;
				if (j == 1) {s.Cat('('); put_end = true;}
				String str = mask->session->key_strings[k];
				str.Replace(" ", "_");
				if (this->atom_values[i])
					str = ToUpper(str);
				else
					str = ToLower(str);
				s << str;
			}
			if (put_end) s.Cat(')');
		}
	}
	return s;
}

void BinaryWorldState::SetIntersection(BinaryWorldState& a, BinaryWorldState& b) {
	int count = min(a.atom_values.GetCount(), b.atom_values.GetCount());
	using_atom.SetCount(count);
	atom_values.SetCount(count);
	for(auto& b : using_atom) b = false;
	for(auto& b : atom_values) b = false;
	for(int i = 0; i < count; i++) {
		if (a.using_atom[i] && b.using_atom[i] &&
			a.atom_values[i] != b.atom_values[i]) {
			using_atom[i] = true;
			atom_values[i] = b.atom_values[i];
		}
	}
	mask = b.mask;
}

void BinaryWorldState::SetDifference(BinaryWorldState& a, BinaryWorldState& b) {
	int count = b.atom_values.GetCount();
	using_atom.SetCount(count);
	atom_values.SetCount(count);
	for(auto& b : using_atom) b = false;
	for(auto& b : atom_values) b = false;
	for(int i = 0; i < count; i++) {
		bool add = false;
		if (b.using_atom[i]) {
			if (i >= a.atom_values.GetCount())
				add = b.atom_values[i];
			else
				add = a.atom_values[i] != b.atom_values[i];
		}
		if (add) {
			using_atom[i] = true;
			atom_values[i] = b.atom_values[i];
		}
	}
	mask = b.mask;
}

bool BinaryWorldState::IsEmpty() const {
	for (auto& b : using_atom)
		if (b)
			return false;
	return true;
}






int BinaryWorldStateSession::Find(const Key& k) const {
	hash_t h = k.GetHashValue();
	return atoms.Find(h);
}

String BinaryWorldStateSession::GetKeyString(int idx) const {
	String res;
	if (idx < 0 || idx >= key_strings.GetCount())
		return res;
	lock.EnterRead();
	res = key_strings[idx];
	lock.LeaveRead();
	return res;
}





bool BinaryWorldStateMask::ParseKey(bool use_params, const String& atom_name, Key& atom_key) {
	Key& k = atom_key;
	if (!use_params) {
		k[0] = session->key_strings.FindAdd(atom_name);
		for(int i = 1; i <= WSKEY_MAX_PARAMS; i++)
			k[i] = -1;
		
	}
	else {
		int a = atom_name.Find("(");
		if (a >= 0) {
			String atom_id = TrimBoth(atom_name.Left(a));
			a++;
			int b = atom_name.Find(")", a);
			ASSERT(b >= 0);
			if (b < 0) return false;
			Vector<String> params = Split(atom_name.Mid(a,b-a), ",", false);
			k[0] = session->key_strings.FindAdd(atom_id);
			if (params.GetCount() > WSKEY_MAX_PARAMS)
				return false;
			for(int i = 0; i < params.GetCount(); i++)
				k[1+i] = session->key_strings.FindAdd(params[i]);
			for(int i = params.GetCount(); i < WSKEY_MAX_PARAMS; i++)
				k[1+i] = -1;
		}
		else {
			k[0] = session->key_strings.FindAdd(atom_name);
			for(int i = 1; i <= WSKEY_MAX_PARAMS; i++)
				k[i] = -1;
		}
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

	
END_UPP_NAMESPACE

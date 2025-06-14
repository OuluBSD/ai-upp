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
	Swap(session, ws.session);
	Swap(atom_values, ws.atom_values);
	Swap(using_atom, ws.using_atom);
}

void BinaryWorldState::Clear() {
	atom_values.Clear();
	using_atom.Clear();
}

bool BinaryWorldState::Set(int index, bool value) {
	if (index < 0) return false;
	if (using_atom.GetCount() <= index) {
		using_atom.SetCount(index+1, false);
		atom_values.SetCount(index+1, false);
	}
	using_atom[index] = true;
	atom_values[index] = value;
	return true;
}

BinaryWorldState& BinaryWorldState::operator = (const BinaryWorldState& src) {
	atom_values <<= src.atom_values;
	using_atom <<= src.using_atom;
	session = src.session;
	return *this;
}

bool BinaryWorldState::operator==(const BinaryWorldState& src) const {
	if (session != src.session)
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
	if (session != src.session)
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


bool BinaryWorldState::FromValue(Value v, Event<String> WhenError) {
	ASSERT(session);
	if (!session) {Clear(); return false;}
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
			Set(j, v);
		}
	}
	else if (first_key.Is<String>()) {
		for(int i = 0; i < ws.GetCount(); i++) {
			String atom_name = ws.GetKey(i);
			Value v = ws.GetValue(i);
			int j = session->atoms.Find(atom_name);
			if (j < 0) {WhenError("the goal atom can't be found: " + atom_name); return false;}
			auto& atom = session->atoms[j];
			atom.goal = v;
			Set(j, v);
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
			if (session)
				s << session->atoms.GetKey(i) << ": ";
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
	for(int i = 0; i < this->using_atom.GetCount(); i++) {
		if (using_atom[i]) {
			if (!s.IsEmpty())
				s.Cat(' ');
			String key = session->atoms.GetKey(i);
			key.Replace(" ", "_");
			if (this->atom_values[i])
				key = ToUpper(key);
			else
				key = ToLower(key);
			s << key;
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
	session = b.session;
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
	session = b.session;
}

bool BinaryWorldState::IsEmpty() const {
	for (auto& b : using_atom)
		if (b)
			return false;
	return true;
}

END_UPP_NAMESPACE

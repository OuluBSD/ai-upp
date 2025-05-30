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


END_UPP_NAMESPACE

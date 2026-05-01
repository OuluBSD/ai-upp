#include "JsValue.h"

NAMESPACE_UPP

// Modeling JsLambda and JsVM structures to be fleshed out later
struct JsLambda : JsValue::RefCount {
	String name;
	JsBuiltin builtin;
	void *user_data;
	// Vector<int> code; // placeholders
	
	JsLambda() : builtin(nullptr), user_data(nullptr) {}
};

String JsTypeName(int type)
{
	switch(type) {
	case JS_UNDEFINED: return "undefined";
	case JS_NULL:      return "object (null)";
	case JS_BOOL:      return "boolean";
	case JS_NUMBER:    return "number";
	case JS_STR:       return "string";
	case JS_ARRAY:     return "object (Array)";
	case JS_OBJECT:    return "object";
	case JS_FUNCTION:  return "function";
	case JS_USERDATA:  return "object (native)";
	}
	return "unknown";
}

void JsValue::Free()
{
	if(!ptr) return;
	switch(type) {
	case JS_STR:       wstr->Release(); break;
	case JS_ARRAY:     array->Release(); break;
	case JS_OBJECT:    object->Release(); break;
	case JS_FUNCTION:  lambda->Release(); break;
	case JS_USERDATA:  userdata->Release(); break;
	}
	ptr = nullptr;
}

void JsValue::Assign(const JsValue& src)
{
	type = src.type;
	ptr = src.ptr;
	if(ptr) {
		switch(type) {
		case JS_BOOL:
		case JS_NUMBER:
			break;
		case JS_STR:       wstr->Retain(); break;
		case JS_ARRAY:     array->Retain(); break;
		case JS_OBJECT:    object->Retain(); break;
		case JS_FUNCTION:  lambda->Retain(); break;
		case JS_USERDATA:  userdata->Retain(); break;
		}
	}
}

JsValue::JsValue(const char *s)
{
	type = JS_STR;
	wstr = new JsStr;
	wstr->s = s;
}

JsValue::JsValue(const WString& s)
{
	type = JS_STR;
	wstr = new JsStr;
	wstr->s = s;
}

JsValue::JsValue(const String& s)
{
	type = JS_STR;
	wstr = new JsStr;
	wstr->s = s.ToWString();
}

JsValue::JsValue(JsUserData *ud)
{
	type = JS_USERDATA;
	userdata = ud;
	if(userdata) userdata->Retain();
}

JsValue::JsValue(int type, void *ptr)
{
	this->type = type;
	this->ptr = ptr;
	if(ptr) {
		switch(type) {
		case JS_STR:       wstr->Retain(); break;
		case JS_ARRAY:     array->Retain(); break;
		case JS_OBJECT:    object->Retain(); break;
		case JS_FUNCTION:  lambda->Retain(); break;
		case JS_USERDATA:  userdata->Retain(); break;
		}
	}
}

bool JsValue::IsTrue() const
{
	switch(type) {
	case JS_UNDEFINED:
	case JS_NULL:      return false;
	case JS_BOOL:      return b;
	case JS_NUMBER:    return f64 != 0;
	case JS_STR:       return wstr->s.GetCount() > 0;
	case JS_ARRAY:
	case JS_OBJECT:
	case JS_FUNCTION:
	case JS_USERDATA:  return true;
	}
	return false;
}

WString JsValue::GetStr() const
{
	return type == JS_STR ? wstr->s : WString();
}

String JsValue::GetString() const
{
	return GetStr().ToString();
}

int JsValue::GetCount() const
{
	if(type == JS_ARRAY) return array->l.GetCount();
	if(type == JS_STR) return wstr->s.GetCount();
	return 0;
}

JsValue JsValue::GetItem(int i) const
{
	if(type == JS_ARRAY && i >= 0 && i < array->l.GetCount())
		return array->l[i];
	if(type == JS_STR && i >= 0 && i < wstr->s.GetCount())
		return JsValue(WString(wstr->s[i], 1));
	return Undefined();
}

void JsValue::SetItem(int i, const JsValue& v)
{
	if(type == JS_ARRAY) {
		if(i >= array->l.GetCount())
			array->l.SetCount(i + 1);
		array->l[i] = v;
	}
}

void JsValue::Add(const JsValue& v)
{
	if(type == JS_ARRAY)
		array->l.Add(v);
}

JsValue JsValue::GetItem(const JsValue& key) const
{
	if(type == JS_OBJECT)
		return object->d.Get(key, Undefined());
	return Undefined();
}

void JsValue::SetItem(const JsValue& key, const JsValue& v)
{
	if(type == JS_OBJECT)
		object->d.GetAdd(key) = v;
}

bool JsValue::Contains(const JsValue& v) const
{
	if(type == JS_OBJECT)
		return object->d.Find(v) >= 0;
	if(type == JS_ARRAY) {
		for(const auto& x : array->l)
			if(x == v) return true;
	}
	return false;
}

Value JsValue::ToValue() const
{
	switch(type) {
	case JS_UNDEFINED:
	case JS_NULL:      return Value();
	case JS_BOOL:      return b;
	case JS_NUMBER:    return f64;
	case JS_STR:       return GetString();
	}
	return Value();
}

String JsValue::ToString() const
{
	switch(type) {
	case JS_UNDEFINED: return "undefined";
	case JS_NULL:      return "null";
	case JS_BOOL:      return b ? "true" : "false";
	case JS_NUMBER:    return Upp::Format("%g", f64);
	case JS_STR:       return GetString();
	case JS_ARRAY:     return "[array]";
	case JS_OBJECT:    return "[object]";
	case JS_FUNCTION:  return "[function]";
	case JS_USERDATA:  return "[native " + userdata->GetTypeName() + "]";
	}
	return "";
}

hash_t JsValue::GetHashValue() const
{
	CombineHash h;
	h << type;
	switch(type) {
	case JS_BOOL:      h << b; break;
	case JS_NUMBER:    h << f64; break;
	case JS_STR:       h << wstr->s; break;
	case JS_ARRAY:     h << (void*)array; break;
	case JS_OBJECT:    h << (void*)object; break;
	case JS_FUNCTION:  h << (void*)lambda; break;
	case JS_USERDATA:  h << (void*)userdata; break;
	}
	return h;
}

bool JsValue::operator==(const JsValue& other) const
{
	if(type != other.type) return false;
	switch(type) {
	case JS_UNDEFINED:
	case JS_NULL:      return true;
	case JS_BOOL:      return b == other.b;
	case JS_NUMBER:    return f64 == other.f64;
	case JS_STR:       return wstr->s == other.wstr->s;
	default:           return ptr == other.ptr;
	}
}

bool JsValue::operator<(const JsValue& other) const
{
	if(type != other.type) return type < other.type;
	switch(type) {
	case JS_UNDEFINED:
	case JS_NULL:      return false;
	case JS_BOOL:      return b < other.b;
	case JS_NUMBER:    return f64 < other.f64;
	case JS_STR:       return wstr->s < other.wstr->s;
	default:           return ptr < other.ptr;
	}
}

JsValue JsValue::Null()
{
	JsValue v;
	v.type = JS_NULL;
	return v;
}

JsValue JsValue::Array()
{
	JsValue v;
	v.type = JS_ARRAY;
	v.array = new JsArray;
	return v;
}

JsValue JsValue::Object()
{
	JsValue v;
	v.type = JS_OBJECT;
	v.object = new JsObject;
	return v;
}

JsValue JsValue::UserData(JsUserData *ud)
{
	return JsValue(ud);
}

JsValue JsValue::Function(const String& name, JsBuiltin builtin, void* user_data)
{
	JsValue v;
	v.type = JS_FUNCTION;
	v.lambda = new JsLambda;
	v.lambda->name = name;
	v.lambda->builtin = builtin;
	v.lambda->user_data = user_data;
	return v;
}

END_UPP_NAMESPACE

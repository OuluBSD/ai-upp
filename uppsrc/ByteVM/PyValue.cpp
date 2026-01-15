#include "ByteVM.h"

namespace Upp {

String PyTypeName(int type)
{
	switch(type) {
	case PY_NONE: return "NoneType";
	case PY_BOOL: return "bool";
	case PY_INT: return "int";
	case PY_FLOAT: return "float";
	case PY_COMPLEX: return "complex";
	case PY_STR: return "str";
	case PY_BYTES: return "bytes";
	case PY_LIST: return "list";
	case PY_TUPLE: return "tuple";
	case PY_DICT: return "dict";
	case PY_SET: return "set";
	case PY_FUNCTION: return "function";
	case PY_ITERATOR: return "iterator";
	case PY_USERDATA: return "userdata";
	case PY_BOUND_METHOD: return "method";
	case PY_STOP_ITERATION: return "StopIteration";
	default: return "unknown";
	}
}

void PyValue::Free()
{
	if (type == PY_NONE) return;
	//RTLOG("PyValue::Free: type=" << PyTypeName(type) << " ptr=" << ptr);
	switch(type) {
	case PY_COMPLEX: complex->Release(); break;
	case PY_STR: wstr->Release(); break;
	case PY_BYTES: bytes->Release(); break;
	case PY_LIST: list->Release(); break;
	case PY_TUPLE: tuple->Release(); break;
	case PY_DICT: dict->Release(); break;
	case PY_SET: set->Release(); break;
	case PY_FUNCTION: lambda->Release(); break;
	case PY_ITERATOR: iter->Release(); break;
	case PY_USERDATA: {
		if (userdata) {
			RTLOG("  Freeing UserData: " << (void*)userdata << " type=" << userdata->GetTypeName() << " count=" << (int)userdata->refcount);
			userdata->Release(); 
		}
		break;
	}
	case PY_BOUND_METHOD: {
		RTLOG("  Freeing BoundMethod: " << (void*)bound << " count=" << (int)bound->refcount);
		bound->Release(); 
		break;
	}
	}
	type = PY_NONE;
	ptr = nullptr;
}

void PyValue::Assign(const PyValue& s)
{
	type = s.type;
	if (type == PY_BOOL) b = s.b;
	else if (type == PY_INT) i64 = s.i64;
	else if (type == PY_FLOAT) f64 = s.f64;
	else if (type == PY_COMPLEX) { complex = s.complex; if (complex) complex->Retain(); }
	else if (type == PY_STR) { wstr = s.wstr; if (wstr) wstr->Retain(); }
	else if (type == PY_BYTES) { bytes = s.bytes; if (bytes) bytes->Retain(); }
	else if (type == PY_LIST) { list = s.list; if (list) list->Retain(); }
	else if (type == PY_TUPLE) { tuple = s.tuple; if (tuple) tuple->Retain(); }
	else if (type == PY_DICT) { dict = s.dict; if (dict) dict->Retain(); }
	else if (type == PY_SET) { set = s.set; if (set) set->Retain(); }
	else if (type == PY_FUNCTION) { lambda = s.lambda; if (lambda) lambda->Retain(); }
	else if (type == PY_ITERATOR) { iter = s.iter; if (iter) iter->Retain(); }
	else if (type == PY_USERDATA) { 
		userdata = s.userdata; 
		if (userdata) {
			RTLOG("  Assigning UserData: " << (void*)userdata << " type=" << userdata->GetTypeName() << " count=" << (int)userdata->refcount);
			userdata->Retain(); 
		}
	}
	else if (type == PY_BOUND_METHOD) {
		bound = s.bound;
		if (bound) {
			RTLOG("  Assigning BoundMethod: " << (void*)bound << " count=" << (int)bound->refcount);
			bound->Retain();
		}
	}
	else ptr = s.ptr;
}

PyValue::PyValue(PyIter *it)
{
	type = PY_ITERATOR;
	iter = it;
	if (iter) iter->Retain();
}

PyValue::PyValue(PyUserData *ud)
{
	type = PY_USERDATA;
	userdata = ud;
	if (userdata) userdata->Retain();
}

PyValue PyValue::UserDataNonOwning(PyUserData *ud)
{
	PyValue v;
	v.type = PY_USERDATA;
	v.userdata = ud;
	return v;
}

PyValue PyRangeIter::Next()
{
	if((step > 0 && current < stop) || (step < 0 && current > stop)) {
		int64 res = current;
		current += step;
		return PyValue(res);
	}
	return PyValue::StopIteration();
}

PyValue PyVectorIter::Next()
{
	if(i < v.GetCount())
		return v[i++];
	return PyValue::StopIteration();
}

PyValue::PyValue(std::complex<double> c)
{
	type = PY_COMPLEX;
	complex = new PyComplex;
	complex->c = c;
}

PyValue::PyValue(const char *s)
{
	type = PY_STR;
	wstr = new PyStr;
	wstr->s = s;
}

PyValue::PyValue(const WString& s)
{
	type = PY_STR;
	wstr = new PyStr;
	wstr->s = s;
}

PyValue::PyValue(const String& s)
{
	type = PY_STR;
	wstr = new PyStr;
	wstr->s = s.ToWString();
}

bool PyValue::IsTrue() const
{
	switch(type) {
	case PY_NONE: return false;
	case PY_BOOL: return b;
	case PY_INT: return i64 != 0;
	case PY_FLOAT: return f64 != 0.0;
	case PY_COMPLEX: return complex->c.real() != 0.0 || complex->c.imag() != 0.0;
	case PY_STR: return !wstr->s.IsEmpty();
	case PY_BYTES: return !bytes->s.IsEmpty();
	case PY_LIST: return !list->l.IsEmpty();
	case PY_TUPLE: return !tuple->l.IsEmpty();
	case PY_DICT: return !dict->d.IsEmpty();
	case PY_SET: return !set->s.IsEmpty();
	case PY_FUNCTION: return true;
	case PY_USERDATA: return true;
	case PY_BOUND_METHOD: return true;
	default: return false;
	}
}

std::complex<double> PyValue::GetComplex() const
{
	if(type == PY_COMPLEX) return complex->c;
	if(type == PY_FLOAT) return std::complex<double>(f64, 0.0);
	if(type == PY_INT) return std::complex<double>((double)i64, 0.0);
	return std::complex<double>(0.0, 0.0);
}

WString PyValue::GetStr() const
{
	if(type == PY_STR) return wstr->s;
	return ToString().ToWString();
}

String PyValue::GetBytes() const
{
	if(type == PY_BYTES) return bytes->s;
	return String();
}

int PyValue::GetCount() const
{
	switch(type) {
	case PY_LIST: return list->l.GetCount();
	case PY_TUPLE: return tuple->l.GetCount();
	case PY_DICT: return dict->d.GetCount();
	case PY_SET: return set->s.GetCount();
	case PY_STR: return wstr->s.GetCount();
	case PY_BYTES: return bytes->s.GetCount();
	default: return 0;
	}
}

PyValue PyValue::GetItem(int i) const
{
	if(type == PY_LIST) return list->l[i];
	if(type == PY_TUPLE) return tuple->l[i];
	if(type == PY_STR) {
		if(i >= 0 && i < wstr->s.GetCount())
			return PyValue(WString(wstr->s[i], 1));
	}
	return PyValue();
}

void PyValue::SetItem(int i, const PyValue& v)
{
	if(type == PY_LIST) list->l[i] = v;
}

void PyValue::Add(const PyValue& v)
{
	if(type == PY_LIST) list->l.Add(v);
}

PyValue PyValue::GetItem(const PyValue& key) const
{
	if(type == PY_DICT) return dict->d.Get(key, PyValue());
	if(type == PY_STR && key.IsInt()) return GetItem(key.AsInt());
	return PyValue();
}

void PyValue::SetItem(const PyValue& key, const PyValue& v)
{
	if(type == PY_DICT) dict->d.GetAdd(key) = v;
}

Value PyValue::ToValue() const
{
	switch(type) {
	case PY_NONE: return Value();
	case PY_BOOL: return b;
	case PY_INT: return i64;
	case PY_FLOAT: return f64;
	case PY_STR: return wstr->s.ToString();
	case PY_LIST: {
		ValueArray va;
		for(const auto& v : list->l) va.Add(v.ToValue());
		return va;
	}
	case PY_DICT: {
		ValueMap vm;
		for(int i = 0; i < dict->d.GetCount(); i++)
			vm.Add(dict->d.GetKey(i).ToValue(), dict->d[i].ToValue());
		return vm;
	}
	default: return Value();
	}
}

String PyValue::ToString() const
{
	switch(type) {
	case PY_NONE: return "None";
	case PY_BOOL: return b ? "True" : "False";
	case PY_INT: return IntStr64(i64);
	case PY_FLOAT: return FormatDouble(f64);
	case PY_COMPLEX: return Format("(%g + %g j)", complex->c.real(), complex->c.imag());
	case PY_STR: return wstr->s.ToString();
	case PY_BYTES: return Format("b'%s'", bytes->s); // Simple escape
	case PY_LIST: {
		String s = "[";
		for(int i = 0; i < list->l.GetCount(); i++) {
			if(i) s << ", ";
			s << list->l[i].ToString();
		}
		s << "]";
		return s;
	}
	case PY_TUPLE: {
		String s = "(";
		for(int i = 0; i < tuple->l.GetCount(); i++) {
			if(i) s << ", ";
			s << tuple->l[i].ToString();
		}
		if(tuple->l.GetCount() == 1) s << ",";
		s << ")";
		return s;
	}
	case PY_DICT: {
		String s = "{";
		for(int i = 0; i < dict->d.GetCount(); i++) {
			if(i) s << ", ";
			s << dict->d.GetKey(i).ToString() << ": " << dict->d[i].ToString();
		}
		s << "}";
		return s;
	}
	case PY_SET: {
		String s = "{";
		for(int i = 0; i < set->s.GetCount(); i++) {
			if(i) s << ", ";
			s << set->s[i].ToString();
		}
		s << "}";
		return s;
	}
	case PY_FUNCTION: return "<function " + lambda->name + ">";
	case PY_USERDATA: return "<userdata " + userdata->GetTypeName() + ">";
	case PY_BOUND_METHOD: return "<method " + bound->func.ToString() + ">";
	default: return "<unknown>";
	}
}

hash_t PyValue::GetHashValue() const
{
	CombineHash h;
	h << type;
	switch(type) {
	case PY_BOOL: h << b; break;
	case PY_INT: h << i64; break;
	case PY_FLOAT: h << f64; break;
	case PY_COMPLEX: h << complex->c.real() << complex->c.imag(); break;
	case PY_STR: h << ::Upp::GetHashValue(wstr->s); break;
	case PY_BYTES: h << ::Upp::GetHashValue(bytes->s); break;
	case PY_TUPLE:
		for(int i = 0; i < tuple->l.GetCount(); i++)
			h << tuple->l[i].GetHashValue();
		break;
	}
	return h;
}

bool PyValue::operator==(const PyValue& other) const
{
	if(type != other.type) {
		if(IsNumber() && other.IsNumber()) {
			if(type == PY_COMPLEX || other.type == PY_COMPLEX)
				return GetComplex() == other.GetComplex();
			return AsDouble() == other.AsDouble();
		}
		return false;
	}
	switch(type) {
	case PY_NONE: return true;
	case PY_BOOL: return b == other.b;
	case PY_INT: return i64 == other.i64;
	case PY_FLOAT: return f64 == other.f64;
	case PY_COMPLEX: return complex->c == other.complex->c;
	case PY_STR: return wstr->s == other.wstr->s;
	case PY_BYTES: return bytes->s == other.bytes->s;
	case PY_LIST: return list->l == other.list->l;
	case PY_TUPLE: return tuple->l == other.tuple->l;
	case PY_DICT: return dict->d == other.dict->d;
	case PY_SET: return set->s == other.set->s;
	case PY_FUNCTION: return lambda == other.lambda;
	default: return ptr == other.ptr;
	}
}

bool PyValue::operator<(const PyValue& other) const
{
	if(type != other.type) {
		if(IsNumber() && other.IsNumber()) {
			if(type == PY_COMPLEX || other.type == PY_COMPLEX)
				return false;
			return AsDouble() < other.AsDouble();
		}
		return type < other.type;
	}
	switch(type) {
	case PY_INT: return i64 < other.i64;
	case PY_FLOAT: return f64 < other.f64;
	case PY_STR: return wstr->s < other.wstr->s;
	case PY_BYTES: return bytes->s < other.bytes->s;
	case PY_LIST:
		for(int i = 0; i < min(list->l.GetCount(), other.list->l.GetCount()); i++) {
			if(list->l[i] < other.list->l[i]) return true;
			if(other.list->l[i] < list->l[i]) return false;
		}
		return list->l.GetCount() < other.list->l.GetCount();
	}
	return false;
}

const Vector<PyValue>& PyValue::GetArray() const
{
	if(type == PY_LIST) return list->l;
	if(type == PY_TUPLE) return tuple->l;
	static Vector<PyValue> empty;
	return empty;
}

PyValue PyValue::List()
{
	PyValue v;
	v.type = PY_LIST;
	v.list = new PyList;
	return v;
}

PyValue PyValue::Tuple()
{
	PyValue v;
	v.type = PY_TUPLE;
	v.tuple = new PyTuple;
	return v;
}

PyValue PyValue::Dict()
{
	PyValue v;
	v.type = PY_DICT;
	v.dict = new PyDict;
	v.dict->d.Clear();
	return v;
}

PyValue PyValue::Set()
{
	PyValue v;
	v.type = PY_SET;
	v.set = new PySet;
	v.set->s.Clear();
	return v;
}

PyValue PyValue::Function(const String& name, PyBuiltin builtin, void* user_data)
{
	PyValue v;
	v.type = PY_FUNCTION;
	v.lambda = new PyLambda;
	v.lambda->name = name;
	v.lambda->builtin = builtin;
	v.lambda->user_data = user_data;
	return v;
}

PyValue::PyValue(int type, void *ptr)
{
	Clear();
	this->type = type;
	this->ptr = ptr;
	if (type == PY_COMPLEX) complex->Retain();
	else if (type == PY_STR) wstr->Retain();
	else if (type == PY_BYTES) bytes->Retain();
	else if (type == PY_LIST) list->Retain();
	else if (type == PY_TUPLE) tuple->Retain();
	else if (type == PY_DICT) dict->Retain();
	else if (type == PY_SET) set->Retain();
	else if (type == PY_FUNCTION) lambda->Retain();
	else if (type == PY_ITERATOR) iter->Retain();
	else if (type == PY_USERDATA) userdata->Retain();
	else if (type == PY_BOUND_METHOD) bound->Retain();
}

PyValue PyValue::BoundMethod(const PyValue& func, const PyValue& self)
{
	RTLOG("PyValue::BoundMethod: func=" << func.ToString() << " self=" << self.ToString());
	PyValue v;
	v.type = PY_BOUND_METHOD;
	v.bound = new PyBoundMethod;
	v.bound->func = func;
	v.bound->self = self;
	return v;
}

}

#ifndef _Core_MetaTraits_Bindings_h_
#define _Core_MetaTraits_Bindings_h_

#include <ByteVM/ByteVM.h>

NAMESPACE_UPP

// Helper to register a builtin function to a dictionary (module or class)
inline void RegisterFunction(PyValue& dict, const char* name, PyBuiltin fn, void* user_data = nullptr) {
	dict.SetItem(PyValue(name), PyValue::Function(name, fn, user_data));
}

// Module registration
#define PY_MODULE(mod_name, vm) \
	PyValue mod_name##_obj = PyValue::Dict(); \
	(vm).GetGlobals().GetAdd(PyValue(#mod_name)) = mod_name##_obj; \
	PyValue& current_module = mod_name##_obj;

#define PY_MODULE_FUNC(name, fn, user_data) \
	RegisterFunction(current_module, #name, fn, user_data);

// Class registration
#define PY_CLASS(cls_name, type_name) \
	struct Py##cls_name : PyUserData { \
		cls_name val; \
		Py##cls_name() {} \
		Py##cls_name(const cls_name& v) : val(v) {} \
		String GetTypeName() const override { return type_name; } \
		static PyValue& GetClassDict() { \
			static PyValue dict = PyValue::Dict(); \
			return dict; \
		} \
		PyValue GetAttr(const String& name) override { \
			PyValue dict = GetClassDict(); \
			PyValue func = dict.GetItem(PyValue(name)); \
			if (!func.IsNone()) return PyValue::BoundMethod(func, PyValue((PyUserData*)this)); \
			return GetData(name); \
		} \
		virtual PyValue GetData(const String& name); \
	}; \
	static PyValue cls_name##_Ctor(const Vector<PyValue>& args, void* user_data);

#define PY_DATA_BEGIN(cls_name) \
	PyValue Py##cls_name::GetData(const String& name) {

#define PY_DATA(member) \
	if (name == #member) return PyValue((double)val.member);

#define PY_DATA_V(name_str, member) \
	if (name == name_str) return PyValue((double)val.member);

#define PY_DATA_END \
	return PyValue::None(); }

#define PY_DATA_EMPTY(cls_name) \
	PyValue Py##cls_name::GetData(const String& name) { return PyValue::None(); }

#define PY_CLASS_BIND(cls_name) \
	RegisterFunction(current_module, #cls_name, cls_name##_Ctor); \
	PyValue& current_class_dict = Py##cls_name::GetClassDict();

#define PY_METHOD(name, fn) \
	RegisterFunction(current_class_dict, #name, fn);

// Helper to get self from args in method
#define PY_SELF(cls_name) \
	if (args.GetCount() < 1 || !args[0].IsUserData()) return PyValue::None(); \
	cls_name& self = ((Py##cls_name&)args[0].GetUserData()).val;

END_UPP_NAMESPACE

#endif

#ifndef _ByteVM_EcmaScript_JsBindings_h_
#define _ByteVM_EcmaScript_JsBindings_h_

#include "JsVM.h"

NAMESPACE_UPP

inline void RegisterFunction(JsValue& obj, const char* name, JsBuiltin fn, void* user_data = nullptr) {
	obj.SetItem(JsValue(name), JsValue::Function(name, fn, user_data));
}

#define JS_MODULE(mod_name, vm) \
	JsValue mod_name##_obj = JsValue::Object(); \
	(vm).GetGlobals().SetItem(JsValue(#mod_name), mod_name##_obj); \
	JsValue& current_module = mod_name##_obj;

#define JS_MODULE_FUNC(name, fn, user_data) \
	RegisterFunction(current_module, #name, fn, user_data);

#define JS_CLASS(cls_name, type_name) \
	struct Js##cls_name : JsUserData { \
		cls_name val; \
		Js##cls_name() {} \
		Js##cls_name(const cls_name& v) : val(v) {} \
		String GetTypeName() const override { return type_name; } \
		static JsValue& GetClassProto() { \
			static JsValue proto = JsValue::Object(); \
			return proto; \
		} \
		JsValue GetAttr(const String& name) override { \
			JsValue proto = GetClassProto(); \
			JsValue func = proto.GetItem(JsValue(name)); \
			if (!func.IsUndefined()) return func; /* BoundMethod support needs to be added to JsValue */ \
			return GetData(name); \
		} \
		virtual JsValue GetData(const String& name); \
	}; \
	static JsValue cls_name##_Ctor(const Vector<JsValue>& args, void* user_data);

#define JS_DATA_BEGIN(cls_name) \
	JsValue Js##cls_name::GetData(const String& name) {

#define JS_DATA(member) \
	if (name == #member) return JsValue((double)val.member);

#define JS_DATA_END \
	return JsValue::Undefined(); }

#define JS_CLASS_BIND(cls_name) \
	RegisterFunction(current_module, #cls_name, cls_name##_Ctor); \
	JsValue& current_class_proto = Js##cls_name::GetClassProto();

#define JS_METHOD(name, fn) \
	RegisterFunction(current_class_proto, #name, fn);

#define JS_SELF(cls_name) \
	if (args.GetCount() < 1 || !args[0].IsUserData()) return JsValue::Undefined(); \
	cls_name& self = ((Js##cls_name&)args[0].GetUserData()).val;

END_UPP_NAMESPACE

#endif

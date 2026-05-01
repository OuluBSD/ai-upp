#ifndef _ByteVM_EcmaScript_JsValue_h_
#define _ByteVM_EcmaScript_JsValue_h_

#include "EcmaScript.h"

NAMESPACE_UPP

enum JsTypeKind {
	JS_UNDEFINED,
	JS_NULL,
	JS_BOOL,
	JS_NUMBER,
	JS_STR,
	JS_ARRAY,
	JS_OBJECT,
	JS_FUNCTION,
	JS_USERDATA,
};

class JsValue;

typedef JsValue (*JsBuiltin)(const Vector<JsValue>& args, void* user_data);

String JsTypeName(int type);

struct JsLambda;
struct JsUserData;

class JsValue : Moveable<JsValue> {
public:
	struct RefCount {
		Atomic refcount;
		RefCount() { refcount = 1; }
		void Retain() { AtomicInc(refcount); }
		bool Release() { if(AtomicDec(refcount) == 0) { delete this; return true; } return false; }
		virtual ~RefCount() {}
	};

private:
	struct JsStr : RefCount { WString s; };
	struct JsArray : RefCount { Vector<JsValue> l; };
	struct JsObject : RefCount { VectorMap<JsValue, JsValue> d; };

	union {
		bool          b;
		double        f64;
		JsStr        *wstr;
		JsArray      *array;
		JsObject     *object;
		JsLambda     *lambda;
		struct JsUserData *userdata;
		void         *ptr;
	};

	int type;

	void Free();
	void Assign(const JsValue& s);

public:
	void Clear() { type = JS_UNDEFINED; ptr = nullptr; }
	JsValue() { Clear(); }
	JsValue(bool b) { Clear(); type = JS_BOOL; this->b = b; }
	JsValue(double d) { Clear(); type = JS_NUMBER; f64 = d; }
	JsValue(int i) { Clear(); type = JS_NUMBER; f64 = i; }
	JsValue(int64 i) { Clear(); type = JS_NUMBER; f64 = (double)i; }
	JsValue(const char *s);
	JsValue(const WString& s);
	JsValue(const String& s);
	JsValue(struct JsUserData *ud);
	JsValue(int type, void *ptr);
	
	~JsValue() { Free(); }
	JsValue(const JsValue& src) { type = JS_UNDEFINED; Assign(src); }
	JsValue& operator=(const JsValue& s) { if(this != &s) { Free(); Assign(s); } return *this; }

	int GetType() const { return type; }
	bool IsUndefined() const { return type == JS_UNDEFINED; }
	bool IsNull() const { return type == JS_NULL; }
	bool IsBool() const { return type == JS_BOOL; }
	bool IsNumber() const { return type == JS_NUMBER; }
	bool IsString() const { return type == JS_STR; }
	bool IsArray() const { return type == JS_ARRAY; }
	bool IsObject() const { return type == JS_OBJECT; }
	bool IsFunction() const { return type == JS_FUNCTION; }
	bool IsUserData() const { return type == JS_USERDATA; }

	struct JsUserData& GetUserData() const { ASSERT(type == JS_USERDATA && userdata); return *userdata; }

	bool        AsBool() const { return IsTrue(); }
	double      AsDouble() const { return type == JS_NUMBER ? f64 : 0.0; }

	bool        IsTrue() const;
	WString     GetStr() const;
	String      GetString() const;

	// Array operations
	int         GetCount() const;
	JsValue     GetItem(int i) const;
	void        SetItem(int i, const JsValue& v);
	void        Add(const JsValue& v);

	// Object operations
	JsValue     GetItem(const JsValue& key) const;
	void        SetItem(const JsValue& key, const JsValue& v);

	const VectorMap<JsValue, JsValue>& GetObject() const { ASSERT(type == JS_OBJECT); return object->d; }
	VectorMap<JsValue, JsValue>& GetObjectRW() { ASSERT(type == JS_OBJECT); return object->d; }

	bool        Contains(const JsValue& v) const;

	Value       ToValue() const;
	String      ToString() const;

	hash_t      GetHashValue() const;

	bool operator==(const JsValue& other) const;
	bool operator!=(const JsValue& other) const { return !(*this == other); }
	bool operator<(const JsValue& other) const;

	static JsValue Undefined() { return JsValue(); }
	static JsValue Null();
	static JsValue Array();
	static JsValue Object();
	static JsValue UserData(JsUserData *ud);
	static JsValue Function(const String& name, JsBuiltin builtin = nullptr, void* user_data = nullptr);

	const JsLambda& GetLambda() const { ASSERT(type == JS_FUNCTION); return *lambda; }
};

struct JsUserData : JsValue::RefCount {
	virtual String GetTypeName() const = 0;
	virtual JsValue GetAttr(const String& name) { return JsValue::Undefined(); }
	virtual bool SetAttr(const String& name, const JsValue& v) { return false; }
	virtual ~JsUserData() {}
};

template <>
inline hash_t GetHashValue(const JsValue& v) { return v.GetHashValue(); }

END_UPP_NAMESPACE

#endif

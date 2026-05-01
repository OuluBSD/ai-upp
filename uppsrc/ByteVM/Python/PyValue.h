#ifndef _ByteVM_PyValue_h_
#define _ByteVM_PyValue_h_

#include <Core/Core.h>
#include <complex>

NAMESPACE_UPP

class PyValue;

typedef PyValue (*PyBuiltin)(const Vector<PyValue>& args, void* user_data);

// Undefine X11 macros that conflict with Python-style naming
#ifdef None
#undef None
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif

String PyTypeName(int type);

struct PyLambda;
struct PyUserData;
struct PyBoundMethod;
struct PyIR;

class PyValue : Moveable<PyValue> {
public:
	struct RefCount {
		Atomic refcount;
		RefCount() { refcount = 1; }
		void Retain() { AtomicInc(refcount); }
		bool Release() { if(AtomicDec(refcount) == 0) { delete this; return true; } return false; }
		virtual ~RefCount() {}
	};

private:
	struct PyComplex : RefCount { std::complex<double> c; };
	struct PyStr : RefCount { WString s; };
	struct PyBytes : RefCount { String s; };
	struct PyList : RefCount { Vector<PyValue> l; };
	struct PyTuple : RefCount { Vector<PyValue> l; };
	struct PyDict : RefCount { VectorMap<PyValue, PyValue> d; };
	struct PySet : RefCount { Index<PyValue> s; };

	union {
		bool          b;
		int64         i64;
		double        f64;
		PyComplex    *complex;
		PyStr        *wstr;
		PyBytes      *bytes;
		PyList       *list;
		PyTuple      *tuple;
		PyDict       *dict;
		PySet        *set;
		PyLambda     *lambda;
		struct PyIter *iter;
		struct PyUserData *userdata;
		struct PyBoundMethod *bound;
		void         *ptr;
	};

	int type;

	void Free();
	void Assign(const PyValue& s);

public:
	void Clear();
	PyValue() { Clear(); }
	PyValue(bool b);
	PyValue(int64 i);
	PyValue(int i);
	PyValue(double d);
	PyValue(std::complex<double> c);
	PyValue(const char *s);
	PyValue(const WString& s);
	PyValue(const String& s);
	PyValue(struct PyIter *it);
	PyValue(struct PyUserData *ud);
	PyValue(int type, void *ptr);
	
	~PyValue() { Free(); }
	PyValue(const PyValue& src) { type = 0; /* PY_NONE */ Clear(); Assign(src); }
	PyValue& operator=(const PyValue& s) { if(this != &s) { Free(); Assign(s); } return *this; }

	int GetType() const { return type; }
	void* GetPtr() const { return ptr; }
	bool IsNone() const;
	bool IsBool() const;
	bool IsInt() const;
	bool IsFloat() const;
	bool IsFunction() const;
	bool IsNumber() const;
	bool IsIterator() const;
	bool IsUserData() const;
	bool IsUserDataValid() const;
	bool IsBoundMethod() const;
	bool IsStopIteration() const;

	struct PyUserData& GetUserData() const { ASSERT(IsUserData() && userdata); return *userdata; }
	struct PyBoundMethod& GetBound() const { ASSERT(IsBoundMethod() && bound); return *bound; }
	struct PyIter&      GetIter() const { ASSERT(IsIterator() && iter); return *iter; }

	int64       AsInt64() const;
	int         AsInt() const { return (int)AsInt64(); }
	double      AsDouble() const;

	bool        IsTrue() const;
	std::complex<double> GetComplex() const;
	WString     GetStr() const;
	String      GetBytes() const;

	// List/Tuple operations
	int         GetCount() const;
	PyValue     GetItem(int i) const;
	void        SetItem(int i, const PyValue& v);
	void        Add(const PyValue& v);

	const Vector<PyValue>& GetArray() const;

	// Dict operations
	PyValue     GetItem(const PyValue& key) const;
	void        SetItem(const PyValue& key, const PyValue& v);

	const VectorMap<PyValue, PyValue>& GetDict() const;
	VectorMap<PyValue, PyValue>& GetDictRW();

	bool        Contains(const PyValue& v) const;

	Value  ToValue() const;
	String ToString() const;
	String Repr() const;
	hash_t GetHashValue() const;

	bool operator==(const PyValue& other) const;
	bool operator!=(const PyValue& other) const { return !(*this == other); }
	bool operator<(const PyValue& other) const;
	bool IsSameObject(const PyValue& other) const;

	static PyValue None();
	static PyValue True();
	static PyValue False();
	static PyValue List();
	static PyValue Tuple();
	static PyValue Dict();
	static PyValue Set();
	static PyValue Iterator(struct PyIter *it);
	static PyValue UserData(struct PyUserData *ud);
	static PyValue Function(const String& name, PyBuiltin builtin = nullptr, void* user_data = nullptr);
	static PyValue UserDataNonOwning(struct PyUserData *ud);
	static PyValue BoundMethod(const PyValue& func, const PyValue& self);
	static PyValue StopIteration();
	static PyValue FromValue(const Value& v);
	static PyValue FromVector(const Vector<PyValue>& v, bool tuple = false);

	const PyLambda& GetLambda() const;
	PyLambda&       GetLambdaRW();
};

struct PyIR : Moveable<PyIR> {
	int code;
	PyValue arg;
	int iarg;
	int line;
	String file;

	PyIR();
	PyIR(int code, int line = 0, String file = String());
	PyIR(int code, int iarg, int line = 0, String file = String());
	PyIR(int code, PyValue arg, int line = 0, String file = String());

	static PyIR Const(const PyValue& v, int l = 0, String f = String());
	
	String ToString() const;
};

struct PyLambda : PyValue::RefCount {
	String         name;
	Vector<String> arg;
	Vector<PyValue> arg_values;
	VectorMap<String, PyValue> defaults;
	Vector<PyIR>   ir;
	PyBuiltin      builtin = nullptr;
	void          *user_data = nullptr;
	PyValue        globals; // The module-level globals dictionary
	PyValue        closure; // Captured locals from enclosing scope (for closures)
};

struct PyBoundMethod : PyValue::RefCount {
	PyValue func;
	PyValue self;
};

struct PyUserData : PyValue::RefCount {
	virtual String GetTypeName() const = 0;
	virtual PyValue GetAttr(const String& name) { return PyValue::None(); }
	virtual bool SetAttr(const String& name, const PyValue& v) { return false; }
	virtual ~PyUserData() {}
};

struct PyIter : PyValue::RefCount {
	virtual PyValue Next() = 0;
	virtual ~PyIter() {}
};

struct PyRangeIter : PyIter {
	int64 current, stop, step;
	PyRangeIter(int64 start, int64 stop, int64 step) : current(start), stop(stop), step(step) {}
	virtual PyValue Next() override;
};

struct PyVectorIter : PyIter {
	PyValue v;
	int i = 0;
	PyVectorIter(const PyValue& v) : v(v) {}
	virtual PyValue Next() override;
};

template <>
inline hash_t GetHashValue(const PyValue& v) { return v.GetHashValue(); }

END_UPP_NAMESPACE

#endif

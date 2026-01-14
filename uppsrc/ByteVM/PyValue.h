#ifndef _ByteVM_PyValue_h_
#define _ByteVM_PyValue_h_

#include <Core/Core.h>
#include <complex>

NAMESPACE_UPP

enum PyTypeKind {
	PY_NONE,
	PY_BOOL,
	PY_INT,
	PY_FLOAT,
	PY_COMPLEX,
	PY_STR,
	PY_BYTES,
	PY_LIST,
	PY_TUPLE,
	PY_DICT,
	PY_SET,
	PY_FUNCTION,
	PY_ITERATOR,
	PY_USERDATA,
	PY_BOUND_METHOD,
	PY_STOP_ITERATION
};

class PyValue;

typedef PyValue (*PyBuiltin)(const Vector<PyValue>& args, void* user_data);

String PyTypeName(int type);

struct PyLambda;
struct PyUserData;
struct PyBoundMethod;

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
	void Clear() { type = PY_NONE; ptr = nullptr; }
	PyValue() { Clear(); }
	PyValue(bool b) { Clear(); type = PY_BOOL; this->b = b; }
	PyValue(int64 i) { Clear(); type = PY_INT; i64 = i; }
	PyValue(int i) { Clear(); type = PY_INT; i64 = i; }
	PyValue(double d) { Clear(); type = PY_FLOAT; f64 = d; }
	PyValue(std::complex<double> c);
	PyValue(const char *s);
	PyValue(const WString& s);
	PyValue(const String& s);
	PyValue(struct PyIter *it);
	PyValue(struct PyUserData *ud);
	PyValue(int type, void *ptr);
	
	~PyValue() { Free(); }
	PyValue(const PyValue& src) { type = PY_NONE; Assign(src); }
	PyValue& operator=(const PyValue& s) { if(this != &s) { Free(); Assign(s); } return *this; }

	int GetType() const { return type; }
	void* GetPtr() const { return ptr; }
	bool IsNone() const { return type == PY_NONE; }
	bool IsBool() const { return type == PY_BOOL; }
	bool IsInt() const { return type == PY_INT; }
	bool IsFloat() const { return type == PY_FLOAT; }
	bool IsFunction() const { return type == PY_FUNCTION; }
	bool IsNumber() const { return type == PY_INT || type == PY_FLOAT || type == PY_COMPLEX; }
	bool IsIterator() const { return type == PY_ITERATOR; }
	bool IsUserData() const { return type == PY_USERDATA; }
	bool IsUserDataValid() const { return type == PY_USERDATA && userdata; }
	bool IsBoundMethod() const { return type == PY_BOUND_METHOD; }
	bool IsStopIteration() const { return type == PY_STOP_ITERATION; }

	struct PyUserData& GetUserData() const { ASSERT(type == PY_USERDATA && userdata); return *userdata; }
	struct PyBoundMethod& GetBound() const { ASSERT(type == PY_BOUND_METHOD && bound); return *bound; }
	struct PyIter&      GetIter() const { ASSERT(type == PY_ITERATOR && iter); return *iter; }

	int64       AsInt64() const { return type == PY_INT ? i64 : (type == PY_FLOAT ? (int64)f64 : 0); }
	int         AsInt() const { return (int)AsInt64(); }
	double      AsDouble() const { return type == PY_FLOAT ? f64 : (type == PY_INT ? (double)i64 : 0.0); }

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

	const VectorMap<PyValue, PyValue>& GetDict() const { ASSERT(type == PY_DICT); return dict->d; }

	Value       ToValue() const;
	String      ToString() const;
	hash_t      GetHashValue() const;

	bool operator==(const PyValue& other) const;
	bool operator!=(const PyValue& other) const { return !(*this == other); }
	bool operator<(const PyValue& other) const;

	static PyValue None() { return PyValue(); }
	static PyValue True() { return PyValue(true); }
	static PyValue False() { return PyValue(false); }
	static PyValue List();
	static PyValue Tuple();
	static PyValue Dict();
	static PyValue Set();
	static PyValue Function(const String& name, PyBuiltin builtin = nullptr, void* user_data = nullptr);
	static PyValue UserDataNonOwning(PyUserData *ud);
	static PyValue BoundMethod(const PyValue& func, const PyValue& self);
	static PyValue StopIteration() { PyValue v; v.type = PY_STOP_ITERATION; return v; }

	const PyLambda& GetLambda() const { ASSERT(type == PY_FUNCTION); return *lambda; }
	PyLambda&       GetLambdaRW() { ASSERT(type == PY_FUNCTION); return *lambda; }
};

struct PyUserData : PyValue::RefCount {
	virtual String GetTypeName() const = 0;
	virtual PyValue GetAttr(const String& name) { return PyValue::None(); }
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
	const Vector<PyValue>& v;
	int i = 0;
	PyVectorIter(const Vector<PyValue>& v) : v(v) {}
	virtual PyValue Next() override;
};

template <>
inline hash_t GetHashValue(const PyValue& v) { return v.GetHashValue(); }

END_UPP_NAMESPACE

#endif

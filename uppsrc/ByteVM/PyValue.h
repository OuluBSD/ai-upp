#ifndef _ByteVM_PyValue_h_
#define _ByteVM_PyValue_h_

#include <Core/Core.h>
#include <complex>

namespace Upp {

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
	PY_STOP_ITERATION
};

typedef PyValue (*PyBuiltin)(const Vector<PyValue>& args);

String PyTypeName(int type);

struct PyLambda;
class PyValue;

struct PyIter : RefCount {
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
		PyIter       *iter;
		void         *ptr;
	};

	int type;

	void Free();
	void Assign(const PyValue& s);

public:
	PyValue() { type = PY_NONE; ptr = nullptr; }
	PyValue(bool b) { type = PY_BOOL; this->b = b; }
	PyValue(int64 i) { type = PY_INT; i64 = i; }
	PyValue(int i) { type = PY_INT; i64 = i; }
	PyValue(double d) { type = PY_FLOAT; f64 = d; }
	PyValue(std::complex<double> c);
	PyValue(const char *s);
	PyValue(const WString& s);
	PyValue(const String& s);
	PyValue(PyIter *it);
	
	~PyValue() { Free(); }
	PyValue(const PyValue& src) { type = PY_NONE; Assign(src); }
	PyValue& operator=(const PyValue& s) { if(this != &s) { Free(); Assign(s); } return *this; }

	int GetType() const { return type; }
	bool IsNone() const { return type == PY_NONE; }
	bool IsBool() const { return type == PY_BOOL; }
	bool IsInt() const { return type == PY_INT; }
	bool IsFloat() const { return type == PY_FLOAT; }
	bool IsNumber() const { return type == PY_INT || type == PY_FLOAT || type == PY_COMPLEX; }
	bool IsIterator() const { return type == PY_ITERATOR; }
	bool IsStopIteration() const { return type == PY_STOP_ITERATION; }

	bool IsTrue() const;
	
	int64       GetInt() const { return type == PY_INT ? i64 : 0; }
	double      GetFloat() const { return type == PY_FLOAT ? f64 : (type == PY_INT ? (double)i64 : 0.0); }
	std::complex<double> GetComplex() const;
	WString     GetStr() const;
	String      GetBytes() const;
	PyIter&     GetIter() const { ASSERT(type == PY_ITERATOR); return *iter; }

	// List/Tuple operations
	int         GetCount() const;
	PyValue     GetItem(int i) const;
	void        SetItem(int i, const PyValue& v);
	void        Add(const PyValue& v);

	const Vector<PyValue>& GetArray() const;

	// Dict operations
	PyValue     GetItem(const PyValue& key) const;
	void        SetItem(const PyValue& key, const PyValue& v);

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
	static PyValue Function(const String& name);
	static PyValue StopIteration() { PyValue v; v.type = PY_STOP_ITERATION; return v; }

	const PyLambda& GetLambda() const { ASSERT(type == PY_FUNCTION); return *lambda; }
	PyLambda&       GetLambdaRW() { ASSERT(type == PY_FUNCTION); return *lambda; }
};

template <>
inline hash_t GetHashValue(const PyValue& v) { return v.GetHashValue(); }

}

#endif
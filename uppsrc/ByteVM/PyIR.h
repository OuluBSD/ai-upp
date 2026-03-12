#ifndef _ByteVM_PyIR_h_
#define _ByteVM_PyIR_h_

NAMESPACE_UPP

struct PyIR : Moveable<PyIR> {
	int code;
	PyValue arg;
	int iarg;
	int line;
	String file;

	PyIR() : code(PY_NOP), iarg(0), line(0) {}
	PyIR(int code, int line = 0, String file = String()) : code(code), iarg(0), line(line), file(file) {}
	PyIR(int code, int iarg, int line = 0, String file = String()) : code(code), iarg(iarg), line(line), file(file) {}
	PyIR(int code, PyValue arg, int line = 0, String file = String()) : code(code), arg(arg), iarg(0), line(line), file(file) {}

	static PyIR Const(const PyValue& v, int l = 0, String f = String()) {
		PyIR ir(PY_LOAD_CONST, 0, l, f);
		ir.arg = v;
		return ir;
	}
	
	String ToString() const;
};

struct PyLambda : PyValue::RefCount {
	String         name;
	Vector<String> arg;
	Vector<PyValue> arg_values;
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

END_UPP_NAMESPACE

#endif

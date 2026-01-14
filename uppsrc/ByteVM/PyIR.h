#ifndef _ByteVM_PyIR_h_
#define _ByteVM_PyIR_h_

NAMESPACE_UPP

struct PyIR : Moveable<PyIR> {
	int code;
	PyValue arg;
	int iarg;
	int line;

	PyIR() : code(PY_NOP), iarg(0), line(0) {}
	PyIR(int code, int line = 0) : code(code), iarg(0), line(line) {}
	PyIR(int code, int iarg, int line = 0) : code(code), iarg(iarg), line(line) {}
	PyIR(int code, PyValue arg, int line = 0) : code(code), arg(arg), iarg(0), line(line) {}

	static PyIR Const(const PyValue& v, int l = 0) {
		PyIR ir(PY_LOAD_CONST, 0, l);
		ir.arg = v;
		return ir;
	}
	
	String ToString() const;
};

struct PyLambda : PyValue::RefCount {
	String         name;
	Vector<String> arg;
	Vector<PyIR>   ir;
	PyBuiltin      builtin = nullptr;
	void          *user_data = nullptr;
};

struct PyBoundMethod : PyValue::RefCount {
	PyValue func;
	PyValue self;
};

END_UPP_NAMESPACE

#endif

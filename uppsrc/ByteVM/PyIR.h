#ifndef _ByteVM_PyIR_h_
#define _ByteVM_PyIR_h_

namespace Upp {

struct PyIR : Moveable<PyIR> {
	int     code;
	PyValue arg;
	int     iarg; 
	int     line;

	PyIR() : code(PY_NOP), iarg(0), line(0) {}
	PyIR(int c, int l = 0) : code(c), iarg(0), line(l) {}
	PyIR(int c, int i, int l) : code(c), iarg(i), line(l) {}
	
	static PyIR Const(const PyValue& v, int l = 0) {
		PyIR ir(PY_LOAD_CONST, l);
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
	void*          user_data = nullptr;
};

struct PyBoundMethod : PyValue::RefCount {
	PyValue func;
	PyValue self;
};
}

#endif
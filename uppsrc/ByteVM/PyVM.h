#ifndef _ByteVM_PyVM_h_
#define _ByteVM_PyVM_h_

#include "PyIR.h"

namespace Upp {

class PyVM {
public:
	struct Frame {
		const Vector<PyIR> *ir;
		PyValue func;
		int pc = 0;
		VectorMap<PyValue, PyValue> locals;
	};

private:
	Array<Frame> frames;
	Vector<PyValue> stack;
	VectorMap<PyValue, PyValue> globals;

	void Push(const PyValue& v) { stack.Add(v); }
	PyValue Pop() { if(stack.IsEmpty()) return PyValue(); PyValue v = stack.Top(); stack.Drop(); return v; }

	Frame& TopFrame() { return frames.Top(); }

public:
	PyVM();
	
	void SetIR(Vector<PyIR>& _ir);
	void Run();
	
	PyValue GetGlobal(const String& name) { return globals.Get(PyValue(name), PyValue()); }
	void SetGlobal(const String& name, const PyValue& v) { globals.GetAdd(PyValue(name)) = v; }
	
	PyValue Call(const PyValue& func, Vector<PyValue>& args);
};

}

#endif

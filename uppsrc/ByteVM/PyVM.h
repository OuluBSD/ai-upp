#ifndef _ByteVM_PyVM_h_
#define _ByteVM_PyVM_h_

NAMESPACE_UPP

class PyVM {
	struct Frame : Moveable<Frame> {
		PyValue func;
		const Vector<PyIR>* ir;
		int pc;
		VectorMap<PyValue, PyValue> locals;
	};
	
	Vector<Frame> frames;
	Vector<PyValue> stack;
	VectorMap<PyValue, PyValue> globals;

	Frame& TopFrame() { return frames.Top(); }
	void Push(PyValue v) { stack.Add(v); }
	PyValue Pop() { return stack.Pop(); }

public:
	PyVM();
	
	void SetIR(Vector<PyIR>& ir);
	void Run();
	
	VectorMap<PyValue, PyValue>& GetGlobals() { return globals; }
};

END_UPP_NAMESPACE

#endif
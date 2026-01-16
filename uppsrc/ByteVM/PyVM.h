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
	PyValue Run();
	
	VectorMap<PyValue, PyValue>& GetGlobals() { return globals; }
	
	bool    Step();
	PyValue GetLastResult() const { return last_result; }
	bool    IsRunning() const { return !frames.IsEmpty(); }
	int     GetFramesCount() const { return frames.GetCount(); }

private:
	PyValue last_result = PyValue::None();
};

END_UPP_NAMESPACE

#endif

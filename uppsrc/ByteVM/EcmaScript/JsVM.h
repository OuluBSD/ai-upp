#ifndef _ByteVM_EcmaScript_JsVM_h_
#define _ByteVM_EcmaScript_JsVM_h_

#include "JsValue.h"
#include <ByteVM/Python/PyIR.h>

NAMESPACE_UPP

class JsVM {
	struct Frame : Moveable<Frame> {
		JsValue func;
		const Vector<PyIR>* ir;
		int pc;
		VectorMap<JsValue, JsValue> locals;
		JsValue globals;
		int     stack_base = 0;

		struct ExceptHandler {
			int handler_pc;
			int stack_depth;
		};
		Vector<ExceptHandler> except_stack;
	};
	
	Vector<Frame> frames;
	Vector<JsValue> stack;
	JsValue globals;

	Frame& TopFrame() { return frames.Top(); }
	void Push(JsValue v);
	JsValue Pop();

public:
	JsVM();
	~JsVM();
	void Clear();

	void SetIR(const Vector<PyIR>& ir);
	JsValue Run();
	JsValue Call(const JsValue& callable, const Vector<JsValue>& args);

	bool    Step();
	JsValue GetLastResult() const { return last_result; }
	bool    IsRunning() const { return !frames.IsEmpty(); }

	JsValue  GetGlobals()    { return globals; }
	JsValue& GetGlobalsRW()  { return globals; }

	Event<const String&> WhenPrint;

private:
	JsValue last_result = JsValue::Undefined();
};

END_UPP_NAMESPACE

#endif

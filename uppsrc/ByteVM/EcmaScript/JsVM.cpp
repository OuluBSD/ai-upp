#include "JsVM.h"

NAMESPACE_UPP

JsVM::JsVM()
{
	globals = JsValue::Object();
}

JsVM::~JsVM()
{
}

void JsVM::Clear()
{
	frames.Clear();
	stack.Clear();
}

void JsVM::Push(JsValue v)
{
	stack.Add(v);
}

JsValue JsVM::Pop()
{
	if(stack.IsEmpty()) return JsValue::Undefined();
	return stack.Pop();
}

void JsVM::SetIR(const Vector<PyIR>& ir)
{
	Clear();
	Frame& f = frames.Add();
	f.ir = &ir;
	f.pc = 0;
	f.globals = globals;
}

JsValue JsVM::Run()
{
	while(Step());
	return last_result;
}

bool JsVM::Step()
{
	if(frames.IsEmpty()) return false;
	
	Frame& f = TopFrame();
	if(f.pc >= f.ir->GetCount()) {
		frames.Pop();
		return !frames.IsEmpty();
	}
	
	const PyIR& ir = (*f.ir)[f.pc++];
	
	switch(ir.code) {
	case JS_NOP:
		break;
	case JS_LOAD_CONST:
		// Map PyValue in ir.arg to JsValue
		if (ir.arg.IsNone()) Push(JsValue::Undefined());
		else if (ir.arg.IsBool()) Push(JsValue(ir.arg.IsTrue()));
		else if (ir.arg.IsNumber()) Push(JsValue(ir.arg.AsDouble()));
		else if (ir.arg.IsFunction()) {
			// This is tricky, need to wrap PyLambda/JsLambda
			Push(JsValue::Undefined());
		}
		else Push(JsValue(ir.arg.ToString()));
		break;
	case JS_LOAD_NAME:
	case JS_LOAD_GLOBAL:
		{
			JsValue name(ir.arg.ToString());
			if (f.locals.Find(name) >= 0) Push(f.locals.Get(name));
			else Push(f.globals.GetItem(name));
		}
		break;
	case JS_STORE_NAME:
		{
			JsValue val = Pop();
			f.locals.GetAdd(JsValue(ir.arg.ToString())) = val;
		}
		break;
	case JS_BINARY_ADD:
		{
			JsValue b = Pop();
			JsValue a = Pop();
			if(a.IsString() || b.IsString())
				Push(JsValue(a.ToString() + b.ToString()));
			else
				Push(JsValue(a.AsDouble() + b.AsDouble()));
		}
		break;
	case JS_RETURN_VALUE:
		last_result = Pop();
		frames.Pop();
		return !frames.IsEmpty();
	// ... implementation of more opcodes ...
	default:
		// throw Exc(Format("Unknown opcode %d", ir.code));
		break;
	}
	
	return true;
}

JsValue JsVM::Call(const JsValue& callable, const Vector<JsValue>& args)
{
	// Placeholder
	return JsValue::Undefined();
}

END_UPP_NAMESPACE

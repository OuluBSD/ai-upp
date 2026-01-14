#include "ByteVM.h"
#include <Core/Core.h>

namespace Upp {

struct PyKV : Moveable<PyKV> {
	PyValue k, v;
	PyKV() {}
	PyKV(PyValue k, PyValue v) : k(k), v(v) {}
};

static PyValue BuiltinPrint(const Vector<PyValue>& args, void*)
{
	for(int i = 0; i < args.GetCount(); i++) {
		if(i) Cout() << " ";
		Cout() << args[i].ToString();
	}
	Cout() << "\n";
	return PyValue::None();
}

static PyValue BuiltinLen(const Vector<PyValue>& args, void*)
{
	if(args.GetCount() != 1) return PyValue::None();
	return PyValue((int64)args[0].GetCount());
}

static PyValue BuiltinRange(const Vector<PyValue>& args, void*)
{
	int64 start = 0, stop = 0, step = 1;
	if(args.GetCount() == 1) stop = args[0].GetInt();
	else if(args.GetCount() == 2) { start = args[0].GetInt(); stop = args[1].GetInt(); }
	else if(args.GetCount() == 3) { start = args[0].GetInt(); stop = args[1].GetInt(); step = args[2].GetInt(); }
	
	if(step == 0) return PyValue::None();
	return PyValue(new PyRangeIter(start, stop, step));
}

PyVM::PyVM()
{
	PyValue p_print = PyValue::Function("print");
	p_print.GetLambdaRW().builtin = BuiltinPrint;
	globals.GetAdd(PyValue("print")) = p_print;

	PyValue p_len = PyValue::Function("len");
	p_len.GetLambdaRW().builtin = BuiltinLen;
	globals.GetAdd(PyValue("len")) = p_len;
	
	PyValue p_range = PyValue::Function("range");
	p_range.GetLambdaRW().builtin = BuiltinRange;
	globals.GetAdd(PyValue("range")) = p_range;
}

void PyVM::SetIR(Vector<PyIR>& _ir)
{
	frames.Clear();
	Frame& f = frames.Add();
	f.func = PyValue::Function("__main__");
	f.func.GetLambdaRW().ir = pick(_ir);
	f.ir = &f.func.GetLambda().ir;
	f.pc = 0;
}

void PyVM::Run()
{
	if (!frames.IsEmpty()) {
		const auto& ir = *TopFrame().ir;
		LOG("PyVM::Run: total instructions=" << ir.GetCount());
		for (int i = 0; i < ir.GetCount(); i++)
			LOG(Format("[%d] code=%d", i, (int)ir[i].code));
	}
	while(!frames.IsEmpty()) {
		Frame& frame = TopFrame();
		if(frame.pc >= frame.ir->GetCount()) {
			frames.Drop();
			continue;
		}
		
		const PyIR& instr = (*frame.ir)[frame.pc++];
		LOG(Format("PC %d: opcode %d", frame.pc - 1, (int)instr.code));
		
		switch(instr.code) {
		case PY_NOP: break;
		
		case PY_UNARY_POSITIVE: break; // Identity
		
		case PY_UNARY_NEGATIVE: {
			PyValue v = Pop();
			if (v.IsInt()) Push(PyValue(-v.GetInt()));
			else if (v.IsFloat()) Push(PyValue(-v.GetFloat()));
			else Push(PyValue::None());
			break;
		}

		case PY_UNARY_NOT: {
			PyValue v = Pop();
			Push(PyValue(!v.IsTrue()));
			break;
		}

		case PY_UNARY_INVERT: {
			PyValue v = Pop();
			if (v.IsInt()) Push(PyValue(~v.GetInt()));
			else Push(PyValue::None());
			break;
		}

		case PY_POP_TOP:
			Pop();
			break;

		case PY_LOAD_CONST:
			Push(instr.arg);
			break;
			
		case PY_LOAD_NAME: {
			PyValue v = frame.locals.Get(instr.arg, PyValue::None());
			if(v.IsNone()) v = globals.Get(instr.arg, PyValue::None());
			Push(v);
			break;
		}
		
		case PY_STORE_NAME:
			if(frames.GetCount() <= 1)
				globals.GetAdd(instr.arg) = Pop();
			else
				frame.locals.GetAdd(instr.arg) = Pop();
			break;

		case PY_LOAD_GLOBAL:
			Push(globals.Get(instr.arg, PyValue::None()));
			break;
			
		case PY_STORE_GLOBAL:
			globals.GetAdd(instr.arg) = Pop();
			break;

		case PY_LOAD_ATTR: {
			PyValue obj = Pop();
			LOG("PY_LOAD_ATTR: obj type=" << obj.GetType() << " ptr=" << obj.GetPtr() << " attr=" << instr.arg.ToString());
			if (obj.GetType() == PY_DICT) {
				Push(obj.GetItem(instr.arg));
			} else if (obj.IsUserData()) {
				Push(obj.GetUserData().GetAttr(instr.arg.ToString()));
			} else {
				Push(PyValue::None());
			}
			break;
		}

		case PY_STORE_ATTR: {
			PyValue val = Pop();
			PyValue obj = Pop();
			if (obj.GetType() == PY_DICT) {
				obj.SetItem(instr.arg, val);
			} // TODO: support UserData store
			break;
		}
			
		case PY_BUILD_LIST: {
			int n = instr.iarg;
			PyValue list = PyValue::List();
			Vector<PyValue> items;
			for(int i = 0; i < n; i++) items.Add(Pop());
			for(int i = n - 1; i >= 0; i--) list.Add(items[i]);
			Push(list);
			break;
		}
		
		case PY_BUILD_MAP: {
			int n = instr.iarg;
			PyValue dict = PyValue::Dict();
			Vector<PyKV> items;
			for(int i = 0; i < n; i++) {
				PyValue v = Pop();
				PyValue k = Pop();
				items.Add(PyKV(k, v));
			}
			for(int i = n - 1; i >= 0; i--) dict.SetItem(items[i].k, items[i].v);
			Push(dict);
			break;
		}
			
		case PY_BINARY_ADD: {
			PyValue b = Pop();
			PyValue a = Pop();
			if(a.IsInt() && b.IsInt()) Push(PyValue(a.GetInt() + b.GetInt()));
			else if(a.IsNumber() && b.IsNumber()) Push(PyValue(a.GetFloat() + b.GetFloat()));
			else if(a.GetType() == PY_STR && b.GetType() == PY_STR) Push(PyValue(a.GetStr() + b.GetStr()));
			else Push(PyValue::None());
			break;
		}
		
		case PY_BINARY_SUBTRACT: {
			PyValue b = Pop();
			PyValue a = Pop();
			if(a.IsInt() && b.IsInt()) Push(PyValue(a.GetInt() - b.GetInt()));
			else Push(PyValue(a.GetFloat() - b.GetFloat()));
			break;
		}

		case PY_BINARY_MULTIPLY: {
			PyValue b = Pop();
			PyValue a = Pop();
			if(a.IsInt() && b.IsInt()) Push(PyValue(a.GetInt() * b.GetInt()));
			else Push(PyValue(a.GetFloat() * b.GetFloat()));
			break;
		}

		case PY_BINARY_TRUE_DIVIDE: {
			PyValue b = Pop();
			PyValue a = Pop();
			double divisor = b.GetFloat();
			if(divisor == 0) Push(PyValue::None()); 
			else Push(PyValue(a.GetFloat() / divisor));
			break;
		}

		case PY_BINARY_SUBSCR: {
			PyValue sub = Pop();
			PyValue container = Pop();
			if(sub.IsInt() && (container.GetType() == PY_LIST || container.GetType() == PY_TUPLE))
				Push(container.GetItem((int)sub.GetInt()));
			else
				Push(container.GetItem(sub));
			break;
		}

		case PY_COMPARE_OP: {
			PyValue b = Pop();
			PyValue a = Pop();
			bool res = false;
			switch(instr.iarg) {
			case PY_CMP_EQ: res = (a == b); break;
			case PY_CMP_NE: res = (a != b); break;
			case PY_CMP_LT: res = (a < b); break;
			case PY_CMP_LE: res = (a < b || a == b); break;
			case PY_CMP_GT: res = (b < a); break;
			case PY_CMP_GE: res = (b < a || a == b); break;
			}
			Push(PyValue(res));
			break;
		}

		case PY_JUMP_ABSOLUTE:
			frame.pc = instr.iarg;
			break;
			
		case PY_POP_JUMP_IF_FALSE: {
			PyValue v = Pop();
			if(!v.IsTrue()) frame.pc = instr.iarg;
			break;
		}

		case PY_JUMP_IF_FALSE_OR_POP: {
			if(!stack.Top().IsTrue()) frame.pc = instr.iarg;
			else Pop();
			break;
		}

		case PY_JUMP_IF_TRUE_OR_POP: {
			if(stack.Top().IsTrue()) frame.pc = instr.iarg;
			else Pop();
			break;
		}

		case PY_CALL_FUNCTION: {
			int nargs = instr.iarg;
			Vector<PyValue> args;
			for(int i = 0; i < nargs; i++) args.Add(Pop());
			Vector<PyValue> sorted_args;
			for(int i = nargs - 1; i >= 0; i--) sorted_args.Add(args[i]);
			
			PyValue callable = Pop();
			if (callable.IsBoundMethod()) {
				sorted_args.Insert(0, callable.GetBound().self);
				callable = callable.GetBound().func;
			}

			if(callable.GetType() == PY_FUNCTION) {
				const PyLambda& l = callable.GetLambda();
				if(l.builtin) {
					Push(l.builtin(sorted_args, l.user_data));
				}
				else {
					Frame& f = frames.Add();
					f.func = callable;
					f.ir = &l.ir;
					f.pc = 0;
					for(int i = 0; i < min(l.arg.GetCount(), sorted_args.GetCount()); i++)
						f.locals.GetAdd(PyValue(l.arg[i])) = sorted_args[i];
				}
			}
			else {
				Push(PyValue::None());
			}
			break;
		}

		case PY_GET_ITER: {
			PyValue obj = Pop();
			if(obj.GetType() == PY_LIST || obj.GetType() == PY_TUPLE) {
				Push(PyValue(new PyVectorIter(obj.GetArray())));
			}
			else if(obj.IsIterator()) {
				Push(obj);
			}
			else {
				Push(PyValue::None());
			}
			break;
		}
		
		case PY_FOR_ITER: {
			PyValue iterator = Pop();
			if(iterator.IsIterator()) {
				PyValue next_val = iterator.GetIter().Next();
				if(next_val.IsStopIteration()) {
					frame.pc = instr.iarg;
				}
				else {
					Push(iterator);
					Push(next_val);
				}
			}
			else {
				Push(PyValue::None());
			}
			break;
		}

		case PY_RETURN_VALUE: {
			PyValue ret = Pop();
			frames.Drop();
			if(!frames.IsEmpty()) Push(ret);
			break;
		}
			
		default:
			
			Cout() << "Unknown opcode: " << (int)instr.code << " at PC " << frame.pc - 1 << "\n";
			
			return;
			
		}
			

	}
}

}

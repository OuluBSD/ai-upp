#include "ByteVM.h"
#include "PyScheduler.h"

NAMESPACE_UPP

void PyScheduler::AddVM(PyVM& vm)
{
	Mutex::Lock __(mutex);
	vms.Add(&vm);
}

static void NativeThreadEntry(PyVM *vm)
{
	try {
		vm->Run();
	} catch (Exc& e) {
		Cout() << "Thread error: " << e << "\n";
	}
	delete vm;
}

PyValue PyScheduler::CreateThread(PyValue func, Vector<PyValue>&& args)
{
	if(!func.IsFunction()) return PyValue::None();
	
	const PyLambda& l = func.GetLambda();
	
	if(mode == PYTHREAD_NATIVE) {
		PyVM *new_vm = new PyVM();
		
		Vector<PyIR> ir;
		ir.Add(PyIR(PY_LOAD_CONST, func));
		for(const auto& arg : args) ir.Add(PyIR(PY_LOAD_CONST, arg));
		ir.Add(PyIR(PY_CALL_FUNCTION, (int)args.GetCount(), 0));
		ir.Add(PyIR(PY_RETURN_VALUE));
		
		new_vm->SetIR(ir);
		
		Thread t;
		t.Run([new_vm] { NativeThreadEntry(new_vm); });
		t.Detach();
		
		return PyValue("Thread started (native)");
	}
	else {
		PyVM& new_vm = vms.Add();
		
		Vector<PyIR> ir;
		ir.Add(PyIR(PY_LOAD_CONST, func));
		for(const auto& arg : args) ir.Add(PyIR(PY_LOAD_CONST, arg));
		ir.Add(PyIR(PY_CALL_FUNCTION, (int)args.GetCount(), 0));
		ir.Add(PyIR(PY_RETURN_VALUE));
		
		new_vm.SetIR(ir);
		
		return PyValue("Thread added to scheduler");
	}
}

void PyScheduler::Run()
{
	if(mode == PYTHREAD_NATIVE) {
		// Main VM is already running in the main thread.
		// Just wait for all native threads? No, we just return and let them run.
	}
	else {
		while(!vms.IsEmpty()) {
			for(int i = 0; i < vms.GetCount(); i++) {
				if(!vms[i].IsRunning()) {
					vms.Remove(i);
					i--;
					continue;
				}
				
				// Step N instructions
				for(int j = 0; j < 5; j++) {
					if(!vms[i].Step()) break;
				}
			}
			Sleep(1);
		}
	}
}

END_UPP_NAMESPACE

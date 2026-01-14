#include "Geometry.h"
#include <ByteVM/PyBindings.h>

NAMESPACE_UPP

// --- vec2 ---
PY_CLASS(vec2, "vec2")
PY_DATA_BEGIN(vec2)
	PY_DATA_V("x", data[0])
	PY_DATA_V("y", data[1])
PY_DATA_END

static PyValue vec2_Ctor(const Vector<PyValue>& args, void*) {
	float x = args.GetCount() > 0 ? (float)args[0].AsDouble() : 0;
	float y = args.GetCount() > 1 ? (float)args[1].AsDouble() : 0;
	return PyValue(new Pyvec2(vec2(x, y)));
}

static PyValue vec2_ToString(const Vector<PyValue>& args, void*) {
	PY_SELF(vec2)
	return PyValue(self.ToString());
}

// --- vec3 ---
PY_CLASS(vec3, "vec3")
PY_DATA_BEGIN(vec3)
	PY_DATA_V("x", data[0])
	PY_DATA_V("y", data[1])
	PY_DATA_V("z", data[2])
PY_DATA_END

static PyValue vec3_Ctor(const Vector<PyValue>& args, void*) {
	float x = args.GetCount() > 0 ? (float)args[0].AsDouble() : 0;
	float y = args.GetCount() > 1 ? (float)args[1].AsDouble() : 0;
	float z = args.GetCount() > 2 ? (float)args[2].AsDouble() : 0;
	return PyValue(new Pyvec3(vec3(x, y, z)));
}

static PyValue vec3_ToString(const Vector<PyValue>& args, void*) {
	PY_SELF(vec3)
	return PyValue(self.ToString());
}

static PyValue vec3_Length(const Vector<PyValue>& args, void*) {
	PY_SELF(vec3)
	return PyValue((double)self.GetLength());
}

static PyValue vec3_Normalize(const Vector<PyValue>& args, void*) {
	PY_SELF(vec3)
	self.Normalize();
	return args[0];
}

// --- mat4 ---
PY_CLASS(mat4, "mat4")
PY_DATA_EMPTY(mat4)

static PyValue mat4_Ctor(const Vector<PyValue>& args, void*) {
	mat4 m;
	m.SetIdentity();
	return PyValue(new Pymat4(m));
}

static PyValue mat4_ToString(const Vector<PyValue>& args, void*) {
	PY_SELF(mat4)
	return PyValue(self.ToString());
}

static PyValue mat4_SetIdentity(const Vector<PyValue>& args, void*) {
	PY_SELF(mat4)
	self.SetIdentity();
	return args[0];
}

// Global registration
void RegisterGeometryModule(PyVM& vm) {
	PY_MODULE(geometry, vm)
	
	// vec2
	{
		PY_CLASS_BIND(vec2)
		RegisterFunction(current_class_dict, "__str__", vec2_ToString);
	}
	
	// vec3
	{
		PY_CLASS_BIND(vec3)
		RegisterFunction(current_class_dict, "__str__", vec3_ToString);
		RegisterFunction(current_class_dict, "length", vec3_Length);
		RegisterFunction(current_class_dict, "normalize", vec3_Normalize);
	}
	
	// mat4
	{
		PY_CLASS_BIND(mat4)
		RegisterFunction(current_class_dict, "__str__", mat4_ToString);
		RegisterFunction(current_class_dict, "set_identity", mat4_SetIdentity);
	}
}

END_UPP_NAMESPACE

void RegisterGeometry(Upp::PyVM& vm) {
	Upp::RegisterGeometryModule(vm);
}

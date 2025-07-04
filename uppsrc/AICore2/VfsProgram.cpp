#include "AICore.h"

NAMESPACE_UPP


void VfsProgramIteration::Visit(Vis& v) {
	v.Ver(1)
	(1) VIS_(code)
		VIS_(global)
		VIS_(vfsvalue)
		VIS_(log)
		;
}

void VfsProgramSession::Visit(Vis& v) {
	
}

void VfsProgramProject::Visit(Vis& v) {
	
}

bool VfsProgramProject::GetAll(Vector<VirtualNode>& v) {
	VfsValue& n = val;
	v.Reserve(n.sub.GetCount());
	for(int i = 0; i < n.sub.GetCount(); i++) {
		auto& sub = n.sub[i];
		if (sub.id.IsEmpty())
			v.Add().Create(/*data->path +*/VfsPath() + (i), &sub);
		else
			v.Add().Create(/*data->path +*/VfsPath() + (Value)sub.id, &sub);
	}
	return true;
}



VfsProgram::VfsProgram(VfsValue& owner) : Component(owner) {
	
}

bool VfsProgram::Initialize(const WorldState& ws) {
	AddToUpdateList();
	return true;
}

void VfsProgram::Uninitialize() {
	RemoveFromUpdateList();
}

void VfsProgram::Visit(Vis& v) {
	
}

void VfsProgram::Update(double dt) {
	
	if (running) {
		TODO
	}
	
}

INITIALIZER_VFSEXT(VfsProgramIteration, "", "");
INITIALIZER_VFSEXT(VfsProgramSession, "", "");
INITIALIZER_VFSEXT(VfsProgramProject, "", "");
INITIALIZER_COMPONENT(VfsProgram, "vfs.program", "Vfs|Program")

END_UPP_NAMESPACE

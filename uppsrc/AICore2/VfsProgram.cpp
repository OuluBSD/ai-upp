#include "AICore.h"

NAMESPACE_UPP


void VfsProgramIteration::Visit(Vis& v) {
	v.Ver(1)
	(1) VIS_(name)
		VIS_(code)
		VIS_(global)
		VIS_(vfsvalue);
}

void VfsProgramSession::Visit(Vis& v) {
	v.Ver(1)
	(1) VIS_(name);
}

void VfsProgramProject::Visit(Vis& v) {
	v.Ver(1)
	(1) VIS_(name);
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

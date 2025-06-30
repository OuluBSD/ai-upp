#include "AICore.h"

NAMESPACE_UPP


AiProgram::AiProgram(VfsValue& owner) : Component(owner) {
	
}

bool AiProgram::Initialize(const WorldState& ws) {
	AddToUpdateList();
	return true;
}

void AiProgram::Uninitialize() {
	RemoveFromUpdateList();
}

void AiProgram::Visit(Vis& v) {
	v.Ver(1)
	(1)	//("sessions", sessions, VISIT_VECTOR)
		("stage_name_presets", stage_name_presets)
	;
}

void AiProgram::Update(double dt) {
	
	if (running) {
		TODO
	}
	
}

INITIALIZER_COMPONENT(AiProgram, "ai.program", "AI|Program")


END_UPP_NAMESPACE

#include "AI.h"


NAMESPACE_UPP


VersionControlSystem::VersionControlSystem() {
	
	
}

VersionControlSystem::~VersionControlSystem() {
	Close();
}

void VersionControlSystem::Initialize(String path) {
	Panic("TODO");
	
}

void VersionControlSystem::Close() {
	Panic("TODO");
	
}

bool VersionControlSystem::IsStoring() const {
	return storing;
}

void VersionControlSystem::SetStoring() {
	Panic("TODO");
	storing = true;
}

void VersionControlSystem::SetLoading() {
	Panic("TODO");
	storing = false;
}


END_UPP_NAMESPACE

#include "ide.h"

NAMESPACE_UPP


AiServiceProviderManager::AiServiceProviderManager() {
	
}

AiServiceProvider& AiServiceProviderManager::Add() {
	return providers.Add();
}

void AiServiceProviderManager::Remove(int i) {
	if (i >= 0 && i < providers.GetCount())
		providers.Remove(i);
}

AiServiceProvider& AiServiceProviderManager::operator[](int i) {
	return providers[i];
}

int AiServiceProviderManager::GetCount() const {
	return providers.GetCount();
}


AiServiceProviderManager& AiManager() {
	ASSERT(TheIde());
	return TheIde()->ai_manager;
}

END_UPP_NAMESPACE

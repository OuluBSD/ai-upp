
NAMESPACE_UPP

inline
AiServiceProviderManager::AiServiceProviderManager() {
	
}

inline
AiServiceProvider& AiServiceProviderManager::Add() {
	return providers.Add();
}

inline
void AiServiceProviderManager::Remove(int i) {
	if (i >= 0 && i < providers.GetCount())
		providers.Remove(i);
}

inline
AiServiceProvider& AiServiceProviderManager::operator[](int i) {
	return providers[i];
}

inline
int AiServiceProviderManager::GetCount() const {
	return providers.GetCount();
}


inline
AiServiceProviderManager& AiManager() {
	static AiServiceProviderManager s;
	return s;
}


END_UPP_NAMESPACE

#include "Eon.h"

NAMESPACE_UPP

void ErrorSourceBuffered::DumpMessages() {
	for(const ProcMsg& pm : messages) {
		LOG(pm.ToString());
	}
}


bool& EnvState::SetBool(dword key, bool b) {
	Value& o = data.GetAdd(key);
	auto& v = CreateRawValue<bool>(o);
	v = b;
	return v;
}

int& EnvState::SetInt(dword key, int i) {
	Value& o = data.GetAdd(key);
	auto& v = CreateRawValue<int>(o);
	v = i;
	return v;
}

bool& EnvState::GetBool(dword key) {
	Value& o = data.GetAdd(key);
	if (!o.Is<bool>())
		CreateRawValue<bool>(o) = false;
	return const_cast<bool&>(o.To<bool>());
}

int& EnvState::GetInt(dword key) {
	Value& o = data.GetAdd(key);
	if (!o.Is<int>())
		o = CreateRawValue<int>(o) = 0;
	return const_cast<int&>(o.To<int>());
}

END_UPP_NAMESPACE

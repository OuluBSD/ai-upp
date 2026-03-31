#include "MluiScript.h"

NAMESPACE_UPP

// ============================================================
// MluiScript helpers
// ============================================================

int MluiScript::FindSlot(const String& sid) const {
	for(int i = 0; i < slots.GetCount(); i++)
		if(slots[i].slot_id == sid) return i;
	return -1;
}

const MluiScriptSlot& MluiScript::GetSlot(const String& sid) const {
	int i = FindSlot(sid);
	ASSERT_(i >= 0, "MluiScript: slot not found: " + sid);
	return slots[i];
}

MluiScriptSlot& MluiScript::GetSlot(const String& sid) {
	int i = FindSlot(sid);
	ASSERT_(i >= 0, "MluiScript: slot not found: " + sid);
	return slots[i];
}

// ============================================================
// Load / Save
// ============================================================

bool LoadMluiScript(MluiScript& s, const String& path) {
	String json = LoadFile(path);
	if(json.IsEmpty()) return false;
	try {
		LoadFromJson(s, json);
		return true;
	} catch(...) {
		return false;
	}
}

bool SaveMluiScript(const MluiScript& s, const String& path) {
	try {
		String json = StoreAsJson(s, true);
		return SaveFile(path, json);
	} catch(...) {
		return false;
	}
}

END_UPP_NAMESPACE


#include "Maestro.h"
#include "QuotaManager.h"

namespace Upp {

VectorMap<String, QuotaManager::ModelStatus> QuotaManager::statuses;
Mutex                          QuotaManager::mutex;
bool                           QuotaManager::loaded = false;

String QuotaManager::GetConfigPath()
{
	String path = AppendFileName(GetHomeDirectory(), ".maestro");
	RealizeDirectory(path);
	return AppendFileName(path, "quota.json");
}

void QuotaManager::Load()
{
	Mutex::Lock __(mutex);
	if(loaded) return;
	
	String content = LoadFile(GetConfigPath());
	if(!content.IsEmpty()) {
		Value v = ParseJSON(content);
		if(v.Is<ValueMap>()) {
			ValueMap m = v;
			for(int i = 0; i < m.GetCount(); i++) {
				ModelStatus& s = statuses.GetAdd(m.GetKey(i));
				s.exhausted_until = m.GetValue(i)["exhausted_until"];
			}
		}
	}
	loaded = true;
}

void QuotaManager::Save()
{
	Mutex::Lock __(mutex);
	ValueMap m;
	for(int i = 0; i < statuses.GetCount(); i++) {
		ValueMap status;
		status.Add("exhausted_until", statuses[i].exhausted_until);
		m.Add(statuses.GetKey(i), status);
	}
	SaveFile(GetConfigPath(), StoreAsJson(m));
}

bool QuotaManager::IsModelExhausted(const String& model)
{
	Load();
	Mutex::Lock __(mutex);
	int q = statuses.Find(model);
	if(q < 0) return false;
	return GetSysTime() < statuses[q].exhausted_until;
}

void QuotaManager::MarkModelExhausted(const String& model, int hours)
{
	Load();
	{
		Mutex::Lock __(mutex);
		statuses.GetAdd(model).exhausted_until = GetSysTime() + hours * 3600;
	}
	Save();
}

String QuotaManager::GetBestGeminiModel()
{
	static const char *models[] = {
		"gemini-3-pro-preview",
		"gemini-3-flash-preview",
		"gemini-1.5-pro",
		"gemini-1.5-flash",
		NULL
	};
	
	for(int i = 0; models[i]; i++) {
		if(!IsModelExhausted(models[i]))
			return models[i];
	}
	
	return "gemini-3-flash-preview"; // Fallback to default
}

void QuotaManager::Reset()
{
	Mutex::Lock __(mutex);
	statuses.Clear();
	Save();
}

}

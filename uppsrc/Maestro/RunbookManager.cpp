#include "Maestro.h"

namespace Upp {

RunbookManager::RunbookManager(const String& maestro_root)
{
	base_path = AppendFileName(NormalizePath(maestro_root), "docs/maestro/runbooks");
	items_dir = AppendFileName(base_path, "items");
	index_path = AppendFileName(base_path, "index.json");
	
	RealizeDirectory(items_dir);
}

Array<Runbook> RunbookManager::ListRunbooks()
{
	Array<Runbook> list;
	FindFile ff(AppendFileName(items_dir, "*.json"));
	while(ff) {
		Runbook& rb = list.Add();
		if(!LoadFromJsonFile(rb, ff.GetPath()))
			list.Drop();
		ff.Next();
	}
	return list;
}

Runbook RunbookManager::LoadRunbook(const String& id)
{
	Runbook rb;
	LoadFromJsonFile(rb, AppendFileName(items_dir, id + ".json"));
	return rb;
}

bool RunbookManager::SaveRunbook(const Runbook& rb)
{
	if(rb.id.IsEmpty()) return false;
	bool success = StoreAsJsonFile(rb, AppendFileName(items_dir, rb.id + ".json"), true);
	if(success) UpdateIndex(rb);
	return success;
}

bool RunbookManager::DeleteRunbook(const String& id)
{
	String path = AppendFileName(items_dir, id + ".json");
	if(FileExists(path)) {
		DeleteFile(path);
		RebuildIndex();
		return true;
	}
	return false;
}

void RunbookManager::UpdateIndex(const Runbook& rb)
{
	Vector<ValueMap> index;
	if(FileExists(index_path))
		LoadFromJsonFile(index, index_path);
	
	bool found = false;
	for(auto& entry : index) {
		if(AsString(entry["id"]) == rb.id) {
			entry("title") = rb.title;
			entry("updated_at") = GetSysTime();
			found = true;
			break;
		}
	}
	
	if(!found) {
		ValueMap& entry = index.Add();
		entry("id") = rb.id;
		entry("title") = rb.title;
		entry("updated_at") = GetSysTime();
	}
	
	StoreAsJsonFile(index, index_path, true);
}

void RunbookManager::RebuildIndex()
{
	Vector<ValueMap> index;
	FindFile ff(AppendFileName(items_dir, "*.json"));
	while(ff) {
		Runbook rb;
		if(LoadFromJsonFile(rb, ff.GetPath())) {
			ValueMap& entry = index.Add();
			entry("id") = rb.id;
			entry("title") = rb.title;
			entry("updated_at") = GetSysTime();
		}
		ff.Next();
	}
	StoreAsJsonFile(index, index_path, true);
}

Runbook RunbookManager::Resolve(const String& text, bool use_ai)
{
	Runbook rb;
	if(!use_ai) {
		rb.title = text.Left(50);
		rb.id = "rb-" + FormatIntHex(Random(), 8);
		rb.goal = text;
		return rb;
	}
	
	CliMaestroEngine engine;
	ConfigureGemini(engine);
	
	String prompt;
	prompt << "You are a Maestro AI Assistant. Create a structured runbook JSON based on the following user request.\n"
	       << "USER REQUEST: " << text << "\n\n"
	       << "SCHEMA:\n"
	       << "{\n"
	       << "  \"id\": \"rb-unique-id\",\n"
	       << "  \"title\": \"Descriptive Title\",\n"
	       << "  \"goal\": \"The ultimate objective\",\n"
	       << "  \"steps\": [\n"
	       << "    { \"n\": 1, \"actor\": \"user\", \"action\": \"Description of action\", \"command\": \"Optional CLI command\", \"expected\": \"Expected UI state or outcome\" }\n"
	       << "  ]\n"
	       << "}\n\n"
	       << "Return ONLY the raw JSON object. Do not use markdown blocks.";
	
	String response;
	bool done = false;
	engine.Send(prompt, [&](const MaestroEvent& ev) {
		if(ev.type == "message") response << ev.text;
		else if(ev.type == "done") done = true;
	});
	
	while(!done && engine.Do()) Sleep(10);
	
	// Extract JSON robustly
	int first = response.Find("{");
	int last = response.ReverseFind("}");
	if(first >= 0 && last > first) {
		String json = response.Mid(first, last - first + 1);
		try {
			LoadFromJson(rb, json);
		} catch (Exc& e) {
			Cerr() << "Error parsing runbook JSON: " << e << "\n";
			Cerr() << "RAW RESPONSE: " << response << "\n";
		}
	}
	
	if(rb.id.IsEmpty()) rb.id = "rb-" + FormatIntHex(Random(), 8);
	if(rb.title.IsEmpty()) rb.title = "Untitled Runbook";
	
	return rb;
}

}

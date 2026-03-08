#include "Maestro.h"

namespace Upp {

String Playbook::GetVersionHash() const
{
	return SHA256String(StoreAsJson(*this)).Left(16);
}

PlaybookManager::PlaybookManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	playbooks_dir = AppendFileName(base_path, "docs/maestro/playbooks");
	binding_file = AppendFileName(base_path, "docs/maestro/convert/playbook_binding.json");
	overrides_file = AppendFileName(base_path, "docs/maestro/convert/playbook_overrides.json");
	
	RealizeDirectory(playbooks_dir);
}

Array<Playbook> PlaybookManager::ListPlaybooks()
{
	Array<Playbook> list;
	FindFile ff(AppendFileName(playbooks_dir, "*"));
	while(ff) {
		if(ff.IsDirectory()) {
			String pb_file = AppendFileName(ff.GetPath(), "playbook.json");
			if(FileExists(pb_file)) {
				Playbook& pb = list.Add();
				if(!LoadFromJsonFile(pb, pb_file))
					list.Drop();
			}
		}
		ff.Next();
	}
	return list;
}

Playbook* PlaybookManager::LoadPlaybook(const String& id)
{
	String pb_file = AppendFileName(AppendFileName(playbooks_dir, id), "playbook.json");
	if(!FileExists(pb_file)) return nullptr;
	
	One<Playbook> pb;
	pb.Create();
	if(LoadFromJsonFile(*pb, pb_file))
		return pb.Detach();
	
	return nullptr;
}

bool PlaybookManager::SavePlaybook(const Playbook& pb)
{
	String dir = AppendFileName(playbooks_dir, pb.id);
	RealizeDirectory(dir);
	return StoreAsJsonFile(pb, AppendFileName(dir, "playbook.json"), true);
}

bool PlaybookManager::BindPlaybook(const String& id)
{
	Playbook* pb = LoadPlaybook(id);
	if(!pb) return false;
	
	PlaybookBinding b;
	b.playbook_id = pb->id;
	b.playbook_version = pb->version;
	b.version_hash = pb->GetVersionHash();
	b.bound_at = GetSysTime().Get();
	b.bound_by = GetUserName();
	
	delete pb;
	
	RealizeDirectory(GetFileDirectory(binding_file));
	return StoreAsJsonFile(b, binding_file, true);
}

PlaybookBinding PlaybookManager::GetActiveBinding()
{
	PlaybookBinding b;
	if(FileExists(binding_file))
		LoadFromJsonFile(b, binding_file);
	return b;
}

bool PlaybookManager::RecordOverride(const String& task_id, const String& violation_type, const String& reason)
{
	Array<PlaybookOverride> overrides;
	if(FileExists(overrides_file))
		LoadFromJsonFile(overrides, overrides_file);
	
	PlaybookOverride& o = overrides.Add();
	o.task_id = task_id;
	o.violation_type = violation_type;
	o.reason = reason;
	o.timestamp = GetSysTime().Get();
	o.overridden_by = GetUserName();
	
	RealizeDirectory(GetFileDirectory(overrides_file));
	return StoreAsJsonFile(overrides, overrides_file, true);
}

Array<PlaybookOverride> PlaybookManager::GetOverrides()
{
	Array<PlaybookOverride> overrides;
	if(FileExists(overrides_file))
		LoadFromJsonFile(overrides, overrides_file);
	return overrides;
}

bool PlaybookManager::Validate(const Playbook& pb)
{
	if(pb.id.IsEmpty() || pb.title.IsEmpty() || pb.version.IsEmpty()) return false;
	return true;
}

}

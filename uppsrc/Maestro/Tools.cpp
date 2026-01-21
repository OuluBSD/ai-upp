#include "Maestro.h"

NAMESPACE_UPP

class ListPackagesTool : public MaestroTool {
public:
	virtual String GetName() const override { return "maestro_list_packages"; }
	virtual String GetDescription() const override { return "Returns the list of assemblies and packages in the current project."; }
	virtual Value  GetSchema() const override { return Value(); } // Simple tool, no params
	
	virtual Value Execute(const ValueMap& params) const override {
		// Use RepoScanner to get structure
		RepoScanner rs;
		rs.Scan(GetCurrentDirectory());
		rs.DetectAssemblies();
		
		ValueMap res;
		ValueArray assemblies;
		for(const auto& a : rs.assemblies) {
			ValueMap am;
			am.Add("name", a.name);
			am.Add("type", a.assembly_type);
			assemblies.Add(am);
		}
		res.Add("assemblies", assemblies);
		
		ValueArray packages;
		for(const auto& p : rs.packages) {
			ValueMap pm;
			pm.Add("name", p.name);
			pm.Add("path", p.path);
			packages.Add(pm);
		}
		res.Add("packages", packages);
		
		return res;
	}
};

class GetPlanTool : public MaestroTool {
public:
	virtual String GetName() const override { return "maestro_get_plan"; }
	virtual String GetDescription() const override { return "Returns the current project plan (tracks, phases, tasks)."; }
	virtual Value  GetSchema() const override { return Value(); }
	
	virtual Value Execute(const ValueMap& params) const override {
		PlanParser pp;
		pp.LoadMaestroTracks(GetCurrentDirectory());
		
		ValueArray tracks;
		for(const auto& t : pp.tracks) {
			ValueMap tm;
			tm.Add("id", t.id);
			tm.Add("name", t.name);
			tm.Add("status", t.status);
			tm.Add("completion", t.completion);
			tracks.Add(tm);
		}
		return tracks;
	}
};

class ReadFileTool : public MaestroTool {
public:
	virtual String GetName() const override { return "maestro_read_file"; }
	virtual String GetDescription() const override { return "Reads a file from the current project."; }
	virtual Value  GetSchema() const override {
		ValueMap vm;
		vm.Add("path", "string");
		return vm;
	}
	
	virtual Value Execute(const ValueMap& params) const override {
		String path = params["path"];
		if(FileExists(path))
			return LoadFile(path);
		return "Error: File not found: " + path;
	}
};

void RegisterMaestroTools(MaestroToolRegistry& reg) {
	reg.Add(new ListPackagesTool());
	reg.Add(new GetPlanTool());
	reg.Add(new ReadFileTool());
}

END_UPP_NAMESPACE

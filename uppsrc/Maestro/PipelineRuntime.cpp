#include "Maestro.h"

namespace Upp {

PipelineRuntime::PipelineRuntime(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	RealizeDirectory(AppendFileName(base_path, "docs/maestro/convert/pipelines"));
}

String PipelineRuntime::GetPipelinePath(const String& id)
{
	return AppendFileName(base_path, "docs/maestro/convert/pipelines/" + id + ".json");
}

ConversionPipeline PipelineRuntime::CreatePipeline(const String& name, const String& source, const String& target)
{
	ConversionPipeline p;
	p.id = FormatIntHex(Random(), 8);
	p.name = name;
	p.source = source;
	p.target = target;
	p.created_at = p.updated_at = GetSysTime();
	
	static const char* stage_names[] = {
		"overview", "core_builds", "grow_from_main", "full_tree_check", "refactor"
	};
	for(const char* s : stage_names)
		p.stages.Add().name = s;
	
	SavePipeline(p);
	return p;
}

bool PipelineRuntime::SavePipeline(const ConversionPipeline& pipeline)
{
	return StoreAsJsonFile(pipeline, GetPipelinePath(pipeline.id), true);
}

ConversionPipeline PipelineRuntime::LoadPipeline(const String& id)
{
	ConversionPipeline p;
	LoadFromJsonFile(p, GetPipelinePath(id));
	return p;
}

Array<ConversionPipeline> PipelineRuntime::ListPipelines()
{
	Array<ConversionPipeline> list;
	FindFile ff(AppendFileName(base_path, "docs/maestro/convert/pipelines/*.json"));
	while(ff) {
		ConversionPipeline& p = list.Add();
		if(!LoadFromJsonFile(p, ff.GetPath()))
			list.Drop();
		ff.Next();
	}
	return list;
}

void PipelineRuntime::MarkStage(ConversionStage& stage, const String& status, const ValueMap& details)
{
	stage.status = status;
	if(status == "running")
		stage.started_at = GetSysTime();
	else if(status == "completed" || status == "failed" || status == "skipped")
		stage.completed_at = GetSysTime();
	
	if(details.GetCount() > 0)
		stage.details = clone(details);
}

}

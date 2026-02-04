#ifndef _Maestro_PipelineRuntime_h_
#define _Maestro_PipelineRuntime_h_

class PipelineRuntime {
	String base_path;

public:
	PipelineRuntime(const String& maestro_root = ".");
	
	ConversionPipeline CreatePipeline(const String& name, const String& source, const String& target);
	bool               SavePipeline(const ConversionPipeline& pipeline);
	ConversionPipeline LoadPipeline(const String& id);
	Array<ConversionPipeline> ListPipelines();
	
	void MarkStage(ConversionStage& stage, const String& status, const ValueMap& details = ValueMap());
	
private:
	String GetPipelinePath(const String& id);
};

#endif

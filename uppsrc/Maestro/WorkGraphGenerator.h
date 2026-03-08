
#ifndef _Maestro_WorkGraphGenerator_h_
#define _Maestro_WorkGraphGenerator_h_

class WorkGraphGenerator {
	MaestroEngine& engine;
	bool verbose = false;

public:
	WorkGraphGenerator(MaestroEngine& engine, bool verbose = false) 
		: engine(engine), verbose(verbose) {}

	WorkGraph Generate(const String& freeform, const ValueMap& discovery, const String& domain = "general", const String& profile = "default");
	
private:
	String CreatePrompt(const String& freeform, const ValueMap& discovery, const String& domain, const String& profile);
	String ExtractJSON(const String& response);
};

#endif

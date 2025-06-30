#ifndef _AICore2_AiProgram_h_
#define _AICore2_AiProgram_h_


struct AiProgram : Component {
	Vector<String> stage_name_presets;
	bool running = false;
public:
	CLASSTYPE(AiProgram)
	AiProgram(VfsValue& n);
	void Visit(Vis& v) override;
	bool Initialize(const WorldState& ws) override;
	void Uninitialize() override;
	void Update(double dt) override;
};

INITIALIZE(AiProgram);


#endif

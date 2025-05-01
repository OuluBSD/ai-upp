#ifndef _Eon_ModelCache_h_
#define _Eon_ModelCache_h_


class ModelCache :
	public System<ModelCache>
{
	
protected:
	//CalibrationData				calib;
	ArrayMap<String, ModelLoader> model_cache;
	double time = 0;
	
protected:
    bool Initialize() override;
    void Start() override;
    void Update(double dt) override;
    void Stop() override;
    void Uninitialize() override;
    
public:
	using Base = System<ModelCache>;
    SYS_CTOR(ModelCache)
	SYS_DEF_VISIT
	
	ModelRef Attach(Model* mdl);
	ModelRef GetAddModelFile(String path);
	
	//void CalibrationEvent(CtrlEvent& ev);
	
};

using ModelCacheRef = Ref<ModelCache>;


#endif

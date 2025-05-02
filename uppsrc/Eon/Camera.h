#ifndef _Eon_Camera_h_
#define _Eon_Camera_h_

namespace Ecs {
using namespace Parallel;


class Viewable :
	public Component<Viewable>
{
	
public:
	RTTI_COMP0(Viewable)
	COMP_DEF_VISIT
	
	void Etherize(Ether& e) override {}
	void Initialize() override;
	void Uninitialize() override;
	
	void operator=(const Viewable& c) {}
	
	
	Callback1<GfxShader&> cb;
	
};


class Viewport : public Component<Viewport> {
public:
	RTTI_COMP0(Viewport)
	COMP_DEF_VISIT
	
	
	vec3 target = zero<vec3>();
	double fov = M_PI/2;
	double angle = 0;
	
	
	vec3 GetTarget() const {return target;}
	void SetTraget(const vec3& v) {target = v;}
	
	void Etherize(Ether& e) override {e % target % fov % angle;}
	bool Arg(String key, Value value) override;
	
	void operator=(const Viewport& vp) {
		target = vp.target;
	}
	
};


struct CameraBase : Pte<CameraBase>
{
	bool use_stereo = false;
	CalibrationData calib;
	
	mat4 view_stereo[2];
	mat4 proj_stereo[2];
	mat4 mvp_stereo[2];
	
	
	virtual bool Load(GfxDataState& state) = 0;
	virtual void UpdateCalibration() = 0;
	void Etherize(Ether& e);
	
};

class ChaseCam :
	public Component<ChaseCam>,
	public CameraBase
{
	TransformPtr trans;
	TransformPtr target;
	ViewablePtr viewable;
	ViewportPtr vport;
	
	mat4 view;
	mat4 projection;
	mat4 port;
	mat4 port_stereo;
	vec2 viewport_sz;
	
	bool test_log = false;
	double time = 0;
	double phase_time = 1.5;
	float fov = 110;
	float used_fov = 0;
	
	
	float GetUsedFov();
	
public:
	typedef ChaseCam CLASSNAME;
	RTTI_COMP1(ChaseCam, CameraBase)
	
	void Etherize(Ether& e) override;
	void Visit(Vis& vis) override {vis.VisitT<ComponentT>(this); vis & target & viewable & vport;}
	void Initialize() override;
	void Uninitialize() override;
	void Update(double dt) override;
	bool Arg(String key, Value value) override;
	bool Load(GfxDataState& state) override;
	void UpdateCalibration() override;
	
	void CheckUpdateProjection();
	void UpdateProjection();
	void UpdateView();
	void SetViewportSize(Size sz);
	void SetTarget(TransformPtr tgt) {target = tgt;}
	
	void operator=(const ChaseCam& vp) {
		target = vp.target;
	}
};


struct CameraPrefab : EntityPrefab<Transform, Viewport, Viewable>
{
    static Components Make(Entity& e)
    {
        auto components = EntityPrefab::Make(e);
		
		TransformPtr t = components.Get<TransformRef>();
		t->data.mode = TransformMatrix::MODE_POSITION;
		t->data.position[2] = 10.0;
		t->data.position[1] = 3.0;
		t->data.position[0] = 0.0;
		
        return components;
    }
};


}

#endif

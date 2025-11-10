#ifndef _EonLib_Gui_h_
#define _EonLib_Gui_h_


#ifdef flagGUI

#include <Geometry/GeomEvent.h>
#include <CtrlCore/CtrlCore.h>
#include <GuboCore/CtrlEvent.h>

struct DefaultGuiAppComponent :
	public Component,
	public BinderIfaceVideo,
	public BinderIfaceEvents
{
	//RTTI_DECL2(DefaultGuiAppComponent, ComponentT, BinderIfaceVideo)
	
	Point prev_mouse;
	
	//WindowSystemRef wins;
	Geom2DComponentPtr cw;
	TransformPtr trans;
	Transform2DPtr trans2;
	
	
	DefaultGuiAppComponent();
	void operator=(const DefaultGuiAppComponent& t) {Panic("Can't copy DefaultGuiAppComponent");}
	
	virtual void Serialize(Stream& e);
	bool Initialize(const WorldState&) override;
	void Uninitialize() override;
	void Visit(Vis& v) override;
	void Update(double dt) override;
	bool Render(Draw& d) override;
	bool RenderProg(DrawCommand*& begin, DrawCommand*& end) override;
	virtual void Dispatch(const Upp::CtrlEvent& state) override;
	bool Arg(const String& key, const String& value) override;
	
	void DrawObj(GfxStateDraw& fb, bool use_texture);
	void StateStartup(GfxDataState& s);
	
	
};
#endif


#endif

#include "Lib.h"

#if 0
NAMESPACE_UPP


#ifdef flagGUI

DefaultGuiAppComponent::DefaultGuiAppComponent(VfsValue& v) : Component(v) {
	prev_mouse = Point(0,0);
	
}

void DefaultGuiAppComponent::Visit(Vis& v) {
	VIS_THIS(Component);
	/*if (test) vis % *test;*/
	
	//vis & wins;
	v ("prev_mouse", prev_mouse)
	  //("cw", cw)
	  //("trans", trans)
	  //("trans2", trans2)
	  ;
}

bool DefaultGuiAppComponent::Initialize(const WorldState& ws) {
	AddToUpdateList();
	HandleVideoBase::AddBinder(this);
	EventStateBase::AddBinder(this);
	
	cw = GetEntity()->val.Find<Geom2DComponent>();
	trans2 = GetEntity()->val.Find<Transform2D>();
	
	return true;
}

void DefaultGuiAppComponent::Uninitialize() {
	//wins.Clear();
	cw = 0;
	trans = 0;
	trans2 = 0;
	
	RemoveFromUpdateList();
	HandleVideoBase::RemoveBinder(this);
	EventStateBase::RemoveBinder(this);
}

void DefaultGuiAppComponent::Update(double dt) {
	
}

void DefaultGuiAppComponent::StateStartup(GfxDataState& state) {
	RenderingSystemPtr rend = val.FindOwnerWith<RenderingSystem>();
	if (!rend) {
		LOG("DefaultGuiAppComponent::StateStartup: EntityStore or RenderingSystem not available yet");
		return;
	}
	//PoolRef models = ents->GetRoot()->GetAddPool("models");
	
	TODO
}

void DefaultGuiAppComponent::Dispatch(const CtrlEvent& e) {
	TODO
	#if 0
	if (cw && trans2) {
		Ctrl* ctrl = cw->GetWindowCtrl();
		if (!ctrl)
			return;
		Point cw_pos(trans2->position[0], trans2->position[1]);
		Point win_pt = e.pt - cw_pos;
		if (e.type == EVENT_WINDOW_RESIZE) {
			
		}
		else if (e.type == EVENT_MOUSEMOVE) {
			ctrl->DeepMouseMove(e.pt, e.value);
		}
		else if (e.type == EVENT_MOUSEWHEEL) {
			ctrl->DeepMouseWheel(e.pt, e.n, e.value);
		}
		else if (e.type == EVENT_KEYDOWN || e.type == EVENT_KEYUP) {
			ctrl->DeepKey(e.value, e.n);
		}
		else if (e.type == EVENT_MOUSE_EVENT) {
			ctrl->DeepMouse(e.n, e.pt, e.value);
		}
		else {
			LOG("TODO DefaultGuiAppComponent::Dispatch " << e.ToString());
		}
	}
	#endif
}

bool DefaultGuiAppComponent::Render(Draw& d) {
	ASSERT_(0, "DefaultGuiAppComponent cannot be used with draw (yet)");
	return false;
}

bool DefaultGuiAppComponent::RenderProg(DrawCommand*& begin, DrawCommand*& end) {
	TODO
	#if 0
	Ctrl* ctrl = cw->GetWindowCtrl();
	begin = &ctrl->GetCommandBegin();
	end = &ctrl->GetCommandEnd();
	
	return begin != NULL;
	#endif
}

void DefaultGuiAppComponent::DrawObj(GfxStateDraw& fb, bool use_texture) {
	
	TODO
}

bool DefaultGuiAppComponent::Arg(const String& key, const String& value) {
	
	return true;
}

#endif


END_UPP_NAMESPACE

#endif

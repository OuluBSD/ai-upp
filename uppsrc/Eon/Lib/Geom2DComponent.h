#ifndef _EonLib_Geom2DComponent_h_
#define _EonLib_Geom2DComponent_h_

#ifdef flagGUI

#include <Eon/Ecs/CommonComponents.h>
#include <Painter/Painter.h>
#include <Vfs/Ecs/Component.h>
#include <Vfs/Ecs/Entity.h>


#if 0

class Windows;

class WindowDecoration : public GeomInteraction2D {
	Geom2DComponent* win = NULL;
	String label;
	bool left_down;
	Point left_down_pt;
	
public:
	ATOM_CTOR_(WindowDecoration, GeomInteraction2D)
	typedef WindowDecoration CLASSNAME;
	WindowDecoration(Geom2DComponent*);
	
	virtual void Paint(Draw& draw) ;
	
	void SetLabel(String str) {label = str;}
	
	String GetLabel() const {return label;}
	
	virtual void LeftDown(Point p, dword keyflags) ;
	virtual void LeftDouble(Point p, dword keyflags) ;
	virtual void LeftUp(Point p, dword keyflags) ;
	virtual void MouseMove(Point p, dword keyflags) ;
	virtual void RightDown(Point p, dword keyflags) ;
	
	void LocalMenu(Bar& bar);
	
	
};

#endif

struct Geom2DComponentLink;

class Geom2DComponent :
	/*public Absolute2D,*/
	public Component
{
	ECS_COMPONENT_CTOR(Geom2DComponent)
using Geom2DComponentLinkPtr = Ptr<Geom2DComponentLink>;
	
	#if 0
	struct ResizeFrame : public CtrlFrame {
		ATOM_CTOR_(ResizeFrame, CtrlFrame)
	
		Geom2DComponent* win = NULL;
		Size sz;
		int frame_width = 8;
		int corner_width = 24;
		bool is_active = false;
		bool is_resizing = false;
		Point resize_start_pt;
		Rect resize_start_r;
		int resize_area = 0;
		Point resize_diff;
		bool used_momentum = false;
		
		enum {CENTER, TL, TR, BL, BR, TOP, BOTTOM, LEFT, RIGHT};
		int GetArea(Point pt);
		void SetActive(bool b) {frame_width = b ? 8 : 0;}
		virtual void FrameLayout(Rect& r) ;
		virtual void FramePaint(Draw& w, const Rect& r) ;
		virtual void FrameAddSize(Size& sz) ;
		virtual void MouseEnter(Point frame_p, dword keyflags) ;
		virtual void MouseMove(Point frame_p, dword keyflags) ;
		virtual void MouseLeave();
		virtual void LeftDown(Point p, dword keyflags) ;
		virtual void LeftUp(Point p, dword keyflags) ;
		virtual void ContinueGlobalMouseMomentum();
		void DoResize();
	};
	#endif
	
public:
	//One<Absolute2DInterface> owned_aw;
	//Absolute2DInterface* aw = 0;
	//Windows* wins = NULL;
	void (Geom2DComponent::*reset_fn)() = 0;
	Geom2DComponentLink* linked = NULL;
	TransformPtr    transform;
	Transform2DPtr  transform2d;
	
	//ResizeFrame resize_frame;
	//WindowDecoration decor;
	//Button minimize, maximize, close;
	Rect stored_rect;
	int id = -1;
	bool pending_partial_redraw = false;
	
protected:
	friend class Windows;
	friend class SDL2;
	
	bool maximized;
	
	//void SetContent(Windows* wins, int id);
	void SetMaximized(bool b=true);
	
public:
	Geom2DComponent();
	Geom2DComponent(const Geom2DComponent& cw) = delete;  // Copy constructor deleted due to component constraints
	Geom2DComponent& operator=(const Geom2DComponent& cw) = delete;  // Assignment operator deleted
	
	#if 0
	template <class T>
	void ResetTopWindow() {
		Clear();
		T* t = new T();
		aw = static_cast<Absolute2DInterface*>(t);
	}
	template <class T>
	T& Create() {
		Clear();
		T* t = new T();
		owned_aw = static_cast<Absolute2DInterface*>(t);
		aw = &*owned_aw;
		reset_fn = &Geom2DComponent::ResetTopWindow<T>;
		return *t;
	}
	#endif
	
	void Clear();
	//void DeepLayout();
	Point GetGlobalMouse();
	//bool IsGlobalMouseOverridden() const {return pending_partial_redraw;}
	//void SetGlobalMouse(Point pt) {is_global_mouse_override = true; global_mouse = pt;}
	void Title(String label);
	void StoreRect();
	void LoadRect();
	void SetStoredRect(Rect r);
	void SetPendingPartialRedraw();
	
	//GLuint GetTexture() {return fb.GetTexture();}
	//const Framebuffer& GetFramebuffer() const {return fb;}
	int GetId() const;
	Rect GetStoredRect() const;
	//Absolute2DInterface* GetAbsolute2D();
	bool IsMaximized() const;
	bool IsActive() const;
	void MoveWindow(Point pt);
	void Maximize();
	void Restore();
	void Minimize();
	//void Close();
	//void FocusEvent();
	void ToggleMaximized();
	bool IsPendingPartialRedraw() const;
	void Wait();
	//Windows* GetWindows() const {return wins;}
	TopWindow* GetTopWindow() const;
	
	void Serialize(Stream& e) ;
	bool Initialize(const WorldState&) ;
	virtual void Uninitialize() override;
	//String GetTitle() const ;
	void Layout();
	//bool IsGeomDrawBegin();
	//void SetFrameRect(const Rect& r) ;
	bool Redraw(bool only_pending) ;
	void LeftDown(Point p, dword keyflags) ;
	void ChildGotFocus();
	void Paint(Draw& id) ;
	
	COMP_DEF_VISIT
	
	virtual void ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags);
};

using Geom2DComponentPtr = Ptr<Geom2DComponent>;


struct Geom2DComponentLink : public Component {
	ECS_COMPONENT_CTOR(Geom2DComponentLink)
	//ATOMTYPE(Geom2DComponentLink)
	COMP_DEF_VISIT
	
	
	Geom2DComponent* linked = 0;
	
	
	void Serialize(Stream& e) ;
	bool Initialize(const WorldState&) ;
	virtual void Uninitialize() override;
	
	void Link(Geom2DComponent* cw);
	void Unlink(Geom2DComponent* cw);
	void Unlink();
	Geom2DComponent& GetWindow() const;
	
};

using Geom2DComponentLinkRef = Ptr<Geom2DComponentLink>;


//struct Window2D :
//	EntityPrefab<
//		Geom2DComponent,
//		Transform2D,
//		DefaultGuiAppComponent
//	> {
//    static Components Make(Entity& e)
//    {
//        auto components = EntityPrefab::Make(e);
//		
//		components.Get<Geom2DComponentPtr>()->transform2d = components.Get<Transform2DPtr>();
//		
//		components.Get<Transform2DPtr>()->position = vec2(0, 1);
//		components.Get<Transform2DPtr>()->size = vec2(320, 240);
//		
//        return components;
//    }
//};


// Window3D is commented out to avoid circular dependencies
// It can be defined in a separate header file that includes all necessary components
/*
// Window3D needs to be linked to Window2D
struct Window3D :
	EntityPrefab<
		Geom2DComponentLink,
		Transform,
		ModelComponent
	> {
    static Components Make(Entity& e, const WorldState& ws)
    {
        auto components = EntityPrefab::Make(e, ws);

		Geom2DComponentLinkPtr win = components.Get<Geom2DComponentLinkPtr>();

		#if 0
		ModelComponentPtr mdl = components.Get<ModelComponentPtr>();
		mdl->Arg("builtin", "box");
		#endif

		TransformPtr trans = components.Get<TransformPtr>();
		trans->data.position = vec3(0, 2, 0);

        return components;
    }
};
*/


#endif
#endif

#ifndef _Scene3D_Render_Renderer_h_
#define _Scene3D_Render_Renderer_h_


struct EditRenderer : public Ctrl {
	Scene3DRenderContext* ctx = 0;
	ViewMode view_mode = VIEWMODE_YZ;
	CameraSource cam_src = CAMSRC_FOCUS;
	
	Point cap_mouse_pos;
	vec3 cap_begin_pos;
	quat cap_begin_orientation;
	bool is_captured_mouse = false;
	
	typedef enum {
		CAPMODE_MOVE_XY,
		CAPMODE_MOVE_YZ,
		CAPMODE_ROTATE
	} CapMode;
	
	CapMode cap_mode;
	
public:
	typedef EditRenderer CLASSNAME;
	EditRenderer();

	Event<> WhenChanged;
	Event<Bar&> WhenMenu;
	
	void Paint(Draw& d) override;
	void LeftDown(Point p, dword keyflags) override;
	void LeftUp(Point p, dword keyflags) override;
	void MouseMove(Point p, dword keyflags) override;
	void MouseWheel(Point p, int zdelta, dword keyflags) override;
	void RightDown(Point p, dword keyflags) override;
	bool Key(dword key, int count) override;
	
	
	void Move(const vec3& v);
	void MoveRel(const vec3& v);
	void Rotate(const axes3& v);
	void RotateRel(const axes3& v);
	void SetViewMode(ViewMode i) {view_mode = i;}
	void SetCameraSource(CameraSource cs) {cam_src = cs;}
	void PaintObject(Draw& d, const GeomObjectState& o, const mat4& view, const Frustum& frustum);
	
	GeomCamera& GetGeomCamera() const;
	
};


#endif

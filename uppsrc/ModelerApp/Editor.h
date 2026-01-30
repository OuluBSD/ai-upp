#ifndef _ModelerApp_Editor_h_
#define _ModelerApp_Editor_h_

struct Edit3D;

struct GeomProjectCtrl : Ctrl {
	Edit3D* e;
	
	Splitter metasplit, hsplit, vsplit;
	TreeCtrl tree;
	ArrayCtrl props;
	FixedGridCtrl grid;
	TimelineCtrl time;
	EditRenderer rends[4];
	
	int tree_scenes = -1;
	
	
	typedef GeomProjectCtrl CLASSNAME;
	GeomProjectCtrl(Edit3D* e);
	void Update(double dt);
	void Data();
	void TimelineData();
	void TreeSelect();
	void OnCursor(int kp_i);
	void TreeDirectory(int id, GeomDirectory& dir);
	void RefreshRenderer(int i);
	
};

struct Edit3D : TopWindow {
	typedef enum {
		VIEW_NONE,
		VIEW_GEOMPROJECT,
		VIEW_VIDEOIMPORT,
	} ViewType;
	
	ViewType view = VIEW_NONE;
	
	GeomProjectCtrl v0;
	VideoImportCtrl v1;
	//RemoteDebugCtrl v_rdbg;
	MenuBar menu;
	ToolBar tool;
	
	Scene3DRenderConfig conf;
	Scene3DRenderContext render_ctx;
	
	
	GeomProject prj;
	GeomWorldState state;
	GeomAnim anim;
	//GeomVideo video;
	GeomStagedVideo video;
	TimeCallback tc;
	TimeStop ts;
	
	void CreateDefaultInit();
	void CreateDefaultPostInit();
	void LoadTestCirclingCube();
	void LoadTestOctree();
	
public:
	typedef Edit3D CLASSNAME;
	Edit3D();
	
	void SetView(ViewType view);
	void Update();
	void Data();
	void Exit();
	void RefreshData();
	void Stop();
	void Pause();
	void Play();
	void RefrehRenderers();
	void RefrehToolbar();
	void OnSceneEnd();
	void OnDebugMetadata();
	void Toolbar(Bar& bar);
	
	void LoadEmptyProject();
	void LoadTestProject(int test_i);
	void LoadWmrStereoPointcloud(String directory);
	bool LoadScene3D(const String& path);
	bool SaveScene3D(const String& path, bool pretty = true);
	
	GeomScene& GetActiveScene();
	
};


#endif

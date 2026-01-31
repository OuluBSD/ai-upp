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
	Index<hash_t> warned_tree_types;
	
	
	typedef GeomProjectCtrl CLASSNAME;
	GeomProjectCtrl(Edit3D* e);
	void Update(double dt);
	void Data();
	void TimelineData();
	void TreeSelect();
	void OnCursor(int kp_i);
	void TreeValue(int id, VfsValue& node);
	void RefreshRenderer(int i);
	
};

struct FilePoolCtrl : TopWindow {
	Edit3D* owner = 0;
	ArrayCtrl files;

	typedef FilePoolCtrl CLASSNAME;
	FilePoolCtrl(Edit3D* e);
	void Data();
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
	FilePoolCtrl file_pool;
	//RemoteDebugCtrl v_rdbg;
	MenuBar menu;
	ToolBar tool;
	
	Scene3DRenderConfig conf;
	Scene3DRenderContext render_ctx;
	
	
	VfsValue prj_val;
	GeomProject* prj = 0;
	VfsValue state_val;
	GeomWorldState* state = 0;
	VfsValue anim_val;
	GeomAnim* anim = 0;
	//GeomVideo video;
	GeomStagedVideo video;
	TimeCallback tc;
	TimeStop ts;
	String scene3d_path;
	String scene3d_created;
	String scene3d_modified;
	String scene3d_data_dir;
	Array<Scene3DExternalFile> scene3d_external_files;
	Array<Scene3DMetaEntry> scene3d_meta;
	bool scene3d_use_json = true;
	bool repeat_playback = false;
	
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
	void UpdateWindowTitle();
	void SetScene3DFormat(bool use_json);
	void ToggleRepeatPlayback();
	void OpenScene3D();
	void OpenFilePool();
	void SaveScene3DInteractive();
	void SaveScene3DAs();
	void SaveScene3DAsJson();
	void SaveScene3DAsBinary();
	bool SaveScene3DWithDialog(bool use_json);
	bool LoadScene3DWithDialog();
	bool IsScene3DBinaryPath(const String& path) const;
	bool IsScene3DJsonPath(const String& path) const;
	String EnsureScene3DExtension(const String& path, bool use_json) const;
	
	void LoadEmptyProject();
	void LoadTestProject(int test_i);
	void LoadWmrStereoPointcloud(String directory);
	bool LoadScene3D(const String& path);
	bool SaveScene3D(const String& path, bool use_json, bool pretty = true);
	
	GeomScene& GetActiveScene();
	
};


#endif

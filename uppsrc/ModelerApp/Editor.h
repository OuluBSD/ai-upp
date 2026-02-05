#ifndef _ModelerApp_Editor_h_
#define _ModelerApp_Editor_h_

struct Edit3D;

struct GeomProjectCtrl : Ctrl {
	Edit3D* e;
	
	TreeArrayCtrl tree;
	TreeArrayCtrl props;
	FixedGridCtrl grid;
	TimelineCtrl time;
	EditRendererBase* rends[4] = {0};
	EditRendererV1 rends_v1[4];
	EditRendererV2 rends_v2[4];
	int rend_version[4] = {2, 2, 2, 2};
	
	int tree_scenes = -1;
	int tree_col_visible = -1;
	int tree_col_locked = -1;
	int tree_col_read = -1;
	int tree_col_write = -1;
	Index<hash_t> warned_tree_types;
	int props_col_value = -1;

	struct TreeNodeRef {
		enum Kind {
			K_VFS,
			K_PROGRAM,
			K_FOCUS,
		};
		Kind kind = K_VFS;
		VfsValue* vfs = 0;
	};

	Vector<TreeNodeRef> tree_nodes;
	bool program_read = true;
	bool program_write = false;
	bool focus_read = true;
	bool focus_write = false;
	int focus_tree_id = -1;
	GeomObject* selected_obj = 0;
	TreeNodeRef* selected_ref = 0;
	GeomPointcloudDataset* selected_dataset = 0;
	bool props_refreshing = false;

	struct PropRef {
		enum Kind {
			P_ROOT,
			P_TRANSFORM,
			P_POSITION,
			P_ORIENTATION,
			P_EFFECTS,
			P_EFFECT_POSITION,
			P_EFFECT_ORIENTATION,
			P_COMPONENTS,
			P_SCRIPT,
			P_SCRIPT_FILE,
			P_SCRIPT_ENABLED,
			P_SCRIPT_RUN_ON_LOAD,
			P_SCRIPT_RUN_EACH_FRAME,
			P_SCRIPT_EDIT,
			P_SCRIPT_RUN,
			P_POINTCLOUD,
			P_DATASET,
		};
		Kind kind = P_ROOT;
		GeomScript* script = 0;
	};

	Vector<PropRef> props_nodes;
	Vector<One<Ctrl>> props_ctrls;
	
	
	typedef GeomProjectCtrl CLASSNAME;
	GeomProjectCtrl(Edit3D* e);
	void Update(double dt);
	void Data();
	void TimelineData();
	void TreeSelect();
	void TreeMenu(Bar& bar);
	void PropsMenu(Bar& bar);
	void UpdateTreeFocus(int new_id);
	void PropsData();
	void PropsApply();
	TreeNodeRef* GetNodeRef(const Value& v);
	GeomObject* GetNodeObject(const Value& v);
	GeomPointcloudDataset* GetNodeDataset(const Value& v);
	void OnCursor(int kp_i);
	void TreeValue(int id, VfsValue& node);
	void RefreshRenderer(int i);
	void RefreshAll();
	void BuildViewMenu(Bar& bar, int i);
	void SetRendererVersion(int i, int version);
	void RebuildGrid();
	
};

struct FilePoolCtrl : ParentCtrl {
	Edit3D* owner = 0;
	ArrayCtrl files;

	typedef FilePoolCtrl CLASSNAME;
	FilePoolCtrl(Edit3D* e);
	void Data();
};

struct ScriptEditorDlg : TopWindow {
	CodeEditor editor;
	ToolBar tool;
	String path;
	bool dirty = false;
	
	typedef ScriptEditorDlg CLASSNAME;
	ScriptEditorDlg();
	void OpenFile(const String& p);
	void Save();
	void SaveAs(const String& p);
	void OnChange();
};

struct HmdCapture {
	HMD::System sys;
	One<StereoSource> source;
	HMD::SoftHmdFusion fusion;
	Image bright;
	Image dark;
	int64 bright_serial = -1;
	int64 dark_serial = -1;
	bool running = false;
	bool recording = false;

	typedef HmdCapture CLASSNAME;
	bool Start();
	void Stop();
	void ResetTracking();
	void Poll();
	bool IsRunning() const { return running; }
	bool IsRecording() const { return recording; }
	const Octree* GetPointcloud(bool bright) const;
};

struct Edit3D : DockWindow {
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
	DockableCtrl* dock_tree = 0;
	DockableCtrl* dock_props = 0;
	DockableCtrl* dock_time = 0;
	DockableCtrl* dock_video = 0;
	DockableCtrl* dock_pool = 0;
	
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
	HmdCapture hmd;
	SyntheticPointcloudConfig sim_cfg;
	SyntheticPointcloudState sim_state;
	PointcloudObservation sim_obs;
	Vector<ControllerObservation> sim_ctrl_obs;
	PointcloudPose sim_fake_hmd_pose;
	PointcloudPose sim_localized_pose;
	bool sim_has_state = false;
	bool sim_has_obs = false;
	bool sim_has_ctrl_obs = false;
	bool sim_observation_effect_visible = true;
	bool sim_observation_effect_locked = false;
	GeomObject* sim_pointcloud_obj = 0;
	GeomObject* sim_observation_obj = 0;
	GeomObject* sim_controller_obj[2] = {0, 0};
	GeomObject* sim_controller_model_obj[2] = {0, 0};
	GeomObject* sim_hmd_pointcloud_obj = 0;
	One<Octree> sim_obs_octree;
	String scene3d_path;
	String scene3d_created;
	String scene3d_modified;
	String scene3d_data_dir;
	Array<Scene3DExternalFile> scene3d_external_files;
	Array<Scene3DMetaEntry> scene3d_meta;
	bool scene3d_use_json = true;
	bool repeat_playback = false;
	bool record_pointcloud = false;
	GeomObject* hmd_pointcloud = 0;
	String project_dir;
	TimeStop script_timer;

	enum EditTool {
		TOOL_SELECT,
		TOOL_POINT,
		TOOL_LINE,
		TOOL_FACE,
		TOOL_ERASE,
	};
	enum EditPlaneMode {
		PLANE_VIEW,
		PLANE_XY,
		PLANE_XZ,
		PLANE_YZ,
		PLANE_LOCAL,
	};
	EditTool edit_tool = TOOL_SELECT;
	EditPlaneMode edit_plane = PLANE_VIEW;
	int edit_line_start = -1;
	Vector<int> edit_face_points;
	double edit_pick_radius_px = 10.0;
	bool edit_snap_enable = false;
	double edit_snap_step = 0.1;
	bool edit_snap_local = true;
	
	struct ScriptInstance {
		GeomScript* script = 0;
		VfsValue* owner = 0;
		String abs_path;
		Time file_time;
		PyVM vm;
		bool loaded = false;
		bool has_load = false;
		bool has_start = false;
		bool has_frame = false;
		Vector<PyIR> main_ir;
		Vector<PyIR> load_ir;
		Vector<PyIR> start_ir;
		Vector<PyIR> frame_ir;
	};
	Array<ScriptInstance> script_instances;
	One<ScriptEditorDlg> script_editor;
	
	struct ScriptEventHandler : Moveable<ScriptEventHandler> {
		String event;
		PyValue func;
		PyVM* vm = nullptr;
		VfsValue* node = nullptr;
	};
	Vector<ScriptEventHandler> script_event_handlers;
	
	void CreateDefaultInit();
	void CreateDefaultPostInit();
	void LoadTestCirclingCube();
	void LoadTestOctree();
	void LoadTestHmdPointcloud();
	void EnsureHmdSceneObjects();
	void EnsureSimSceneObjects();
	void TogglePointcloudRecording();
	void StartPointcloudRecording();
	void StopPointcloudRecording();
	void UpdateHmdCameraPose();
	void RunSyntheticPointcloudSimDialog();
	String RunLocalizationLog(bool show_dialog);
	String RunControllerLocalizationLog(bool show_dialog);
	void RefreshSimObservation();
	void DebugGeneratePointcloud();
	void DebugSimulateObservation();
	void DebugRunLocalization();
	void DebugRunControllerLocalization();
	void DebugSimulateControllerObservations();
	void DebugClearSynthetic();
	void GenerateSyntheticPointcloudFor(GeomObject& obj);
	void EnsureScriptInstances();
	void UpdateScriptInstance(ScriptInstance& inst, bool force_reload);
	void RunScriptOnLoad(ScriptInstance& inst, bool force);
	void RunScriptOnStart(ScriptInstance& inst, bool force);
	void RunScriptFrame(ScriptInstance& inst, double dt);
	void AddScriptEventHandler(const String& event, PyVM* vm, VfsValue* node, const PyValue& func);
	void RemoveScriptEventHandlers(PyVM* vm);
	void DispatchScriptEvent(const String& event, VfsValue* node, const PyValue& payload);
	void DispatchInputEvent(const String& type, const Point& p, dword flags, int key, int view_i);
	void DispatchFrameEvents(double dt);
	void RegisterScriptVM(PyVM& vm);
	void SetEditTool(EditTool tool);
	void CreateEditableMeshObject();
	bool ScreenToPlaneWorldPoint(int view_i, const Point& p, const vec3& origin, const vec3& normal, vec3& out) const;
	int PickNearestPoint(const GeomEditableMesh& mesh, int view_i, const Point& p, double radius_px) const;
	void RemoveEditablePoint(GeomEditableMesh& mesh, int idx);
	void OpenScriptEditor(GeomScript& script);
	void RunScriptOnce(GeomScript& script);
	GeomScript& AddScriptComponent(GeomObject& obj);
	GeomScript& AddScriptComponent(GeomDirectory& dir);
	GeomScript& AddScriptComponent(GeomScene& scene);
	void GetScriptsFromNode(VfsValue& node, Vector<GeomScript*>& out);
	String EnsureScriptFile(GeomScript& script, String base_name);
	String GetScriptAbsPath(const String& rel) const;
	void SetProjectDir(String dir);
	const String& GetProjectDir() const { return project_dir; }
	void SyncPointcloudDatasetsExternalFiles();
	
public:
	typedef Edit3D CLASSNAME;
	Edit3D();
	
	void SetView(ViewType view);
	virtual void DockInit();
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
	void RunSyntheticSimVisual(bool log_stdout, bool verbose);
	bool LoadScene3D(const String& path);
	bool SaveScene3D(const String& path, bool use_json, bool pretty = true);
	
	GeomScene& GetActiveScene();
	
};


#endif

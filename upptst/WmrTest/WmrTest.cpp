#include <CtrlLib/CtrlLib.h>
#include <SoftHMD/SoftHMD.h>
#include <Ctrl/Camera/Camera.h>
#include <atomic>

using namespace Upp;

namespace {

enum TrackViewMode {
	TRACKVIEW_YZ,
	TRACKVIEW_XZ,
	TRACKVIEW_XY,
	TRACKVIEW_PERSPECTIVE,
};

enum PointcloudMode {
	POINTCLOUD_BRIGHT,
	POINTCLOUD_DARK,
	POINTCLOUD_BOTH,
};

struct TrackCamera {
	vec3 position = vec3(0, 0, 2);
	quat orientation = Identity<quat>();
	float scale = 1.0f;
	float fov = 120.0f;

	void LoadCamera(TrackViewMode mode, Camera& cam, Size sz, float far = 1000.0f) const {
		float aspect = (float)sz.cx / (float)sz.cy;
		vec3 cam_pos = position;
		quat cam_orient = orientation;
		float len = 2.0f;
		bool move_camera = true;

		switch (mode) {
		case TRACKVIEW_YZ:
			cam_orient = MatQuat(YRotation(M_PI / 2));
			cam.SetOrthographic(len * aspect, len, 0.1f, far);
			break;
		case TRACKVIEW_XZ:
			cam_orient = MatQuat(XRotation(-M_PI / 2));
			cam.SetOrthographic(len * aspect, len, 0.1f, far);
			break;
		case TRACKVIEW_XY:
			cam_orient = MatQuat(YRotation(0));
			cam.SetOrthographic(len * aspect, len, 0.1f, far);
			break;
		case TRACKVIEW_PERSPECTIVE:
			cam.SetPerspective(fov, aspect, 0.1f, far);
			move_camera = false;
			break;
		}

		if (move_camera)
			cam_pos = cam_pos - VecMul(QuatMat(cam_orient), VEC_FWD) * scale * 0.01f;
		cam.SetWorld(cam_pos, cam_orient, scale);
	}
};

void DrawRect3D(Size sz, Draw& d, const mat4& view, const vec3& p, Size rect_sz, const Color& c) {
	vec3 pp = VecMul(view, p);
	float x = (pp[0] + 1) * 0.5f * sz.cx - rect_sz.cx / 2;
	float y = (-pp[1] + 1) * 0.5f * sz.cy - rect_sz.cy / 2;
	d.DrawRect((int)x, (int)y, rect_sz.cx, rect_sz.cy, c);
}

void DrawLine3D(Size sz, Draw& d, const mat4& view, const vec3& a, const vec3& b, int line_width, const Color& c) {
	vec3 ap = VecMul(view, a);
	vec3 bp = VecMul(view, b);
	if ((ap[0] < -1 || ap[0] > +1) && (ap[1] < -1 || ap[1] > +1) &&
		(bp[0] < -1 || bp[0] > +1) && (bp[1] < -1 || bp[1] > +1))
		return;
	float x0 = (ap[0] + 1) * 0.5f * sz.cx;
	float x1 = (bp[0] + 1) * 0.5f * sz.cx;
	float y0 = (-ap[1] + 1) * 0.5f * sz.cy;
	float y1 = (-bp[1] + 1) * 0.5f * sz.cy;
	d.DrawLine((int)x0, (int)y0, (int)x1, (int)y1, line_width, c);
}

struct TrackRenderer : public Ctrl {
	HMD::SoftHmdVisualTracker* bright = 0;
	HMD::SoftHmdVisualTracker* dark = 0;
	HMD::SoftHmdFusion* fusion = 0;
	TrackViewMode view_mode = TRACKVIEW_PERSPECTIVE;
	PointcloudMode cloud_mode = POINTCLOUD_BRIGHT;
	TrackCamera camera;

	void SetTrackers(HMD::SoftHmdVisualTracker* b, HMD::SoftHmdVisualTracker* d) { bright = b; dark = d; }
	void SetFusion(HMD::SoftHmdFusion* f) { fusion = f; }
	void SetViewMode(TrackViewMode m) { view_mode = m; }
	void SetPointcloudMode(PointcloudMode m) { cloud_mode = m; }

	virtual void Paint(Draw& d) override {
		Size sz = GetSize();
		d.DrawRect(sz, Black());
		if (!bright && !dark) {
			d.DrawText(10, 10, "No tracker", Arial(16).Bold(), White());
			return;
		}

		const Octree* octree_bright = bright ? bright->GetPointcloud() : 0;
		const Octree* octree_dark = dark ? dark->GetPointcloud() : 0;
		const Octree* octree = 0;
		if (cloud_mode == POINTCLOUD_BRIGHT)
			octree = octree_bright;
		else if (cloud_mode == POINTCLOUD_DARK)
			octree = octree_dark;
		
		if (cloud_mode != POINTCLOUD_BOTH && !octree) {
			d.DrawText(10, 10, "No pointcloud", Arial(16).Bold(), White());
			return;
		}

		Camera cam;
		camera.LoadCamera(view_mode, cam, sz);
		Frustum frustum = cam.GetFrustum();
		mat4 view = cam.GetViewMatrix();

		if (cloud_mode == POINTCLOUD_BOTH) {
			if (octree_bright) {
				OctreeFrustumIterator iter = const_cast<Octree*>(octree_bright)->GetFrustumIterator(frustum);
				while (iter) {
					const OctreeNode& n = *iter;
					for (const auto& one_obj : n.objs) {
						const OctreeObject& obj = *one_obj;
						vec3 pos = obj.GetPosition();
						DrawRect3D(sz, d, view, pos, Size(2, 2), LtYellow());
					}
					iter++;
				}
			}
			if (octree_dark) {
				OctreeFrustumIterator iter = const_cast<Octree*>(octree_dark)->GetFrustumIterator(frustum);
				while (iter) {
					const OctreeNode& n = *iter;
					for (const auto& one_obj : n.objs) {
						const OctreeObject& obj = *one_obj;
						vec3 pos = obj.GetPosition();
						DrawRect3D(sz, d, view, pos, Size(2, 2), LtBlue());
					}
					iter++;
				}
			}
		}
		else {
			OctreeFrustumIterator iter = const_cast<Octree*>(octree)->GetFrustumIterator(frustum);
			while (iter) {
				const OctreeNode& n = *iter;
				for (const auto& one_obj : n.objs) {
					const OctreeObject& obj = *one_obj;
					vec3 pos = obj.GetPosition();
					DrawRect3D(sz, d, view, pos, Size(2, 2), White());
				}
				iter++;
			}
		}

		FusionState fs;
		bool has_pose = fusion && fusion->GetState(fs);
		vec3 pos = has_pose ? fs.position : (bright ? bright->GetPosition() : vec3(0,0,0));
		quat orient = has_pose ? fs.orientation : (bright ? bright->GetOrientation() : Identity<quat>());
		if (has_pose || (bright && bright->HasPose())) {
			mat4 rot = QuatMat(orient);
			vec3 axes[3] = { vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) };
			Color clr[3] = { LtRed(), LtGreen(), LtBlue() };
			float len = 0.2f;
			for (int i = 0; i < 3; i++) {
				vec3 axis = VecMul(rot, axes[i]) * len;
				DrawLine3D(sz, d, view, pos, pos + axis, 2, clr[i]);
			}
			DrawRect3D(sz, d, view, pos, Size(3, 3), Yellow());
		}
	}
};

struct TrackingCtrl : public Ctrl {
	FixedGridCtrl grid;
	TrackRenderer rends[4];

	TrackingCtrl() {
		grid.SetGridSize(2, 2);
		for (int i = 0; i < 4; i++)
			grid.Add(rends[i]);
		rends[0].SetViewMode(TRACKVIEW_YZ);
		rends[1].SetViewMode(TRACKVIEW_XZ);
		rends[2].SetViewMode(TRACKVIEW_XY);
		rends[3].SetViewMode(TRACKVIEW_PERSPECTIVE);
		Add(grid.SizePos());
	}

	void SetTrackers(HMD::SoftHmdVisualTracker* bright, HMD::SoftHmdVisualTracker* dark) {
		for (int i = 0; i < 4; i++)
			rends[i].SetTrackers(bright, dark);
	}
	
	void SetFusion(HMD::SoftHmdFusion* fusion) {
		for (int i = 0; i < 4; i++)
			rends[i].SetFusion(fusion);
	}
	
	void SetPointcloudMode(PointcloudMode mode) {
		for (int i = 0; i < 4; i++)
			rends[i].SetPointcloudMode(mode);
	}
};

} // namespace

static StereoOverlayData ToOverlayData(const HMD::StereoOverlay& src) {
	StereoOverlayData out;
	out.left_size = src.left_size;
	out.right_size = src.right_size;
	out.left_points <<= src.left_points;
	out.right_points <<= src.right_points;
	out.match_left <<= src.match_left;
	out.match_right <<= src.match_right;
	return out;
}

static StereoTrackerStatsData ToStatsData(const HMD::StereoTrackerStats& src) {
	StereoTrackerStatsData out;
	out.frame_count = src.frame_count;
	out.processed_frames = src.processed_frames;
	out.skipped_frames = src.skipped_frames;
	out.last_left_keypoints = src.last_left_keypoints;
	out.last_right_keypoints = src.last_right_keypoints;
	out.last_tracked_points = src.last_tracked_points;
	out.last_tracked_triangles = src.last_tracked_triangles;
	out.last_stereo_matches = src.last_stereo_matches;
	out.last_process_usecs = src.last_process_usecs;
	out.has_pose = src.has_pose;
	return out;
}

class WmrTest : public TopWindow {
public:
	typedef WmrTest CLASSNAME;
	
	HMD::System sys;
	One<StereoSource> source;
	HMD::SoftHmdFusion fusion;
	
	StereoOverlayCtrl camera;
	TrackingCtrl tracking;
	ArrayCtrl list;
	Splitter splitter;
	TabCtrl tabs;
	ParentCtrl camera_tab;
	ParentCtrl tracking_tab;
	TimeCallback tc;
	MenuBar menu;
	PointcloudMode cloud_mode = POINTCLOUD_BRIGHT;
	bool show_descriptors = true;
	bool show_descriptor_ids = false;
	bool show_match_lines = false;
	bool show_match_ids = false;
	bool show_stats_overlay = true;
	bool show_split_view = true;
	bool capture_enabled = true;
	bool calibration_ok = true;
	bool calibration_prompted = false;
	String calibration_error;
	HMD::StereoCalibrationData calib;
	
	VectorMap<String, String> data;
	int async_buffers;
	int transfer_timeout_ms;
	bool heavy_checks;
	String last_track_stream;

	Thread background_thread;
	std::atomic<bool> background_quit;
	Mutex background_mutex;
	Image background_bright, background_dark;
	TransformMatrix background_trans;
	ControllerMatrix background_ev3d;
	
	BiVector<Image> frame_history;

public:
	WmrTest(int async_buffers_, int transfer_timeout_ms_, bool heavy_checks_) {
		async_buffers = async_buffers_;
		transfer_timeout_ms = transfer_timeout_ms_;
		heavy_checks = heavy_checks_;
		Title("WMR / HMD Test");
		Sizeable().Zoomable();
		
		AddFrame(menu);
		menu.Set(THISBACK(MainMenu));
		Add(tabs.SizePos());
		camera_tab.Add(splitter.Horz(camera, list).SizePos());
		tracking_tab.Add(tracking.SizePos());
		tabs.Add(camera_tab.SizePos(), "Camera");
		tabs.Add(tracking_tab.SizePos(), "Tracking");
		splitter.SetPos(7500);
		
		list.AddColumn("Key");
		list.AddColumn("Value");
		
		if(!sys.Initialise()) {
			data.Add("Error", "Failed to initialise HMD system");
		}
		else {
			const bool needs_calib = (sys.vendor_id == 0x03f0 || sys.vendor_id == 0x04b4);
			if (needs_calib) {
				String path = "share/calibration/hp_vr1000/calibration.stcal";
				if (!FileExists(path)) {
					calibration_ok = false;
					calibration_error = "Missing calibration: " + path;
					data.Add("Calibration Error", calibration_error);
					if (!calibration_prompted) {
						calibration_prompted = true;
						PostCallback([=] {
							PromptOK("Calibration missing.\n\nPlease deploy or load a valid calibration file before tracking.");
						});
					}
				}
			}
			fusion.GetBrightTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
			fusion.GetDarkTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
			calib = fusion.GetBrightTracker().GetCalibration();
		}
		
		source = CreateStereoSource("hmd");
		if (!source) {
			data.Add("Camera Error", "Missing HMD stereo source");
		} else {
			if(async_buffers > 0)
				source->SetOption("async_buffers", async_buffers);
			if(transfer_timeout_ms >= 0)
				source->SetOption("transfer_timeout_ms", transfer_timeout_ms);
			if (!calibration_ok) {
				capture_enabled = false;
			}
			else if(!source->Start()) {
				data.Add("Camera Error", "Failed to open HMD camera");
			}
		}

		tracking.SetTrackers(&fusion.GetBrightTracker(), &fusion.GetDarkTracker());
		tracking.SetFusion(&fusion);
		camera.SetShowDescriptors(show_descriptors);
		camera.SetShowDescriptorIds(show_descriptor_ids);
		camera.SetShowMatchLines(show_match_lines);
		camera.SetShowMatchIds(show_match_ids);
		camera.SetShowStatsOverlay(show_stats_overlay);
		camera.SetShowSplitView(show_split_view);
		capture_enabled = calibration_ok && source && source->IsRunning();
		
		// Initial refresh of list to show error if any
		for(int j = 0; j < data.GetCount(); j++) {
			list.Add(data.GetKey(j), data[j]);
		}
		
		background_quit = false;
		background_thread.Start(THISBACK(BackgroundProcess));
		
		tc.Set(-1000/60, THISBACK(Data));
	}
	
	~WmrTest() {
		tc.Kill();
		background_quit = true;
		background_thread.Wait();
		if(source) source->Stop();
		source.Clear();
		sys.Uninitialise();
	}
	
	void CheckVerticalDiscontinuities(const Image& img) {
		Size sz = img.GetSize();
		int64 total_diff = 0;
		int count = 0;
		// Sample differences between adjacent columns
		for(int y = 0; y < sz.cy; y += 10) {
			for(int x = 0; x < sz.cx - 1; x += 10) {
				RGBA a = img[y][x];
				RGBA b = img[y][x+1];
				int diff = abs((int)a.g - (int)b.g);
				total_diff += diff;
				count++;
			}
		}
		if (count == 0) return;
		int avg_diff = (int)(total_diff / count);
		
		int discontinuities = 0;
		for(int x = 0; x < sz.cx - 1; x++) {
			int col_diff = 0;
			int rows = 0;
			for(int y = 0; y < sz.cy; y += 5) {
				RGBA a = img[y][x];
				RGBA b = img[y][x+1];
				col_diff += abs((int)a.g - (int)b.g);
				rows++;
			}
			if (rows == 0) continue;
			int avg_col_diff = col_diff / rows;
			// Threshold: if a column jump is significantly higher than average image noise
			if (avg_col_diff > avg_diff * 5 && avg_col_diff > 30) {
				discontinuities++;
			}
		}
		
		ASSERT(discontinuities < 3); // "Too many vertical discontinuities detected in bright frame!"
	}

	void CheckFrameBrightness(const Image& img) {
		Size sz = img.GetSize();
		int64 sum = 0;
		for(int y = 0; y < sz.cy; y += 5) {
			for(int x = 0; x < sz.cx; x += 5) {
				sum += img[y][x].g;
			}
		}
		int samples = (sz.cy / 5) * (sz.cx / 5);
		if (samples == 0) return;
		int avg = (int)(sum / samples);
		
		// If average brightness is extremely low, it might be a dark frame mislabeled as bright
		ASSERT(avg > 10); // "Bright frame is too dark! Possible frame mismatch."
	}

	void BackgroundProcess() {
		int frames_checked = 0;
		while (!background_quit) {
			sys.UpdateData();
			
			{
				Mutex::Lock __(background_mutex);
				background_trans = sys.trans;
				background_ev3d = sys.ev3d;
			}

			// IMU processing
			if(sys.hmd) {
				ImuSample imu;
				imu.timestamp_us = usecs();
				bool has_imu = false;
				if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_ACCELEROMETER_VECTOR, imu.accel.data) == HMD::HMD_S_OK)
					has_imu = true;
				if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_GYROSCOPE_VECTOR, imu.gyro.data) == HMD::HMD_S_OK)
					has_imu = true;
				if(has_imu)
					fusion.PutImu(imu);
			}

			if (source && source->IsRunning()) {
				CameraFrame lf, rf;
				if (!source->ReadFrame(lf, rf, false)) {
					Sleep(1);
					continue;
				}

				Image combined;
				if (!JoinStereoImage(lf.img, rf.img, combined)) {
					Sleep(1);
					continue;
				}
				
				// Delivery images to GUI immediately to avoid frozen preview during heavy tracking
				{
					Mutex::Lock __(background_mutex);
					if (lf.is_bright) background_bright = combined;
					else background_dark = combined;
				}

				// Prevent use-after-free race condition by keeping image ref alive
				// for a short while, in case fusion.PutVisual is async/queued.
				frame_history.AddTail(combined);
				if(frame_history.GetCount() > 20)
					frame_history.DropHead();

				if(heavy_checks && lf.is_bright) {
					frames_checked++;
					if(frames_checked > 6) { // Wait for a few frames to settle
						CheckVerticalDiscontinuities(combined);
						CheckFrameBrightness(combined);
					}
				}

				VisualFrame vf;
				vf.timestamp_us = usecs();
				vf.format = GEOM_EVENT_CAM_RGBA8;
				vf.width = combined.GetWidth();
				vf.height = combined.GetHeight();
				vf.stride = vf.width * (int)sizeof(RGBA);
				vf.img = combined;
				vf.data = 0;
				vf.data_bytes = combined.GetLength() * (int)sizeof(RGBA);
				vf.flags = lf.is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
				
				if (calibration_ok)
					fusion.PutVisual(vf);
			}
			else {
				Sleep(10);
			}
		}
	}

	void Data() {
		switch (tabs.Get()) {
			case 0: DataCameraTab(); break;
		}
	}

	void DataCameraTab() {
		data.Clear();
		
		TransformMatrix trans;
		ControllerMatrix ev3d;
		{
			Mutex::Lock __(background_mutex);
			camera.SetImages(background_bright, background_dark);
			trans = background_trans;
			ev3d = background_ev3d;
		}

		if(source) {
			data.Add("Camera", source->IsRunning() ? "Open" : "Closed");
			if(source->IsRunning()) {
				VectorMap<String, String> cam_stats;
				source->GetStatsMap(cam_stats);
				for (int i = 0; i < cam_stats.GetCount(); i++)
					data.Add(cam_stats.GetKey(i), cam_stats[i]);
			}
		} else {
			data.Add("Camera", "Missing");
		}
		if (!calibration_ok) {
			data.Add("Calibration Error", calibration_error.IsEmpty() ? "Missing calibration" : calibration_error);
		}

		HMD::StereoTrackerStats tb = fusion.GetBrightTracker().GetStats();
		HMD::StereoTrackerStats td = fusion.GetDarkTracker().GetStats();
		HMD::StereoCalibrationData calib_state = fusion.GetBrightTracker().GetCalibration();
		camera.SetStats(true, ToStatsData(tb));
		camera.SetStats(false, ToStatsData(td));
		if (calibration_ok) {
			data.Add("Track Bright Frames", IntStr(tb.processed_frames));
			data.Add("Track Bright Skips", IntStr(tb.skipped_frames));
			data.Add("Track Bright Keypoints", Format("%d / %d", tb.last_left_keypoints, tb.last_right_keypoints));
			data.Add("Track Bright Matches", IntStr(tb.last_stereo_matches));
			data.Add("Track Bright Points", IntStr(tb.last_tracked_points));
			data.Add("Track Bright Triangles", IntStr(tb.last_tracked_triangles));
			data.Add("Track Bright Process (usecs)", IntStr(tb.last_process_usecs));
			data.Add("Track Dark Frames", IntStr(td.processed_frames));
			data.Add("Track Dark Skips", IntStr(td.skipped_frames));
			data.Add("Track Dark Keypoints", Format("%d / %d", td.last_left_keypoints, td.last_right_keypoints));
			data.Add("Track Dark Matches", IntStr(td.last_stereo_matches));
			data.Add("Track Dark Points", IntStr(td.last_tracked_points));
			data.Add("Track Dark Triangles", IntStr(td.last_tracked_triangles));
			data.Add("Track Dark Process (usecs)", IntStr(td.last_process_usecs));
			data.Add("Track Last Stream", last_track_stream.IsEmpty() ? "-" : last_track_stream);
		} else {
			data.Add("Tracking", "Disabled (missing calibration)");
		}
		data.Add("Calib Enabled", calib_state.is_enabled ? "Yes" : "No");
		data.Add("Calib Eye Dist", Format("%g", (double)calib_state.eye_dist));
		data.Add("Calib Outward Angle", Format("%g", (double)calib_state.outward_angle));
		data.Add("Calib Angle Poly", Format("%g, %g, %g, %g",
			(double)calib_state.angle_to_pixel[0], (double)calib_state.angle_to_pixel[1],
			(double)calib_state.angle_to_pixel[2], (double)calib_state.angle_to_pixel[3]));
		FusionState fs;
		if(fusion.GetState(fs)) {
			data.Add("Track Position", fs.position.ToString());
			data.Add("Track Orientation", fs.orientation.ToString());
		}
		else {
			data.Add("Track Position", "-");
			data.Add("Track Orientation", "-");
		}

		if(!background_bright.IsEmpty())
			data.Add("Camera Resolution", Format("%d x %d (x2)", background_bright.GetWidth(), background_bright.GetHeight()));

		// HMD Transform
		data.Add("HMD Orientation", trans.orientation.ToString());
		data.Add("HMD Position", trans.position.ToString());
		data.Add("HMD Eye Dist", Format("%f", trans.eye_dist));

		// Raw sensor data from HMD device if opened
		if(sys.hmd) {
			float f[3];
			if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_ACCELEROMETER_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("HMD Accel", Format("%f, %f, %f", f[0], f[1], f[2]));
			if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_GYROSCOPE_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("HMD Gyro", Format("%f, %f, %f", f[0], f[1], f[2]));
		}

		// Controllers
		const char* ctrl_names[] = {"Left Controller", "Right Controller"};
		const char* button_names[] = {
			"GENERIC", "TRIGGER", "TRIGGER_CLICK", "SQUEEZE", "MENU", "HOME",
			"ANALOG_X0", "ANALOG_X1", "ANALOG_X2", "ANALOG_X3",
			"ANALOG_Y0", "ANALOG_Y1", "ANALOG_Y2", "ANALOG_Y3",
			"ANALOG_PRESS0", "ANALOG_PRESS1", "ANALOG_PRESS2", "ANALOG_PRESS3",
			"BUTTON_A", "BUTTON_B", "BUTTON_X", "BUTTON_Y",
			"VOLUME_PLUS", "VOLUME_MINUS", "MIC_MUTE"
		};

		for(int i = 0; i < 2; i++) {
			ControllerMatrix::Ctrl& h = ev3d.ctrl[i];
			String prefix = ctrl_names[i];
			data.Add(prefix + " Enabled", h.is_enabled ? "Yes" : "No");
			if(h.is_enabled) {
				data.Add(prefix + " Orient", h.trans.orientation.ToString());
				data.Add(prefix + " Pos", h.trans.position.ToString());
				for(int b = 0; b < (int)ControllerMatrix::VALUE_COUNT; b++) {
					if(h.is_value[b]) {
						data.Add(prefix + " " + button_names[b], Format("%f", h.value[b]));
					}
				}
			}
		}

		HMD::StereoOverlay bright_overlay;
		if (fusion.GetBrightTracker().GetOverlay(bright_overlay))
			camera.SetOverlay(true, ToOverlayData(bright_overlay));
		else
			camera.ClearOverlay(true);
		
		HMD::StereoOverlay dark_overlay;
		if (fusion.GetDarkTracker().GetOverlay(dark_overlay))
			camera.SetOverlay(false, ToOverlayData(dark_overlay));
		else
			camera.ClearOverlay(false);
		
		tracking.Refresh();

		// Refresh list while preserving selection if possible
		int scroll = list.GetScroll();
		int cursor = list.GetCursor();
		list.Clear();
		for(int j = 0; j < data.GetCount(); j++) {
			list.Add(data.GetKey(j), data[j]);
		}
		list.ScrollTo(scroll);
		list.SetCursor(cursor);

		camera.Refresh();
	}

	void MainMenu(Bar& bar) {
		bar.Sub("App", THISBACK(AppMenu));
		bar.Sub("View", THISBACK(ViewMenu));
		bar.Sub("Help", THISBACK(HelpMenu));
	}
	
	void AppMenu(Bar& bar) {
		if (capture_enabled)
			bar.Add("Stop Capture", [=] { StopCapture(); });
		else
			bar.Add("Start Capture", [=] { StartCapture(); });
		bar.Add("Load Calibration...", [=] { LoadCalibration(); });
		bar.Add("Save Calibration...", [=] { SaveCalibration(); });
		bar.Separator();
		bar.Add("Exit", [=] { Close(); });
	}
	
	void ViewMenu(Bar& bar) {
		bar.Add("Show descriptors", [=] { ToggleShowDescriptors(); })
		   .Check(show_descriptors);
		bar.Add("Show descriptor IDs", [=] { ToggleShowDescriptorIds(); })
		   .Check(show_descriptor_ids);
		bar.Add("Show match lines", [=] { ToggleShowMatchLines(); })
		   .Check(show_match_lines);
		bar.Add("Show match IDs", [=] { ToggleShowMatchIds(); })
		   .Check(show_match_ids);
		bar.Add("Show stats overlay", [=] { ToggleShowStatsOverlay(); })
		   .Check(show_stats_overlay);
		bar.Add("Show split view", [=] { ToggleShowSplitView(); })
		   .Check(show_split_view);
		bar.Separator();
		bar.Add("Pointcloud: Bright", [=] { SetPointcloudMode(POINTCLOUD_BRIGHT); })
		   .Check(cloud_mode == POINTCLOUD_BRIGHT);
		bar.Add("Pointcloud: Dark", [=] { SetPointcloudMode(POINTCLOUD_DARK); })
		   .Check(cloud_mode == POINTCLOUD_DARK);
		bar.Add("Pointcloud: Both", [=] { SetPointcloudMode(POINTCLOUD_BOTH); })
		   .Check(cloud_mode == POINTCLOUD_BOTH);
	}
	
	void HelpMenu(Bar& bar) {
		bar.Add("About", [=] {
			PromptOK("WmrTest\n\nTracking + fusion debug tool.");
		});
	}
	
	void SetPointcloudMode(PointcloudMode mode) {
		cloud_mode = mode;
		tracking.SetPointcloudMode(mode);
		tracking.Refresh();
	}

	void ToggleShowDescriptors() {
		show_descriptors = !show_descriptors;
		camera.SetShowDescriptors(show_descriptors);
		camera.Refresh();
	}

	void ToggleShowDescriptorIds() {
		show_descriptor_ids = !show_descriptor_ids;
		camera.SetShowDescriptorIds(show_descriptor_ids);
		camera.Refresh();
	}

	void ToggleShowMatchLines() {
		show_match_lines = !show_match_lines;
		camera.SetShowMatchLines(show_match_lines);
		camera.Refresh();
	}

	void ToggleShowMatchIds() {
		show_match_ids = !show_match_ids;
		camera.SetShowMatchIds(show_match_ids);
		camera.Refresh();
	}

	void ToggleShowStatsOverlay() {
		show_stats_overlay = !show_stats_overlay;
		camera.SetShowStatsOverlay(show_stats_overlay);
		camera.Refresh();
	}

	void ToggleShowSplitView() {
		show_split_view = !show_split_view;
		camera.SetShowSplitView(show_split_view);
		camera.Refresh();
	}
	
	void StartCapture() {
		if (!calibration_ok) {
			PromptOK("Cannot start capture: calibration missing. Load calibration first.");
			return;
		}
		if (!source)
			return;
		if (source->IsRunning())
			return;
		if (!source->Start()) {
			PromptOK("Failed to start capture.");
			return;
		}
		capture_enabled = true;
	}
	
	void StopCapture() {
		if (!source)
			return;
		if (!source->IsRunning())
			return;
		source->Stop();
		capture_enabled = false;
		camera.SetImages(Image(), Image());
		camera.ClearOverlay(true);
		camera.ClearOverlay(false);
		camera.Refresh();
	}
	
	void LoadCalibration() {
		FileSel fs;
		fs.Type("Stereo Calibration", "*.stcal");
		fs.AllFilesType();
		if (!fs.ExecuteOpen("Load Stereo Calibration"))
			return;
		String path = fs;
		HMD::StereoCalibrationData loaded;
		if (!HMD::StereoTracker::LoadCalibrationFile(path, loaded)) {
			PromptOK("Failed to load calibration file.");
			return;
		}
		calib = loaded;
		ApplyCalibration(calib);
		calibration_ok = true;
		calibration_error.Clear();
		capture_enabled = source && source->IsRunning();
		PromptOK("Stereo calibration loaded.");
	}
	
	void SaveCalibration() {
		FileSel fs;
		fs.Type("Stereo Calibration", "*.stcal");
		fs.AllFilesType();
		if (!fs.ExecuteSaveAs("Save Stereo Calibration"))
			return;
		String path = fs;
		HMD::StereoCalibrationData data = calib;
		if (!data.is_enabled)
			data = fusion.GetBrightTracker().GetCalibration();
		if (!HMD::StereoTracker::SaveCalibrationFile(path, data)) {
			PromptOK("Failed to save calibration file.");
			return;
		}
		PromptOK("Stereo calibration saved.");
	}

	void ApplyCalibration(const HMD::StereoCalibrationData& data) {
		fusion.GetBrightTracker().SetCalibration(data);
		fusion.GetDarkTracker().SetCalibration(data);
	}

};

void TestDump(int seconds, int async_buffers, int transfer_timeout_ms)
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	One<StereoSource> source = CreateStereoSource("hmd");
	if (!source) {
		Cout() << "Camera Error: Missing HMD stereo source\n";
		return;
	}
	if(async_buffers > 0)
		source->SetOption("async_buffers", async_buffers);
	if(transfer_timeout_ms >= 0)
		source->SetOption("transfer_timeout_ms", transfer_timeout_ms);
	
	if(!source->Start()) {
		Cout() << "Camera Error: Failed to open HMD camera\n";
	}

	HMD::System sys;
	if(!sys.Initialise()) {
		Cout() << "Error: Failed to initialise HMD system\n";
	}
	
	TimeStop ts;
	while(ts.Elapsed() < seconds * 1000) {
		sys.UpdateData();
		VectorMap<String, String> stats;
		if (source)
			source->GetStatsMap(stats);

		String frames = stats.Get("Cam Frame Count", "?");
		String last = stats.Get("Cam Last Transferred", "?");
		String err = stats.Get("Cam Last Error (r)", "?");
		String bright = stats.Get("Cam Avg Brightness", "?");
		Cout() << "\r" << Format("Frames: %s, Last: %s, Error: %s, Bright: %s, HMD: %s",
			frames, last, err, bright, sys.hmd ? "Yes" : "No");
		
		Sleep(100);
	}
	Cout() << "\n";
	
	TransformMatrix& trans = sys.trans;
	Cout() << "Final HMD Orientation: " << trans.orientation.ToString() << "\n";
	Cout() << "Final HMD Position: " << trans.position.ToString() << "\n";
	
	if (source)
		source->Stop();
	source.Clear();
	sys.Uninitialise();
}

void TestTrack(int seconds, int async_buffers, int transfer_timeout_ms)
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	One<StereoSource> source = CreateStereoSource("hmd");
	if (!source) {
		Cout() << "Camera Error: Missing HMD stereo source\n";
		return;
	}
	if(async_buffers > 0)
		source->SetOption("async_buffers", async_buffers);
	if(transfer_timeout_ms >= 0)
		source->SetOption("transfer_timeout_ms", transfer_timeout_ms);
	
	if(!source->Start()) {
		Cout() << "Camera Error: Failed to open HMD camera\n";
	}
	
HMD::System sys;
	if(!sys.Initialise()) {
		Cout() << "Error: Failed to initialise HMD system\n";
	}
	
HMD::SoftHmdFusion fusion;
	fusion.GetBrightTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
	fusion.GetDarkTracker().SetWmrDefaults(sys.vendor_id, sys.product_id);
	TimeStop ts;
	while(ts.Elapsed() < seconds * 1000) {
		sys.UpdateData();
		CameraFrame lf, rf;
		if (source && source->IsRunning() && source->ReadFrame(lf, rf, false)) {
			Image combined;
			if (JoinStereoImage(lf.img, rf.img, combined)) {
				VisualFrame vf;
				vf.timestamp_us = usecs();
				vf.format = GEOM_EVENT_CAM_RGBA8;
				vf.width = combined.GetWidth();
				vf.height = combined.GetHeight();
				vf.stride = vf.width * (int)sizeof(RGBA);
				vf.img = combined;
				vf.data = 0;
				vf.data_bytes = combined.GetLength() * (int)sizeof(RGBA);
				vf.flags = lf.is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
				fusion.PutVisual(vf);
			}
		}
		
		FusionState fs;
		if (fusion.GetState(fs)) {
			Cout() << "\r" << Format("Track: pos=%s orient=%s",
				fs.position.ToString(), fs.orientation.ToString());
		}
		Sleep(50);
	}
	Cout() << "\n";
	
	if (source)
		source->Stop();
	source.Clear();
	sys.Uninitialise();
}

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	const Vector<String>& args = CommandLine();
	int dump_time = -1;
	int track_time = -1;
	bool verbose = false;
	bool heavy_checks = false;
	int async_buffers = -1;
	int transfer_timeout_ms = -1;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--help" || args[i] == "-h") {
			Cout() << "Usage: WmrTest [options]\n\n" 
			       << "Options:\n"
			       << "  --test-dump <secs>    Run data dump test for n seconds\n"
			       << "  --test-track <secs>   Run tracking test for n seconds\n"
			       << "  --async-buffers <n>   Set USB async transfer count (default: 8)\n"
			       << "  --timeout-ms <ms>     Set USB transfer timeout (default: 1000)\n"
			       << "  --heavy-checks        Enable intensive debug checks (asserts on frame corruption)\n"
			       << "  --verbose, -v         Enable verbose logging\n"
			       << "  --help, -h            Show this help message\n";
			return;
		}
		if(args[i] == "--test-dump" && i + 1 < args.GetCount()) {
			dump_time = atoi(args[i+1]);
		}
		if(args[i] == "--test-track" && i + 1 < args.GetCount()) {
			track_time = atoi(args[i+1]);
		}
		if(args[i] == "-v" || args[i] == "--verbose") {
			verbose = true;
		}
		if(args[i] == "--heavy-checks") {
			heavy_checks = true;
		}
		if(args[i] == "--async-buffers" && i + 1 < args.GetCount()) {
			async_buffers = atoi(args[i+1]);
		}
		if(args[i] == "--timeout-ms" && i + 1 < args.GetCount()) {
			transfer_timeout_ms = atoi(args[i+1]);
		}
	}

	if(dump_time >= 0) {
		SetAssertFailedHook([](const char *s) {
			Cout() << "ASSERT: " << s << "\n";
			fflush(stdout);
			abort();
		});
		TestDump(dump_time, async_buffers, transfer_timeout_ms);
		return;
	}
	
	if(track_time >= 0) {
		SetAssertFailedHook([](const char *s) {
			Cout() << "ASSERT: " << s << "\n";
			fflush(stdout);
			abort();
		});
		TestTrack(track_time, async_buffers, transfer_timeout_ms);
		return;
	}

	WmrTest wt(async_buffers, transfer_timeout_ms, heavy_checks);
	if (wt.source)
		wt.source->SetVerbose(verbose);
	wt.Run();
}

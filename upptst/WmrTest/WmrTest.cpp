#include <CtrlLib/CtrlLib.h>
#include <SoftHMD/SoftHMD.h>

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

}

struct CameraCtrl : public Ctrl {
	Image bright, dark;
	HMD::StereoOverlay bright_overlay;
	HMD::StereoOverlay dark_overlay;
	HMD::StereoTrackerStats bright_stats;
	HMD::StereoTrackerStats dark_stats;
	bool has_bright_overlay = false;
	bool has_dark_overlay = false;
	bool show_descriptors = true;
	bool show_descriptor_ids = false;
	bool show_match_lines = false;
	bool show_match_ids = false;
	bool show_stats_overlay = true;
	bool show_split_view = true;

	void SetOverlay(bool is_bright, HMD::StereoOverlay& overlay) {
		if (is_bright) {
			Swap(bright_overlay, overlay);
			has_bright_overlay = true;
		}
		else {
			Swap(dark_overlay, overlay);
			has_dark_overlay = true;
		}
	}

	void ClearOverlay(bool is_bright) {
		if (is_bright) {
			bright_overlay.Clear();
			has_bright_overlay = false;
		}
		else {
			dark_overlay.Clear();
			has_dark_overlay = false;
		}
	}

	void SetStats(bool is_bright, const HMD::StereoTrackerStats& stats) {
		if (is_bright)
			bright_stats = stats;
		else
			dark_stats = stats;
	}

	void SetShowDescriptors(bool b) { show_descriptors = b; }
	void SetShowDescriptorIds(bool b) { show_descriptor_ids = b; }
	void SetShowMatchLines(bool b) { show_match_lines = b; }
	void SetShowMatchIds(bool b) { show_match_ids = b; }
	void SetShowStatsOverlay(bool b) { show_stats_overlay = b; }
	void SetShowSplitView(bool b) { show_split_view = b; }

	void DrawOverlay(Draw& w, const Rect& r, const Image& img, const HMD::StereoOverlay& overlay,
	                 const HMD::StereoTrackerStats& stats, const char* label,
	                 const Color& point_color, const Color& match_color) const {
		if (img.IsEmpty())
			return;
		Size src = img.GetSize();
		if (src.cx <= 0 || src.cy <= 0)
			return;
		if (!show_descriptors && !show_match_lines && !show_stats_overlay)
			return;

		Size dst_sz = r.GetSize();
		double sx = (double)dst_sz.cx / (double)src.cx;
		double sy = (double)dst_sz.cy / (double)src.cy;
		int right_offset = 0;
		if (overlay.left_size.cx > 0 && src.cx == overlay.left_size.cx * 2)
			right_offset = overlay.left_size.cx;

		auto MapPoint = [&](const vec2& p, int offset_x) -> Pointf {
			return Pointf((double)r.left + (p[0] + offset_x) * sx,
			              (double)r.top + p[1] * sy);
		};

		const int max_points = 512;
		if (show_descriptors) {
			int left_count = overlay.left_points.GetCount();
			if (left_count > max_points)
				left_count = max_points;
			for (int i = 0; i < left_count; i++) {
				Pointf pt = MapPoint(overlay.left_points[i], 0);
				w.DrawRect((int)pt.x - 1, (int)pt.y - 1, 3, 3, point_color);
				if (show_descriptor_ids)
					w.DrawText((int)pt.x + 2, (int)pt.y + 2, IntStr(i), Arial(9), point_color);
			}
			int right_count = overlay.right_points.GetCount();
			if (right_count > max_points)
				right_count = max_points;
			for (int i = 0; i < right_count; i++) {
				Pointf pt = MapPoint(overlay.right_points[i], right_offset);
				w.DrawRect((int)pt.x - 1, (int)pt.y - 1, 3, 3, point_color);
				if (show_descriptor_ids)
					w.DrawText((int)pt.x + 2, (int)pt.y + 2, IntStr(i), Arial(9), point_color);
			}
		}

		if (show_match_lines) {
			int match_count = overlay.match_left.GetCount();
			if (match_count > overlay.match_right.GetCount())
				match_count = overlay.match_right.GetCount();
			if (match_count > max_points)
				match_count = max_points;
			for (int i = 0; i < match_count; i++) {
				Pointf a = MapPoint(overlay.match_left[i], 0);
				Pointf b = MapPoint(overlay.match_right[i], right_offset);
				w.DrawLine((int)a.x, (int)a.y, (int)b.x, (int)b.y, 1, match_color);
				if (show_match_ids) {
					Pointf mid((a.x + b.x) * 0.5, (a.y + b.y) * 0.5);
					w.DrawText((int)mid.x + 2, (int)mid.y + 2, IntStr(i), Arial(9), match_color);
				}
			}
		}

		if (show_stats_overlay) {
			int match_count = overlay.match_left.GetCount();
			if (match_count > overlay.match_right.GetCount())
				match_count = overlay.match_right.GetCount();
			String line1 = Format("%s: frames=%d, kp=%d/%d, points=%d",
				label, stats.processed_frames, stats.last_left_keypoints,
				stats.last_right_keypoints, stats.last_tracked_points);
			String line2 = Format("tri=%d, matches=%d, usecs=%d, pose=%s",
				stats.last_tracked_triangles, match_count, stats.last_process_usecs,
				stats.has_pose ? "yes" : "no");
			w.DrawText(r.left + 6, r.top + 6, line1, Arial(12).Bold(), White());
			w.DrawText(r.left + 6, r.top + 22, line2, Arial(11), White());
		}
	}
	
	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, Black());
		int h = sz.cy / 2;
		bool draw_split = show_split_view;
		Rect bright_rc = draw_split ? RectC(0, 0, sz.cx, h) : RectC(0, 0, sz.cx, sz.cy);
		Rect dark_rc = draw_split ? RectC(0, h, sz.cx, h) : Rect();

		if (draw_split) {
			if (bright)
				w.DrawImage(bright_rc, bright);
			if (dark)
				w.DrawImage(dark_rc, dark);

			if (bright && has_bright_overlay)
				DrawOverlay(w, bright_rc, bright, bright_overlay, bright_stats, "Bright", LtYellow(), LtGreen());
			if (dark && has_dark_overlay)
				DrawOverlay(w, dark_rc, dark, dark_overlay, dark_stats, "Dark", LtBlue(), LtGreen());
		}
		else {
			if (bright) {
				w.DrawImage(bright_rc, bright);
				if (has_bright_overlay)
					DrawOverlay(w, bright_rc, bright, bright_overlay, bright_stats, "Bright", LtYellow(), LtGreen());
			}
			else if (dark) {
				w.DrawImage(bright_rc, dark);
				if (has_dark_overlay)
					DrawOverlay(w, bright_rc, dark, dark_overlay, dark_stats, "Dark", LtBlue(), LtGreen());
			}
		}
		
		if(!bright && !dark) {
			w.DrawText(10, 10, "No camera images", Arial(20).Bold(), White());
		}
	}
};

class WmrTest : public TopWindow {
public:
	typedef WmrTest CLASSNAME;
	
	HMD::System sys;
	One<HMD::Camera> cam;
	HMD::SoftHmdFusion fusion;
	
	CameraCtrl camera;
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
	HMD::StereoCalibrationData calib;
	
	VectorMap<String, String> data;
	int async_buffers;
	int transfer_timeout_ms;
	String last_track_stream;

public:
	WmrTest(int async_buffers_, int transfer_timeout_ms_) {
		async_buffers = async_buffers_;
		transfer_timeout_ms = transfer_timeout_ms_;
		Title("WMR / HMD Test");
		Sizeable().Zoomable();
		
		AddFrame(menu);
		menu.Set(THISBACK(MainMenu));
		Add(tabs.SizePos());
		camera_tab.Add(splitter.Horz(camera, list).SizePos());
		tracking_tab.Add(tracking.SizePos());
		tabs.Add(camera_tab, "Camera");
		tabs.Add(tracking_tab, "Tracking");
		splitter.SetPos(7500);
		
		list.AddColumn("Key");
		list.AddColumn("Value");
		
		if(!sys.Initialise()) {
			data.Add("Error", "Failed to initialise HMD system");
		}
		
		cam.Create();
		if(async_buffers > 0)
			cam->SetAsyncBuffers(async_buffers);
		if(transfer_timeout_ms >= 0)
			cam->SetTransferTimeoutMs(transfer_timeout_ms);
		if(!cam->Open()) {
			data.Add("Camera Error", "Failed to open HMD camera");
		}

		tracking.SetTrackers(&fusion.GetBrightTracker(), &fusion.GetDarkTracker());
		tracking.SetFusion(&fusion);
		camera.SetShowDescriptors(show_descriptors);
		camera.SetShowDescriptorIds(show_descriptor_ids);
		camera.SetShowMatchLines(show_match_lines);
		camera.SetShowMatchIds(show_match_ids);
		camera.SetShowStatsOverlay(show_stats_overlay);
		camera.SetShowSplitView(show_split_view);
		capture_enabled = cam && cam->IsOpen();
		
		// Initial refresh of list to show error if any
		for(int j = 0; j < data.GetCount(); j++) {
			list.Add(data.GetKey(j), data[j]);
		}
		
		tc.Set(-1000/60, THISBACK(Data));
	}
	
	~WmrTest() {
		tc.Kill();
		if(cam) cam->Close();
		cam.Clear();
		sys.Uninitialise();
	}
	
	void Data() {
		sys.UpdateData();
		data.Clear();

		if(cam) {
			Vector<HMD::CameraFrame> frames;
			cam->PopFrames(frames);
			int prev_bright = fusion.GetBrightTracker().GetStats().processed_frames;
			int prev_dark = fusion.GetDarkTracker().GetStats().processed_frames;
			for(const auto& f : frames) {
				if(f.is_bright) camera.bright = f.img;
				else camera.dark = f.img;
				
				VisualFrame vf;
				vf.timestamp_us = usecs();
				vf.format = GEOM_EVENT_CAM_RGBA8;
				vf.width = f.img.GetWidth();
				vf.height = f.img.GetHeight();
				vf.stride = vf.width * (int)sizeof(RGBA);
				vf.data = (const byte*)~f.img;
				vf.data_bytes = f.img.GetLength() * (int)sizeof(RGBA);
				vf.flags = f.is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
				fusion.PutVisual(vf);
				last_track_stream = f.is_bright ? "Bright" : "Dark";
			}
			if (fusion.GetBrightTracker().GetStats().processed_frames != prev_bright ||
			    fusion.GetDarkTracker().GetStats().processed_frames != prev_dark)
				tracking.Refresh();
			HMD::StereoOverlay bright_overlay;
			if (fusion.GetBrightTracker().GetOverlay(bright_overlay))
				camera.SetOverlay(true, bright_overlay);
			else
				camera.ClearOverlay(true);
			HMD::StereoOverlay dark_overlay;
			if (fusion.GetDarkTracker().GetOverlay(dark_overlay))
				camera.SetOverlay(false, dark_overlay);
			else
				camera.ClearOverlay(false);

			HMD::CameraStats cs = cam->GetStats();
			data.Add("Camera", cam->IsOpen() ? "Open" : "Closed");
			if(cam->IsOpen()) {
				data.Add("Cam Frame Count", IntStr(cs.frame_count));
				data.Add("Cam Bright Frames", IntStr(cs.bright_frames));
				data.Add("Cam Dark Frames", IntStr(cs.dark_frames));
				data.Add("Cam Bright Balance", IntStr(cs.bright_balance));
				data.Add("Cam Last Exposure", IntStr(cs.last_exposure));
				data.Add("Cam Last Transferred", IntStr(cs.last_transferred));
				data.Add("Cam Min/Max Transferred", Format("%d / %d", cs.min_transferred, cs.max_transferred));
				data.Add("Cam Last Error (r)", IntStr(cs.last_r));
				data.Add("Cam Mutex Fails", IntStr(cs.mutex_fails));
				data.Add("Cam USB Errors", IntStr(cs.usb_errors));
				data.Add("Cam Timeout Errors", IntStr(cs.timeout_errors));
				data.Add("Cam Overflow Errors", IntStr(cs.overflow_errors));
				data.Add("Cam Skips", IntStr(cs.other_errors));
				data.Add("Cam No-Device Errors", IntStr(cs.no_device_errors));
				data.Add("Cam Resubmit Fails", IntStr(cs.resubmit_failures));
				data.Add("Cam Resubmit Skips", IntStr(cs.resubmit_skips));
				data.Add("Cam Halt Clears", Format("%d / %d", cs.halt_clear_attempts, cs.halt_clear_failures));
				data.Add("Cam Async Buffers", IntStr(cs.async_buffers));
				data.Add("Cam Timeout (ms)", IntStr(cs.transfer_timeout_ms));
				data.Add("Cam Handle (usecs)", IntStr(cs.handle_usecs));
				data.Add("Cam Avg Brightness", Format("%.2f", cs.avg_brightness));
				data.Add("Cam Pixel Range", Format("%d - %d", (int)cs.min_pixel, (int)cs.max_pixel));
				static const char* status_names[] = {
					"COMPLETED", "ERROR", "TIMED_OUT", "CANCELLED", "STALL", "NO_DEVICE", "OVERFLOW"
				};
				const int status_name_count = (int)(sizeof(status_names) / sizeof(status_names[0]));
				String status_line;
				for(int i = 0; i <= LIBUSB_TRANSFER_OVERFLOW; i++) {
					if(i) status_line << ", ";
					if(i < status_name_count)
						status_line << status_names[i] << "=" << cs.status_counts[i];
					else
						status_line << i << "=" << cs.status_counts[i];
				}
				data.Add("Cam Status Counts", status_line);
			}
		} else {
			data.Add("Camera", "Missing");
		}

		HMD::StereoTrackerStats tb = fusion.GetBrightTracker().GetStats();
		HMD::StereoTrackerStats td = fusion.GetDarkTracker().GetStats();
		camera.SetStats(true, tb);
		camera.SetStats(false, td);
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
		FusionState fs;
		if(fusion.GetState(fs)) {
			data.Add("Track Position", fs.position.ToString());
			data.Add("Track Orientation", fs.orientation.ToString());
		}
		else {
			data.Add("Track Position", "-");
			data.Add("Track Orientation", "-");
		}

		if(camera.bright)
			data.Add("Camera Resolution", Format("%d x %d (x2)", camera.bright.GetWidth(), camera.bright.GetHeight()));

		// HMD Transform
		data.Add("HMD Orientation", sys.trans.orientation.ToString());
		data.Add("HMD Position", sys.trans.position.ToString());
		data.Add("HMD Eye Dist", Format("%f", sys.trans.eye_dist));

		// Raw sensor data from HMD device if opened
		if(sys.hmd) {
			float f[3];
			if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_ACCELEROMETER_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("HMD Accel", Format("%f, %f, %f", f[0], f[1], f[2]));
			if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_GYROSCOPE_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("HMD Gyro", Format("%f, %f, %f", f[0], f[1], f[2]));
			
			ImuSample imu;
			imu.timestamp_us = usecs();
			if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_ACCELEROMETER_VECTOR, imu.accel.data) == HMD::HMD_S_OK)
				fusion.PutImu(imu);
			else if(HMD::GetDeviceFloat(sys.hmd, HMD::HMD_GYROSCOPE_VECTOR, imu.gyro.data) == HMD::HMD_S_OK)
				fusion.PutImu(imu);
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
			ControllerMatrix::Ctrl& h = sys.ev3d.ctrl[i];
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
		if (!cam)
			return;
		if (cam->IsOpen())
			return;
		if (!cam->Open()) {
			PromptOK("Failed to start capture.");
			return;
		}
		capture_enabled = true;
	}
	
	void StopCapture() {
		if (!cam)
			return;
		if (!cam->IsOpen())
			return;
		cam->Close();
		capture_enabled = false;
		camera.bright.Clear();
		camera.dark.Clear();
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
		if (!LoadCalibrationFile(path, loaded)) {
			PromptOK("Failed to load calibration file.");
			return;
		}
		calib = loaded;
		ApplyCalibration(calib);
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
		if (!SaveCalibrationFile(path, data)) {
			PromptOK("Failed to save calibration file.");
			return;
		}
		PromptOK("Stereo calibration saved.");
	}

	void ApplyCalibration(const HMD::StereoCalibrationData& data) {
		fusion.GetBrightTracker().SetCalibration(data);
		fusion.GetDarkTracker().SetCalibration(data);
	}

	bool SaveCalibrationFile(const String& path, const HMD::StereoCalibrationData& data) {
		Vector<String> lines;
		lines.Add("enabled=" + String(data.is_enabled ? "1" : "0"));
		lines.Add(Format("eye_dist=%g", (double)data.eye_dist));
		lines.Add(Format("outward_angle=%g", (double)data.outward_angle));
		lines.Add(Format("angle_poly=%g,%g,%g,%g",
			(double)data.angle_to_pixel[0], (double)data.angle_to_pixel[1],
			(double)data.angle_to_pixel[2], (double)data.angle_to_pixel[3]));
		String text = Join(lines, "\n") + "\n";
		return SaveFile(path, text);
	}

	bool LoadCalibrationFile(const String& path, HMD::StereoCalibrationData& out) {
		String text = LoadFile(path);
		if (text.IsEmpty())
			return false;
		HMD::StereoCalibrationData data;
		Vector<String> lines = Split(text, '\n');
		for (String line : lines) {
			line = TrimBoth(line);
			if (line.IsEmpty() || line[0] == '#')
				continue;
			int eq = line.Find('=');
			if (eq < 0)
				continue;
			String key = TrimBoth(line.Left(eq));
			String val = TrimBoth(line.Mid(eq + 1));
			if (key == "enabled")
				data.is_enabled = atoi(val) != 0;
			else if (key == "eye_dist")
				data.eye_dist = (float)atof(val);
			else if (key == "outward_angle")
				data.outward_angle = (float)atof(val);
			else if (key == "angle_poly") {
				Vector<String> parts = Split(val, ',');
				if (parts.GetCount() >= 4) {
					data.angle_to_pixel[0] = (float)atof(parts[0]);
					data.angle_to_pixel[1] = (float)atof(parts[1]);
					data.angle_to_pixel[2] = (float)atof(parts[2]);
					data.angle_to_pixel[3] = (float)atof(parts[3]);
				}
			}
		}
		out = data;
		return true;
	}

};

void TestDump(int seconds, int async_buffers, int transfer_timeout_ms)
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	One<HMD::Camera> cam;
	cam.Create();
	if(async_buffers > 0)
		cam->SetAsyncBuffers(async_buffers);
	if(transfer_timeout_ms >= 0)
		cam->SetTransferTimeoutMs(transfer_timeout_ms);
	
	if(!cam->Open()) {
		Cout() << "Camera Error: Failed to open HMD camera\n";
	}

	HMD::System sys;
	if(!sys.Initialise()) {
		Cout() << "Error: Failed to initialise HMD system\n";
	}
	
	TimeStop ts;
	while(ts.Elapsed() < seconds * 1000) {
		sys.UpdateData();
		HMD::CameraStats cs = cam->GetStats();
		
		Cout() << "\r" << Format("Frames: %d, Last: %d, Error: %d, Bright: %.2f, HMD: %s", 
			cs.frame_count, cs.last_transferred, cs.last_r, cs.avg_brightness,
			sys.hmd ? "Yes" : "No");
		
		Sleep(100);
	}
	Cout() << "\n";
	
	TransformMatrix& trans = sys.trans;
	Cout() << "Final HMD Orientation: " << trans.orientation.ToString() << "\n";
	Cout() << "Final HMD Position: " << trans.position.ToString() << "\n";
	
	cam->Close();
	cam.Clear();
	sys.Uninitialise();
}

void TestTrack(int seconds, int async_buffers, int transfer_timeout_ms)
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	One<HMD::Camera> cam;
	cam.Create();
	if(async_buffers > 0)
		cam->SetAsyncBuffers(async_buffers);
	if(transfer_timeout_ms >= 0)
		cam->SetTransferTimeoutMs(transfer_timeout_ms);
	
	if(!cam->Open()) {
		Cout() << "Camera Error: Failed to open HMD camera\n";
	}
	
	HMD::System sys;
	if(!sys.Initialise()) {
		Cout() << "Error: Failed to initialise HMD system\n";
	}
	
	HMD::SoftHmdFusion fusion;
	TimeStop ts;
	while(ts.Elapsed() < seconds * 1000) {
		sys.UpdateData();
		Vector<HMD::CameraFrame> frames;
		cam->PopFrames(frames);
		for(const auto& f : frames) {
			VisualFrame vf;
			vf.timestamp_us = usecs();
			vf.format = GEOM_EVENT_CAM_RGBA8;
			vf.width = f.img.GetWidth();
			vf.height = f.img.GetHeight();
			vf.stride = vf.width * (int)sizeof(RGBA);
			vf.data = (const byte*)~f.img;
			vf.data_bytes = f.img.GetLength() * (int)sizeof(RGBA);
			vf.flags = f.is_bright ? VIS_FRAME_BRIGHT : VIS_FRAME_DARK;
			fusion.PutVisual(vf);
		}
		
		FusionState fs;
		if (fusion.GetState(fs)) {
			Cout() << "\r" << Format("Track: pos=%s orient=%s",
				fs.position.ToString(), fs.orientation.ToString());
		}
		Sleep(50);
	}
	Cout() << "\n";
	
	cam->Close();
	cam.Clear();
	sys.Uninitialise();
}

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	const Vector<String>& args = CommandLine();
	int dump_time = -1;
	int track_time = -1;
	bool verbose = false;
	int async_buffers = -1;
	int transfer_timeout_ms = -1;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--test-dump" && i + 1 < args.GetCount()) {
			dump_time = atoi(args[i+1]);
		}
		if(args[i] == "--test-track" && i + 1 < args.GetCount()) {
			track_time = atoi(args[i+1]);
		}
		if(args[i] == "-v" || args[i] == "--verbose") {
			verbose = true;
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

	WmrTest wt(async_buffers, transfer_timeout_ms);
	wt.cam->SetVerbose(verbose);
	wt.Run();
}

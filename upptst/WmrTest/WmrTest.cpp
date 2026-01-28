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
	HMD::SoftHmdVisualTracker* tracker = 0;
	TrackViewMode view_mode = TRACKVIEW_PERSPECTIVE;
	TrackCamera camera;

	void SetTracker(HMD::SoftHmdVisualTracker* t) { tracker = t; }
	void SetViewMode(TrackViewMode m) { view_mode = m; }

	virtual void Paint(Draw& d) override {
		Size sz = GetSize();
		d.DrawRect(sz, Black());
		if (!tracker) {
			d.DrawText(10, 10, "No tracker", Arial(16).Bold(), White());
			return;
		}

		const Octree* octree = tracker->GetPointcloud();
		if (!octree) {
			d.DrawText(10, 10, "No pointcloud", Arial(16).Bold(), White());
			return;
		}

		Camera cam;
		camera.LoadCamera(view_mode, cam, sz);
		Frustum frustum = cam.GetFrustum();
		mat4 view = cam.GetViewMatrix();

		OctreeFrustumIterator iter = octree->GetFrustumIterator(frustum);
		while (iter) {
			const OctreeNode& n = *iter;
			for (const auto& one_obj : n.objs) {
				const OctreeObject& obj = *one_obj;
				vec3 pos = obj.GetPosition();
				DrawRect3D(sz, d, view, pos, Size(2, 2), White());
			}
			iter++;
		}

		if (tracker->HasPose()) {
			vec3 pos = tracker->GetPosition();
			quat orient = tracker->GetOrientation();
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

	void SetTracker(HMD::SoftHmdVisualTracker* tracker) {
		for (int i = 0; i < 4; i++)
			rends[i].SetTracker(tracker);
	}
};

}

struct CameraCtrl : public Ctrl {
	Image bright, dark;
	
	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, Black());
		int h = sz.cy / 2;
		if(bright) {
			w.DrawImage(0, 0, sz.cx, h, bright);
		}
		if(dark) {
			w.DrawImage(0, h, sz.cx, h, dark);
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
	HMD::SoftHmdVisualTracker tracker;
	
	CameraCtrl camera;
	TrackingCtrl tracking;
	ArrayCtrl list;
	Splitter splitter;
	TabCtrl tabs;
	ParentCtrl camera_tab;
	ParentCtrl tracking_tab;
	TimeCallback tc;
	
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

		tracking.SetTracker(&tracker);
		
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
			int prev_processed = tracker.GetStats().processed_frames;
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
				tracker.PutFrame(vf);
				last_track_stream = f.is_bright ? "Bright" : "Dark";
			}
			if (tracker.GetStats().processed_frames != prev_processed)
				tracking.Refresh();

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

		HMD::StereoTrackerStats ts = tracker.GetStats();
		data.Add("Track Frames", IntStr(ts.processed_frames));
		data.Add("Track Skips", IntStr(ts.skipped_frames));
		data.Add("Track Keypoints", Format("%d / %d", ts.last_left_keypoints, ts.last_right_keypoints));
		data.Add("Track Points", IntStr(ts.last_tracked_points));
		data.Add("Track Triangles", IntStr(ts.last_tracked_triangles));
		data.Add("Track Process (usecs)", IntStr(ts.last_process_usecs));
		data.Add("Track Last Stream", last_track_stream.IsEmpty() ? "-" : last_track_stream);
		if(ts.has_pose) {
			data.Add("Track Position", tracker.GetPosition().ToString());
			data.Add("Track Orientation", tracker.GetOrientation().ToString());
		} else {
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

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	const Vector<String>& args = CommandLine();
	int dump_time = -1;
	bool verbose = false;
	int async_buffers = -1;
	int transfer_timeout_ms = -1;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--test-dump" && i + 1 < args.GetCount()) {
			dump_time = atoi(args[i+1]);
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
		TestDump(dump_time, async_buffers, transfer_timeout_ms);
		return;
	}

	WmrTest wt(async_buffers, transfer_timeout_ms);
	wt.cam->SetVerbose(verbose);
	wt.Run();
}

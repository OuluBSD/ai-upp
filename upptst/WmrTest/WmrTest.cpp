#include <CtrlLib/CtrlLib.h>
#include <SoftHMD/SoftHMD.h>

using namespace Upp;

struct CameraCtrl : public Ctrl {
	Image img;
	
	virtual void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, Black());
		if(img) {
			w.DrawImage(0, 0, sz.cx, sz.cy, img);
		}
		else {
			w.DrawText(10, 10, "No camera image", Arial(20).Bold(), White());
		}
	}
};

class WmrTest : public TopWindow {
public:
	typedef WmrTest CLASSNAME;
	
	HMD::System sys;
	HMD::Camera cam;
	
	CameraCtrl camera;
	ArrayCtrl list;
	Splitter splitter;
	TimeCallback tc;
	
	VectorMap<String, String> data;

public:
	WmrTest() {
		Title("WMR / HMD Test");
		Sizeable().Zoomable();
		
		Add(splitter.Horz(camera, list).SizePos());
		splitter.SetPos(7500);
		
		list.AddColumn("Key");
		list.AddColumn("Value");
		
		if(!sys.Initialise()) {
			data.Add("Error", "Failed to initialise HMD system");
		}
		
		if(!cam.Open()) {
			data.Add("Camera Error", "Failed to open HMD camera");
		}
		
		// Initial refresh of list to show error if any
		for(int j = 0; j < data.GetCount(); j++) {
			list.Add(data.GetKey(j), data[j]);
		}
		
		tc.Set(-1000/60, THISBACK(Data));
	}
	
	~WmrTest() {
		tc.Kill();
		cam.Close();
		sys.Uninitialise();
	}
	
	void Data() {
		sys.UpdateData();
		camera.img = cam.GetImage();
		HMD::Camera::Stats cs = cam.GetStats();
		
		data.Clear();
		data.Add("Camera", cam.IsOpen() ? "Open" : "Closed");
		if(cam.IsOpen()) {
			data.Add("Cam Frame Count", IntStr(cs.frame_count));
			data.Add("Cam Bright Frames", IntStr(cs.bright_frames));
			data.Add("Cam Dark Frames", IntStr(cs.dark_frames));
			data.Add("Cam Last Exposure", IntStr(cs.last_exposure));
			data.Add("Cam Last Transferred", IntStr(cs.last_transferred));
			data.Add("Cam Min/Max Transferred", Format("%d / %d", cs.min_transferred, cs.max_transferred));
			data.Add("Cam Last Error (r)", IntStr(cs.last_r));
			data.Add("Cam Avg Brightness", Format("%.2f", cs.avg_brightness));
			data.Add("Cam Pixel Range", Format("%d - %d", (int)cs.min_pixel, (int)cs.max_pixel));
		}
		if(camera.img)
			data.Add("Camera Resolution", Format("%d x %d", camera.img.GetWidth(), camera.img.GetHeight()));
		
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

void TestDump(int seconds)
{
	HMD::System sys;
	HMD::Camera cam;
	
	if(!sys.Initialise()) {
		Cout() << "Error: Failed to initialise HMD system\n";
	}
	
	if(!cam.Open()) {
		Cout() << "Camera Error: Failed to open HMD camera\n";
	}
	
	TimeStop ts;
	while(ts.Elapsed() < seconds * 1000) {
		sys.UpdateData();
		HMD::Camera::Stats cs = cam.GetStats();
		
		Cout() << "\r" << Format("Frames: %d, Last: %d, Error: %d, Bright: %.2f", 
			cs.frame_count, cs.last_transferred, cs.last_r, cs.avg_brightness);
		
		Sleep(100);
	}
	Cout() << "\n";
	
	TransformMatrix& trans = sys.trans;
	Cout() << "Final HMD Orientation: " << trans.orientation.ToString() << "\n";
	Cout() << "Final HMD Position: " << trans.position.ToString() << "\n";
	
	cam.Close();
	sys.Uninitialise();
}

GUI_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	int dump_time = -1;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--test-dump" && i + 1 < args.GetCount()) {
			dump_time = atoi(args[i+1]);
		}
	}

	if(dump_time >= 0) {
		TestDump(dump_time);
		return;
	}

	WmrTest().Run();
}

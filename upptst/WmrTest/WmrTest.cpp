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
	
	HMD::Context* ctx;
	HMD::Device* hmd;
	
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
		
		ctx = HMD::CreateContext();
		hmd = NULL;
		
		if(ctx) {
			int n = HMD::ProbeContext(ctx);
			if(n > 0) {
				for(int i = 0; i < n; i++) {
					const char* vendor = HMD::GetListString(ctx, i, HMD::HMD_VENDOR);
					const char* product = HMD::GetListString(ctx, i, HMD::HMD_PRODUCT);
					const char* path = HMD::GetListString(ctx, i, HMD::HMD_PATH);
					int flags = 0;
					HMD::GetListInt(ctx, i, HMD::HMD_DEVICE_FLAGS, &flags);
					
					data.Add(Format("Device %d", i), Format("%s %s (%s)", vendor, product, path));
					
					if(!hmd && !(flags & HMD::HMD_DEVICE_FLAGS_NULL_DEVICE)) {
						hmd = HMD::OpenListDevice(ctx, i);
						if(!hmd) {
							data.Add("Error Opening", HMD::GetContextError(ctx));
						}
						else {
							data.Add("Opened Device", product);
						}
					}
				}
			}
			else {
				data.Add("Error", "No devices found");
			}
		}
		else {
			data.Add("Error", "Failed to create context");
		}
		
		// Initial refresh of list to show error if any
		for(int j = 0; j < data.GetCount(); j++) {
			list.Add(data.GetKey(j), data[j]);
		}
		
		tc.Set(-1000/60, THISBACK(Data));
	}
	
	~WmrTest() {
		tc.Kill();
		if(hmd) HMD::CloseDevice(hmd);
		if(ctx) HMD::DestroyContext(ctx);
	}
	
	void Data() {
		if(!ctx) return;
		
		HMD::UpdateContext(ctx);
		
		if(hmd) {
			data.Clear();
			
			float f[16];
			int i[16];
			
			if(HMD::GetDeviceFloat(hmd, HMD::HMD_ROTATION_QUAT, f) == HMD::HMD_S_OK)
				data.Add("Rotation", Format("%f, %f, %f, %f", f[0], f[1], f[2], f[3]));
			
			if(HMD::GetDeviceFloat(hmd, HMD::HMD_POSITION_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("Position", Format("%f, %f, %f", f[0], f[1], f[2]));

			if(HMD::GetDeviceFloat(hmd, HMD::HMD_ACCELEROMETER_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("Accelerometer", Format("%f, %f, %f", f[0], f[1], f[2]));

			if(HMD::GetDeviceFloat(hmd, HMD::HMD_GYROSCOPE_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("Gyroscope", Format("%f, %f, %f", f[0], f[1], f[2]));

			if(HMD::GetDeviceFloat(hmd, HMD::HMD_MAGNETOMETER_VECTOR, f) == HMD::HMD_S_OK)
				data.Add("Magnetometer", Format("%f, %f, %f", f[0], f[1], f[2]));
			
			int ctrl_count = 0;
			if(HMD::GetDeviceInt(hmd, HMD::HMD_CONTROL_COUNT, &ctrl_count) == HMD::HMD_S_OK) {
				data.Add("Control Count", IntStr(ctrl_count));
				
				float values[64];
				int hints[64];
				int types[64];
				
				HMD::GetDeviceFloat(hmd, HMD::HMD_CONTROLS_STATE, values);
				HMD::GetDeviceInt(hmd, HMD::HMD_CONTROLS_HINTS, hints);
				HMD::GetDeviceInt(hmd, HMD::HMD_CONTROLS_TYPES, types);
				
				const char* hint_names[] = {
					"generic", "trigger", "trigger_click", "squeeze", "menu", "home",
					"analog-x", "analog-y", "analog_press", "button-a", "button-b",
					"button-x", "button-y", "volume-up", "volume-down", "mic-mute"
				};
				
				for(int j = 0; j < ctrl_count; j++) {
					String key = Format("Ctrl %d (%s)", j, hints[j] < 16 ? hint_names[hints[j]] : "unknown");
					data.Add(key, Format("%f (%s)", values[j], types[j] == HMD::HMD_ANALOG ? "analog" : "digital"));
				}
			}
			
			if(HMD::GetDeviceInt(hmd, HMD::HMD_SCREEN_HORIZONTAL_RESOLUTION, &i[0]) == HMD::HMD_S_OK)
				data.Add("H-Res", IntStr(i[0]));
			if(HMD::GetDeviceInt(hmd, HMD::HMD_SCREEN_VERTICAL_RESOLUTION, &i[0]) == HMD::HMD_S_OK)
				data.Add("V-Res", IntStr(i[0]));
			
			// Refresh list
			list.Clear();
			for(int j = 0; j < data.GetCount(); j++) {
				list.Add(data.GetKey(j), data[j]);
			}
		}
		
		// camera.img = ... // Placeholder for future
		camera.Refresh();
	}
};

GUI_APP_MAIN
{
	WmrTest().Run();
}

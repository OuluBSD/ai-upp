#ifndef _AMP_AMPDeviceInventory_h_
#define _AMP_AMPDeviceInventory_h_

struct AmpDeviceInfo : Moveable<AmpDeviceInfo> {
	int index = -1;
	String description;
	String device_path;
	bool is_emulated = false;
	bool has_display = false;
	int64 dedicated_memory = 0;
};

Vector<AmpDeviceInfo> EnumerateAmpDevices();
bool SaveAmpDeviceInventory(const String& path, const Vector<AmpDeviceInfo>& devices);
bool LoadAmpDeviceInventory(const String& path, Vector<AmpDeviceInfo>& devices, String& error);
bool SelectAmpDevice(const Vector<AmpDeviceInfo>& devices, int index, AmpDeviceInfo& selected, String& error);

#endif

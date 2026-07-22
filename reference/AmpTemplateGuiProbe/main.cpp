#include "AmpTemplateGuiProbe.h"

NAMESPACE_UPP

static String GetArgValue(const Vector<String>& args, const String& name)
{
	for(int i = 0; i + 1 < args.GetCount(); i++)
		if(args[i] == name)
			return args[i + 1];
	return String();
}

static bool WriteAmpInventory(const String& path)
{
	Vector<AmpDeviceInfo> devices = EnumerateAmpDevices();
	return SaveAmpDeviceInventory(path, devices);
}

static bool ReadAmpInventory(const String& path, int index, String& device_path, String& description)
{
	Vector<AmpDeviceInfo> devices;
	String error;
	if(!LoadAmpDeviceInventory(path, devices, error)) {
		COUTLOG(Format("amp_inventory_load_failed path=%s error=%s", ~path, ~error));
		return false;
	}
	AmpDeviceInfo selected;
	if(!SelectAmpDevice(devices, index, selected, error)) {
		COUTLOG(Format("amp_inventory_select_failed error=%s", ~error));
		return false;
	}
	device_path = selected.device_path;
	description = selected.description;
	return true;
}

void RunAmpTemplateGuiProbe() {
	TopWindow window;
	window.Title("AMP GUI probe");
	window.Open();
#ifndef HAVE_SYSTEM_AMP
	COUTLOG("amp_gui_backend=native unavailable");
#else
	const Vector<String>& args = CommandLine();
	String inventory_path = GetArgValue(args, "--amp-inventory");
	if(!inventory_path.IsEmpty()) {
		if(!WriteAmpInventory(inventory_path) ) {
			COUTLOG("amp_inventory_status=fail");
		}
		else {
			COUTLOG("amp_inventory_status=pass");
		}
		window.Close();
		return;
	}

	String selected_path = "PCI\\VEN_10DE&DEV_1C03&SUBSYS_86181043&REV_A1\\4&5AC7D5A&0&0010";
	String selected_description = "explicit-device-path";
	String replay_path = GetArgValue(args, "--amp-device-inventory");
	String index_arg = GetArgValue(args, "--amp-device-index");
	if(!replay_path.IsEmpty()) {
		int index = index_arg.IsEmpty() ? 0 : StrInt(index_arg);
		if(!ReadAmpInventory(replay_path, index, selected_path, selected_description)) {
			COUTLOG(Format("amp_inventory_replay=fail path=%s index=%d", ~replay_path, index));
			window.Close();
			return;
		}
		COUTLOG(Format("amp_inventory_replay=pass path=%s index=%d device=%s", ~replay_path,
		               index, ~selected_description));
	}

	concurrency::accelerator device = concurrency::accelerator(selected_path.ToWString().ToStd());
	COUTLOG(Format("amp_gui_backend=native device=%s emulated=%d", ~selected_description,
	               device.is_emulated));
	Vector<int> values;
	for(int i = 0; i < 16; i++)
		values.Add(i);
	concurrency::array_view<int, 1> view(values.GetCount(), values.Begin());
	concurrency::parallel_for_each(device.default_view, view.extent,
	                               [=](concurrency::index<1> index) PARALLEL_AMP {
		view[index] += 100;
	});
	view.synchronize();
	int checksum = 0;
	for(int value : values)
		checksum += value;
	COUTLOG(Format("amp_gui_backend=native checksum=%d count=%d", checksum, values.GetCount()));
#endif
	window.Close();
}

END_UPP_NAMESPACE

GUI_APP_MAIN {
	Upp::RunAmpTemplateGuiProbe();
}

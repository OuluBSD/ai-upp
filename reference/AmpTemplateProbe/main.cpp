#include "AmpTemplateProbe.h"

CONSOLE_APP_MAIN {
	Upp::RunAmpTemplateProbe();
}

NAMESPACE_UPP

int RunAmpTemplateProbe() {
#ifndef HAVE_SYSTEM_AMP
	COUTLOG("amp_backend=native unavailable");
	return 2;
#else
	const Vector<String>& args = CommandLine();
	String inventory_path;
	String replay_path;
	String atlas_selftest_path;
	int device_index = 0;
	for(int i = 0; i + 1 < args.GetCount(); i++) {
		if(args[i] == "--amp-inventory")
			inventory_path = args[i + 1];
		else if(args[i] == "--amp-device-inventory")
			replay_path = args[i + 1];
		else if(args[i] == "--amp-device-index")
			device_index = StrInt(args[i + 1]);
	}
	for(const String& arg : args)
		if(arg == "--amp-atlas-selftest")
			atlas_selftest_path = "amp_atlas_selftest.json";
	if(!atlas_selftest_path.IsEmpty()) {
		AmpTemplateAtlasManifest manifest;
		manifest.atlas_name = "selftest-atlas";
		manifest.atlas_width = 32;
		manifest.atlas_height = 16;
		AmpTemplateAtlasEntry& entry = manifest.entries.Add();
		entry.id = "rank-0-board";
		entry.kind = "rank";
		entry.scale = "board";
		entry.width = 8;
		entry.height = 8;
		entry.preprocessing = "grayscale";
		String error;
		bool valid = manifest.Validate(error);
		entry.x = 25;
		bool rejected = !manifest.Validate(error);
		entry.x = 0;
		bool roundtrip = manifest.Save(atlas_selftest_path);
		AmpTemplateAtlasManifest loaded;
		roundtrip = roundtrip && loaded.Load(atlas_selftest_path, error);
		COUTLOG(Format("amp_atlas_selftest valid=%d bounds_rejected=%d roundtrip=%d",
		               valid, rejected, roundtrip));
		return valid && rejected && roundtrip ? 0 : 1;
	}
	if(!inventory_path.IsEmpty()) {
		Vector<AmpDeviceInfo> devices = EnumerateAmpDevices();
		bool ok = SaveAmpDeviceInventory(inventory_path, devices);
		COUTLOG(Format("amp_inventory_status=%s", ok ? "pass" : "fail"));
		return ok ? 0 : 1;
	}

	String selected_path = "PCI\\VEN_10DE&DEV_1C03&SUBSYS_86181043&REV_A1\\4&5AC7D5A&0&0010";
	String selected_description = "explicit-device-path";
	if(!replay_path.IsEmpty()) {
		Vector<AmpDeviceInfo> devices;
		String error;
		AmpDeviceInfo selected;
		if(!LoadAmpDeviceInventory(replay_path, devices, error) ||
		   !SelectAmpDevice(devices, device_index, selected, error)) {
			COUTLOG(Format("amp_inventory_replay=fail path=%s index=%d error=%s", ~replay_path,
			               device_index, ~error));
			return 1;
		}
		selected_path = selected.device_path;
		selected_description = selected.description;
		COUTLOG(Format("amp_inventory_replay=pass path=%s index=%d device=%s", ~replay_path,
		               device_index, ~selected_description));
	}

	try {
		concurrency::accelerator device(selected_path.ToWString().ToStd());
		COUTLOG(Format("amp_backend=native device=%s emulated=%d", ~selected_description,
		               device.is_emulated));
		Vector<int> values;
		for(int i = 0; i < 16; i++)
			values.Add(i);
		concurrency::array_view<int, 1> view(values.GetCount(), values.Begin());
		COUTLOG("amp_kernel=launch");
		concurrency::parallel_for_each(device.default_view, view.extent,
		                               [=](concurrency::index<1> index) PARALLEL_AMP {
			view[index] += 100;
		});
		view.synchronize();
		COUTLOG("amp_kernel=synchronized");
		int checksum = 0;
		for(int value : values)
			checksum += value;
		COUTLOG(Format("amp_backend=native checksum=%d count=%d", checksum, values.GetCount()));
		return checksum == 1720 ? 0 : 1;
	}
	catch(const std::exception& e) {
		COUTLOG(Format("amp_runtime_failure=%s", e.what()));
		return 1;
	}
#endif
}

END_UPP_NAMESPACE

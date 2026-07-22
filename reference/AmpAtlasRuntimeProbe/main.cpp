#include "AmpAtlasRuntimeProbe.h"

CONSOLE_APP_MAIN {
	Upp::RunAmpAtlasRuntimeProbe();
}

NAMESPACE_UPP

static bool ReadArguments(String& atlas_path, String& manifest_path,
	                       String& inventory_path, int& device_index)
{
	const Vector<String>& args = CommandLine();
	for(int i = 0; i + 1 < args.GetCount(); i++) {
		if(args[i] == "--atlas")
			atlas_path = args[i + 1];
		else if(args[i] == "--manifest")
			manifest_path = args[i + 1];
		else if(args[i] == "--amp-device-inventory")
			inventory_path = args[i + 1];
		else if(args[i] == "--amp-device-index")
			device_index = StrInt(args[i + 1]);
	}
	return !atlas_path.IsEmpty() && !manifest_path.IsEmpty() && !inventory_path.IsEmpty();
}

static byte Gray(const RGBA& p)
{
	return (byte)((p.r * 77 + p.g * 150 + p.b * 29) >> 8);
}

static bool BuildCompactTemplates(const Image& atlas,
	                              const AmpTemplateAtlasManifest& manifest,
	                              Vector<int>& pixels, int& nonempty, String& error)
{
	if(atlas.IsEmpty() || atlas.GetWidth() != manifest.atlas_width ||
	   atlas.GetHeight() != manifest.atlas_height) {
		error = "atlas dimensions do not match manifest";
		return false;
	}
	nonempty = 0;
	for(const AmpTemplateAtlasEntry& entry : manifest.entries) {
		int entry_nonempty = 0;
		for(int y = entry.y; y < entry.y + entry.height; y++) {
			const RGBA* row = atlas[y];
			for(int x = entry.x; x < entry.x + entry.width; x++) {
				byte value = row[x].a ? Gray(row[x]) : 0;
				pixels.Add((int)value);
				entry_nonempty += value != 0;
			}
		}
		if(entry_nonempty == 0) {
			error = Format("empty template entry: %s", ~entry.id);
			return false;
		}
		nonempty += entry_nonempty;
		COUTLOG(Format("amp_atlas_entry id=%s pixels=%d nonempty=%d",
		               ~entry.id, entry.width * entry.height, entry_nonempty));
	}
	return true;
}

int RunAmpAtlasRuntimeProbe()
{
	String atlas_path, manifest_path, inventory_path;
	int device_index = 0;
	if(!ReadArguments(atlas_path, manifest_path, inventory_path, device_index)) {
		COUTLOG("usage=--atlas <atlas.png> --manifest <manifest.json> "
		        "--amp-device-inventory <inventory.json> [--amp-device-index n]");
		return 2;
	}
#ifndef HAVE_SYSTEM_AMP
	COUTLOG("amp_atlas_runtime=native-unavailable");
	return 2;
#else
	int64 started = msecs();
	AmpTemplateAtlasManifest manifest;
	String error;
	if(!manifest.Load(manifest_path, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail manifest=%s", ~error));
		return 1;
	}
	Image atlas = StreamRaster::LoadFileAny(atlas_path);
	Vector<int> pixels;
	int nonempty = 0;
	if(!BuildCompactTemplates(atlas, manifest, pixels, nonempty, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail pixels=%s", ~error));
		return 1;
	}
	int64 prepare_ms = msecs() - started;
	Vector<AmpDeviceInfo> devices;
	AmpDeviceInfo selected;
	if(!LoadAmpDeviceInventory(inventory_path, devices, error) ||
	   !SelectAmpDevice(devices, device_index, selected, error)) {
		COUTLOG(Format("amp_atlas_runtime=fail device=%s", ~error));
		return 1;
	}
	try {
		concurrency::accelerator device(selected.device_path.ToWString().ToStd());
		concurrency::array_view<int, 1> view(pixels.GetCount(), pixels.Begin());
		COUTLOG(Format("amp_atlas_upload=launch device=%s pixels=%d",
		               ~selected.description, pixels.GetCount()));
		concurrency::parallel_for_each(device.default_view, view.extent,
		                               [=](concurrency::index<1> index) PARALLEL_AMP {
			view[index] = view[index] + (index[0] & 7);
		});
		view.synchronize();
		int checksum = 0;
		for(int value : pixels)
			checksum += value;
		int64 total_ms = msecs() - started;
		COUTLOG(Format("amp_atlas_runtime=pass entries=%d nonempty=%d pixels=%d "
		               "checksum=%d prepare_ms=%d total_ms=%d",
		               manifest.entries.GetCount(), nonempty, pixels.GetCount(), checksum,
		               prepare_ms, total_ms));
		return checksum > 0 ? 0 : 1;
	}
	catch(const std::exception& e) {
		COUTLOG(Format("amp_atlas_runtime=native-failure error=%s", e.what()));
		return 1;
	}
#endif
}

END_UPP_NAMESPACE

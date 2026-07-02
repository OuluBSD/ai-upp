#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmSessionPaths

VsmSessionPaths VsmSessionPaths::From(const String& root_path)
{
	VsmSessionPaths p;
	p.root            = root_path;
	p.frames_dir      = AppendFileName(root_path, "frames");
	p.crops_dir       = AppendFileName(root_path, "crops");
	p.events_dir      = AppendFileName(root_path, "events");
	p.annotations_dir = AppendFileName(root_path, "annotations");
	p.cache_dir       = AppendFileName(root_path, "cache");
	p.reports_dir     = AppendFileName(root_path, "reports");
	return p;
}

void VsmSessionPaths::Realize() const
{
	RealizeDirectory(root);
	RealizeDirectory(frames_dir);
	RealizeDirectory(crops_dir);
	RealizeDirectory(events_dir);
	RealizeDirectory(annotations_dir);
	RealizeDirectory(cache_dir);
	RealizeDirectory(reports_dir);
}

// ---------------------------------------------------------------------------
// Jsonize helpers

void VsmFrameAsset::Jsonize(JsonIO& json)
{
	json("frame_index",    frame_index)
	    ("relative_path",  relative_path)
	    ("format",         format);
	// Omit ts_ms when -1 (unknown) to keep manifests backward-compatible.
	if(json.IsStoring()) {
		if(ts_ms >= 0)
			json("ts_ms", ts_ms);
	} else {
		json("ts_ms", ts_ms);
	}
}

void VsmCropAsset::Jsonize(JsonIO& json)
{
	json("region_id",     region_id)
	    ("relative_path", relative_path)
	    ("format",        format);
}

void VsmAssetRef::Jsonize(JsonIO& json)
{
	json("relative_path", relative_path)
	    ("asset_type",    asset_type);
}

void VsmSessionManifest::Jsonize(JsonIO& json)
{
	json("schema",       schema)
	    ("session_id",   session_id)
	    ("source_type",  source_type)
	    ("created_at",   created_at)
	    ("frame_width",  frame_width)
	    ("frame_height", frame_height)
	    ("image_format", image_format)
	    ("frames",       frames)
	    ("crops",        crops);
}

// ---------------------------------------------------------------------------
// VsmSessionStore

bool VsmSessionStore::Create(const String& root_path, const String& session_id,
                              int frame_w, int frame_h, const String& source_type)
{
	paths_ = VsmSessionPaths::From(root_path);
	paths_.Realize();

	manifest_              = VsmSessionManifest();
	manifest_.session_id   = session_id;
	manifest_.source_type  = source_type;
	Time t = GetUtcTime();
	manifest_.created_at = Format("%04d-%02d-%02dT%02d:%02d:%02d.000Z",
	                               t.year, t.month, t.day, t.hour, t.minute, t.second);
	manifest_.frame_width  = frame_w;
	manifest_.frame_height = frame_h;
	manifest_.image_format = "placeholder";

	if(!SaveManifest()) return false;
	is_open_ = true;
	LogInfo(log_, "VsmSession", "Created session '" + session_id + "' at: " + root_path);
	return true;
}

bool VsmSessionStore::Open(const String& root_path)
{
	if(!DirectoryExists(root_path)) {
		LogError(log_, "VsmSession", "Session directory not found: " + root_path);
		return false;
	}
	paths_   = VsmSessionPaths::From(root_path);
	is_open_ = false;
	if(!LoadManifest()) return false;
	is_open_ = true;
	LogInfo(log_, "VsmSession", "Opened session '" + manifest_.session_id + "' from: " + root_path);
	return true;
}

VsmAssetRef VsmSessionStore::AllocateFrame(int frame_index)
{
	// Check if already allocated
	for(const VsmFrameAsset& fa : manifest_.frames)
		if(fa.frame_index == frame_index) {
			VsmAssetRef ref;
			ref.relative_path = fa.relative_path;
			ref.asset_type    = "frame";
			return ref;
		}

	String filename = Format("frames/%08d.placeholder", frame_index);
	String abs_path = AppendFileName(paths_.root, filename);
	// Write placeholder bytes
	SaveFile(abs_path, Format("FRAME_PLACEHOLDER frame=%d\n", frame_index));

	VsmFrameAsset& fa = manifest_.frames.Add();
	fa.frame_index    = frame_index;
	fa.relative_path  = filename;
	fa.format         = "placeholder";

	VsmAssetRef ref;
	ref.relative_path = filename;
	ref.asset_type    = "frame";
	return ref;
}

VsmAssetRef VsmSessionStore::AllocateCrop(const String& region_id)
{
	for(const VsmCropAsset& ca : manifest_.crops)
		if(ca.region_id == region_id) {
			VsmAssetRef ref;
			ref.relative_path = ca.relative_path;
			ref.asset_type    = "crop";
			return ref;
		}

	String filename = "crops/" + region_id + ".placeholder";
	String abs_path = AppendFileName(paths_.root, filename);
	SaveFile(abs_path, Format("CROP_PLACEHOLDER region=%s\n", region_id));

	VsmCropAsset& ca  = manifest_.crops.Add();
	ca.region_id      = region_id;
	ca.relative_path  = filename;
	ca.format         = "placeholder";

	VsmAssetRef ref;
	ref.relative_path = filename;
	ref.asset_type    = "crop";
	return ref;
}

VsmAssetRef VsmSessionStore::SaveFrameImage(int frame_index, const VsmImageBuffer& img,
                                             int64 ts_ms)
{
	String filename = Format("frames/%08d.vsm", frame_index);
	String abs_path = AppendFileName(paths_.root, filename);
	if(!VsmImageAsset::Save(abs_path, img)) {
		LogError(log_, "VsmSession", "Cannot write frame image: " + abs_path);
		VsmAssetRef bad; return bad;
	}
	// Update or create manifest entry
	bool found = false;
	for(VsmFrameAsset& fa : manifest_.frames) {
		if(fa.frame_index == frame_index) {
			fa.relative_path = filename;
			fa.format        = "vsm";
			fa.ts_ms         = ts_ms;
			found            = true;
			break;
		}
	}
	if(!found) {
		VsmFrameAsset& fa = manifest_.frames.Add();
		fa.frame_index    = frame_index;
		fa.relative_path  = filename;
		fa.format         = "vsm";
		fa.ts_ms          = ts_ms;
	}
	manifest_.image_format = "vsm";
	VsmAssetRef ref;
	ref.relative_path = filename;
	ref.asset_type    = "frame";
	return ref;
}

bool VsmSessionStore::LoadFrameImage(int frame_index, VsmImageBuffer& out) const
{
	for(const VsmFrameAsset& fa : manifest_.frames) {
		if(fa.frame_index == frame_index && fa.format == "vsm") {
			String abs = AppendFileName(paths_.root, fa.relative_path);
			return VsmImageAsset::Load(abs, out);
		}
	}
	return false;
}

VsmAssetRef VsmSessionStore::SaveCropImage(const String& region_id, const VsmImageBuffer& img)
{
	String filename = "crops/" + region_id + ".vsm";
	String abs_path = AppendFileName(paths_.root, filename);
	if(!VsmImageAsset::Save(abs_path, img)) {
		LogError(log_, "VsmSession", "Cannot write crop image: " + abs_path);
		VsmAssetRef bad; return bad;
	}
	bool found = false;
	for(VsmCropAsset& ca : manifest_.crops) {
		if(ca.region_id == region_id) {
			ca.relative_path = filename;
			ca.format        = "vsm";
			found            = true;
			break;
		}
	}
	if(!found) {
		VsmCropAsset& ca  = manifest_.crops.Add();
		ca.region_id      = region_id;
		ca.relative_path  = filename;
		ca.format         = "vsm";
	}
	VsmAssetRef ref;
	ref.relative_path = filename;
	ref.asset_type    = "crop";
	return ref;
}

bool VsmSessionStore::LoadCropImage(const String& region_id, VsmImageBuffer& out) const
{
	for(const VsmCropAsset& ca : manifest_.crops) {
		if(ca.region_id == region_id && ca.format == "vsm") {
			String abs = AppendFileName(paths_.root, ca.relative_path);
			return VsmImageAsset::Load(abs, out);
		}
	}
	return false;
}

String VsmSessionStore::Resolve(const VsmAssetRef& ref) const
{
	if(ref.IsEmpty()) return String();
	return AppendFileName(paths_.root, ref.relative_path);
}

bool VsmSessionStore::SaveManifest()
{
	String path = AppendFileName(paths_.root, "manifest.json");
	String json = StoreAsJson(manifest_, true);
	if(!SaveFile(path, json)) {
		LogError(log_, "VsmSession", "Cannot write manifest: " + path);
		return false;
	}
	return true;
}

bool VsmSessionStore::LoadManifest()
{
	String path = AppendFileName(paths_.root, "manifest.json");
	String json = LoadFile(path);
	if(json.IsEmpty()) {
		LogError(log_, "VsmSession", "Cannot read manifest: " + path);
		return false;
	}
	String err;
	if(!LoadFromJson(manifest_, json)) {
		LogError(log_, "VsmSession", "Cannot parse manifest: " + path);
		return false;
	}
	return true;
}

} // namespace Upp

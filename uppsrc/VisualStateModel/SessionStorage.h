#ifndef _VisualStateModel_SessionStorage_h_
#define _VisualStateModel_SessionStorage_h_

namespace Upp {

// Session directory layout:
//   session/
//     manifest.json
//     frames/        raw frame placeholders or image files
//     crops/         cropped region assets
//     events/        replay.json and other event logs
//     annotations/   annotations.json
//     cache/         fingerprints and other derived data
//     reports/       generated markdown reports

struct VsmSessionPaths {
	String root;
	String frames_dir;
	String crops_dir;
	String events_dir;
	String annotations_dir;
	String cache_dir;
	String reports_dir;

	static VsmSessionPaths From(const String& root_path);
	void Realize() const;
};

struct VsmFrameAsset : Moveable<VsmFrameAsset> {
	int    frame_index = -1;
	String relative_path; // e.g. "frames/00000001.placeholder"
	String format;        // "placeholder", "jpg", "png"
	void Jsonize(JsonIO& json);
};

struct VsmCropAsset : Moveable<VsmCropAsset> {
	String region_id;
	String relative_path; // e.g. "crops/rgn-0001.placeholder"
	String format;
	void Jsonize(JsonIO& json);
};

struct VsmAssetRef : Moveable<VsmAssetRef> {
	String relative_path;
	String asset_type; // "frame", "crop", "report", "cache"
	bool IsEmpty() const { return relative_path.IsEmpty(); }
	void Jsonize(JsonIO& json);
};

struct VsmSessionManifest : Moveable<VsmSessionManifest> {
	int    schema       = 1;
	String session_id;
	String source_type;
	String created_at;
	int    frame_width  = 0;
	int    frame_height = 0;
	String image_format = "placeholder"; // "placeholder", "jpg", "png"
	Vector<VsmFrameAsset> frames;
	Vector<VsmCropAsset>  crops;
	void Jsonize(JsonIO& json);
};

class VsmSessionStore {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	bool Create(const String& root_path, const String& session_id,
	            int frame_w, int frame_h, const String& source_type = "synthetic");
	bool Open(const String& root_path);

	bool IsOpen() const { return is_open_; }
	const VsmSessionPaths&    GetPaths()    const { return paths_; }
	const VsmSessionManifest& GetManifest() const { return manifest_; }

	VsmAssetRef AllocateFrame(int frame_index);
	VsmAssetRef AllocateCrop(const String& region_id);
	String      Resolve(const VsmAssetRef& ref) const;

	bool SaveManifest();
	bool LoadManifest();

private:
	bool               is_open_  = false;
	VsmSessionPaths    paths_;
	VsmSessionManifest manifest_;
	CoreLog            log_;
};

} // namespace Upp

#endif

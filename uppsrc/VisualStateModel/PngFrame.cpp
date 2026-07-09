#include "VisualStateModel.h"
#include <Draw/Draw.h>

namespace Upp {

// ---------------------------------------------------------------------------
// VsmLoadPngFrame

bool VsmLoadPngFrame(const String& path, VsmFrameImage& out)
{
	if(!FileExists(path))
		return false;

	Image img = StreamRaster::LoadFileAny(path);
	if(img.IsEmpty())
		return false;

	Size sz = img.GetSize();
	int w = sz.cx, h = sz.cy;
	if(w <= 0 || h <= 0)
		return false;

	Buffer<byte> rgba((size_t)w * h * 4);
	const RGBA* src = ~img;
	byte* dst = rgba;
	for(int i = 0; i < w * h; i++) {
		dst[i * 4 + 0] = src[i].r;
		dst[i * 4 + 1] = src[i].g;
		dst[i * 4 + 2] = src[i].b;
		dst[i * 4 + 3] = src[i].a;
	}

	out.Set(w, h, rgba);
	return true;
}

// ---------------------------------------------------------------------------
// VsmReadM01M02SessionInfo / VsmLoadM01M02SessionFrame

bool VsmReadM01M02SessionInfo(const String& session_root, VsmM01M02SessionInfo& out)
{
	String meta_path = AppendFileName(session_root, "metadata.json");
	if(!FileExists(meta_path))
		return false;

	Value meta_value;
	try {
		meta_value = ParseJSON(LoadFile(meta_path));
	}
	catch(...) {
		return false;
	}
	if(!meta_value.Is<ValueMap>())
		return false;

	ValueMap meta = meta_value;
	out.table_width  = (int)meta.Get("table_width", 0);
	out.table_height = (int)meta.Get("table_height", 0);
	out.frame_count  = (int)meta.Get("frame_count", 0);
	out.provider     = (String)meta.Get("provider", String());
	out.session_id   = (String)meta.Get("session_id", String());

	return out.table_width > 0 && out.table_height > 0 && out.frame_count > 0;
}

bool VsmLoadM01M02SessionFrame(const String& session_root, int frame_id, VsmFrameImage& out)
{
	String frame_name = Format("%08d.png", frame_id);
	String frame_path = AppendFileName(AppendFileName(session_root, "frames"), frame_name);
	return VsmLoadPngFrame(frame_path, out);
}

} // namespace Upp

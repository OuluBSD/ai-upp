#include "VideoServerFrameRecorder.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoServerFrameRecorder\n\n"
	       << "Usage: VideoServerFrameRecorder [options]\n\n"
	       << "Options:\n"
	       << "  --host <host>       VideoServer host (default 127.0.0.1)\n"
	       << "  --port <port>       VideoServer port (default 8082)\n"
	       << "  --frames <count>    Frames to save (default 5)\n"
	       << "  --poll-ms <ms>      Poll interval for empty replies (default 33)\n"
	       << "  --timeout-ms <ms>   Socket timeout (default 5000)\n"
	       << "  --out <dir>         Output directory (default tmp/vsm_live_capture_<timestamp>)\n"
	       << "  --help, -h          Show help\n";
}

static RecorderOptions ParseOptions(const Vector<String>& args)
{
	RecorderOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--host" && i + 1 < args.GetCount())
			opt.host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount())
			opt.port = StrInt(args[++i]);
		else if(args[i] == "--frames" && i + 1 < args.GetCount())
			opt.frames = max(1, StrInt(args[++i]));
		else if(args[i] == "--poll-ms" && i + 1 < args.GetCount())
			opt.poll_ms = max(1, StrInt(args[++i]));
		else if(args[i] == "--timeout-ms" && i + 1 < args.GetCount())
			opt.timeout_ms = max(100, StrInt(args[++i]));
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_dir = args[++i];
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	if(opt.out_dir.IsEmpty()) {
		Time t = GetSysTime();
		opt.out_dir = AppendFileName(GetCurrentDirectory(),
			Format("tmp/vsm_live_capture_%04d%02d%02d_%02d%02d%02d",
			       t.year, t.month, t.day, t.hour, t.minute, t.second));
	}
	return opt;
}

static int ClipByte(int v)
{
	return minmax(v, 0, 255);
}

static bool DecodeYuv0(const String& payload, Image& out, String& error)
{
	if(payload.GetCount() < 16 || payload.Mid(0, 4) != "YUV0") {
		error = "not a YUV0 payload";
		return false;
	}
	const byte* p = (const byte*)~payload;
	uint32 w = Peek32le(p + 4);
	uint32 h = Peek32le(p + 8);
	uint32 bytes = Peek32le(p + 12);
	if(w == 0 || h == 0 || bytes != w * h * 2 || payload.GetCount() < 16 + (int)bytes) {
		error = Format("invalid YUV0 header width=%d height=%d bytes=%d payload=%d",
		               (int)w, (int)h, (int)bytes, payload.GetCount());
		return false;
	}
	ImageBuffer ib((int)w, (int)h);
	const byte* src = p + 16;
	for(int y = 0; y < (int)h; y++) {
		RGBA* dst = ib[y];
		const byte* row = src + y * (int)w * 2;
		for(int x = 0; x < (int)w; x += 2) {
			int y0 = row[x * 2 + 0] - 16;
			int u  = row[x * 2 + 1] - 128;
			int y1 = row[x * 2 + 2] - 16;
			int v  = row[x * 2 + 3] - 128;
			int c0 = max(y0, 0);
			int c1 = max(y1, 0);
			dst[x].r = (byte)ClipByte((298 * c0 + 409 * v + 128) >> 8);
			dst[x].g = (byte)ClipByte((298 * c0 - 100 * u - 208 * v + 128) >> 8);
			dst[x].b = (byte)ClipByte((298 * c0 + 516 * u + 128) >> 8);
			dst[x].a = 255;
			if(x + 1 < (int)w) {
				dst[x + 1].r = (byte)ClipByte((298 * c1 + 409 * v + 128) >> 8);
				dst[x + 1].g = (byte)ClipByte((298 * c1 - 100 * u - 208 * v + 128) >> 8);
				dst[x + 1].b = (byte)ClipByte((298 * c1 + 516 * u + 128) >> 8);
				dst[x + 1].a = 255;
			}
		}
	}
	ib.SetKind(IMAGE_OPAQUE);
	out = ib;
	return true;
}

static bool SavePayloadFrame(const String& payload, const String& path, Size& out_size,
                             String& out_format, String& error)
{
	if(payload.GetCount() >= 4 && payload.Mid(0, 4) == "YUV0") {
		Image img;
		if(!DecodeYuv0(payload, img, error))
			return false;
		out_size = img.GetSize();
		out_format = "YUV0";
		if(!JPGEncoder().Quality(95).SaveFile(path, img)) {
			error = "failed to save decoded YUV0 JPEG: " + path;
			return false;
		}
		return true;
	}

	Image img = JPGRaster().LoadString(payload);
	if(img.IsEmpty()) {
		error = Format("payload is not decodable JPEG/YUV0, bytes=%d", payload.GetCount());
		return false;
	}
	out_size = img.GetSize();
	out_format = "JPEG";
	if(!SaveFile(path, payload)) {
		error = "failed to save JPEG payload: " + path;
		return false;
	}
	return true;
}

static String JsonString(const String& s)
{
	String out;
	for(int i = 0; i < s.GetCount(); i++) {
		byte c = s[i];
		if(c == '\\')
			out << "\\\\";
		else if(c == '\"')
			out << "\\\"";
		else if(c == '\n')
			out << "\\n";
		else if(c == '\r')
			out << "\\r";
		else if(c == '\t')
			out << "\\t";
		else
			out.Cat(c);
	}
	return out;
}

static String SummaryJson(const RecorderOptions& opt, const Vector<RecorderFrameInfo>& frames)
{
	String out;
	out << "{\n";
	out << "  \"host\": \"" << JsonString(opt.host) << "\",\n";
	out << "  \"port\": " << opt.port << ",\n";
	out << "  \"frames_saved\": " << frames.GetCount() << ",\n";
	out << "  \"frames\": [\n";
	for(int i = 0; i < frames.GetCount(); i++) {
		const RecorderFrameInfo& f = frames[i];
		out << "    {\"index\": " << f.index
		    << ", \"id\": " << f.id
		    << ", \"path\": \"" << JsonString(f.path) << "\""
		    << ", \"width\": " << f.size.cx
		    << ", \"height\": " << f.size.cy
		    << ", \"format\": \"" << JsonString(f.format) << "\"}";
		if(i + 1 < frames.GetCount())
			out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return out;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	RecorderOptions opt = ParseOptions(CommandLine());
	if(opt.help) {
		PrintHelp();
		return;
	}

	RealizeDirectory(opt.out_dir);
	TcpSocket socket;
	if(!socket.Timeout(opt.timeout_ms).Connect(opt.host, opt.port)) {
		Cerr() << "ERROR: failed to connect VideoServer at " << opt.host << ":" << opt.port << "\n";
		SetExitCode(1);
		return;
	}

	Cout() << "Connected VideoServer " << opt.host << ":" << opt.port
	       << " saving " << opt.frames << " frames to " << opt.out_dir << "\n";

	uint32 last_id = 0;
	Vector<RecorderFrameInfo> saved;
	int empty_replies = 0;
	int attempts = 0;
	while(saved.GetCount() < opt.frames && attempts < opt.frames * 300) {
		attempts++;
		if(!socket.Timeout(opt.timeout_ms).Put(&last_id, 4)) {
			Cerr() << "ERROR: failed to send last frame id\n";
			SetExitCode(1);
			return;
		}
		uint32 frame_id = 0;
		uint32 size = 0;
		if(!socket.Timeout(opt.timeout_ms).GetAll(&frame_id, 4) ||
		   !socket.Timeout(opt.timeout_ms).GetAll(&size, 4)) {
			Cerr() << "ERROR: failed to read frame header\n";
			SetExitCode(1);
			return;
		}
		if(size == 0) {
			empty_replies++;
			Sleep(opt.poll_ms);
			continue;
		}
		String payload = socket.Timeout(opt.timeout_ms).GetAll((int)size);
		if(payload.GetCount() != (int)size) {
			Cerr() << "ERROR: failed to read payload expected=" << size
			       << " got=" << payload.GetCount() << "\n";
			SetExitCode(1);
			return;
		}

		int frame_index = saved.GetCount();
		RecorderFrameInfo& info = saved.Add();
		info.index = frame_index;
		info.id = frame_id;
		info.path = AppendFileName(opt.out_dir, Format("frame_%06d_id_%d.jpg", info.index, (int)info.id));
		String error;
		if(!SavePayloadFrame(payload, info.path, info.size, info.format, error)) {
			Cerr() << "ERROR: " << error << "\n";
			SetExitCode(1);
			return;
		}
		last_id = frame_id;
		Cout() << "saved frame index=" << info.index
		       << " id=" << info.id
		       << " size=" << info.size.cx << "x" << info.size.cy
		       << " format=" << info.format
		       << " path=" << info.path << "\n";
	}

	String summary_path = AppendFileName(opt.out_dir, "summary.json");
	SaveFile(summary_path, SummaryJson(opt, saved));
	Cout() << "summary=" << summary_path << " empty_replies=" << empty_replies
	       << " attempts=" << attempts << "\n";
	if(saved.GetCount() < opt.frames) {
		Cerr() << "ERROR: saved only " << saved.GetCount() << " of " << opt.frames << " frames\n";
		SetExitCode(1);
	}
}

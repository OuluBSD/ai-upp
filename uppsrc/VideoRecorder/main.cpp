#include "VideoRecorder.h"

NAMESPACE_UPP

static void PrintHelp()
{
	Cout() << "VideoRecorder\n\n"
	       << "Usage: VideoRecorder [options]\n\n"
	       << "Options:\n"
	       << "  --host <host>       VideoServer host (default 127.0.0.1)\n"
	       << "  --port <port>       VideoServer port (default 8082)\n"
	       << "  --seconds <n>       Recording duration in seconds (default 60)\n"
	       << "  --minutes <n>       Recording duration in minutes\n"
	       << "  --fps <n>           Target capture/encode FPS (default 10)\n"
	       << "  --out <file>        Output video path (default bin/video_record_<timestamp>.mp4)\n"
	       << "  --work-dir <dir>    Temporary frame directory\n"
	       << "  --ffmpeg <exe>      ffmpeg executable (default ffmpeg from PATH)\n"
	       << "  --codec <name>      ffmpeg video codec (default libx264)\n"
	       << "  --keep-frames       Keep captured JPEG frames after encoding\n"
	       << "  --help, -h          Show help\n";
}

static VideoRecorderOptions ParseOptions(const Vector<String>& args)
{
	VideoRecorderOptions opt;
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] == "--host" && i + 1 < args.GetCount())
			opt.host = args[++i];
		else if(args[i] == "--port" && i + 1 < args.GetCount())
			opt.port = StrInt(args[++i]);
		else if(args[i] == "--seconds" && i + 1 < args.GetCount())
			opt.seconds = max(1, StrInt(args[++i]));
		else if(args[i] == "--minutes" && i + 1 < args.GetCount())
			opt.seconds = max(1, StrInt(args[++i])) * 60;
		else if(args[i] == "--fps" && i + 1 < args.GetCount())
			opt.fps = max(1, StrInt(args[++i]));
		else if(args[i] == "--poll-ms" && i + 1 < args.GetCount())
			opt.poll_ms = max(1, StrInt(args[++i]));
		else if(args[i] == "--timeout-ms" && i + 1 < args.GetCount())
			opt.timeout_ms = max(100, StrInt(args[++i]));
		else if(args[i] == "--out" && i + 1 < args.GetCount())
			opt.out_path = args[++i];
		else if(args[i] == "--work-dir" && i + 1 < args.GetCount())
			opt.work_dir = args[++i];
		else if(args[i] == "--ffmpeg" && i + 1 < args.GetCount())
			opt.ffmpeg = args[++i];
		else if(args[i] == "--codec" && i + 1 < args.GetCount())
			opt.codec = args[++i];
		else if(args[i] == "--keep-frames")
			opt.keep_frames = true;
		else if(args[i] == "--help" || args[i] == "-h")
			opt.help = true;
	}
	Time now = GetSysTime();
	String stamp = Format("%04d%02d%02d_%02d%02d%02d",
	                      now.year, now.month, now.day, now.hour, now.minute, now.second);
	if(opt.out_path.IsEmpty())
		opt.out_path = AppendFileName(GetCurrentDirectory(),
		                               AppendFileName("bin", "video_record_" + stamp + ".mp4"));
	if(opt.work_dir.IsEmpty())
		opt.work_dir = AppendFileName(GetCurrentDirectory(),
		                               AppendFileName("tmp", "video_record_frames_" + stamp));
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
		else if(c == '"')
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

static String ManifestJson(const VideoRecorderOptions& opt, const Vector<RecordedFrame>& frames,
                           int ffmpeg_exit, const String& ffmpeg_output)
{
	String out;
	out << "{\n";
	out << "  \"tool\": \"VideoRecorder\",\n";
	out << "  \"host\": \"" << JsonString(opt.host) << "\",\n";
	out << "  \"port\": " << opt.port << ",\n";
	out << "  \"seconds_requested\": " << opt.seconds << ",\n";
	out << "  \"fps\": " << opt.fps << ",\n";
	out << "  \"frames_saved\": " << frames.GetCount() << ",\n";
	out << "  \"output_video\": \"" << JsonString(opt.out_path) << "\",\n";
	out << "  \"work_dir\": \"" << JsonString(opt.work_dir) << "\",\n";
	out << "  \"ffmpeg\": \"" << JsonString(opt.ffmpeg) << "\",\n";
	out << "  \"ffmpeg_exit\": " << ffmpeg_exit << ",\n";
	out << "  \"ffmpeg_output\": \"" << JsonString(ffmpeg_output) << "\",\n";
	out << "  \"frames\": [\n";
	for(int i = 0; i < frames.GetCount(); i++) {
		const RecordedFrame& f = frames[i];
		out << "    {\"index\": " << f.index
		    << ", \"id\": " << f.id
		    << ", \"width\": " << f.size.cx
		    << ", \"height\": " << f.size.cy
		    << ", \"format\": \"" << JsonString(f.format)
		    << "\", \"path\": \"" << JsonString(f.path) << "\"}";
		if(i + 1 < frames.GetCount())
			out << ",";
		out << "\n";
	}
	out << "  ]\n";
	out << "}\n";
	return out;
}

static bool CaptureFrames(const VideoRecorderOptions& opt, Vector<RecordedFrame>& frames)
{
	TcpSocket socket;
	if(!socket.Timeout(opt.timeout_ms).Connect(opt.host, opt.port)) {
		Cerr() << "ERROR: failed to connect VideoServer at " << opt.host << ":" << opt.port << "\n";
		return false;
	}
	int target_frames = max(1, opt.seconds * opt.fps);
	uint32 last_id = 0;
	int64 started = msecs();
	int empty_replies = 0;
	Cout() << "recording_frames target=" << target_frames << " seconds=" << opt.seconds
	       << " fps=" << opt.fps << " work_dir=" << opt.work_dir << "\n";
	while(frames.GetCount() < target_frames) {
		if(!socket.Timeout(opt.timeout_ms).Put(&last_id, 4)) {
			Cerr() << "ERROR: failed to send last frame id\n";
			return false;
		}
		uint32 frame_id = 0;
		uint32 size = 0;
		if(!socket.Timeout(opt.timeout_ms).GetAll(&frame_id, 4) ||
		   !socket.Timeout(opt.timeout_ms).GetAll(&size, 4)) {
			Cerr() << "ERROR: failed to read frame header\n";
			return false;
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
			return false;
		}
		RecordedFrame& frame = frames.Add();
		frame.index = frames.GetCount() - 1;
		frame.id = frame_id;
		frame.path = AppendFileName(opt.work_dir, Format("frame_%06d.jpg", frame.index));
		String error;
		if(!SavePayloadFrame(payload, frame.path, frame.size, frame.format, error)) {
			Cerr() << "ERROR: " << error << "\n";
			return false;
		}
		last_id = frame_id;
		if(frame.index == 0 || (frame.index + 1) % max(1, opt.fps * 5) == 0) {
			int elapsed = (int)((msecs() - started) / 1000);
			Cout() << "progress frames=" << frames.GetCount() << "/" << target_frames
			       << " elapsed=" << elapsed << "s"
			       << " empty_replies=" << empty_replies << "\n";
		}
	}
	return true;
}

static int EncodeVideo(const VideoRecorderOptions& opt, String& output)
{
	RealizeDirectory(GetFileDirectory(opt.out_path));
	Vector<String> args;
	args << "-y"
	     << "-framerate" << AsString(opt.fps)
	     << "-i" << AppendFileName(opt.work_dir, "frame_%06d.jpg")
	     << "-c:v" << opt.codec
	     << "-pix_fmt" << opt.pix_fmt
	     << opt.out_path;
	Cout() << "encoding_video output=" << opt.out_path << "\n";
	int code = Sys(opt.ffmpeg, args, output);
	if(!output.IsEmpty())
		Cout() << output;
	Cout() << "ffmpeg_exit=" << code << "\n";
	return code;
}

END_UPP_NAMESPACE

using namespace Upp;

CONSOLE_APP_MAIN
{
	VideoRecorderOptions opt = ParseOptions(CommandLine());
	if(opt.help) {
		PrintHelp();
		return;
	}

	RealizeDirectory(opt.work_dir);
	Vector<RecordedFrame> frames;
	if(!CaptureFrames(opt, frames)) {
		SetExitCode(1);
		return;
	}
	String ffmpeg_output;
	int ffmpeg_exit = EncodeVideo(opt, ffmpeg_output);
	String manifest_path = opt.out_path + ".json";
	SaveFile(manifest_path, ManifestJson(opt, frames, ffmpeg_exit, ffmpeg_output));
	Cout() << "manifest=" << manifest_path << "\n";
	if(ffmpeg_exit != 0 || !FileExists(opt.out_path)) {
		Cerr() << "ERROR: ffmpeg failed or output video missing: " << opt.out_path << "\n";
		SetExitCode(1);
		return;
	}
	if(!opt.keep_frames)
		DeleteFolderDeep(opt.work_dir);
	Cout() << "video_recording_done output=" << opt.out_path
	       << " frames=" << frames.GetCount() << "\n";
}


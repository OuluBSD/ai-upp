#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

// ---------------------------------------------------------------------------
// VsmMjpegSource — implements VsmFrameSource using VsmMjpegParser
//
// JPEG DECODE BLOCKER:
//   Converting raw JPEG payload bytes to VsmImageBuffer requires
//   Draw::JPGRaster which is not available in the headless package.
//   ReadFrame() extracts the payload but returns an empty VsmImageBuffer
//   until JPEG decode is wired up.
//
//   To enable decode:
//   1. Add Draw to the .upp dependencies.
//   2. In ReadFrame(), call: Image img = JPGRaster().LoadString(payload);
//   3. Convert RGBA pixel data to VsmImageBuffer (grayscale or 3-channel).

class VsmMjpegSource : public VsmFrameSource {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); parser_.SetLog(sink); }

	// Open a URL or a pre-loaded MJPEG stream for testing.
	// Set test_stream non-empty to bypass HTTP and feed from memory.
	bool Open(const String& uri) override {
		test_uri_   = uri;
		is_ready_   = false;
		last_error_ = String();
		String bound;

		if(!test_stream_.IsEmpty()) {
			// Deterministic mode: parse pre-loaded stream directly
			bound = test_boundary_;
		} else {
			// Live HTTP mode: read boundary from Content-Type header
			// (not attempted in this prototype — see BLOCKER note above)
			last_error_ = "Live HTTP source not implemented in prototype";
			return false;
		}

		parser_.Reset(bound);
		parser_.Feed(test_stream_);
		is_ready_ = true;
		width_    = test_width_;
		height_   = test_height_;
		LogInfo(log_, "VsmMjpegSource", "Opened test stream: " + String(uri) +
		        " boundary=" + bound + " bytes=" + IntStr(test_stream_.GetCount()));
		return true;
	}

	void SetTestStream(const String& stream, const String& boundary,
	                   int w, int h) {
		test_stream_   = stream;
		test_boundary_ = boundary;
		test_width_    = w;
		test_height_   = h;
	}

	void Close() override {
		is_ready_ = false;
	}

	bool IsReady()  const override { return is_ready_; }
	int  GetWidth() const override { return width_; }
	int  GetHeight()const override { return height_; }
	int  GetFPS()   const override { return 0; }

	bool ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms) override {
		if(!is_ready_) { last_error_ = "Source not open"; return false; }
		String payload;
		VsmMjpegPartHeader header;
		if(!parser_.ExtractNextPayload(payload, header)) return false; // end of stream

		// JPEG DECODE BLOCKER: Draw::JPGRaster required for decode.
		// Without it, return an empty VsmImageBuffer with correct dimensions.
		// To enable:  Image img = JPGRaster().LoadString(payload);
		//             Convert RGBA→VsmImageBuffer here.
		out_frame = VsmImageBuffer();
		out_frame.Create(width_, height_, 1);
		// Fill with a gray placeholder so the consumer gets a non-zero buffer
		memset(out_frame.pixels.Begin(), 128, out_frame.pixels.GetCount());

		out_ts_ms = (int64)parser_.GetPayloadCount() * 33;
		return true;
	}

	String GetLastError()  const override { return last_error_; }
	String GetSourceInfo() const override {
		return "mjpeg:" + test_uri_ + " payloads=" + IntStr(parser_.GetPayloadCount());
	}

private:
	CoreLog        log_;
	VsmMjpegParser parser_;
	bool          is_ready_     = false;
	int           width_        = 0;
	int           height_       = 0;
	String        last_error_;
	String        test_uri_;
	String        test_stream_;
	String        test_boundary_;
	int           test_width_   = 0;
	int           test_height_  = 0;
};

// ---------------------------------------------------------------------------

static bool Fail(const char* label)
{
	Cout() << "FAIL: " << label << "\n";
	SetExitCode(1);
	return false;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);

	const Vector<String>& args = CommandLine();
	String mjpeg_source;

	// Parse command-line arguments
	for(const String& arg : args) {
		if(arg == "--help") {
			Cout() << "Usage: VisualStateMjpegSource [<mjpeg_source>]\n";
			SetExitCode(0);
			return;
		} else {
			mjpeg_source = arg;
		}
	}

	// Check for unsupported real MJPEG source
	if(!mjpeg_source.IsEmpty()) {
		Cout() << "ERROR: Real MJPEG file/URL loading not supported by VsmMjpegSource\n";
		Cout() << "The underlying headless API only supports pre-loaded test streams.\n";
		SetExitCode(1);
		return;
	}

	Cout() << "=== VisualStateModel MJPEG Source Prototype ===\n\n";

	AppLog log;
	log.SetForwardToUppLog(false);

	// --- Test 1: VsmMjpegParser boundary extraction ---
	Cout() << "--- Test 1: Boundary parser ---\n";
	String boundary = "frameboundary";
	String fake_frame = String('A', 64); // 64 bytes of fake "JPEG" data
	String stream = VsmMakeSyntheticMjpeg(boundary, fake_frame, 5);
	Cout() << "Synthetic stream: " << stream.GetCount() << " bytes for 5 frames\n";

	VsmMjpegParser parser;
	parser.SetLog(&log);
	parser.Reset(boundary);
	parser.Feed(stream);

	int pcount = 0;
	String payload;
	VsmMjpegPartHeader hdr;
	while(parser.ExtractNextPayload(payload, hdr)) {
		pcount++;
		if(payload != fake_frame) { Fail("payload mismatch"); return; }
	}
	if(pcount != 5) { Fail("expected 5 payloads"); return; }
	Cout() << "OK: extracted " << pcount << " payloads\n\n";

	// --- Test 2: VsmMjpegSource with test stream ---
	Cout() << "--- Test 2: VsmMjpegSource (deterministic test stream) ---\n";
	VsmMjpegSource src;
	src.SetLog(&log);
	src.SetTestStream(stream, boundary, 320, 240);

	if(!src.Open("test://mjpeg-sample")) { Fail("Open failed"); return; }
	if(!src.IsReady()) { Fail("IsReady"); return; }
	if(src.GetWidth() != 320 || src.GetHeight() != 240) { Fail("Dimensions"); return; }
	Cout() << "Open: OK — " << src.GetSourceInfo() << "\n";

	int frames = 0;
	VsmImageBuffer buf; int64 ts = 0;
	while(src.ReadFrame(buf, ts)) {
		if(buf.IsEmpty()) { Fail("ReadFrame returned empty buffer"); return; }
		frames++;
	}
	if(frames != 5) { Fail("expected 5 frames from source"); return; }
	Cout() << "ReadFrame: " << frames << " frames OK (placeholder pixels; JPEG decode deferred)\n";

	// --- Test 3: VsmMjpegSource with capture sink ---
	Cout() << "\n--- Test 3: Capture sink integration ---\n";
	src.SetTestStream(stream, boundary, 320, 240);
	src.Open("test://mjpeg-capture");

	String cap_dir = AppendFileName(GetTempPath(), "vsm_mjpeg_cap");
	if(DirectoryExists(cap_dir)) DeleteFolderDeep(cap_dir);

	VsmCaptureSinkOptions copts;
	copts.output_dir = cap_dir;
	copts.session_id = "mjpeg-test-001";

	VsmCaptureSink sink;
	sink.SetLog(&log);
	VsmCaptureSummary csummary = sink.Record(src, copts);

	if(!csummary.success) { Fail("CaptureSink Record failed"); return; }
	if(csummary.frames_recorded != 5) { Fail("Expected 5 captured frames"); return; }
	Cout() << "CaptureSink: recorded " << csummary.frames_recorded << " frames OK\n";

	// Replay captured session
	VsmSessionStoreSource replay;
	replay.SetLog(&log);
	if(!replay.Open(cap_dir)) { Fail("Cannot open captured session"); return; }
	int replayed = 0;
	while(replay.ReadFrame(buf, ts)) replayed++;
	if(replayed != 5) { Fail("Replay frame count mismatch"); return; }
	Cout() << "Replay: " << replayed << " frames OK\n";

	DeleteFolderDeep(cap_dir);

	Cout() << "\n--- Decode Blocker ---\n";
	Cout() << "JPEG decode: NOT IMPLEMENTED (requires Draw::JPGRaster)\n";
	Cout() << "To enable: add Draw to .upp deps; call JPGRaster().LoadString(payload)\n";
	Cout() << "           convert RGBA Image to VsmImageBuffer in ReadFrame()\n\n";

	Cout() << "All MJPEG prototype checks passed.\n";
}

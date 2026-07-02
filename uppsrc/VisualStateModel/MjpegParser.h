#ifndef _VisualStateModel_MjpegParser_h_
#define _VisualStateModel_MjpegParser_h_

namespace Upp {

// ---------------------------------------------------------------------------
// Parsed MJPEG part header fields

struct VsmMjpegPartHeader : Moveable<VsmMjpegPartHeader> {
	String content_type;   // e.g. "image/jpeg"
	int    content_length = -1; // -1 if unknown

	void Jsonize(JsonIO& json) {
		json("content_type",   content_type)
		    ("content_length", content_length);
	}
};

// ---------------------------------------------------------------------------
// VsmMjpegParser
//
// Headless multipart/x-mixed-replace boundary parser.
// Feed raw HTTP body data with Feed(); call ExtractNextPayload() to retrieve
// the next complete JPEG payload.
//
// Does NOT decode JPEG (requires Draw::JPGRaster).
// Does NOT open network connections.

class VsmMjpegParser {
public:
	void SetLog(AppLog* sink) { log_.SetSink(sink); }

	// Set boundary string (without leading "--").
	// Call before any Feed().
	void Reset(const String& boundary);

	// Feed raw HTTP response body bytes.
	void Feed(const String& data);

	// Returns true if a complete payload is ready.
	bool HasPayload() const { return has_payload_; }

	// Extracts the next ready payload.
	// Returns false if no payload is available.
	bool ExtractNextPayload(String& out_payload, VsmMjpegPartHeader& out_header);

	// Total payloads extracted since Reset()
	int GetPayloadCount() const { return payload_count_; }

	// Payloads seen but skipped / corrupt
	int GetSkipCount() const { return skip_count_; }

private:
	enum State { S_FIND_BOUNDARY, S_READ_HEADERS, S_READ_PAYLOAD };

	CoreLog log_;
	String  boundary_;
	String  buf_;
	State   state_      = S_FIND_BOUNDARY;
	int     payload_count_ = 0;
	int     skip_count_    = 0;
	bool    has_payload_   = false;

	VsmMjpegPartHeader pending_header_;
	String             pending_payload_;

	void Process(); // advance state machine over buf_
};

// ---------------------------------------------------------------------------
// Build a synthetic MJPEG stream for testing (no network required)
// Each payload is the given bytes array, repeated count times.

String VsmMakeSyntheticMjpeg(const String& boundary,
                              const String& payload_bytes,
                              int count);

} // namespace Upp

#endif

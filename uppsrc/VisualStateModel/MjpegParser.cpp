#include "VisualStateModel.h"

namespace Upp {

// ---------------------------------------------------------------------------
// VsmMjpegParser

void VsmMjpegParser::Reset(const String& boundary)
{
	boundary_      = boundary;
	buf_           = String();
	state_         = S_FIND_BOUNDARY;
	payload_count_ = 0;
	skip_count_    = 0;
	has_payload_   = false;
	pending_header_ = VsmMjpegPartHeader();
	pending_payload_ = String();
}

void VsmMjpegParser::Feed(const String& data)
{
	buf_.Cat(data);
	Process();
}

bool VsmMjpegParser::ExtractNextPayload(String& out_payload, VsmMjpegPartHeader& out_header)
{
	if(!has_payload_) return false;
	out_payload    = pick(pending_payload_);
	out_header     = pending_header_;
	has_payload_   = false;
	pending_header_ = VsmMjpegPartHeader();
	pending_payload_ = String();
	Process(); // advance in case more data is buffered
	return true;
}

void VsmMjpegParser::Process()
{
	while(!has_payload_) {
		if(state_ == S_FIND_BOUNDARY) {
			// Look for "--boundary"
			String delim = "--" + boundary_;
			int pos = buf_.Find(delim);
			if(pos < 0) {
				// Trim buf_ to avoid unbounded growth; keep last len(delim)-1 bytes
				int keep = delim.GetCount() - 1;
				if(buf_.GetCount() > keep)
					buf_ = buf_.Right(keep);
				break;
			}
			buf_ = buf_.Mid(pos + delim.GetCount());
			// Skip optional "--" (end boundary) or CRLF
			if(buf_.StartsWith("--")) {
				buf_.Clear(); // end of stream marker
				break;
			}
			// Skip CRLF after boundary
			if(buf_.StartsWith("\r\n")) buf_ = buf_.Mid(2);
			else if(buf_.StartsWith("\n")) buf_ = buf_.Mid(1);
			state_ = S_READ_HEADERS;
			pending_header_ = VsmMjpegPartHeader();
		}
		else if(state_ == S_READ_HEADERS) {
			// Read headers until blank line
			int blank = buf_.Find("\r\n\r\n");
			int blank2 = buf_.Find("\n\n");
			int end_pos = -1, skip = 4;
			if(blank >= 0) { end_pos = blank; skip = 4; }
			if(blank2 >= 0 && (end_pos < 0 || blank2 < end_pos)) { end_pos = blank2; skip = 2; }
			if(end_pos < 0) break; // need more data

			String header_block = buf_.Left(end_pos);
			buf_ = buf_.Mid(end_pos + skip);

			// Parse header lines
			Vector<String> lines = Split(header_block, '\n', true);
			for(String& line : lines) {
				line = TrimBoth(line);
				if(line.IsEmpty()) continue;
				int colon = line.Find(':');
				if(colon < 0) continue;
				String key = ToLower(TrimBoth(line.Left(colon)));
				String val = TrimBoth(line.Mid(colon + 1));
				if(key == "content-type")
					pending_header_.content_type = val;
				else if(key == "content-length")
					pending_header_.content_length = StrInt(val);
			}
			state_ = S_READ_PAYLOAD;
		}
		else if(state_ == S_READ_PAYLOAD) {
			int n = pending_header_.content_length;
			if(n >= 0) {
				// Read exactly n bytes
				if(buf_.GetCount() < n) break; // need more data
				pending_payload_ = buf_.Left(n);
				buf_ = buf_.Mid(n);
				// Skip trailing CRLF
				if(buf_.StartsWith("\r\n")) buf_ = buf_.Mid(2);
				else if(buf_.StartsWith("\n")) buf_ = buf_.Mid(1);
			} else {
				// No Content-Length: read until next boundary
				String delim = "\r\n--" + boundary_;
				int pos = buf_.Find(delim);
				if(pos < 0) {
					delim = "\n--" + boundary_;
					pos = buf_.Find(delim);
				}
				if(pos < 0) break; // need more data
				pending_payload_ = buf_.Left(pos);
				buf_ = buf_.Mid(pos + 2); // skip CRLF before next boundary
			}
			payload_count_++;
			has_payload_ = true;
			state_ = S_FIND_BOUNDARY;
			LogInfo(log_, "MjpegParser",
			        "Payload " + IntStr(payload_count_) + ": " +
			        IntStr(pending_payload_.GetCount()) + " bytes, type=" +
			        pending_header_.content_type);
			break; // wait for ExtractNextPayload before continuing
		}
	}
}

// ---------------------------------------------------------------------------
// Synthetic MJPEG generator

String VsmMakeSyntheticMjpeg(const String& boundary, const String& payload_bytes, int count)
{
	String out;
	for(int i = 0; i < count; i++) {
		out << "--" << boundary << "\r\n";
		out << "Content-Type: image/jpeg\r\n";
		out << "Content-Length: " << IntStr(payload_bytes.GetCount()) << "\r\n";
		out << "\r\n";
		out << payload_bytes;
		out << "\r\n";
	}
	out << "--" << boundary << "--\r\n";
	return out;
}

} // namespace Upp

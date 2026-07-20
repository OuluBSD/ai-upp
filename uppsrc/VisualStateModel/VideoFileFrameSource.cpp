#include "VisualStateModel.h"

#ifdef _WIN32
#include <windows.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
}
#endif

namespace Upp {

#ifdef _WIN32

static String VsmAvError(int code)
{
	char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
	av_strerror(code, buffer, sizeof(buffer));
	return buffer;
}

static String GetDefaultVsmFfmpegDllDirectory()
{
	return AppendFileName(AppendFileName(AppendFileName(AppendFileName(GetHomeDirectory(), "vcpkg"),
	                                                   "installed"),
	                                    "x64-windows"),
	                      "bin");
}

static bool PrepareVsmFfmpegRuntime(const String& override_dir, String& error)
{
	String dir = override_dir.IsEmpty() ? GetDefaultVsmFfmpegDllDirectory() : override_dir;
	if(!DirectoryExists(dir)) {
		error = "FFmpeg DLL directory does not exist: " + dir;
		return false;
	}
	if(!SetDllDirectoryA(~dir)) {
		error = "SetDllDirectoryA failed for: " + dir;
		return false;
	}
	return true;
}

struct VsmVideoFileFrameSource::Data {
	AVFormatContext *format = nullptr;
	AVCodecContext  *codec = nullptr;
	SwsContext      *sws = nullptr;
	AVFrame         *frame = nullptr;
	AVPacket        *packet = nullptr;
	int              video_stream_index = -1;
	int              sws_width = 0;
	int              sws_height = 0;
	int              sws_input_format = AV_PIX_FMT_NONE;
	int64            pts_origin = 0;
	bool             packet_pending = false;
	bool             demux_eof = false;
	bool             drain_sent = false;

	~Data()
	{
		if(sws)
			sws_freeContext(sws);
		if(frame)
			av_frame_free(&frame);
		if(packet)
			av_packet_free(&packet);
		if(codec)
			avcodec_free_context(&codec);
		if(format)
			avformat_close_input(&format);
	}

	void ResetDecodeState()
	{
		if(packet)
			av_packet_unref(packet);
		packet_pending = false;
		demux_eof = false;
		drain_sent = false;
	}
};

#else

struct VsmVideoFileFrameSource::Data {};

#endif

VsmVideoFileFrameSource::VsmVideoFileFrameSource() = default;

VsmVideoFileFrameSource::~VsmVideoFileFrameSource()
{
	Close();
}

bool VsmVideoFileFrameSource::Open(const String& uri)
{
	Close();
	last_error_.Clear();
	uri_ = uri;
#ifdef _WIN32
	if(!FileExists(uri)) {
		last_error_ = "Video file does not exist: " + uri;
		return false;
	}
	if(!PrepareVsmFfmpegRuntime(ffmpeg_dll_directory_, last_error_))
		return false;

	data_.Create();
	Data& data = *data_;
	int rc = avformat_open_input(&data.format, ~uri, nullptr, nullptr);
	if(rc < 0 || !data.format) {
		last_error_ = "avformat_open_input failed for " + uri + ": " + VsmAvError(rc);
		Close();
		return false;
	}
	rc = avformat_find_stream_info(data.format, nullptr);
	if(rc < 0) {
		last_error_ = "avformat_find_stream_info failed: " + VsmAvError(rc);
		Close();
		return false;
	}
	const AVCodec *decoder = nullptr;
	data.video_stream_index = av_find_best_stream(data.format, AVMEDIA_TYPE_VIDEO,
	                                              -1, -1, &decoder, 0);
	if(data.video_stream_index < 0 || !decoder) {
		last_error_ = "No decodable video stream found in " + uri;
		Close();
		return false;
	}
	AVStream *stream = data.format->streams[data.video_stream_index];
	data.codec = avcodec_alloc_context3(decoder);
	if(!data.codec) {
		last_error_ = "avcodec_alloc_context3 failed";
		Close();
		return false;
	}
	rc = avcodec_parameters_to_context(data.codec, stream->codecpar);
	if(rc < 0) {
		last_error_ = "avcodec_parameters_to_context failed: " + VsmAvError(rc);
		Close();
		return false;
	}
	rc = avcodec_open2(data.codec, decoder, nullptr);
	if(rc < 0) {
		last_error_ = "avcodec_open2 failed: " + VsmAvError(rc);
		Close();
		return false;
	}
	data.frame = av_frame_alloc();
	data.packet = av_packet_alloc();
	if(!data.frame || !data.packet) {
		last_error_ = "av_frame_alloc/av_packet_alloc failed";
		Close();
		return false;
	}

	width_ = data.codec->width;
	height_ = data.codec->height;
	AVRational frame_rate = av_guess_frame_rate(data.format, stream, nullptr);
	fps_ = frame_rate.num > 0 && frame_rate.den > 0
	     ? max(1, (int)(av_q2d(frame_rate) + 0.5)) : 0;
	data.pts_origin = stream->start_time == AV_NOPTS_VALUE ? 0 : stream->start_time;
	if(stream->duration != AV_NOPTS_VALUE)
		duration_ms_ = av_rescale_q(stream->duration, stream->time_base, AVRational{1, 1000});
	else if(data.format->duration != AV_NOPTS_VALUE)
		duration_ms_ = data.format->duration / (AV_TIME_BASE / 1000);
	is_ready_ = true;
	return true;
#else
	last_error_ = "Direct libavcodec video input is only supported on Windows in this build";
	return false;
#endif
}

void VsmVideoFileFrameSource::Close()
{
	data_.Clear();
	uri_.Clear();
	width_ = 0;
	height_ = 0;
	fps_ = 0;
	duration_ms_ = -1;
	decoded_frames_ = 0;
	loops_ = 0;
	last_pts_ms_ = -1;
	is_ready_ = false;
	eof_ = false;
}

bool VsmVideoFileFrameSource::ReadImage(Image& out_image, int64& out_ts_ms)
{
	out_image = Null;
	out_ts_ms = -1;
	last_error_.Clear();
#ifdef _WIN32
	if(!is_ready_ || !data_) {
		last_error_ = "Video decoder is not open";
		return false;
	}
	Data& data = *data_;
	for(;;) {
		int rc = avcodec_receive_frame(data.codec, data.frame);
		if(rc == 0) {
			int width = data.frame->width;
			int height = data.frame->height;
			int input_format = data.frame->format;
			if(width <= 0 || height <= 0) {
				last_error_ = "Invalid decoded frame size";
				return false;
			}
			if(!data.sws || data.sws_width != width || data.sws_height != height
			   || data.sws_input_format != input_format) {
				if(data.sws)
					sws_freeContext(data.sws);
				data.sws = sws_getContext(width, height, (AVPixelFormat)input_format,
				                          width, height, AV_PIX_FMT_BGRA, SWS_BILINEAR,
				                          nullptr, nullptr, nullptr);
				if(!data.sws) {
					last_error_ = "sws_getContext failed";
					return false;
				}
				data.sws_width = width;
				data.sws_height = height;
				data.sws_input_format = input_format;
			}
			ImageBuffer image(width, height);
			byte *destination[4] = {(byte*)image[0], nullptr, nullptr, nullptr};
			int linesize[4] = {width * (int)sizeof(RGBA), 0, 0, 0};
			sws_scale(data.sws, data.frame->data, data.frame->linesize, 0, height,
			          destination, linesize);
			image.SetKind(IMAGE_OPAQUE);
			out_image = image;
			int64 pts = data.frame->best_effort_timestamp;
			if(pts == AV_NOPTS_VALUE)
				pts = data.frame->pts;
			if(pts != AV_NOPTS_VALUE) {
				AVStream *stream = data.format->streams[data.video_stream_index];
				out_ts_ms = av_rescale_q(pts - data.pts_origin, stream->time_base,
				                         AVRational{1, 1000});
				last_pts_ms_ = out_ts_ms;
			}
			decoded_frames_++;
			eof_ = false;
			return true;
		}
		if(rc == AVERROR_EOF) {
			if(!looping_) {
				eof_ = true;
				return false;
			}
			if(!SeekMs(0))
				return false;
			loops_++;
			continue;
		}
		if(rc != AVERROR(EAGAIN)) {
			last_error_ = "avcodec_receive_frame failed: " + VsmAvError(rc);
			return false;
		}

		if(data.packet_pending) {
			rc = avcodec_send_packet(data.codec, data.packet);
			if(rc == AVERROR(EAGAIN))
				continue;
			av_packet_unref(data.packet);
			data.packet_pending = false;
			if(rc < 0) {
				last_error_ = "avcodec_send_packet failed: " + VsmAvError(rc);
				return false;
			}
			continue;
		}

		if(data.demux_eof) {
			if(data.drain_sent) {
				last_error_ = "Decoder stalled while draining";
				return false;
			}
			rc = avcodec_send_packet(data.codec, nullptr);
			if(rc < 0 && rc != AVERROR_EOF) {
				last_error_ = "avcodec flush failed: " + VsmAvError(rc);
				return false;
			}
			data.drain_sent = true;
			continue;
		}

		for(;;) {
			av_packet_unref(data.packet);
			rc = av_read_frame(data.format, data.packet);
			if(rc == AVERROR_EOF) {
				data.demux_eof = true;
				break;
			}
			if(rc < 0) {
				last_error_ = "av_read_frame failed: " + VsmAvError(rc);
				return false;
			}
			if(data.packet->stream_index == data.video_stream_index) {
				data.packet_pending = true;
				break;
			}
		}
	}
#else
	last_error_ = "Direct libavcodec video input is only supported on Windows in this build";
	return false;
#endif
}

bool VsmVideoFileFrameSource::ReadFrame(VsmImageBuffer& out_frame, int64& out_ts_ms)
{
	Image image;
	if(!ReadImage(image, out_ts_ms))
		return false;
	int width = image.GetWidth();
	int height = image.GetHeight();
	out_frame.Create(width, height, 4);
	const RGBA *source = ~image;
	byte *destination = out_frame.pixels.Begin();
	for(int i = 0; i < width * height; i++) {
		destination[i * 4 + 0] = source[i].r;
		destination[i * 4 + 1] = source[i].g;
		destination[i * 4 + 2] = source[i].b;
		destination[i * 4 + 3] = source[i].a;
	}
	return true;
}

bool VsmVideoFileFrameSource::NextFrame(Image& out_image, String& error)
{
	int64 timestamp_ms = -1;
	if(ReadImage(out_image, timestamp_ms))
		return true;
	error = last_error_.IsEmpty() && eof_ ? "End of video stream" : last_error_;
	return false;
}

bool VsmVideoFileFrameSource::SeekMs(int64 timestamp_ms)
{
	last_error_.Clear();
#ifdef _WIN32
	if(!is_ready_ || !data_) {
		last_error_ = "Video decoder is not open";
		return false;
	}
	Data& data = *data_;
	AVStream *stream = data.format->streams[data.video_stream_index];
	int64 target_ms = timestamp_ms < 0 ? 0 : timestamp_ms;
	int64 target = av_rescale_q(target_ms, AVRational{1, 1000},
	                            stream->time_base) + data.pts_origin;
	int rc = av_seek_frame(data.format, data.video_stream_index, target, AVSEEK_FLAG_BACKWARD);
	if(rc < 0)
		rc = avformat_seek_file(data.format, data.video_stream_index,
		                        INT64_MIN, target, INT64_MAX, AVSEEK_FLAG_BACKWARD);
	if(rc < 0) {
		last_error_ = "Video seek failed: " + VsmAvError(rc);
		return false;
	}
	avcodec_flush_buffers(data.codec);
	data.ResetDecodeState();
	eof_ = false;
	last_pts_ms_ = -1;
	return true;
#else
	last_error_ = "Direct libavcodec video input is only supported on Windows in this build";
	return false;
#endif
}

String VsmVideoFileFrameSource::GetSourceInfo() const
{
	if(!is_ready_)
		return "VsmVideoFileFrameSource (not open)";
	return Format("VsmVideoFileFrameSource %s %d`x%d fps=%d duration_ms=", ~uri_,
	              width_, height_, fps_) + AsString(duration_ms_);
}

double VsmVideoFileFrameSource::LastPtsSeconds() const
{
	return last_pts_ms_ < 0 ? -1.0 : last_pts_ms_ / 1000.0;
}

} // namespace Upp

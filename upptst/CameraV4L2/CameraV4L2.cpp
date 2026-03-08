#include <Core/Core.h>
#include <plugin/libv4l2/libv4l2.h>
#include <plugin/jpg/jpg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

using namespace Upp;

void ListDevices() {
	// Simple enumeration of /dev/video*
	for(int i = 0; i < 64; i++) {
		String path = "/dev/video" + AsString(i);
		if(FileExists(path)) {
			Cout() << "Found device: " << path << "\n";
		}
	}
}

void DeviceInfo(const String& devName) {
	V4L2DeviceParameters param(devName.Begin(), 0, 0, 0, 0, 1);
	// We use V4l2Capture to initialize the device as V4l2Device::init is protected
	V4l2Capture* capture = V4l2Capture::create(param);
	if(capture) {
		Cout() << "Device initialized: " << devName << "\n";
		// Info is logged to stdout by libv4l2 internal logging during init
		delete capture;
	} else {
		Cerr() << "Failed to init device: " << devName << "\n";
	}
}

// YUYV to RGB conversion
// YUYV is Y0 U0 Y1 V0
// Pixel 0: Y0, U0, V0
// Pixel 1: Y1, U0, V0
void YUYVToImage(const unsigned char* src, int w, int h, Image& img) {
	ImageBuffer ib(w, h);
	
	const unsigned char* s = src;
	RGBA* t = ib.Begin();
	
	for(int i = 0; i < w * h / 2; i++) {
		int y0 = s[0];
		int u0 = s[1];
		int y1 = s[2];
		int v0 = s[3];
		s += 4;
		
		auto YUV2RGB = [](int y, int u, int v, RGBA& p) {
			int c = y - 16;
			int d = u - 128;
			int e = v - 128;
			
			int r = (298 * c           + 409 * e + 128) >> 8;
			int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
			int b = (298 * c + 516 * d           + 128) >> 8;
			
			p.r = (byte)clamp(r, 0, 255);
			p.g = (byte)clamp(g, 0, 255);
			p.b = (byte)clamp(b, 0, 255);
			p.a = 255;
		};
		
		YUV2RGB(y0, u0, v0, *t++);
		YUV2RGB(y1, u0, v0, *t++);
	}
	
	img = ib;
}

void CaptureImage(const String& devName, const String& fileName, int width = 0, int height = 0, const String& formatStr = "") {
	unsigned int format = 0;
	String fmtStr = formatStr;
	if(fmtStr == "MJPEG") fmtStr = "MJPG";
	
	if(fmtStr.GetCount() == 4)
		format = V4l2Device::fourcc(fmtStr.Begin());

	V4L2DeviceParameters param(devName.Begin(), format, width, height, 0, 1);
	V4l2Capture* capture = V4l2Capture::create(param);
	
	if(!capture) {
		Cerr() << "Failed to create capture device\n";
		return;
	}

	if(capture->isReady()) {
		String fourcc = V4l2Device::fourcc(capture->getFormat()).c_str();
		Cout() << "Capturing from " << devName << " (" << capture->getWidth() << "x" << capture->getHeight() << " " << fourcc << ")\n";
		
		Buffer<char> buffer(capture->getBufferSize());
		
		timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		
		if(capture->isReadable(&tv)) {
			size_t n = capture->read(buffer, capture->getBufferSize());
			if(n > 0) {
				if(GetFileExt(fileName) == ".jpg" && fourcc == "YUYV") {
					Image img;
					YUYVToImage((const unsigned char*)~buffer, capture->getWidth(), capture->getHeight(), img);
					JPGEncoder jpg;
					jpg.SaveFile(fileName, img);
					Cout() << "Captured and converted to " << fileName << "\n";
				}
				else {
					// If MJPEG and .jpg, or raw format, just save the buffer
					FileOut out(fileName);
					if(out.IsOpen()) {
						out.Put(buffer, (int)n);
						out.Close();
						Cout() << "Captured " << n << " bytes to " << fileName << "\n";
					} else {
						Cerr() << "Failed to create output file\n";
					}
				}
			} else {
				Cerr() << "Read returned 0 bytes\n";
			}
		} else {
			Cerr() << "Device not readable (timeout)\n";
		}
	} else {
		Cerr() << "Device not ready\n";
	}
	
	delete capture;
}

void CaptureVideo(const String& devName, const String& fileName, int duration, int width = 0, int height = 0, const String& formatStr = "") {
	unsigned int format = 0;
	String fmtStr = formatStr;
	if(fmtStr == "MJPEG") fmtStr = "MJPG";
	
	if(fmtStr.GetCount() == 4)
		format = V4l2Device::fourcc(fmtStr.Begin());

	// Initialize V4L2 Capture
	V4L2DeviceParameters param(devName.Begin(), format, width, height, 30, 1);
	V4l2Capture* capture = V4l2Capture::create(param);
	
	if(!capture) {
		Cerr() << "Failed to create capture device\n";
		return;
	}
	
	if(!capture->isReady()) {
		Cerr() << "Device not ready\n";
		delete capture;
		return;
	}

	Cout() << "Capturing from " << devName << " (" << capture->getWidth() << "x" << capture->getHeight() << " " << V4l2Device::fourcc(capture->getFormat()) << ")\n";

	// Prepare ffmpeg arguments
	Vector<String> args;
	args.Add("ffmpeg");
	args.Add("-y");
	args.Add("-f"); 
	
	String fourcc = V4l2Device::fourcc(capture->getFormat()).c_str();
	if(fourcc == "MJPG") {
		args.Add("mjpeg");
	}
	else if(fourcc == "YUYV") {
		args.Add("rawvideo");
		args.Add("-pix_fmt"); args.Add("yuyv422");
		args.Add("-s"); args.Add(AsString(capture->getWidth()) + "x" + AsString(capture->getHeight()));
		args.Add("-r"); args.Add("30");
	}
	else {
		Cerr() << "Unsupported format for video capture: " << fourcc << "\n";
		delete capture;
		return;
	}
	
	args.Add("-i"); args.Add("-"); // Read from stdin
	args.Add("-t"); args.Add(AsString(duration));
	args.Add(fileName);
	
	// Convert args to char* array for execvp
	Buffer<char*> argv(args.GetCount() + 1);
	for(int i = 0; i < args.GetCount(); i++)
		argv[i] = (char*)~args[i];
	argv[args.GetCount()] = NULL;

	Cout() << "Starting encoder: " << Join(args, " ") << "\n";

	int pipefd[2];
	if(pipe(pipefd) == -1) {
		Cerr() << "Failed to create pipe\n";
		delete capture;
		return;
	}

	pid_t pid = fork();
	if(pid == -1) {
		Cerr() << "Failed to fork\n";
		close(pipefd[0]);
		close(pipefd[1]);
		delete capture;
		return;
	}

	if(pid == 0) {
		// Child process (ffmpeg)
		close(pipefd[1]); // Close write end
		dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to read end
		close(pipefd[0]); // Close original read end
		
		execvp("ffmpeg", argv);
		Cerr() << "Failed to exec ffmpeg\n";
		_exit(1);
	}

	// Parent process
	close(pipefd[0]); // Close read end
	int writeFd = pipefd[1];

	Buffer<char> buffer(capture->getBufferSize());
	int64 startTime = msecs();
	int64 frameCount = 0;
	
	while((msecs() - startTime) < duration * 1000) {
		// Check if child is still running
		int status;
		if(waitpid(pid, &status, WNOHANG) == pid) {
			Cerr() << "ffmpeg exited prematurely\n";
			break;
		}

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000; // 100ms timeout
		
		if(capture->isReadable(&tv)) {
			size_t n = capture->read(buffer, capture->getBufferSize());
			if(n > 0) {
				const char* ptr = ~buffer;
				size_t remaining = n;
				while(remaining > 0) {
					ssize_t written = write(writeFd, ptr, remaining);
					if(written == -1) {
						if(errno == EINTR) continue;
						Cerr() << "Error writing to pipe\n";
						goto end_capture;
					}
					ptr += written;
					remaining -= written;
				}
				frameCount++;
				if(frameCount % 30 == 0) Cout() << ".";
			}
		}
	}

end_capture:
	close(writeFd); // Close pipe to signal EOF to ffmpeg
	
	// Wait for child to finish
	int status;
	waitpid(pid, &status, 0);
	
	Cout() << "\nCaptured " << frameCount << " frames.\n";
	
	delete capture;
}

CONSOLE_APP_MAIN
{
	const Vector<String>& args = CommandLine();
	if(args.GetCount() == 0) {
		Cout() << "Usage: \n"
		       << "  list\n"
		       << "  info <device>\n"
		       << "  capture <device> <output_file> [width] [height] [format]\n"
		       << "  video <device> <output_file> <duration_sec> [width] [height] [format]\n"
		       << "\nExamples:\n"
		       << "  bin/CameraV4L2 video /dev/video0 video.mkv 5 1280 720 MJPEG\n"
		       << "  bin/CameraV4L2 capture /dev/video0 output.jpg 1280 960 YUYV\n";
		return;
	}

	if(args[0] == "list") {
		ListDevices();
	}
	else if(args[0] == "info" && args.GetCount() >= 2) {
		DeviceInfo(args[1]);
	}
	else if(args[0] == "capture" && args.GetCount() >= 3) {
		int w = args.GetCount() >= 4 ? StrInt(args[3]) : 0;
		int h = args.GetCount() >= 5 ? StrInt(args[4]) : 0;
		String fmt = args.GetCount() >= 6 ? args[5] : "";
		CaptureImage(args[1], args[2], w, h, fmt);
	}
	else if(args[0] == "video" && args.GetCount() >= 4) {
		int w = args.GetCount() >= 5 ? StrInt(args[4]) : 0;
		int h = args.GetCount() >= 6 ? StrInt(args[5]) : 0;
		String fmt = args.GetCount() >= 7 ? args[6] : "";
		CaptureVideo(args[1], args[2], StrInt(args[3]), w, h, fmt);
	}
	else {
		Cout() << "Invalid arguments.\n";
	}
}

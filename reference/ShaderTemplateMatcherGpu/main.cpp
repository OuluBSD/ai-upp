#include <CtrlLib/CtrlLib.h>
#include <GLCtrl/GLCtrl.h>
#include <VisualStateModel/VisualStateModel.h>

using namespace Upp;

struct ShaderProbe : GLCtrl {
	typedef ShaderProbe CLASSNAME;
	VsmImageBuffer frame;
	VsmImageBuffer crop_map;
	VsmShaderTemplateManifest manifest;
	VsmShaderEvidence cpu;
	String error;
	GLuint program = 0;
	GLuint occupancy_program = 0;
	GLuint frame_texture = 0;
	GLuint crop_texture = 0;
	GLuint evidence_texture = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	bool finished = false;
	bool ok = false;
	String report_path = "tmp/vsm_shader_gpu_selftest.json";
	int dispatch_ms = 0;
	int occupancy_dispatch_ms = 0;
	bool occupancy_ok = false;
	String frame_path;
	String crop_map_path;
	String manifest_path;
	bool external_fixture = false;

	ShaderProbe()
	{
		frame = VsmImageBuffer::MakeSolid(16, 8, 0, 1);
		crop_map = VsmImageBuffer::MakeSolid(12, 6, 0, 1);
		for(int y = 0; y < 2; y++)
			for(int x = 0; x < 2; x++) {
				byte value = (byte)(x == y ? 240 : 120);
				crop_map.Set(x, y, value);
				frame.Set(6 + x, 3 + y, value);
			}
		for(int y = 0; y < 3; y++)
			for(int x = 0; x < 3; x++) {
				byte value = (byte)((x + y) % 2 ? 210 : 70);
				crop_map.Set(5 + x, 1 + y, value);
				frame.Set(2 + x, 1 + y, value);
			}
		frame.Set(2, 1, 0);
		manifest.crop_map_width = crop_map.width;
		manifest.crop_map_height = crop_map.height;
		VsmShaderTemplate& templ = manifest.templates.Add();
		templ.id = "synthetic-digit";
		templ.label = templ.id;
		templ.w = templ.foreground_w = 2;
		templ.h = templ.foreground_h = 2;
		VsmShaderTemplate& second = manifest.templates.Add();
		second.id = "synthetic-marker";
		second.label = second.id;
		second.x = 5;
		second.y = 1;
		second.w = second.foreground_w = 3;
		second.h = second.foreground_h = 3;
	}

	void SetFixture(const String& frame_file, const String& crop_file,
	               const String& manifest_file)
	{
		frame_path = frame_file;
		crop_map_path = crop_file;
		manifest_path = manifest_file;
		external_fixture = true;
	}

	static bool ToGray(VsmImageBuffer& image, String& error)
	{
		if(image.IsEmpty()) { error = "fixture image is empty"; return false; }
		if(image.channels == 1) return true;
		if(image.channels < 3) { error = "fixture image must have one or three channels"; return false; }
		VsmImageBuffer gray;
		gray.Create(image.width, image.height, 1);
		for(int y = 0; y < image.height; y++)
			for(int x = 0; x < image.width; x++) {
				int r = image.Get(x, y, 0), g = image.Get(x, y, 1), b = image.Get(x, y, 2);
				gray.Set(x, y, (byte)((299 * r + 587 * g + 114 * b + 500) / 1000));
			}
		image = pick(gray);
		return true;
	}

	bool LoadExternalFixture()
	{
		if(!frame.Load(frame_path)) {
			error = "cannot load frame fixture: " + frame_path;
			return false;
		}
		if(!crop_map.Load(crop_map_path)) {
			error = "cannot load crop-map fixture: " + crop_map_path;
			return false;
		}
		if(!manifest.Load(manifest_path)) {
			error = "cannot load manifest fixture: " + manifest_path;
			return false;
		}
		if(!ToGray(frame, error) || !ToGray(crop_map, error)) return false;
		manifest.crop_map_width = crop_map.width;
		manifest.crop_map_height = crop_map.height;
		return true;
	}

	static bool Check(GLuint value, const char *what, String& error)
	{
		if(value) return true;
		error = what;
		return false;
	}

	static GLuint Compile(GLenum type, const String& source, String& error)
	{
		GLuint shader = glCreateShader(type);
		if(!Check(shader, "glCreateShader failed", error)) return 0;
		const char *text = source.Begin();
		GLint length = source.GetCount();
		glShaderSource(shader, 1, &text, &length);
		glCompileShader(shader);
		GLint status = GL_FALSE;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if(status == GL_TRUE) return shader;
		char log[2048] = {};
		GLsizei written = 0;
		glGetShaderInfoLog(shader, sizeof(log), &written, log);
		error = String("shader compile failed: ") + String(log, written);
		glDeleteShader(shader);
		return 0;
	}

	bool BuildProgram()
	{
		String vertex_error, fragment_error;
		GLuint vertex = Compile(GL_VERTEX_SHADER,
			VsmCpuShaderTemplateMatcher::VertexShaderSource(), vertex_error);
		if(!vertex) { error = vertex_error; return false; }
		GLuint fragment = Compile(GL_FRAGMENT_SHADER,
			VsmCpuShaderTemplateMatcher::FragmentShaderSource(), fragment_error);
		if(!fragment) { glDeleteShader(vertex); error = fragment_error; return false; }
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		GLint status = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if(status == GL_TRUE) return true;
		char log[2048] = {};
		GLsizei written = 0;
		glGetProgramInfoLog(program, sizeof(log), &written, log);
		error = String("shader link failed: ") + String(log, written);
		return false;
	}

	bool BuildOccupancyProgram()
	{
		String vertex_error, fragment_error;
		GLuint vertex = Compile(GL_VERTEX_SHADER,
			VsmCpuShaderTemplateMatcher::VertexShaderSource(), vertex_error);
		if(!vertex) { error = vertex_error; return false; }
		String fragment_source =
			"#version 330 core\n"
			"uniform sampler2D evidence_image;\n"
			"uniform ivec2 evidence_size;\n"
			"uniform ivec2 tile_count;\n"
			"uniform int tile_size;\n"
			"uniform int threshold;\n"
			"layout(location=0) out vec4 occupancy;\n"
			"void main() {\n"
			"  ivec2 tile = ivec2(gl_FragCoord.xy); float found = 0.0;\n"
			"  for(int y=0; y<64; y++) { if(y>=tile_size) break;\n"
			"    for(int x=0; x<64; x++) { if(x>=tile_size) break;\n"
			"      ivec2 p = tile * tile_size + ivec2(x, y);\n"
			"      if(p.x<evidence_size.x && p.y<evidence_size.y &&\n"
			"         texelFetch(evidence_image, p, 0).r * 255.0 >= float(threshold)) found = 1.0;\n"
			"    }\n"
			"  }\n"
			"  occupancy = vec4(found, 0.0, 0.0, 1.0);\n"
			"}\n";
		GLuint fragment = Compile(GL_FRAGMENT_SHADER, fragment_source, fragment_error);
		if(!fragment) { glDeleteShader(vertex); error = fragment_error; return false; }
		occupancy_program = glCreateProgram();
		glAttachShader(occupancy_program, vertex);
		glAttachShader(occupancy_program, fragment);
		glLinkProgram(occupancy_program);
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		GLint status = GL_FALSE;
		glGetProgramiv(occupancy_program, GL_LINK_STATUS, &status);
		if(status == GL_TRUE) return true;
		char log[2048] = {};
		GLsizei written = 0;
		glGetProgramInfoLog(occupancy_program, sizeof(log), &written, log);
		error = String("occupancy shader link failed: ") + String(log, written);
		return false;
	}

	bool UploadTexture(GLuint& texture, const VsmImageBuffer& image)
	{
		glGenTextures(1, &texture);
		if(!texture) { error = "glGenTextures failed"; return false; }
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, image.width, image.height, 0,
		             GL_RED, GL_UNSIGNED_BYTE, image.pixels.Begin());
		if(glGetError() != GL_NO_ERROR) { error = "texture upload failed"; return false; }
		return true;
	}

	bool Setup()
	{
		if(external_fixture && !LoadExternalFixture()) return false;
		if(!manifest.Validate(error)) return false;
		VsmCpuShaderTemplateMatcher matcher;
		if(!matcher.Match(frame, crop_map, manifest, cpu, error)) return false;
		if(!BuildProgram()) return false;
		if(!BuildOccupancyProgram()) return false;
		VsmImageBuffer occupancy_input = VsmImageBuffer::MakeSolid(cpu.image.width, cpu.image.height, 0, 1);
		for(int y = 0; y < cpu.image.height; y++)
			for(int x = 0; x < cpu.image.width; x++)
				occupancy_input.Set(x, y, cpu.image.Get(x, y));
		if(!UploadTexture(frame_texture, frame) || !UploadTexture(crop_texture, crop_map) ||
		   !UploadTexture(evidence_texture, occupancy_input)) return false;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		if(!vao || !vbo) { error = "vertex object creation failed"; return false; }
		const GLfloat vertices[] = {-1, -1, 1, -1, 1, 1, -1, 1};
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		GLint position = glGetAttribLocation(program, "position");
		if(position < 0) { error = "position attribute missing"; return false; }
		glEnableVertexAttribArray((GLuint)position);
		glVertexAttribPointer((GLuint)position, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		return glGetError() == GL_NO_ERROR;
	}

	void GLPaint() override
	{
		if(finished) return;
		if(!Setup()) {
			Cout() << "shader-gpu-selftest backend=opengl status=error error=" << error << "\n";
			SaveFile(report_path, Format("{\"backend\":\"opengl\",\"status\":\"error\",\"error\":\"%s\"}\n", error));
			finished = true;
			SetTimeCallback(100, THISBACK(CloseProbe));
			return;
		}
		glViewport(0, 0, frame.width, frame.height);
		glUseProgram(program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, frame_texture);
		glUniform1i(glGetUniformLocation(program, "frame_image"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, crop_texture);
		glUniform1i(glGetUniformLocation(program, "crop_map"), 1);
		glUniform2i(glGetUniformLocation(program, "frame_size"), frame.width, frame.height);
		glUniform2i(glGetUniformLocation(program, "crop_map_size"), crop_map.width, crop_map.height);
		glUniform1i(glGetUniformLocation(program, "template_count"), manifest.templates.GetCount());
		Vector<GLint> rects(manifest.templates.GetCount() * 4);
		Vector<GLint> foregrounds(manifest.templates.GetCount() * 4);
		Vector<GLint> polarities(manifest.templates.GetCount());
		for(int i = 0; i < manifest.templates.GetCount(); i++) {
			const VsmShaderTemplate& templ = manifest.templates[i];
			rects[i * 4 + 0] = templ.x;
			rects[i * 4 + 1] = templ.y;
			rects[i * 4 + 2] = templ.w;
			rects[i * 4 + 3] = templ.h;
			foregrounds[i * 4 + 0] = templ.foreground_x;
			foregrounds[i * 4 + 1] = templ.foreground_y;
			foregrounds[i * 4 + 2] = templ.foreground_w;
			foregrounds[i * 4 + 3] = templ.foreground_h;
			polarities[i] = templ.polarity;
		}
		glUniform4iv(glGetUniformLocation(program, "template_rects"), manifest.templates.GetCount(), rects.Begin());
		glUniform4iv(glGetUniformLocation(program, "foreground_rects"), manifest.templates.GetCount(), foregrounds.Begin());
		glUniform1iv(glGetUniformLocation(program, "template_polarity"), manifest.templates.GetCount(), polarities.Begin());
		glBindVertexArray(vao);
		dword start = GetTickCount();
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glFinish();
		dispatch_ms = (int)(GetTickCount() - start);
		Vector<byte> pixels(frame.width * frame.height * 4);
		glReadPixels(0, 0, frame.width, frame.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.Begin());
		const VsmShaderMatchHit *hit = nullptr;
		for(const VsmShaderMatchHit& candidate : cpu.best_hits)
			if(!hit || candidate.score > hit->score)
				hit = &candidate;
		if(!hit) { error = "CPU reference produced no hit"; return; }
		int gpu_x = -1, gpu_y = -1;
		byte gpu_score = 0;
		byte gpu_id = 0;
		for(int y = 0; y < frame.height; y++)
			for(int x = 0; x < frame.width; x++) {
				const byte *p = pixels.Begin() + (y * frame.width + x) * 4;
				if(p[0] > gpu_score) {
					gpu_score = p[0]; gpu_id = p[1]; gpu_x = x; gpu_y = y;
				}
			}
		ok = glGetError() == GL_NO_ERROR && gpu_id == hit->template_index &&
		     gpu_x == hit->x && gpu_y == hit->y;
		Cout() << "shader-gpu-selftest backend=opengl status=" << (ok ? "pass" : "fail")
		       << " fixture=" << (external_fixture ? "external" : "synthetic")
		       << " dimensions=" << frame.width << "x" << frame.height
		       << " template_count=" << manifest.templates.GetCount()
		       << " dispatch=fragment-loop"
		       << " dispatch_ms=" << dispatch_ms
		       << " cpu_template=" << hit->template_index
		       << " cpu=" << hit->x << "," << hit->y
		       << " gpu=" << gpu_x << "," << gpu_y
		       << " cpu_score=" << hit->score << " gpu_score=" << (int)gpu_score << "\n";
		SaveFile(report_path, Format("{\"backend\":\"opengl\",\"status\":\"%s\",\"dimensions\":\"%dx%d\",\"template_count\":%d,\"dispatch\":\"fragment-loop\",\"dispatch_ms\":%d,\"cpu_template\":%d,\"cpu_x\":%d,\"cpu_y\":%d,\"gpu_template\":%d,\"gpu_x\":%d,\"gpu_y\":%d,\"cpu_score\":%.6f,\"gpu_score\":%d}\n",
		         ok ? "pass" : "fail", frame.width, frame.height,
		         manifest.templates.GetCount(), dispatch_ms, hit->template_index,
		         hit->x, hit->y, (int)gpu_id, gpu_x, gpu_y, hit->score, (int)gpu_score));
		int threshold = 220;
		int tile_size = 2;
		VsmTileOccupancyMask cpu_tiles = VsmBuildTileOccupancyMask(cpu.image, threshold, tile_size);
		int tile_width = cpu_tiles.width;
		int tile_height = cpu_tiles.height;
		glViewport(0, 0, tile_width, tile_height);
		glUseProgram(occupancy_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, evidence_texture);
		glUniform1i(glGetUniformLocation(occupancy_program, "evidence_image"), 0);
		glUniform2i(glGetUniformLocation(occupancy_program, "evidence_size"), cpu.image.width, cpu.image.height);
		glUniform2i(glGetUniformLocation(occupancy_program, "tile_count"), tile_width, tile_height);
		glUniform1i(glGetUniformLocation(occupancy_program, "tile_size"), tile_size);
		glUniform1i(glGetUniformLocation(occupancy_program, "threshold"), threshold);
		glBindVertexArray(vao);
		dword occupancy_start = GetTickCount();
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glFinish();
		occupancy_dispatch_ms = (int)(GetTickCount() - occupancy_start);
		Vector<byte> occupancy_pixels(tile_width * tile_height * 4);
		glReadPixels(0, 0, tile_width, tile_height, GL_RGBA, GL_UNSIGNED_BYTE, occupancy_pixels.Begin());
		occupancy_ok = glGetError() == GL_NO_ERROR;
		for(int y = 0; occupancy_ok && y < tile_height; y++)
			for(int x = 0; x < tile_width; x++) {
				bool gpu_set = occupancy_pixels[(y * tile_width + x) * 4] >= 128;
				if(gpu_set != cpu_tiles.IsSet(x, y)) occupancy_ok = false;
			}
		Cout() << "shader-gpu-occupancy backend=opengl status=" << (occupancy_ok ? "pass" : "fail")
		       << " threshold=" << threshold << " tile_size=" << tile_size
		       << " dimensions=" << tile_width << "x" << tile_height
		       << " dispatch_ms=" << occupancy_dispatch_ms
		       << " gpu_bytes=" << tile_width * tile_height
		       << " cpu_bytes=" << cpu.image.width * cpu.image.height
		       << " cpu_reference=true\n";
		ok = ok && occupancy_ok;
		finished = true;
		SetTimeCallback(100, THISBACK(CloseProbe));
	}

	void CloseProbe()
	{
		if(program) glDeleteProgram(program);
		if(occupancy_program) glDeleteProgram(occupancy_program);
		if(frame_texture) glDeleteTextures(1, &frame_texture);
		if(crop_texture) glDeleteTextures(1, &crop_texture);
		if(evidence_texture) glDeleteTextures(1, &evidence_texture);
		if(vbo) glDeleteBuffers(1, &vbo);
		if(vao) glDeleteVertexArrays(1, &vao);
		TopWindow *window = dynamic_cast<TopWindow *>(GetTopCtrl());
		if(window) window->Close();
		SetExitCode(ok ? 0 : 1);
	}
};

GUI_APP_MAIN
{
	TopWindow window;
	ShaderProbe probe;
	const Vector<String>& args = CommandLine();
	if(args.GetCount() == 7 && args[0] == "--fixture" && args[1] == "--frame" &&
	   args[3] == "--crop-map" && args[5] == "--manifest")
		probe.SetFixture(args[2], args[4], args[6]);
	else if(!args.IsEmpty()) {
		Cerr() << "usage: ShaderTemplateMatcherGpu [--fixture --frame <vsm> --crop-map <vsm> --manifest <json>]\n";
		SetExitCode(2);
		return;
	}
	window.Title("Shader template GPU self-test");
	window.Add(probe.SizePos());
	window.SetRect(0, 0, 64, 64);
	window.Open();
	probe.Refresh();
	window.Run();
}

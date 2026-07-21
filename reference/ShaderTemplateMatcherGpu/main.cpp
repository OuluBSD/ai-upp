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
	GLuint frame_texture = 0;
	GLuint crop_texture = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	bool finished = false;
	bool ok = false;
	String report_path = "tmp/vsm_shader_gpu_selftest.json";

	ShaderProbe()
	{
		frame = VsmImageBuffer::MakeSolid(16, 8, 0, 1);
		crop_map = VsmImageBuffer::MakeSolid(8, 4, 0, 1);
		for(int y = 0; y < 2; y++)
			for(int x = 0; x < 2; x++) {
				byte value = (byte)(x == y ? 240 : 120);
				crop_map.Set(x, y, value);
				frame.Set(6 + x, 3 + y, value);
			}
		manifest.crop_map_width = crop_map.width;
		manifest.crop_map_height = crop_map.height;
		VsmShaderTemplate& templ = manifest.templates.Add();
		templ.id = "synthetic-digit";
		templ.label = templ.id;
		templ.w = templ.foreground_w = 2;
		templ.h = templ.foreground_h = 2;
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
		if(!manifest.Validate(error)) return false;
		VsmCpuShaderTemplateMatcher matcher;
		if(!matcher.Match(frame, crop_map, manifest, cpu, error)) return false;
		if(!BuildProgram()) return false;
		if(!UploadTexture(frame_texture, frame) || !UploadTexture(crop_texture, crop_map)) return false;
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
		GLint rect[4] = {0, 0, 2, 2};
		GLint foreground[4] = {0, 0, 2, 2};
		GLint polarity[1] = {manifest.templates[0].polarity};
		glUniform4iv(glGetUniformLocation(program, "template_rects"), 1, rect);
		glUniform4iv(glGetUniformLocation(program, "foreground_rects"), 1, foreground);
		glUniform1iv(glGetUniformLocation(program, "template_polarity"), 1, polarity);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glFinish();
		Vector<byte> pixels(frame.width * frame.height * 4);
		glReadPixels(0, 0, frame.width, frame.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.Begin());
		const VsmShaderMatchHit& hit = cpu.best_hits[0];
		int gpu_x = -1, gpu_y = -1;
		byte gpu_score = 0;
		for(int y = 0; y < frame.height; y++)
			for(int x = 0; x < frame.width; x++) {
				const byte *p = pixels.Begin() + (y * frame.width + x) * 4;
				if(p[0] > gpu_score) { gpu_score = p[0]; gpu_x = x; gpu_y = y; }
			}
		ok = glGetError() == GL_NO_ERROR && gpu_x == hit.x && gpu_y == hit.y;
		Cout() << "shader-gpu-selftest backend=opengl status=" << (ok ? "pass" : "fail")
		       << " dimensions=" << frame.width << "x" << frame.height
		       << " cpu=" << hit.x << "," << hit.y
		       << " gpu=" << gpu_x << "," << gpu_y
		       << " cpu_score=" << hit.score << " gpu_score=" << (int)gpu_score << "\n";
		SaveFile(report_path, Format("{\"backend\":\"opengl\",\"status\":\"%s\",\"cpu_x\":%d,\"cpu_y\":%d,\"gpu_x\":%d,\"gpu_y\":%d,\"cpu_score\":%.6f,\"gpu_score\":%d}\n",
		         ok ? "pass" : "fail", hit.x, hit.y, gpu_x, gpu_y, hit.score, (int)gpu_score));
		finished = true;
		SetTimeCallback(100, THISBACK(CloseProbe));
	}

	void CloseProbe()
	{
		if(program) glDeleteProgram(program);
		if(frame_texture) glDeleteTextures(1, &frame_texture);
		if(crop_texture) glDeleteTextures(1, &crop_texture);
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
	window.Title("Shader template GPU self-test");
	window.Add(probe.SizePos());
	window.SetRect(0, 0, 64, 64);
	window.Open();
	probe.Refresh();
	window.Run();
}

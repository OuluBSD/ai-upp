#include "Eon09.h"

#if defined(flagSDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

namespace Upp {

void Run09bScene3DSdlOgl(int method) {
#if !defined(flagSDL2) || !defined(flagOGL)
	throw Exc("Run09bScene3DSdlOgl: SDL2+OGL not available in this build");
#else
	String manifest_path = ShareDirFile("scene3d/projects/driving_softphys/project.exec.json");
	Scene3DTestContext ctx;
	if (!LoadScene3DTestProject(manifest_path, ctx))
		throw Exc("Run09bScene3DSdlOgl: load failed");
	LoadScene3DTestModels(ctx);

	Scene3DRenderConfig conf;
	Scene3DRenderContext rctx;
	GeomVideo video;
	rctx.conf = &conf;
	rctx.state = ctx.state;
	rctx.anim = ctx.anim;
	rctx.video = &video;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw Exc(String().Cat() << "Run09bScene3DSdlOgl: SDL_Init failed: " << SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

	int w = 640;
	int h = 480;
	Uint32 win_flags = SDL_WINDOW_OPENGL;
	if (method > 0)
		win_flags |= SDL_WINDOW_HIDDEN;
	SDL_Window* win = SDL_CreateWindow("Eon09 SDL2 OGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                                  w, h, win_flags);
	if (!win) {
		String err = SDL_GetError();
		SDL_Quit();
		throw Exc(String().Cat() << "Run09bScene3DSdlOgl: SDL_CreateWindow failed: " << err);
	}
	SDL_GLContext gl = SDL_GL_CreateContext(win);
	if (!gl) {
		String err = SDL_GetError();
		SDL_DestroyWindow(win);
		SDL_Quit();
		throw Exc(String().Cat() << "Run09bScene3DSdlOgl: SDL_GL_CreateContext failed: " << err);
	}
	SDL_GL_MakeCurrent(win, gl);

	int frames = method > 0 ? method : 120;
	for (int i = 0; i < frames; i++) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				i = frames;
				break;
			}
			if (ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP) {
				bool down = ev.type == SDL_KEYDOWN;
				int key = (int)ev.key.keysym.sym;
				ctx.runtime.DispatchInputEvent(down ? "keyDown" : "keyUp", Point(0, 0), 0, key, 0);
			}
			else if (ev.type == SDL_MOUSEMOTION) {
				Point p(ev.motion.x, ev.motion.y);
				ctx.runtime.DispatchInputEvent("mouseMove", p, 0, 0, 0);
			}
			else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP) {
				Point p(ev.button.x, ev.button.y);
				int btn = 0;
				if (ev.button.button == SDL_BUTTON_LEFT) {
					btn = 0;
				}
				else if (ev.button.button == SDL_BUTTON_RIGHT) {
					btn = 1;
				}
				else if (ev.button.button == SDL_BUTTON_MIDDLE) {
					btn = 2;
				}
				ctx.runtime.DispatchInputEvent(ev.type == SDL_MOUSEBUTTONDOWN ? "mouseDown" : "mouseUp",
				                              p, 0, btn, 0);
			}
			else if (ev.type == SDL_MOUSEWHEEL) {
				Point p(0, 0);
				int zdelta = ev.wheel.y;
				ctx.runtime.DispatchInputEvent("mouseWheel", p, 0, zdelta, 0);
			}
		}
		ctx.anim->Update(1.0 / 60.0);
		ctx.runtime.Update(1.0 / 60.0);
		Scene3DRenderStats stats;
		Image img;
		bool ok = RenderSceneV2Headless(rctx, Size(w, h), &stats, &img, nullptr, false);
		if (!ok || img.IsEmpty())
			throw Exc("Run09bScene3DSdlOgl: render failed");

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glViewport(0, 0, (GLsizei)w, (GLsizei)h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, w, 0, h, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRasterPos2i(0, h);
		glPixelZoom(1, -1);
		ImageBuffer ib(img);
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, ib.Begin());
		glPixelZoom(1, 1);
		glFlush();
		SDL_GL_SwapWindow(win);
		if (method == 0)
			SDL_Delay(16);
	}

	SDL_GL_DeleteContext(gl);
	SDL_DestroyWindow(win);
	SDL_Quit();
#endif
}

} // namespace Upp

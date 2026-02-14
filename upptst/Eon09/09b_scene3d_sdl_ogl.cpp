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
	SDL_Window* win = SDL_CreateWindow("Eon09 SDL2 OGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                                  w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
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

	int frames = method > 0 ? method : 3;
	for (int i = 0; i < frames; i++) {
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
	}

	SDL_GL_DeleteContext(gl);
	SDL_DestroyWindow(win);
	SDL_Quit();
#endif
}

} // namespace Upp

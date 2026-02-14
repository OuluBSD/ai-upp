#include "Eon09.h"

namespace Upp {

void Run09aScene3DHeadless(int method) {
	String manifest_path = ShareDirFile("scene3d/projects/driving_softphys/project.exec.json");
	Scene3DTestContext ctx;
	if (!LoadScene3DTestProject(manifest_path, ctx))
		throw Exc("Run09aScene3DHeadless: load failed");
	LoadScene3DTestModels(ctx);

	Scene3DRenderConfig conf;
	Scene3DRenderContext rctx;
	GeomVideo video;
	rctx.conf = &conf;
	rctx.state = ctx.state;
	rctx.anim = ctx.anim;
	rctx.video = &video;

	int frames = method > 0 ? method : 3;
	for (int i = 0; i < frames; i++) {
		ctx.anim->Update(1.0 / 60.0);
		ctx.runtime.Update(1.0 / 60.0);
	}

	Scene3DRenderStats stats;
	Image img;
	bool ok = RenderSceneV2Headless(rctx, Size(640, 480), &stats, &img, nullptr, false);
	if (!ok || !stats.rendered || img.IsEmpty())
		throw Exc("Run09aScene3DHeadless: render failed");
	Cout() << "Run09aScene3DHeadless: rendered pixels=" << stats.pixels << '\n';
}

} // namespace Upp

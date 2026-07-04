#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>
#include <Ctrl/Compositing/Compositing.h>

using namespace Upp;

static int g_fail = 0;

// Standard straight-alpha "over" formula, per channel, fg over an opaque bg:
//   result = (fg * fa + bg * (255 - fa)) / 255
static int AlphaOver(int fg, int bg, int fa)
{
	return (fg * fa + bg * (255 - fa)) / 255;
}

static void CheckOverlapBlend()
{
	Size sz(40, 40);
	Color bg_color(200, 50, 40);
	Color fg_color(10, 220, 130);
	double fg_alpha01 = 0.4;
	int fa = (int)(fg_alpha01 * 255.0 + 0.5); // 102 -- matches Compositing.cpp's own rounding

	CompositedLayer bg_layer, fg_layer;
	bg_layer.Paint(sz, [&](Draw& w) { w.DrawRect(sz, bg_color); });
	Rect square(10, 10, 30, 30); // 20x20 opaque square in the middle of a 40x40, else-transparent buffer
	fg_layer.Paint(sz, [&](Draw& w) { w.DrawRect(square, fg_color); });

	Vector<LayerEntry> layers;
	LayerEntry& e0 = layers.Add();
	e0.pos = Point(0, 0);
	e0.alpha01 = 1.0;
	e0.layer = &bg_layer;
	LayerEntry& e1 = layers.Add();
	e1.pos = Point(0, 0);
	e1.alpha01 = fg_alpha01;
	e1.layer = &fg_layer;

	SImageDraw canvas(sz);
	CompositeLayers(canvas, layers);
	Image result = canvas;
	ImageBuffer rb(result);

	// Inside the overlap: bg_color blended with fg_color at fa.
	RGBA inside = rb[20][20];
	int exp_r = AlphaOver(fg_color.GetR(), bg_color.GetR(), fa);
	int exp_g = AlphaOver(fg_color.GetG(), bg_color.GetG(), fa);
	int exp_b = AlphaOver(fg_color.GetB(), bg_color.GetB(), fa);
	int tol = 2;
	bool inside_ok = abs((int)inside.r - exp_r) <= tol &&
	                 abs((int)inside.g - exp_g) <= tol &&
	                 abs((int)inside.b - exp_b) <= tol &&
	                 abs((int)inside.a - 255) <= tol;

	Cout() << "Overlap blend check (fg=(" << fg_color.GetR() << "," << fg_color.GetG() << "," << fg_color.GetB()
	       << ") fa=" << fa << " over bg=(" << bg_color.GetR() << "," << bg_color.GetG() << "," << bg_color.GetB()
	       << ")):\n";
	Cout() << "  expected = (" << exp_r << "," << exp_g << "," << exp_b << ",255)\n";
	Cout() << "  actual   = (" << (int)inside.r << "," << (int)inside.g << "," << (int)inside.b
	       << "," << (int)inside.a << ") -- " << (inside_ok ? "PASS" : "FAIL") << "\n";
	if(!inside_ok)
		g_fail++;

	// Outside the overlap (but still inside the fg layer's own 40x40 buffer,
	// just outside its painted 20x20 square): fg layer contributed nothing
	// there (its own buffer stayed fully transparent), so it must be
	// untouched pure bg_color -- proves CompositedLayer::Paint really does
	// clear to fully transparent first, not e.g. leak the fg color at a
	// lower alpha everywhere.
	RGBA outside = rb[2][2];
	bool outside_ok = abs((int)outside.r - bg_color.GetR()) <= tol &&
	                   abs((int)outside.g - bg_color.GetG()) <= tol &&
	                   abs((int)outside.b - bg_color.GetB()) <= tol &&
	                   abs((int)outside.a - 255) <= tol;

	Cout() << "Outside-overlap check (must equal untouched bg):\n";
	Cout() << "  expected = (" << bg_color.GetR() << "," << bg_color.GetG() << "," << bg_color.GetB() << ",255)\n";
	Cout() << "  actual   = (" << (int)outside.r << "," << (int)outside.g << "," << (int)outside.b
	       << "," << (int)outside.a << ") -- " << (outside_ok ? "PASS" : "FAIL") << "\n";
	if(!outside_ok)
		g_fail++;
}

// Confirms CompositedLayer::Paint clears to fully transparent first: an
// untouched pixel in a freshly-painted layer's own buffer must read back as
// RGBA(0,0,0,0), before any compositing happens at all.
static void CheckFreshLayerIsTransparent()
{
	Size sz(16, 16);
	CompositedLayer layer;
	layer.Paint(sz, [&](Draw& w) { w.DrawRect(Rect(4, 4, 12, 12), Color(255, 0, 0)); });
	Image img = layer.GetImage();
	ImageBuffer ib(img);
	RGBA untouched = ib[0][0];
	RGBA painted = ib[8][8];

	bool ok = untouched.r == 0 && untouched.g == 0 && untouched.b == 0 && untouched.a == 0 &&
	          painted.a == 255;

	Cout() << "Fresh-layer transparency check: untouched=(" << (int)untouched.r << "," << (int)untouched.g
	       << "," << (int)untouched.b << "," << (int)untouched.a << ") painted.a=" << (int)painted.a
	       << " -- " << (ok ? "PASS" : "FAIL") << "\n";
	if(!ok)
		g_fail++;
}

GUI_APP_MAIN
{
	Cout() << "CompositedLayer / CompositeLayers sanity checks\n";
	CheckFreshLayerIsTransparent();
	CheckOverlapBlend();

	Cout() << "\n" << (g_fail == 0 ? "ALL CHECKS PASSED" : Format("%d CHECK(S) FAILED", g_fail)) << "\n";
	SetExitCode(g_fail == 0 ? 0 : 1);
}

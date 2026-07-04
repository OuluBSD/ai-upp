#include <Ctrl/Xform3D/Xform3D.h>

using namespace Upp;

static int g_fail = 0;

static void CheckIdentity()
{
	Xform3D xf;
	xf.Set(0.0, 0.0, Size(100, 100));
	Vector<Pointf> g = xf.ProjectGrid(1); // 2x2 -> 4 corners
	// row-major: 0=top-left, 1=top-right, 2=bottom-left, 3=bottom-right
	Pointf tl = g[0], tr = g[1], bl = g[2], br = g[3];

	double tol = 2.0;
	bool ok = fabs(tl.x - -50) < tol && fabs(tl.y - -50) < tol &&
	          fabs(tr.x - 50) < tol && fabs(tr.y - -50) < tol &&
	          fabs(bl.x - -50) < tol && fabs(bl.y - 50) < tol &&
	          fabs(br.x - 50) < tol && fabs(br.y - 50) < tol;

	Cout() << "Identity check: corners = "
	       << "(" << tl.x << "," << tl.y << ") "
	       << "(" << tr.x << "," << tr.y << ") "
	       << "(" << bl.x << "," << bl.y << ") "
	       << "(" << br.x << "," << br.y << ") -- "
	       << (ok ? "PASS (within tolerance)" : "FAIL (outside tolerance)") << "\n";
	if(!ok)
		g_fail++;
}

static void CheckCollapse()
{
	Xform3D identity;
	identity.Set(0.0, 0.0, Size(100, 100));
	Vector<Pointf> gi = identity.ProjectGrid(1);
	double identity_width = fabs(gi[1].x - gi[0].x); // top-right.x - top-left.x

	Xform3D edge;
	edge.Set(0.0, M_PI / 2 - 0.001, Size(100, 100));
	Vector<Pointf> ge = edge.ProjectGrid(1);
	double rotated_width = fabs(ge[1].x - ge[0].x);

	bool finite = std::isfinite(ge[0].x) && std::isfinite(ge[0].y) &&
	              std::isfinite(ge[1].x) && std::isfinite(ge[1].y) &&
	              std::isfinite(ge[2].x) && std::isfinite(ge[2].y) &&
	              std::isfinite(ge[3].x) && std::isfinite(ge[3].y);

	bool ok = finite && rotated_width < identity_width * 0.5;

	Cout() << "Collapse check: identity_width=" << identity_width
	       << " rotated_width=" << rotated_width << " -- "
	       << (ok ? "PASS (foreshortened)" : "FAIL") << "\n";
	if(!ok)
		g_fail++;
}

static void CheckRowOrder()
{
	Xform3D xf;
	xf.Set(0.0, 0.3, Size(100, 100));
	Vector<Pointf> g = xf.ProjectGrid(1);
	// row-major, stride 2: row0 = indices 0,1 ; row1 = indices 2,3
	double row0_y = (g[0].y + g[1].y) / 2.0;
	double row1_y = (g[2].y + g[3].y) / 2.0;

	bool ok = row0_y < row1_y;

	Cout() << "Row order check: row0_y=" << row0_y << " row1_y=" << row1_y << " -- "
	       << (ok ? "PASS (row0 above row1)" : "FAIL (inverted)") << "\n";
	if(!ok)
		g_fail++;
}

// Shoelace area of the 4 projected corners (n=1 grid), taken in perimeter
// order (tl, tr, br, bl = indices 0, 1, 3, 2). A rotate-about-center
// perspective composition of two hinge tilts genuinely turns the quad into
// a skewed quadrilateral -- one corner (the one nearest the camera) can end
// up projected *larger* than in the identity case, so a plain bounding-box
// width/height comparison is not a reliable "is this foreshortened" signal
// (verified empirically below; see the report notes). Area is: it shrinks
// monotonically as either hinge rotates away from face-on, for any camera
// distance/FOV, because perspective foreshortening always reduces projected
// solid angle overall even while spreading it unevenly across the quad.
static double ShoelaceArea(const Vector<Pointf>& g)
{
	Pointf p[4] = { g[0], g[1], g[3], g[2] }; // tl, tr, br, bl
	double sum = 0;
	for(int i = 0; i < 4; i++) {
		const Pointf& a = p[i];
		const Pointf& b = p[(i + 1) % 4];
		sum += a.x * b.y - b.x * a.y;
	}
	return fabs(sum) / 2.0;
}

static void CheckCombinedAngle()
{
	Xform3D identity;
	identity.Set(0.0, 0.0, Size(100, 100));
	double identity_area = ShoelaceArea(identity.ProjectGrid(1));

	Xform3D single_x;
	single_x.Set(0.4, 0.0, Size(100, 100));
	double single_x_area = ShoelaceArea(single_x.ProjectGrid(1));

	Xform3D single_y;
	single_y.Set(0.0, 0.4, Size(100, 100));
	double single_y_area = ShoelaceArea(single_y.ProjectGrid(1));

	Xform3D combined;
	combined.Set(0.4, 0.4, Size(100, 100));
	Vector<Pointf> gc = combined.ProjectGrid(1);
	double combined_area = ShoelaceArea(gc);

	bool finite = true;
	for(int i = 0; i < gc.GetCount(); i++)
		finite = finite && std::isfinite(gc[i].x) && std::isfinite(gc[i].y);

	// Both angles nonzero must foreshorten (shrink projected area) *more* than
	// either single-axis rotation alone, and more than the flat identity case
	// -- if the composition only applied one of the two rotations, combined_area
	// would exactly equal single_x_area or single_y_area; if the two rotations
	// cancelled each other out, combined_area would equal identity_area.
	bool ok = finite &&
	          combined_area < identity_area * 0.999 &&
	          combined_area < single_x_area * 0.999 &&
	          combined_area < single_y_area * 0.999;

	Cout() << "Combined-angle check: identity_area=" << identity_area
	       << " angle_x-only_area=" << single_x_area
	       << " angle_y-only_area=" << single_y_area
	       << " combined_area=" << combined_area << " -- "
	       << (ok ? "PASS (combined foreshortens more than either single axis)" : "FAIL") << "\n";
	if(!ok)
		g_fail++;
}

CONSOLE_APP_MAIN
{
	Cout() << "Xform3D sanity checks\n";
	CheckIdentity();
	CheckCollapse();
	CheckRowOrder();
	CheckCombinedAngle();

	Cout() << "\n" << (g_fail == 0 ? "ALL CHECKS PASSED" : Format("%d CHECK(S) FAILED", g_fail)) << "\n";
	SetExitCode(g_fail == 0 ? 0 : 1);
}

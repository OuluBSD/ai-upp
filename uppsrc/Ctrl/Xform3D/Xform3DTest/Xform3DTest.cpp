#include <Ctrl/Xform3D/Xform3D.h>

using namespace Upp;

static int g_fail = 0;

static void CheckIdentity()
{
	Xform3D xf;
	xf.Set(Xform3D::ROT_Y, 0.0, Size(100, 100));
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
	identity.Set(Xform3D::ROT_Y, 0.0, Size(100, 100));
	Vector<Pointf> gi = identity.ProjectGrid(1);
	double identity_width = fabs(gi[1].x - gi[0].x); // top-right.x - top-left.x

	Xform3D edge;
	edge.Set(Xform3D::ROT_Y, M_PI / 2 - 0.001, Size(100, 100));
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
	xf.Set(Xform3D::ROT_Y, 0.3, Size(100, 100));
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

CONSOLE_APP_MAIN
{
	Cout() << "Xform3D sanity checks\n";
	CheckIdentity();
	CheckCollapse();
	CheckRowOrder();

	Cout() << "\n" << (g_fail == 0 ? "ALL CHECKS PASSED" : Format("%d CHECK(S) FAILED", g_fail)) << "\n";
	SetExitCode(g_fail == 0 ? 0 : 1);
}

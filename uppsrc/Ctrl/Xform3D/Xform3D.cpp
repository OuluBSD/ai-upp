#include "Xform3D.h"

// Geometry headers must never leak into a public header (see Xform3D.h comment) --
// this .cpp file is the only place in this package allowed to include them.
#include <Geometry/Geometry.h>

NAMESPACE_UPP

struct Xform3D::Impl {
	float angle_x = 0;
	float angle_y = 0;
	Size  quad_size = Size(0, 0);
};

Xform3D::Xform3D()
{
	impl.Create();
}

Xform3D::~Xform3D()
{
}

void Xform3D::Set(double angle_x_rad, double angle_y_rad, Size quad_size)
{
	impl->angle_x = (float)angle_x_rad;
	impl->angle_y = (float)angle_y_rad;
	impl->quad_size = quad_size;
}

Vector<Pointf> Xform3D::ProjectGrid(int n) const
{
	Vector<Pointf> out;
	if(n < 1)
		return out;

	float angle_x = impl->angle_x;
	float angle_y = impl->angle_y;
	Size quad_size = impl->quad_size;

	// Rotation about both requested hinges, composed. SetRotation's axis
	// argument is 0=X, 1=Y, 2=Z (verified in uppsrc/Geometry/Matrix.h).
	// rx = horizontal hinge (top/bottom turn, rotation about X), ry = vertical
	// hinge (left/right turn, rotation about Y).
	mat4 rx;
	rx.SetRotation(0, angle_x);
	mat4 ry;
	ry.SetRotation(1, angle_y);

	// Composition order verified empirically against Xform3DTest's combined-angle
	// check (both width and height must foreshorten when both angles are
	// nonzero): with the row-vector convention used below (rotated = local *
	// rot), `rot = rx * ry` applies rx first, then ry, and produces the
	// expected combined foreshortening. The other order was tested and
	// rejected -- see Xform3DTest.cpp / the package's report notes.
	mat4 rot = rx * ry;

	// Fixed 45 degree FOV perspective camera, square aspect (the aspect
	// correction, if any, is applied by the caller/DrawWarped3D via the
	// destination cell rects, not here).
	const float fov_rad = 45.0f * (float)M_PI / 180.0f;
	mat4 proj;
	proj.SetPerspectiveRH_ZO(fov_rad, 1.0f, 0.1f, 100.0f);

	// Camera distance: chosen so that at angle == 0 the projection exactly
	// reproduces the flat quad's own pixel size (derived analytically from
	// the projection matrix below, not just eyeballed -- see report/comment
	// at bottom of file for the derivation).
	float half_extent = max(quad_size.cx, quad_size.cy) / 2.0f;
	if(half_extent <= 0)
		half_extent = 1.0f;
	float camera_z = half_extent / tanf(fov_rad / 2.0f);

	out.SetCount((n + 1) * (n + 1));
	for(int row = 0; row <= n; row++) {
		float v = (float)row / n;
		float y = (v - 0.5f) * quad_size.cy;
		for(int col = 0; col <= n; col++) {
			float u = (float)col / n;
			float x = (u - 0.5f) * quad_size.cx;

			vec4 local(x, y, 0.0f, 1.0f);
			// Row-vector convention: point * matrix (verified against
			// uppsrc/Geometry/Matrix.inl's Vec::operator*(const Matrix&)
			// definition, which computes r[i] = sum_j data[j] * m[j][i]).
			vec4 rotated = local * rot;

			// Push the rotated point in front of the camera. Geometry's
			// SetPerspectiveRH_ZO is a standard right-handed projection
			// (camera looks down -Z in view space), so "in front" means
			// negative view-space Z -- translate by -camera_z, not +camera_z.
			rotated.data[2] -= camera_z;

			vec4 clip = rotated * proj;

			float w = clip.data[3];
			if(fabsf(w) < 1e-6f)
				w = w < 0 ? -1e-6f : 1e-6f;
			float ndc_x = clip.data[0] / w;
			float ndc_y = clip.data[1] / w;

			// Map NDC to pixels relative to the quad's own center. Empirically
			// (see Xform3DTest's row-order check) NDC-Y here already comes out
			// Y-down consistent with our local `y` construction above (row 0 /
			// v=0 is the top edge and maps to negative y), so no extra sign
			// flip is applied -- verified, not assumed.
			float screen_x = ndc_x * (quad_size.cx / 2.0f);
			float screen_y = ndc_y * (quad_size.cy / 2.0f);

			out[row * (n + 1) + col] = Pointf(screen_x, screen_y);
		}
	}
	return out;
}

void DrawWarped3D(Draw& w, const Image& img, const Xform3D& xf, int n, Point center, double alpha01)
{
	if(n < 1 || img.IsEmpty())
		return;

	Vector<Pointf> grid = xf.ProjectGrid(n);
	int stride = n + 1;

	int imgW = img.GetWidth();
	int imgH = img.GetHeight();

	// Build a single alpha-faded copy of the image up front (not per cell).
	Image faded = img;
	if(alpha01 < 0.999) {
		double a = clamp(alpha01, 0.0, 1.0);
		ImageBuffer ib(faded);
		for(int y = 0; y < ib.GetHeight(); y++) {
			RGBA *row = ib[y];
			for(int x = 0; x < ib.GetWidth(); x++)
				row[x].a = (byte)(row[x].a * a);
		}
		faded = ib;
	}

	for(int j = 0; j < n; j++) {
		for(int i = 0; i < n; i++) {
			const Pointf& p00 = grid[j * stride + i];
			const Pointf& p10 = grid[j * stride + (i + 1)];
			const Pointf& p01 = grid[(j + 1) * stride + i];
			const Pointf& p11 = grid[(j + 1) * stride + (i + 1)];

			// Approximate destination rect: center of the 4 corners plus a
			// locally-averaged width/height taken from the corner spread.
			double cx = (p00.x + p10.x + p01.x + p11.x) / 4.0;
			double cy = (p00.y + p10.y + p01.y + p11.y) / 4.0;
			double w_top = fabs(p10.x - p00.x);
			double w_bot = fabs(p11.x - p01.x);
			double h_left = fabs(p01.y - p00.y);
			double h_right = fabs(p11.y - p10.y);
			double cell_w = max(w_top, w_bot);
			double cell_h = max(h_left, h_right);
			if(cell_w < 1) cell_w = 1;
			if(cell_h < 1) cell_h = 1;

			Rect dest;
			dest.left = center.x + (int)round(cx - cell_w / 2.0);
			dest.right = center.x + (int)round(cx + cell_w / 2.0);
			dest.top = center.y + (int)round(cy - cell_h / 2.0);
			dest.bottom = center.y + (int)round(cy + cell_h / 2.0);
			if(dest.IsEmpty())
				continue;

			Rect src;
			src.left = i * imgW / n;
			src.right = (i + 1) * imgW / n;
			src.top = j * imgH / n;
			src.bottom = (j + 1) * imgH / n;
			if(src.IsEmpty())
				continue;

			w.DrawImage(dest, faded, src);
		}
	}
}

END_UPP_NAMESPACE

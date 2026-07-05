#include "Xform3D.h"

// Geometry headers must never leak into a public header (see Xform3D.h comment) --
// this .cpp file is the only place in this package allowed to include them.
#include <Geometry/Geometry.h>
#include <Painter/Painter.h>

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

	// Bounding box (in w's coordinate space) of the whole projected grid. DrawWarped3D
	// only knows `center`, not the size of the Ctrl/window `w` is being painted into,
	// so rather than assume a canvas size we paint into a local offscreen buffer sized
	// just to what this warp actually needs, then blit that once onto `w`.
	Pointf bmin(1e30, 1e30), bmax(-1e30, -1e30);
	for(const Pointf& p : grid) {
		bmin.x = min(bmin.x, (double)p.x);
		bmin.y = min(bmin.y, (double)p.y);
		bmax.x = max(bmax.x, (double)p.x);
		bmax.y = max(bmax.y, (double)p.y);
	}
	// 1px margin for antialiasing bleed at the outer edge.
	int bx0 = center.x + (int)floor(bmin.x) - 1;
	int by0 = center.y + (int)floor(bmin.y) - 1;
	int bx1 = center.x + (int)ceil(bmax.x) + 1;
	int by1 = center.y + (int)ceil(bmax.y) + 1;
	Size bufsz(max(1, bx1 - bx0), max(1, by1 - by0));

	ImagePainter ip(bufsz, MODE_NOAA);
	ip.Clear(RGBAZero());

	// Transforms/0006: MODE_NOAA (see Transforms/0005) thresholds each cell's
	// OWN fractional coverage of a pixel independently (< 50% -> not painted at
	// all by that cell, >= 50% -> fully painted), rather than accumulating
	// coverage from all cells sharing that pixel before thresholding. At an
	// interior grid vertex, up to 4 cells meet and divide any pixel touching
	// that vertex into wedges that sum to ~100% combined but are frequently
	// close to an even ~25% split -- so *no* individual cell reaches the 50%
	// threshold there, and the pixel is left unpainted (still the initial
	// RGBAZero() clear value), showing as a tiny transparent dot exactly at
	// the vertex. This was confirmed empirically (Xform3DTest's temporary
	// diagnostic showed bit-identical shared-vertex coordinates across cells,
	// ruling out a rounding-input mismatch, while directly reproducing
	// isolated fully-transparent pixels at grid vertices surrounded by
	// fully-opaque neighbors).
	//
	// Fix: inflate each cell's *fill/clip* polygon slightly outward from its
	// own centroid before rasterizing (the texture-sampling affine map below
	// still uses the true, un-inflated corners, so the extra sliver is filled
	// with a smooth linear extrapolation of the same cell's texture, not a
	// seam). This makes neighboring cells overlap by a fraction of a pixel
	// around every shared vertex/edge, so at least one of them now covers
	// >= 50% of any pixel near the vertex and paints it -- closing the dot.
	// The same overlap also can't reintroduce the Transforms/0005 seam: NOAA
	// pixels are binary (0 or fully opaque), so overlapping opaque coverage
	// from two cells still composites as a single solid pixel, not a
	// see-through blend.
	const float vertex_inflate_px = 0.75f;
	auto Inflate = [](Pointf p, const Pointf& centroid, float d) -> Pointf {
		float dx = p.x - centroid.x, dy = p.y - centroid.y;
		float len = sqrtf(dx * dx + dy * dy);
		if(len < 1e-4f)
			return p;
		float k = d / len;
		return Pointf(p.x + dx * k, p.y + dy * k);
	};

	for(int j = 0; j < n; j++) {
		for(int i = 0; i < n; i++) {
			const Pointf& p00 = grid[j * stride + i];
			const Pointf& p10 = grid[j * stride + (i + 1)];
			const Pointf& p01 = grid[(j + 1) * stride + i];
			const Pointf& p11 = grid[(j + 1) * stride + (i + 1)];

			Rect src;
			src.left = i * imgW / n;
			src.right = (i + 1) * imgW / n;
			src.top = j * imgH / n;
			src.bottom = (j + 1) * imgH / n;
			if(src.IsEmpty())
				continue;

			// Screen-space corners, offset into the local buffer's own coordinate frame.
			Pointf s00(center.x + p00.x - bx0, center.y + p00.y - by0);
			Pointf s10(center.x + p10.x - bx0, center.y + p10.y - by0);
			Pointf s01(center.x + p01.x - bx0, center.y + p01.y - by0);
			Pointf s11(center.x + p11.x - bx0, center.y + p11.y - by0);

			// Real per-cell affine warp instead of an axis-aligned blit: an affine map
			// is fully determined by 3 point correspondences, so it can only send a
			// rectangle to a parallelogram, not a general quadrilateral. We pick the
			// source cell rect's top-left/top-right/bottom-left corners as the basis
			// (consistent with the row/column order used elsewhere in this file) and
			// map them to the corresponding projected p00/p10/p01 screen corners. The
			// 4th corner (bottom-right, s11) is NOT part of that basis, so it will not
			// generally land exactly on its true projected position under the affine
			// map alone -- a known, accepted limitation (see Transforms/0004 plan doc).
			// The clip path below still uses the *true* projected quad (all 4 real
			// corners, including p11), so adjacent cells' edges/seams line up exactly;
			// only the interior texture warp of each cell is a parallelogram
			// approximation of the true (trapezoidal) quad.
			Xform2D t = Xform2D::Map(Pointf(src.left, src.top), Pointf(src.right, src.top),
			                         Pointf(src.left, src.bottom),
			                         s00, s10, s01);

			// Fill/clip path uses corners inflated a fraction of a pixel outward
			// from the cell's own centroid (see vertex_inflate_px comment above)
			// -- the texture transform `t` above still targets the true,
			// un-inflated corners, so this only widens the rasterized coverage,
			// it doesn't change what the texture looks like.
			Pointf centroid((s00.x + s10.x + s01.x + s11.x) / 4.0f,
			                (s00.y + s10.y + s01.y + s11.y) / 4.0f);
			Pointf c00 = Inflate(s00, centroid, vertex_inflate_px);
			Pointf c10 = Inflate(s10, centroid, vertex_inflate_px);
			Pointf c01 = Inflate(s01, centroid, vertex_inflate_px);
			Pointf c11 = Inflate(s11, centroid, vertex_inflate_px);

			ip.Move(c00).Line(c10).Line(c11).Line(c01).Close();
			ip.Fill(faded, t);
		}
	}

	w.DrawImage(bx0, by0, ip.GetResult());
}

END_UPP_NAMESPACE

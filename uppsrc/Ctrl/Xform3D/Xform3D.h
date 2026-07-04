#ifndef _Ctrl_Xform3D_Xform3D_h_
#define _Ctrl_Xform3D_Xform3D_h_

#include <CtrlCore/CtrlCore.h>

NAMESPACE_UPP

// Genuine 3D perspective rotate+project helper for a flat rectangular image.
//
// This wraps the real linear-algebra machinery in uppsrc/Geometry (Matrix<T,4,4>,
// Vec<T,4>, rotation and perspective-projection matrix construction) behind a
// pImpl, so that Geometry's heavy headers are never pulled into this public
// header's include graph -- only Xform3D.cpp includes <Geometry/Geometry.h>.
class Xform3D {
	struct Impl;
	One<Impl> impl;

public:
	Xform3D();
	~Xform3D();

	// Configures the rotation as two composed hinge rotations: `angle_x_rad` is
	// the horizontal-hinge (top/bottom turn) component, `angle_y_rad` is the
	// vertical-hinge (left/right turn) component. Each is in [0, pi/2] where 0 =
	// fully face-on (no rotation) and pi/2 = fully edge-on; passing 0 for one of
	// them reproduces a pure single-axis turn around the other. `quad_size` is
	// the flat rectangle's size in pixels, e.g. the card image's on-screen size
	// before any rotation.
	void Set(double angle_x_rad, double angle_y_rad, Size quad_size);

	// Projects an (n+1) x (n+1) grid of points evenly covering the quad
	// (from one edge to the opposite edge in both directions) through the
	// last Set() call's rotation and a fixed perspective camera, returning
	// 2D points in "pixels relative to the quad's own center" (i.e. at
	// angle_rad == 0, the returned points exactly reproduce a flat,
	// undistorted (n+1) x (n+1) grid spanning -quad_size/2 .. +quad_size/2).
	// Row-major order: index = row * (n+1) + col, row 0 = top, col 0 = left.
	Vector<Pointf> ProjectGrid(int n) const;
};

// Draws `img` warped through `xf`'s current Set() configuration, centered
// at `center` (screen coordinates), by subdividing into an n x n grid of
// cells and filling each cell with a locally-affine image map (so the
// overall result approximates the true perspective warp). `alpha01`
// (0..1) fades the whole image by multiplying its existing alpha channel.
void DrawWarped3D(Draw& w, const Image& img, const Xform3D& xf, int n, Point center, double alpha01 = 1.0);

END_UPP_NAMESPACE

#endif

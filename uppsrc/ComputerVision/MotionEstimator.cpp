#include "ComputerVision.h"

NAMESPACE_UPP


bool HaveCollinearPoints(Vector<Keypoint>& points, int count) {
	int i = (count - 1);
	double dx1 = 0.0, dy1 = 0.0, dx2 = 0.0, dy2 = 0.0;
	
	// check that the i-th selected point does not belong
	// to a line connecting some previously selected points
	for (int j = 0; j < i; ++j) {
		dx1 = points[j].x - points[i].x;
		dy1 = points[j].y - points[i].y;
		for (int k = 0; k < j; ++k) {
			dx2 = points[k].x - points[i].x;
			dy2 = points[k].y - points[i].y;
			if (abs(dx2*dy1 - dy2*dx1) <= EPSILON*(abs(dx1) + abs(dy1) + abs(dx2) + abs(dy2)))
				return true;
		}
	}
	return false;
}


Affine2D::Affine2D() :
	T0(3, 3, 1),
	T1(3, 3, 1),
	AtA(6, 6, 1),
	AtB(6, 1, 1)
{}

bool Affine2D::Run(const Vector<Keypoint>& from, Vector<Keypoint>& to, FloatMat& model) {
	int count = from.GetCount();
	
	int dt = 1;
	auto& md = model.data;
	auto&t0d = T0.data;
	auto&t1d = T1.data;
	
	IsoNormalizePoints(from, to, t0d, t1d, count);
	
	a_mt.SetSize(6, 2*count, 1);
	b_mt.SetSize(1, 2*count, 1);
	
	auto& ad = a_mt.data;
	auto&bd = b_mt.data;
	
	for (int i = 0; i < count; ++i) {
		const Keypoint& pt0 = from[i];
		const Keypoint& pt1 = to[i];
		
		double px = t0d[0] * pt0.x + t0d[1] * pt0.y + t0d[2];
		double py = t0d[3] * pt0.x + t0d[4] * pt0.y + t0d[5];
		
		int j = i * 2 * 6;
		ad[j] = px;
		ad[j+1] = py;
		ad[j+2] = 1.0;
		ad[j+3] = 0.0;
		ad[j+4] = 0.0;
		ad[j+5] = 0.0;
		
		j += 6;
		ad[j] = 0.0;
		ad[j+1] = 0.0;
		ad[j+2] = 0.0;
		ad[j+3] = px;
		ad[j+4] = py;
		ad[j+5] = 1.0;
		
		bd[i<<1] = t1d[0] * pt1.x + t1d[1] * pt1.y + t1d[2];
		bd[(i<<1)+1] = t1d[3] * pt1.x + t1d[4] * pt1.y + t1d[5];
	}
	
	MultiplyAtA(AtA, a_mt);
	MultiplyAtB(AtB, a_mt, b_mt);
	
	LUSolve(AtA, AtB);
	
	md[0] = AtB.data[0], md[1] = AtB.data[1], md[2] = AtB.data[2];
	md[3] = AtB.data[3], md[4] = AtB.data[4], md[5] = AtB.data[5];
	md[6] = 0.0, md[7] = 0.0, md[8] = 1.0; // fill last row
	
	// denormalize
	Invert3x3(T1, T1);
	Multiply3x3(model, T1, model);
	Multiply3x3(model, model, T0);
	
	return true;
}

void Affine2D::Error(const Vector<Keypoint>& from, const Vector<Keypoint>& to, const FloatMat& model, Vector<double>& err) {
	int count = from.GetCount();
	const auto& m = model.data;
	
	auto f = from.Begin();
	auto t = to.Begin();
	for (auto& e : err) {
		const Keypoint& pt0 = *f;
		const Keypoint& pt1 = *t;
		
		e = Square(pt1.x - m[0] * pt0.x - m[1] * pt0.y - m[2]) +
		    Square(pt1.y - m[3] * pt0.x - m[4] * pt0.y - m[5]);
		
		f++;
		t++;
	}
}

bool Affine2D::CheckSubset(const Vector<Keypoint>& from, const Vector<Keypoint>& to, int count) {
	return true; // all good
}


Homography2D::Homography2D() :
	mLtL(9, 9, 1),
	Evec(9, 9, 1)
{
	// empty constructor
	T0.SetSize(3, 3, 1);
	T1.SetSize(3, 3, 1);
	mLtL.SetSize(9, 9, 1);
	Evec.SetSize(9, 9, 1);
}

void Homography2D::Error(const Vector<Keypoint>& from, const Vector<Keypoint>& to, const FloatMat& model, Vector<double>& err) {
	const auto& m = model.data;
	
	auto f = from.Begin();
	auto t = to.Begin();
	ASSERT(from.GetCount() == to.GetCount());
	err.SetCount(from.GetCount());
	for (auto& e : err) {
		const Keypoint& pt0 = *f;
		const Keypoint& pt1 = *t;
		
		double ww = 1.0 / (m[6] * pt0.x + m[7] * pt0.y + 1.0);
		double dx = (m[0] * pt0.x + m[1] * pt0.y + m[2]) * ww - pt1.x;
		double dy = (m[3] * pt0.x + m[4] * pt0.y + m[5]) * ww - pt1.y;
		e = (dx * dx + dy * dy);
		
		f++;
		t++;
	}
}

bool Homography2D::CheckSubset(const Vector<Keypoint>& from, Vector<Keypoint>& to, int count) {
	return true; // all good
}

bool Homography2D::Run(const Vector<Keypoint>& from, Vector<Keypoint>& to, FloatMat& model) {
	auto& md = model.data;
	auto& t0d = T0.data;
	auto& t1d = T1.data;
	auto& LtL = mLtL.data;
	
	int count = from.GetCount();
	
	double x = 0.0, y = 0.0, X = 0.0, Y = 0.0;
	
	// norm
	double smx = 0.0, smy = 0.0, cmx = 0.0, cmy = 0.0, sMx = 0.0, sMy = 0.0, cMx = 0.0, cMy = 0.0;
	
	for (int i = 0; i < count; ++i) {
		cmx += to[i].x;
		cmy += to[i].y;
		cMx += from[i].x;
		cMy += from[i].y;
	}
	
	cmx /= count;
	cmy /= count;
	cMx /= count;
	cMy /= count;
	
	for (int i = 0; i < count; ++i) {
		smx += abs(to[i].x - cmx);
		smy += abs(to[i].y - cmy);
		sMx += abs(from[i].x - cMx);
		sMy += abs(from[i].y - cMy);
	}
	
	if (   abs(smx) < EPSILON
		|| abs(smy) < EPSILON
		|| abs(sMx) < EPSILON
		|| abs(sMy) < EPSILON)
		return 0;
		
	smx = count / smx;
	smy = count / smy;
	sMx = count / sMx;
	sMy = count / sMy;
	
	t0d[0] = sMx;
	t0d[1] = 0;
	t0d[2] = -cMx * sMx;
	t0d[3] = 0;
	t0d[4] = sMy;
	t0d[5] = -cMy * sMy;
	t0d[6] = 0;
	t0d[7] = 0;
	t0d[8] = 1;
	
	t1d[0] = 1.0 / smx;
	t1d[1] = 0;
	t1d[2] = cmx;
	t1d[3] = 0;
	t1d[4] = 1.0 / smy;
	t1d[5] = cmy;
	t1d[6] = 0;
	t1d[7] = 0;
	t1d[8] = 1;
	//
	
	// construct system
	double mLtL_db[81];
	for (int i = 0; i < 81; i++) mLtL_db[i] = 0.0;

	for (int i = 0; i < count; ++i) {
		x = (to[i].x - cmx) * smx;
		y = (to[i].y - cmy) * smy;
		X = (from[i].x - cMx) * sMx;
		Y = (from[i].y - cMy) * sMy;
		
		double r[2][9] = {
			{-X, -Y, -1,  0,  0,  0, x*X, x*Y, x},
			{ 0,  0,  0, -X, -Y, -1, y*X, y*Y, y}
		};
		
		for (int k = 0; k < 9; k++) {
			for (int l = 0; l < 9; l++) {
				mLtL_db[k * 9 + l] += r[0][k] * r[0][l] + r[1][k] * r[1][l];
			}
		}
	}
	for (int i = 0; i < 81; i++) LtL[i] = (float)mLtL_db[i];
	//
	
	EigenVV(mLtL, &Evec);
	
	// The smallest eigenvalue is at the end (index 8) because EigenVV sorts them descending.
	// The corresponding eigenvector is the 9th column (index 8) of Evec.
	for (int i = 0; i < 9; ++i) {
		md[i] = Evec.data[i * 9 + 8];
	}
	
	// denormalize
	Multiply3x3(model, T1, model);
	Multiply3x3(model, model, T0);
	
	// set bottom right to 1.0
	x = 1.0 / md[8];
	md[0] *= x;
	md[1] *= x;
	md[2] *= x;
	md[3] *= x;
	md[4] *= x;
	md[5] *= x;
	md[6] *= x;
	md[7] *= x;
	md[8] = 1.0;
	
	return true;
}


END_UPP_NAMESPACE

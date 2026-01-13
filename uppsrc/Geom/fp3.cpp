#include "Geom.h"

namespace Upp {

Pointf3 Orthogonal(Pointf3 v, Pointf3 target)
{
	return v - target * (v ^ target) / Squared(target);
}

Pointf3 Orthonormal(Pointf3 v, Pointf3 target)
{
	Pointf3 o = Orthogonal(v, target);
	double l = Length(o);
	return l > 1e-100 ? o / l : o;
}

Pointf3 FarthestAxis(Pointf3 v)
{
	double ax = fabs(v.x), ay = fabs(v.y), az = fabs(v.z);
	if(ax < ay)
		return (ax < az ? Pointf3(1, 0, 0) : Pointf3(0, 0, 1));
	else
		return (ay < az ? Pointf3(0, 1, 0) : Pointf3(0, 0, 1));
}

//////////////////////////////////////////////////////////////////////
// Plane3::

Plane3::Plane3(const Nuller& n)
{
	delta = Null;
}

Plane3::Plane3(double d, Pointf3 n)
{
	delta = d;
	normal = n;
}

Plane3::Plane3(Pointf3 p, Pointf3 n)
{
	normal = UnitV(n);
	delta = -(p ^ normal);
}

Plane3::Plane3(Pointf3 p1, Pointf3 p2, Pointf3 p3)
{
	normal = UnitV((p2 - p1) % (p3 - p1));
	delta = -(p1 ^ normal);
}

void Plane3::Serialize(Stream& stream)
{
	stream % delta % normal;
}

Plane3 operator * (Plane3 p, const Matrixf3& m)
{
	Pointf3 p1 = Pointf3(-p.delta, 0, 0) * m;
	Pointf3 p2 = (Pointf3(-p.delta, 0, 0) + Orthonormal(FarthestAxis(p.normal), p.normal)) * m;
	Pointf3 p3 = (Pointf3(-p.delta, 0, 0) + (p2 - p1) % p.normal) * m;
	return Plane3(p1, p2, p3);
}

Plane3 UnitV(Plane3 p)
{
	double nt = Length(p.normal);
	if(nt > 1e-100)
		return Plane3(p.delta / nt, p.normal / nt);
	return p;
}

double Intersect(Plane3 p, Pointf3 la, Pointf3 lb)
{
	double an = la ^ p.normal, bn = lb ^ p.normal;
	if(an == bn)
		return 0;
	return (an + p.delta) / (an - bn);
}

//////////////////////////////////////////////////////////////////////
// Box3::

void Box3::Union(Pointf3 pt)
{
	double c0 = pt ^ normal[0], c1 = pt ^ normal[1], c2 = pt ^ normal[2];
	if(c0 < low[0]) low[0] = c0;
	if(c0 > high[0]) high[0] = c0;
	if(c1 < low[1]) low[1] = c0;
	if(c1 > high[1]) high[1] = c0;
	if(c2 < low[2]) low[2] = c0;
	if(c2 > high[2]) high[2] = c0;
}

//////////////////////////////////////////////////////////////////////
// Matrix3::

#define GLOBAL_VARP_INIT(a, b, c) a & b() { static a m c; return m; }

GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_0, (0, 0, 0, 0, 0, 0, 0, 0, 0))
GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_1, (1, 0, 0, 0, 1, 0, 0, 0, 1))
GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_Null, (0, 0, 0, 0, 0, 0, 0, 0, 0, Null, Null, Null))
GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_MirrorX, (-1, 0, 0, 0, 1, 0, 0, 0, 1))
GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_MirrorY, (1, 0, 0, 0, -1, 0, 0, 0, 1))
GLOBAL_VARP_INIT(const Matrixf3, Matrixf3_MirrorZ, (1, 0, 0, 0, 1, 0, 0, 0, -1))

void Matrixf3::Serialize(Stream& stream)
{
	stream % x % y % z % a;
}

Matrixf3& Matrixf3::operator *= (const Matrixf3& m)
{
	Pointf3 nx(x ^ m.CX(), x ^ m.CY(), x ^ m.CZ());
	Pointf3 ny(y ^ m.CX(), y ^ m.CY(), y ^ m.CZ());
	Pointf3 nz(z ^ m.CX(), z ^ m.CY(), z ^ m.CZ());
	x = nx;
	y = ny;
	z = nz;
	a *= m;
	return *this;
}

String Matrixf3::ToString() const
{
	return String().Cat() << "[x = " << x << ", y = " << y << ", z = " << z << ", a = " << a << "]";
}

Matrixf3 Matrixf3Move(Pointf3 vector)
{
	return Matrixf3(Pointf3(1, 0, 0), Pointf3(0, 1, 0), Pointf3(0, 0, 1), vector);
}

Matrixf3 Matrixf3RotateX(double angle, Pointf3 fix)
{
	double c = cos(angle), s = sin(angle);
	return Matrixf3(1, 0, 0, 0, c, s, 0, -s, c).Fix(fix);
}

Matrixf3 Matrixf3RotateY(double angle, Pointf3 fix)
{
	double c = cos(angle), s = sin(angle);
	return Matrixf3(c, 0, s, 0, 1, 0, -s, 0, c).Fix(fix);
}

Matrixf3 Matrixf3RotateZ(double angle, Pointf3 fix)
{
	double c = cos(angle), s = sin(angle);
	return Matrixf3(c, s, 0, -s, c, 0, 0, 0, 1).Fix(fix);
}

Matrixf3 Matrixf3Scale(double scale, Pointf3 fix)
{
	return Matrixf3(Pointf3(scale, 0, 0), Pointf3(0, scale, 0),
		Pointf3(0, 0, scale), fix * (1 - scale));
}

Matrixf3 Matrixf3Scale(Pointf3 scale, Pointf3 fix)
{
	return Matrixf3(Pointf3(scale.x, 0, 0), Pointf3(0, scale.y, 0),
		Pointf3(0, 0, scale.z), fix - scale * fix);
}

Matrixf3 Matrixf3Inverse(const Matrixf3& mx)
{
	double det = Determinant(mx);
	Matrixf3 m(
		Pointf3(
			(mx.y.y * mx.z.z - mx.z.y * mx.y.z),
			(mx.z.y * mx.x.z - mx.x.y * mx.z.z),
			(mx.x.y * mx.y.z - mx.y.y * mx.x.z)) / det,
		Pointf3(
			(mx.y.z * mx.z.x - mx.z.z * mx.y.x),
			(mx.z.z * mx.x.x - mx.x.z * mx.z.x),
			(mx.x.z * mx.y.x - mx.y.z * mx.x.x)) / det,
		Pointf3(
			(mx.y.x * mx.z.y - mx.z.x * mx.y.y),
			(mx.z.x * mx.x.y - mx.x.x * mx.z.y),
			(mx.x.x * mx.y.y - mx.y.x * mx.x.y)) / det);
	m.a = -mx.a % m;
	return m;
}

Matrixf3 Matrixf3Affine(Pointf3 src1, Pointf3 dest1, Pointf3 src2, Pointf3 dest2)
{
	return Matrixf3Affine(src1, dest1, src2, dest2, src1 + FarthestAxis(src2 - src1),
		dest1 + FarthestAxis(dest2 - dest1));
}

Matrixf3 Matrixf3Affine(Pointf3 src1, Pointf3 dest1, Pointf3 src2, Pointf3 dest2,
	Pointf3 src3, Pointf3 dest3)
{
	return Matrixf3Affine(src1, dest1, src2, dest2, src3, dest3,
		src1 + (src2 - src1) % (src3 - src1),
		dest1 + (dest2 - dest1) % (dest3 - dest1));
}

Matrixf3 Matrixf3Affine(Pointf3 src1, Pointf3 dest1, Pointf3 src2, Pointf3 dest2,
	Pointf3 src3, Pointf3 dest3, Pointf3 src4, Pointf3 dest4)
{
	Matrixf3 rev(src2 - src1, src3 - src1, src4 - src1, src1);
	if(fabs(Determinant(rev)) <= 1e-100)
		return Matrixf3Move((dest1 - src1 + dest2 - src2 + dest3 - src3 + dest4 - src4) / 4.0);
	return Matrixf3Inverse(rev) * Matrixf3(dest2 - dest1, dest3 - dest1, dest4 - dest1, dest1);
}

double Determinant(const Matrixf3& mx)
{
	return mx.x.x * mx.y.y * mx.z.z
		+  mx.y.x * mx.z.y * mx.x.z
		+  mx.z.x * mx.x.y * mx.y.z
		-  mx.x.z * mx.y.y * mx.z.x
		-  mx.y.z * mx.z.y * mx.x.x
		-  mx.z.z * mx.x.y * mx.y.x;
}

double Matrixf3Measure(const Matrixf3& mx)
{
	double d = Determinant(mx);
	return (d >= 0 ? +1 : -1) * pow(fabs(d), 1.0 / 3.0);
}

Matrixf3 Matrixf3Rotate(Pointf3 axis, double angle)
{
	Matrixf3 to_space = Matrixf3_1();
	to_space.z = UnitV(axis);
	to_space.y = Orthonormal(FarthestAxis(to_space.z), to_space.z);
	to_space.x = to_space.y % to_space.z;
	double ca = cos(angle), sa = sin(angle);
	return Matrixf3Inverse(to_space) * Matrixf3(ca, -sa, 0, sa, ca, 0, 0, 0, 1) * to_space;
}

//////////////////////////////////////////////////////////////////////
// Camera3::

Camera3::Camera3()
{
	ViewingAngle(120 * DEGRAD); // default viewing angle in radians
	Location(Pointf3(-100, 100, 0)); // 150 m from origin in the -x+y distance
	Target(Pointf3(0, 0, 0));
	Upwards(Pointf3(0, 1, 0));
	FarDistance(10000); // 10 km
	NearDistance(100); // 100 m
	Viewport(320, 240);
	ViewportOffset(0, 0);
	ViewportSize(1, 1);
	Projection(Matrixf3_1());
	Update();
}

void Camera3::Serialize(Stream& stream)
{
	int version = StreamHeading(stream, 1, 1, 1, "Camera");
	if(version >= 1)
	{
		stream % location % target % upwards
			% viewing_angle % width_mm % height_mm
			% shift_x % shift_y % near_distance % far_distance
			% projection_matrix;
	}
}

void Camera3::Update()
{
	direction = UnitV(target - location);
	distance_delta = -(location ^ direction);
	viewing_distance = double(min(width_mm, height_mm) / (2e3 * tan(viewing_angle / 2.0)));
	Pointf3 up = Orthogonal(upwards, direction);
	if(Squared(up) <= 1e-10)
		up = Orthogonal(FarthestAxis(direction), direction);
	straight_up = UnitV(up);
	straight_right = direction % straight_up;
	camera_matrix = Matrixf3(straight_right, straight_up, direction, location);
	invcam_matrix = Matrixf3Inverse(camera_matrix);
	double dw = viewing_distance * view_px * 2e3 / width_mm;
	double dh = viewing_distance * view_py * 2e3 / height_mm;
	double z_times = far_distance - near_distance;
	z_delta = -near_distance / z_times;
	transform_matrix = invcam_matrix * Matrixf3(
		Pointf3(dw / z_times, 0, 0),
		Pointf3(0, dh / z_times, 0),
		Pointf3(shift_x, shift_y, 1) / z_times);
}

Pointf3 Camera3::Transform(Pointf3 point) const
{
	Pointf3 axon = point * transform_matrix;
	static const double MIN_DISTANCE = 1e-3; // 1 mm should be good enough
	if(axon.z >= -MIN_DISTANCE && axon.z <= +MIN_DISTANCE)
		axon.z = MIN_DISTANCE;
	return Pointf3(axon.x / axon.z, axon.y / axon.z, axon.z + z_delta);
}

void Camera3::SetPolar(Pointf3 dz, Pointf3 dx, double d, double a, double b, double r)
{
	Pointf3 dy = dz % dx;
	Pointf3 az = RotateX(Pointf3(0, 0, -1), -b);
	az = d * RotateY(az, a);
	Pointf3 loc = az * Matrixf3(dx, dy, dz, target);
	Pointf3 dir = UnitV(target - loc);
	Pointf3 su = Orthogonal(dy, dir);
	if(Length(su) <= 1e-10)
		su = Orthogonal(FarthestAxis(dir), dir);
	su = UnitV(su);
	Location(loc).Upwards(Rotate(su, dir, r));
}

void Camera3::GetPolar(Pointf3 dz, Pointf3 dx,
double& distance, double& azimuth, double& bearing, double& rotation) const
{
	distance = Distance(location, target);
	Pointf3 dy = dz % dx;
	Pointf3 back = direction * Matrixf3Inverse(Matrixf3(dx, dy, dz));
	azimuth = GetZXBearing(back);
	bearing = asin(back.y);
	double r = Determinant(straight_up, dy, direction);
	rotation = asin(2 * r) * 0.5;
}

bool Camera3::TransformClip(Pointf3 a, Pointf3 b, Pointf& outa, Pointf& outb) const
{
	Pointf3 a1 = a * transform_matrix, b1 = b * transform_matrix;
	double az = a1.z + z_delta, bz = b1.z + z_delta;
	bool na = (az <= 0), nb = (bz <= 0);
	bool fa = (az >= 1), fb = (bz >= 1);
	if(na && nb || fa && fb)
		return false;
	/**/ if(na) a1 += (b1 - a1) * (0 - az) / (b1.z - a1.z);
	else if(nb) b1 += (a1 - b1) * (0 - bz) / (a1.z - b1.z);
	/**/ if(fa) a1 += (b1 - a1) * (1 - az) / (b1.z - a1.z);
	else if(fb) b1 += (a1 - b1) * (1 - bz) / (a1.z - b1.z);
	outa.x = a1.x / a1.z;
	outa.y = a1.y / a1.z;
	outb.x = b1.x / b1.z;
	outb.y = b1.y / b1.z;
	return ClipLine(outa, outb, Rectf(-view_px, -view_py, +view_px, +view_py));
}

Pointf3 Camera3::InverseXY(Pointf point, double z) const
{
	double dax = point.x * transform_matrix.x.z - transform_matrix.x.x;
	double day = point.x * transform_matrix.y.z - transform_matrix.y.x;
	double dar = z * transform_matrix.z.x + transform_matrix.a.x - point.x * (z * transform_matrix.z.z + transform_matrix.a.z);
	double dbx = point.y * transform_matrix.x.z - transform_matrix.x.y;
	double dby = point.y * transform_matrix.y.z - transform_matrix.y.y;
	double dbr = z * transform_matrix.z.y + transform_matrix.a.y - point.y * (z * transform_matrix.z.z + transform_matrix.a.z);
	double dx = dar * dby - dbr * day;
	double dy = dax * dbr - dar * dbx;
	double dr = dax * dby - dbx * day;
	double lim = fabs(dr) * 1e6;
	if(lim <= fabs(dx) || lim <= fabs(dy))
		return Null;
	return Pointf3(dx / dr, dy / dr, z);
}

}
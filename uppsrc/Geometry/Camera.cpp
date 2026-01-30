#include "Geometry.h"

NAMESPACE_UPP


Camera::Camera() {
	
}

mat4 Camera::GetWorldMatrix() {
	return this->world;
}

mat4 Camera::GetViewportMatrix() {
	float ratio = (float)height / (float)width;
	return GetViewport(-1 * ratio, -1, 2 * ratio, 2, 1);
}

mat4 Camera::GetViewMatrix() {
	return proj * world;
}

float Camera::GetAspect() {
	return aspect;
}

mat4 Camera::GetProjectionMatrix() {
	return proj;
}

void Camera::SetResolution(int width, int height) {
	this->width = (float)width;
	this->height = (float)height;
	
	UpdateMatrices();
}

void Camera::SetResolution(Size sz) {
	width = (float)sz.cx;
	height = (float)sz.cy;
	
	UpdateMatrices();
}

void Camera::UpdateMatrices() {
	this->aspect = (float)width / (float)height;

	if (this->proj_mode == 0) {
		this->proj = Perspective(DEG2RADf(this->fov * 0.5f), this->aspect, this->near, this->far);
	}
	else if (this->proj_mode == 1) {

		float halfW = this->width * 0.5f;
		float halfH = this->height * 0.5f;

		this->proj = Ortho(-halfW, halfW, halfH, -halfH, this->near, this->far);
	}
	// this->proj_mode == 2
		// User defined
}

bool Camera::IsOrthographic() {
	return this->proj_mode == 1;
}

bool Camera::IsPerspective() {
	return this->proj_mode == 0;
}

void Camera::SetPerspective(float fov_angle, float aspect, float zNear, float zFar) {
	this->fov = fov_angle;
	this->aspect = aspect;
	this->near = zNear;
	this->far = zFar;
	
	this->proj = Perspective(DEG2RADf(fov_angle * 0.5f), aspect, zNear, zFar);
	this->proj_mode = 0;
}

void Camera::SetOrthographic(float width, float height, float zNear, float zFar) {
	this->width = width;
	this->height = height;
	this->near = zNear;
	this->far = zFar;

	float halfW = width * 0.5f;
	float halfH = height * 0.5f;

	this->proj =
		Ortho(-halfW, halfW, halfH, -halfH, zNear, zFar)
		* Scale(vec3(+1,-1,+1));
	this->proj_mode = 1;
}

void Camera::SetProjection(const mat4& projection) {
	this->proj = projection;
	this->proj_mode = 2;
}

void Camera::SetWorld(const mat4& world) {
	this->world = world;
}

void Camera::SetWorld(const vec3& position, const quat& orient) {
	world = (Translate(position) * QuatMat(orient)).GetInverse();
}

void Camera::SetWorld(const vec3& position, const quat& orient, float scale) {
	world = (Translate(position) * QuatMat(orient) * Scale(vec3(scale))).GetInverse();
}

Camera CreatePerspective(float fieldOfView, float aspectRatio, float nearPlane, float farPlane) {
	Camera result;
	result.SetPerspective(fieldOfView, aspectRatio, nearPlane, farPlane);
	return result;
}

Camera CreateOrthographic(float width, float height, float nearPlane, float farPlane) {
	Camera result;
	result.SetOrthographic(width, height, nearPlane, farPlane);
	return result;
}



void FromNumbers(const vec4& numbers, Plane& p) {
	vec3 abc = numbers.Splice();
	float mag = abc.GetLength();
	abc.Normalize();
	
	p.normal = abc;
	p.distance = numbers[3] / mag;
}

void NormalizePlane(Plane& p) {
	float magnitude = sqrtf(p.normal.data[0] * p.normal.data[0] + p.normal.data[1] * p.normal.data[1] + p.normal.data[2] * p.normal.data[2]);
	p.normal.data[0] /= magnitude;
	p.normal.data[1] /= magnitude;
	p.normal.data[2] /= magnitude;
	p.distance /= magnitude;
}

Frustum Camera::GetFrustum() {
	Frustum result;
	
	mat4 vp;
	mat4 proj = GetProjectionMatrix();
	mat4 world = GetWorldMatrix();
	vp = proj * world;
	
	float clip[16];

	clip[0] = world.data[0].data[0] * proj.data[0].data[0] + world.data[0].data[1] * proj.data[1].data[0] + world.data[0].data[2] * proj.data[2].data[0] + world.data[0].data[3] * proj.data[3].data[0];
	clip[1] = world.data[0].data[0] * proj.data[0].data[1] + world.data[0].data[1] * proj.data[1].data[1] + world.data[0].data[2] * proj.data[2].data[1] + world.data[0].data[3] * proj.data[3].data[1];
	clip[2] = world.data[0].data[0] * proj.data[0].data[2] + world.data[0].data[1] * proj.data[1].data[2] + world.data[0].data[2] * proj.data[2].data[2] + world.data[0].data[3] * proj.data[3].data[2];
	clip[3] = world.data[0].data[0] * proj.data[0].data[3] + world.data[0].data[1] * proj.data[1].data[3] + world.data[0].data[2] * proj.data[2].data[3] + world.data[0].data[3] * proj.data[3].data[3];

	clip[4] = world.data[1].data[0] * proj.data[0].data[0] + world.data[1].data[1] * proj.data[1].data[0] + world.data[1].data[2] * proj.data[2].data[0] + world.data[1].data[3] * proj.data[3].data[0];
	clip[5] = world.data[1].data[0] * proj.data[0].data[1] + world.data[1].data[1] * proj.data[1].data[1] + world.data[1].data[2] * proj.data[2].data[1] + world.data[1].data[3] * proj.data[3].data[1];
	clip[6] = world.data[1].data[0] * proj.data[0].data[2] + world.data[1].data[1] * proj.data[1].data[2] + world.data[1].data[2] * proj.data[2].data[2] + world.data[1].data[3] * proj.data[3].data[2];
	clip[7] = world.data[1].data[0] * proj.data[0].data[3] + world.data[1].data[1] * proj.data[1].data[3] + world.data[1].data[2] * proj.data[2].data[3] + world.data[1].data[3] * proj.data[3].data[3];

	clip[8] = world.data[2].data[0] * proj.data[0].data[0] + world.data[2].data[1] * proj.data[1].data[0] + world.data[2].data[2] * proj.data[2].data[0] + world.data[2].data[3] * proj.data[3].data[0];
	clip[9] = world.data[2].data[0] * proj.data[0].data[1] + world.data[2].data[1] * proj.data[1].data[1] + world.data[2].data[2] * proj.data[2].data[1] + world.data[2].data[3] * proj.data[3].data[1];
	clip[10] = world.data[2].data[0] * proj.data[0].data[2] + world.data[2].data[1] * proj.data[1].data[2] + world.data[2].data[2] * proj.data[2].data[2] + world.data[2].data[3] * proj.data[3].data[2];
	clip[11] = world.data[2].data[0] * proj.data[0].data[3] + world.data[2].data[1] * proj.data[1].data[3] + world.data[2].data[2] * proj.data[2].data[3] + world.data[2].data[3] * proj.data[3].data[3];

	clip[12] = world.data[3].data[0] * proj.data[0].data[0] + world.data[3].data[1] * proj.data[1].data[0] + world.data[3].data[2] * proj.data[2].data[0] + world.data[3].data[3] * proj.data[3].data[0];
	clip[13] = world.data[3].data[0] * proj.data[0].data[1] + world.data[3].data[1] * proj.data[1].data[1] + world.data[3].data[2] * proj.data[2].data[1] + world.data[3].data[3] * proj.data[3].data[1];
	clip[14] = world.data[3].data[0] * proj.data[0].data[2] + world.data[3].data[1] * proj.data[1].data[2] + world.data[3].data[2] * proj.data[2].data[2] + world.data[3].data[3] * proj.data[3].data[2];
	clip[15] = world.data[3].data[0] * proj.data[0].data[3] + world.data[3].data[1] * proj.data[1].data[3] + world.data[3].data[2] * proj.data[2].data[3] + world.data[3].data[3] * proj.data[3].data[3];

	// This will extract the LEFT side of the frustum.
	result.planes[1].normal.data[0] = clip[3] - clip[0];
	result.planes[1].normal.data[1] = clip[7] - clip[4];
	result.planes[1].normal.data[2] = clip[11] - clip[8];
	result.planes[1].distance = clip[15] - clip[12];

	NormalizePlane(result.planes[1]);

	// This will extract the RIGHT side of the frustum.
	result.planes[0].normal.data[0] = clip[3] + clip[0];
	result.planes[0].normal.data[1] = clip[7] + clip[4];
	result.planes[0].normal.data[2] = clip[11] + clip[8];
	result.planes[0].distance = clip[15] + clip[12];

	NormalizePlane(result.planes[0]);

	// This will extract the BOTTOM side of the frustum.
	result.planes[2].normal.data[0] = clip[3] + clip[1];
	result.planes[2].normal.data[1] = clip[7] + clip[5];
	result.planes[2].normal.data[2] = clip[11] + clip[9];
	result.planes[2].distance = clip[15] + clip[13];

	NormalizePlane(result.planes[2]);

	// This will extract the TOP side of the frustum.
	result.planes[3].normal.data[0] = clip[3] - clip[1];
	result.planes[3].normal.data[1] = clip[7] - clip[5];
	result.planes[3].normal.data[2] = clip[11] - clip[9];
	result.planes[3].distance = clip[15] - clip[13];

	NormalizePlane(result.planes[3]);

	// This will extract the BACK side of the frustum.
	result.planes[4].normal.data[0] = clip[3] + clip[2];
	result.planes[4].normal.data[1] = clip[7] + clip[6];
	result.planes[4].normal.data[2] = clip[11] + clip[10];
	result.planes[4].distance = clip[15] + clip[14];

	NormalizePlane(result.planes[4]);

	// This will extract the FRONT side of the frustum.
	result.planes[5].normal.data[0] = clip[3] - clip[2];
	result.planes[5].normal.data[1] = clip[7] - clip[6];
	result.planes[5].normal.data[2] = clip[11] - clip[10];
	result.planes[5].distance = clip[15] - clip[14];

	NormalizePlane(result.planes[5]);
	
	return result;
}






#if 0

OrbitCamera::OrbitCamera() {
	target = vec3(0, 0, 0);
	zoom_distance = 10.0f;
	zoom_speed = 200.0f;
	rot_speed = vec2(250.0f, 120.0f);
	y_rot_limit = vec2(-20.0f, 80.0f);
	zoom_distance_limit = vec2(3.0f, 15.0f);
	cur_rot = vec2(0, 0);
	pan_speed = vec2(180.0f, 180.0f);
}

void OrbitCamera::Rotate(const vec2& drot, float dt) {
	this->cur_rot[0] += drot[0] * this->rot_speed[0] * this->zoom_distance* dt;
	this->cur_rot[1] += drot[1] * this->rot_speed[1] * this->zoom_distance * dt;

	this->cur_rot[0] = ClampAngle(this->cur_rot[0], -360, 360);
	this->cur_rot[1] = ClampAngle(this->cur_rot[1], this->y_rot_limit[0], this->y_rot_limit[1]);
}

void OrbitCamera::Zoom(float dzoom, float dt) {
	this->zoom_distance = this->zoom_distance + dzoom  * this->zoom_speed * dt;
	if (this->zoom_distance < this->zoom_distance_limit[0]) {
		this->zoom_distance = this->zoom_distance_limit[0];
	}
	if (this->zoom_distance > this->zoom_distance_limit[1]) {
		this->zoom_distance = this->zoom_distance_limit[1];
	}
}

void OrbitCamera::Pan(const vec2& dpan, float dt) {
	vec3 right = world[0].Splice();

	target = target - (right * (dpan[0] * pan_speed[0] * dt));
	target = target + (vec3(0, 1, 0) * (dpan[1] * this->pan_speed[1] * dt));

	float midZoom = this->zoom_distance_limit[0] + (this->zoom_distance_limit[1] - this->zoom_distance_limit[0]) * 0.5f;
	this->zoom_distance = midZoom - this->zoom_distance;
	vec3 rotation = vec3(this->cur_rot[1], this->cur_rot[0], 0);
	mat3 orient = Rotation3x3(rotation[0], rotation[1], rotation[2]);
	vec3 dir = MultiplyVector( vec3(0.0, 0.0, -this->zoom_distance), orient);
	target = target - dir;
	this->zoom_distance = midZoom;
}

void OrbitCamera::Update(float dt) {
	vec3 rotation = vec3(this->cur_rot[1], this->cur_rot[0], 0);
	mat3 orient = Rotation3x3(rotation[0], rotation[1], rotation[2]);
	vec3 dir = MultiplyVector( vec3(0.0, 0.0, -this->zoom_distance), orient);
	vec3 position = dir + target;
	this->world = (LookAt(position, target, vec3(0, 1, 0))).GetInverse();
}

float OrbitCamera::ClampAngle(float angle, float min, float max) {
	while (angle < -360) {
		angle += 360;
	}
	while (angle > 360) {
		angle -= 360;
	}
	if (angle < min) {
		angle = min;
	}
	if (angle > max) {
		angle = max;
	}
	return angle;
}

void OrbitCamera::PrintDebug() {
	LOG("Target: (" << target[0] << ", " << target[1] << ", " << target[2] << ")");
	LOG("Zoom distance: " << zoom_distance);
	LOG("Rotation: (" << cur_rot[0] << ", " << cur_rot[1] << ")");
}

void OrbitCamera::SetTarget(const vec3& new_tgt) {
	target = new_tgt;
}

void OrbitCamera::SetZoom(float zoom) {
	this->zoom_distance = zoom;
}

void OrbitCamera::SetRotation(const vec2& rotation) {
	this->cur_rot = rotation;
}

#endif










VirtualStereoCamera::VirtualStereoCamera() {
	
}

void VirtualStereoCamera::Render(const Octree& o, DescriptorImage& l_img, DescriptorImage& r_img) {
	l_img.ClearDescriptors();
	r_img.ClearDescriptors();
	
	Frustum f = GetFrustum();
	
	auto iter = const_cast<Octree&>(o).GetIterator(f);
	
	mat4 world = GetViewMatrix();
	
	LensPoly::SetSize(Size((int)width, (int)height));
	
	
	while (iter) {
		const OctreeNode& n = *iter;
		//LOG(n.GetAABB().ToString());
		
		for (const auto& one_obj : n.objs) {
			const OctreeObject& obj = *one_obj;
			vec3 obj_pos = obj.GetPosition();
			vec3 local_obj_pos = (obj_pos.Embed() * world).Splice();
			
			if (local_obj_pos[2] * SCALAR_FWD_Z <= 0)
				continue;
			
			// Random additional descriptor values
			float angle = (float)Randomf() * 2*M_PIf;
			union {
				byte desc[32];
				uint32 u32[8];
				uint32 u64[4];
				const OctreeObject* ptr[4];
			};
			for(int i = 0; i < 4; i++)
				ptr[i] = &obj;
			
			axes2 l, r;
			LookAtStereoAngles(eye_dist, local_obj_pos, l, r);
			vec2 l_px = Project(0, l);
			vec2 r_px = Project(1, r);
			
			l_img.AddDescriptor(l_px[0], l_px[1], angle, desc);
			r_img.AddDescriptor(r_px[0], r_px[1], angle, desc);
			
			//LOG(l_px.ToString() << ", " << l.ToString()); LOG(r_px.ToString() << ", " << r.ToString());
		}
		
		iter++;
	}
	//LOG("");
}











void LensPoly::SetAnglePixel(float a, float b, float c, float d) {
	angle_to_pixel_poly[0] = a;
	angle_to_pixel_poly[1] = b;
	angle_to_pixel_poly[2] = c;
	angle_to_pixel_poly[3] = d;
}

void LensPoly::SetPrincipalPoint(float cx, float cy) {
	principal_point = vec2(cx, cy);
	use_principal_point = true;
}

void LensPoly::ClearPrincipalPoint() {
	use_principal_point = false;
	principal_point = vec2(img_sz.cx / 2.f, img_sz.cy / 2.f);
}

void LensPoly::SetRightTilt(float pitch, float roll) {
	right_pitch = pitch;
	right_roll = roll;
}

float LensPoly::AngleToPixel(float angle) const {
	return
		angle_to_pixel_poly.data[0] * angle +
		angle_to_pixel_poly.data[1] * angle * angle +
		angle_to_pixel_poly.data[2] * angle * angle * angle +
		angle_to_pixel_poly.data[3] * angle * angle * angle * angle;
}

float LensPoly::PixelToAngle(float pixel_radius) const {
	if (pixel_to_angle.IsEmpty())
		return 0;
	if (pixel_radius <= 0)
		return 0;
	int leni0 = (int)(pixel_radius * PIX_MUL);
	if (leni0 < 0)
		return 0;
	if (leni0 >= pixel_to_angle.GetCount())
		leni0 = pixel_to_angle.GetCount() - 1;
	int leni1 = leni0 + 1 < pixel_to_angle.GetCount() ? leni0 + 1 : leni0;
	float f1 = fmodf(pixel_radius, 1.0f);
	float f0 = 1.0f - f1;
	return pixel_to_angle[leni0] * f0 + pixel_to_angle[leni1] * f1;
}

vec2 LensPoly::Project(int lens_i, axes2 axes) {
	ASSERT(img_sz.cx && img_sz.cy);

	vec3 dir_head = GetAxesDir(axes);
	vec3 dir_cam = dir_head;
	if (lens_i == 1) {
		mat4 rot = AxesMat(outward_angle, right_pitch, right_roll);
		dir_cam = (rot.GetTransposed() * dir_head.Embed()).Splice();
	}
	dir_cam.Normalize();
	double zf = IS_NEGATIVE_Z ? -dir_cam[2] : dir_cam[2];
	zf = zf < -1.0 ? -1.0 : (zf > 1.0 ? 1.0 : zf);
	double theta = atan2(sqrt((double)dir_cam[0] * dir_cam[0] + (double)dir_cam[1] * dir_cam[1]), zf);
	double roll_angle = atan2(-dir_cam[1], dir_cam[0]);
	float pix_dist = AngleToPixel((float)theta);

	vec2 px;
	px.data[0] = (float)(principal_point[0] + pix_dist * cos(roll_angle));
	px.data[1] = (float)(principal_point[1] + pix_dist * sin(roll_angle));
	return px;
}

axes2 LensPoly::Unproject(int lens_i, const vec2& pixel) {
	ASSERT(img_sz.cx && img_sz.cy);

	vec2 ct_rel = pixel - principal_point;
	float len = ct_rel.GetLength();
	int leni0 = (int)(len * PIX_MUL);
	if (leni0 < 0)
		return axes2(0,0);
	if (leni0 >= pixel_to_angle.GetCount())
		leni0 = pixel_to_angle.GetCount() - 1;
	float angle = PixelToAngle(len);
	float roll_angle = atan2f(ct_rel[1], ct_rel[0]);

	float sin_theta = sinf(angle);
	float cos_theta = cosf(angle);
	vec3 dir_cam(
		sin_theta * cosf(roll_angle),
		-sin_theta * sinf(roll_angle),
		IS_NEGATIVE_Z ? -cos_theta : cos_theta);

	vec3 dir_head = dir_cam;
	if (lens_i == 1) {
		mat4 rot = AxesMat(outward_angle, right_pitch, right_roll);
		dir_head = (rot * dir_cam.Embed()).Splice();
	}
	axes2 axes = GetDirAxes(dir_head).Splice();
	return axes;
}

void LensPoly::SetSize(Size sz) {
	if (img_sz != sz) {
		img_sz = sz;
		if (!use_principal_point)
			principal_point = vec2(img_sz.cx / 2.f, img_sz.cy / 2.f);
		MakePixelToAngle();
	}
}

void LensPoly::MakePixelToAngle() {
	
	pixel_to_angle.SetCount(0);
	int max_len = (int)sqrt(img_sz.cx * img_sz.cx + img_sz.cy * img_sz.cy) + 1;
	max_len *= PIX_MUL;
	
	pixel_to_angle.SetCount(max_len, 0);
	
	ASSERT(angle_to_pixel_poly.data[0] != 0);
	
	float step = 0.0001f / (float)PIX_MUL;
	for (float angle = 0; angle <= M_PI; angle += step) {
		float pix_dist =
			angle_to_pixel_poly.data[0] * angle +
			angle_to_pixel_poly.data[1] * angle * angle +
			angle_to_pixel_poly.data[2] * angle * angle * angle +
			angle_to_pixel_poly.data[3] * angle * angle * angle * angle;
		
		int i = (int)(pix_dist * PIX_MUL);
		// ASSERT(i >= 0);
		if (i < 0)
			continue;
		if (i >= max_len)
			break;
		
		float& pa = pixel_to_angle[i];
		if (pa == 0)
			pa = angle;
	}
	
	//DUMPC(pixel_to_angle);
	
}





END_UPP_NAMESPACE

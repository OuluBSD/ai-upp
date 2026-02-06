#include "Render.h"

NAMESPACE_UPP


void DrawRect(Size sz, Draw& d, const mat4& view, const vec3& p, Size rect_sz, const Color& c, bool z_cull) {
	vec3 pp = VecMul(view, p);
	/*if (z_cull && pp[2] * SCALAR_FWD_Z > 0)
		return;*/
	float x = (pp[0] + 1) * 0.5 * sz.cx - rect_sz.cx / 2;
	float y = (-pp[1] + 1) * 0.5 * sz.cy - rect_sz.cy / 2;
	d.DrawRect(x, y, rect_sz.cx, rect_sz.cy, c);
}

void DrawLine(Size sz, Draw& d, const mat4& view, const vec3& a, const vec3& b, int line_width, const Color& c, bool z_cull) {
	vec3 ap = VecMul(view, a);
	vec3 bp = VecMul(view, b);
	/*if (z_cull && (ap[2] * SCALAR_FWD_Z > 0 || bp[2] * SCALAR_FWD_Z > 0))
		return;*/
	if ((ap[0] < -1 || ap[0] > +1) && (ap[1] < -1 || ap[1] > +1) &&
		(bp[0] < -1 || bp[0] > +1) && (bp[0] < -1 || bp[0] > +1))
		return;
	float x0 = (ap[0] + 1) * 0.5 * sz.cx;
	float x1 = (bp[0] + 1) * 0.5 * sz.cx;
	float y0 = (-ap[1] + 1) * 0.5 * sz.cy;
	float y1 = (-bp[1] + 1) * 0.5 * sz.cy;
	d.DrawLine(x0, y0, x1, y1, line_width, c);
}

void DrawLine(Size sz, Draw& d, const mat4& view, const Vertex& a, const Vertex& b, int line_width, const Color& c, bool z_cull) {
	DrawLine(sz, d, view, a.position.Splice(), b.position.Splice(), line_width, c, z_cull);
}

void DrawCameraGizmo(Size sz, Draw& d, const mat4& view, const vec3& pos, const quat& orient,
                     float fov_deg, float scale, const Color& clr, bool z_cull) {
	vec3 fwd = VectorTransform(VEC_FWD, orient);
	vec3 right = VectorTransform(VEC_RIGHT, orient);
	vec3 up = VectorTransform(VEC_UP, orient);
	float axis_len = scale * 0.25f;
	DrawLine(sz, d, view, pos, pos + right * axis_len, 1, LtRed(), z_cull);
	DrawLine(sz, d, view, pos, pos + up * axis_len, 1, LtGreen(), z_cull);
	DrawLine(sz, d, view, pos, pos + fwd * axis_len, 1, LtBlue(), z_cull);
	
	float near_d = scale * 0.2f;
	float far_d = scale * 0.7f;
	float half = (fov_deg * 0.5f) * (float)M_PI / 180.0f;
	float near_h = tan(half) * near_d;
	float far_h = tan(half) * far_d;
	vec3 near_c = pos + fwd * near_d;
	vec3 far_c = pos + fwd * far_d;
	vec3 n0 = near_c - right * near_h - up * near_h;
	vec3 n1 = near_c + right * near_h - up * near_h;
	vec3 n2 = near_c + right * near_h + up * near_h;
	vec3 n3 = near_c - right * near_h + up * near_h;
	vec3 f0 = far_c - right * far_h - up * far_h;
	vec3 f1 = far_c + right * far_h - up * far_h;
	vec3 f2 = far_c + right * far_h + up * far_h;
	vec3 f3 = far_c - right * far_h + up * far_h;
	
	DrawLine(sz, d, view, n0, n1, 1, clr, z_cull);
	DrawLine(sz, d, view, n1, n2, 1, clr, z_cull);
	DrawLine(sz, d, view, n2, n3, 1, clr, z_cull);
	DrawLine(sz, d, view, n3, n0, 1, clr, z_cull);
	
	DrawLine(sz, d, view, f0, f1, 1, clr, z_cull);
	DrawLine(sz, d, view, f1, f2, 1, clr, z_cull);
	DrawLine(sz, d, view, f2, f3, 1, clr, z_cull);
	DrawLine(sz, d, view, f3, f0, 1, clr, z_cull);
	
	DrawLine(sz, d, view, n0, f0, 1, clr, z_cull);
	DrawLine(sz, d, view, n1, f1, 1, clr, z_cull);
	DrawLine(sz, d, view, n2, f2, 1, clr, z_cull);
	DrawLine(sz, d, view, n3, f3, 1, clr, z_cull);
}

void DrawEditableMeshOverlay(Size sz, Draw& d, const mat4& view, const GeomObjectState& os,
                             const GeomEditableMesh& mesh, bool z_cull,
                             const Vector<float>* weights = nullptr, bool show_weights = false) {
	Color pt_clr(220, 220, 255);
	Color line_clr(200, 200, 200);
	Color face_clr(160, 200, 240);
	mat4 o_world = (QuatMat(os.orientation) * Translate(os.position) * Scale(os.scale)).GetInverse();
	mat4 o_view = view * o_world;
	for (const GeomFace& f : mesh.faces) {
		if (f.a < 0 || f.b < 0 || f.c < 0 || f.a >= mesh.points.GetCount() || f.b >= mesh.points.GetCount() || f.c >= mesh.points.GetCount())
			continue;
		const vec3& p0 = mesh.points[f.a];
		const vec3& p1 = mesh.points[f.b];
		const vec3& p2 = mesh.points[f.c];
		DrawLine(sz, d, o_view, p0, p1, 1, face_clr, z_cull);
		DrawLine(sz, d, o_view, p1, p2, 1, face_clr, z_cull);
		DrawLine(sz, d, o_view, p2, p0, 1, face_clr, z_cull);
	}
	for (const GeomEdge& e : mesh.lines) {
		if (e.a < 0 || e.b < 0 || e.a >= mesh.points.GetCount() || e.b >= mesh.points.GetCount())
			continue;
		DrawLine(sz, d, o_view, mesh.points[e.a], mesh.points[e.b], 1, line_clr, z_cull);
	}
	for (int i = 0; i < mesh.points.GetCount(); i++) {
		Color clr = pt_clr;
		if (show_weights && weights && i < weights->GetCount()) {
			float w = (*weights)[i];
			w = Clamp(w, 0.0f, 1.0f);
			clr = Blend(Color(60, 80, 180), Color(220, 80, 80), w);
		}
		DrawRect(sz, d, o_view, mesh.points[i], Size(3, 3), clr, z_cull);
	}
}

void Draw2DLayerOverlay(Size sz, Draw& d, const mat4& view, const GeomObjectState& os,
                        const Geom2DLayer& layer, bool z_cull) {
	if (!layer.visible)
		return;
	auto local_to_world = [&](const vec2& p) {
		vec3 local(p[0] * os.scale[0], p[1] * os.scale[1], 0);
		return os.position + VectorTransform(local, os.orientation);
	};
	auto draw_poly = [&](const Vector<vec2>& pts, const Color& clr, float width, bool closed) {
		if (pts.GetCount() < 2)
			return;
		for (int i = 1; i < pts.GetCount(); i++) {
			DrawLine(sz, d, view, local_to_world(pts[i - 1]), local_to_world(pts[i]), (int)width, clr, z_cull);
		}
		if (closed)
			DrawLine(sz, d, view, local_to_world(pts.Top()), local_to_world(pts[0]), (int)width, clr, z_cull);
	};
	for (const Geom2DShape& shape : layer.shapes) {
		Color clr = shape.stroke;
		float w = shape.width <= 0 ? 1.0f : shape.width;
		switch (shape.type) {
		case Geom2DShape::S_LINE:
			if (shape.points.GetCount() >= 2)
				DrawLine(sz, d, view, local_to_world(shape.points[0]), local_to_world(shape.points[1]), (int)w, clr, z_cull);
			break;
		case Geom2DShape::S_RECT:
			if (shape.points.GetCount() >= 2) {
				vec2 a = shape.points[0];
				vec2 b = shape.points[1];
				vec2 p0(min(a[0], b[0]), min(a[1], b[1]));
				vec2 p1(max(a[0], b[0]), min(a[1], b[1]));
				vec2 p2(max(a[0], b[0]), max(a[1], b[1]));
				vec2 p3(min(a[0], b[0]), max(a[1], b[1]));
				DrawLine(sz, d, view, local_to_world(p0), local_to_world(p1), (int)w, clr, z_cull);
				DrawLine(sz, d, view, local_to_world(p1), local_to_world(p2), (int)w, clr, z_cull);
				DrawLine(sz, d, view, local_to_world(p2), local_to_world(p3), (int)w, clr, z_cull);
				DrawLine(sz, d, view, local_to_world(p3), local_to_world(p0), (int)w, clr, z_cull);
			}
			break;
		case Geom2DShape::S_CIRCLE:
			if (shape.points.GetCount() >= 1) {
				vec2 c = shape.points[0];
				float r = shape.radius;
				if (r <= 0 && shape.points.GetCount() >= 2) {
					vec2 d2 = shape.points[1] - shape.points[0];
					r = sqrt(Dot(d2, d2));
				}
				if (r > 0) {
					const int steps = 32;
					Vector<vec2> pts;
					pts.SetCount(steps);
					for (int i = 0; i < steps; i++) {
						float a = (float)i / (float)steps * 2.0f * (float)M_PI;
						pts[i] = c + vec2(cos(a), sin(a)) * r;
					}
					draw_poly(pts, clr, w, true);
				}
			}
			break;
		case Geom2DShape::S_POLY:
			draw_poly(shape.points, clr, w, shape.closed);
			break;
		default:
			break;
		}
	}
}

void DrawSkeletonRecursive(Size sz, Draw& d, const mat4& view, const vec3& parent_pos,
                           const quat& parent_ori, const vec3& parent_scale, VfsValue& bone_node,
                           VfsValue* selected, bool z_cull) {
	if (!IsVfsType(bone_node, AsTypeHash<GeomBone>()))
		return;
	GeomBone& bone = bone_node.GetExt<GeomBone>();
	vec3 local = bone.position;
	vec3 scaled(local[0] * parent_scale[0], local[1] * parent_scale[1], local[2] * parent_scale[2]);
	vec3 pos = parent_pos + VectorTransform(scaled, parent_ori);
	quat ori = parent_ori * bone.orientation;
	float avg_scale = (parent_scale[0] + parent_scale[1] + parent_scale[2]) / 3.0f;
	Color clr = (&bone_node == selected) ? Color(255, 220, 120) : Color(200, 200, 255);
	DrawLine(sz, d, view, parent_pos, pos, 1, clr, z_cull);
	DrawRect(sz, d, view, pos, Size(3, 3), clr, z_cull);
	vec3 tip = pos + VectorTransform(VEC_FWD, ori) * (bone.length * avg_scale);
	DrawLine(sz, d, view, pos, tip, 1, clr, z_cull);
	for (auto& sub : bone_node.sub) {
		if (IsVfsType(sub, AsTypeHash<GeomBone>()))
			DrawSkeletonRecursive(sz, d, view, pos, ori, parent_scale, sub, selected, z_cull);
	}
}

Color CameraColor(const GeomObject& go) {
	String name = ToLower(go.name);
	if (name.Find("fake") >= 0)
		return Color(120, 220, 120);
	if (name.Find("localized") >= 0)
		return Color(220, 120, 120);
	if (name.Find("hmd") >= 0)
		return Color(120, 180, 240);
	return Color(220, 220, 120);
}

bool ClipLineNdc(vec2& a, vec2& b) {
	vec2 d2 = b - a;
	float p[4] = {-d2[0], d2[0], -d2[1], d2[1]};
	float q[4] = {a[0] + 1.0f, 1.0f - a[0], a[1] + 1.0f, 1.0f - a[1]};
	float u1 = 0.0f;
	float u2 = 1.0f;
	for (int i = 0; i < 4; i++) {
		if (p[i] == 0.0f) {
			if (q[i] < 0.0f)
				return false;
		}
		else {
			float t = q[i] / p[i];
			if (p[i] < 0.0f)
				u1 = max(u1, t);
			else
				u2 = min(u2, t);
			if (u1 > u2)
				return false;
		}
	}
	vec2 a0 = a;
	a = a0 + d2 * u1;
	b = a0 + d2 * u2;
	return true;
}

bool ClipLineClipSpace(vec4& a, vec4& b, float* out_u1 = nullptr, float* out_u2 = nullptr) {
	vec4 d = b - a;
	float u1 = 0.0f;
	float u2 = 1.0f;
	auto clip = [&](float fa, float fb) -> bool {
		float da = fa;
		float db = fb;
		float p = db - da;
		if (p == 0.0f) {
			return da >= 0.0f;
		}
		float t = da / (da - db);
		if (p < 0.0f)
			u2 = min(u2, t);
		else
			u1 = max(u1, t);
		return u1 <= u2;
	};
	// planes: x + w >= 0, -x + w >= 0, y + w >= 0, -y + w >= 0
	if (!clip(a[0] + a[3], b[0] + b[3])) return false;
	if (!clip(-a[0] + a[3], -b[0] + b[3])) return false;
	if (!clip(a[1] + a[3], b[1] + b[3])) return false;
	if (!clip(-a[1] + a[3], -b[1] + b[3])) return false;
	// ZO clip space: 0 <= z <= w
	if (!clip(a[2], b[2])) return false;
	if (!clip(-a[2] + a[3], -b[2] + b[3])) return false;
	if (out_u1)
		*out_u1 = u1;
	if (out_u2)
		*out_u2 = u2;
	vec4 a0 = a;
	a = a0 + d * u1;
	b = a0 + d * u2;
	return true;
}

void DrawGroundGrid(Size sz, Draw& d, const mat4& proj, const mat4& cam_world, const vec3& cam_pos,
                    const Scene3DRenderConfig& conf, bool z_cull) {
	if (!conf.show_grid)
		return;
	float major = conf.grid_major_step;
	if (major <= 0.0001f)
		return;
	int divs = conf.grid_minor_divs;
	if (divs < 1)
		divs = 1;
	float minor = major / (float)divs;
	float extent = conf.grid_extent;
	if (extent < major)
		extent = major;
	float start_x = floor((cam_pos[0] - extent) / minor) * minor;
	float end_x = ceil((cam_pos[0] + extent) / minor) * minor;
	float start_z = floor((cam_pos[2] - extent) / minor) * minor;
	float end_z = ceil((cam_pos[2] + extent) / minor) * minor;
	auto draw_grid_line = [&](const vec3& a, const vec3& b, const Color& clr) {
		vec3 ap_cam = VecMul(cam_world, a);
		vec3 bp_cam = VecMul(cam_world, b);
		vec3 a_world = a;
		vec3 b_world = b;
		if (z_cull) {
			float za = ap_cam[2] * SCALAR_FWD_Z;
			float zb = bp_cam[2] * SCALAR_FWD_Z;
			if (za < 0 && zb < 0)
				return;
			if ((za < 0 && zb >= 0) || (zb < 0 && za >= 0)) {
				float t = za / (za - zb);
				t = Clamp(t, 0.0f, 1.0f);
				vec3 cut = a_world + (b_world - a_world) * t;
				if (za < 0)
					a_world = cut;
				else
					b_world = cut;
			}
		}
		vec4 ap4 = proj * (cam_world * a_world.Embed());
		vec4 bp4 = proj * (cam_world * b_world.Embed());
		if (!ClipLineClipSpace(ap4, bp4))
			return;
		if (ap4[3] == 0 || bp4[3] == 0)
			return;
		vec3 ap = ap4.Splice() / ap4[3];
		vec3 bp = bp4.Splice() / bp4[3];
		vec2 a2(ap[0], ap[1]);
		vec2 b2(bp[0], bp[1]);
		if (!ClipLineNdc(a2, b2))
			return;
		float x0 = (a2[0] + 1) * 0.5 * sz.cx;
		float x1 = (b2[0] + 1) * 0.5 * sz.cx;
		float y0 = (-a2[1] + 1) * 0.5 * sz.cy;
		float y1 = (-b2[1] + 1) * 0.5 * sz.cy;
		d.DrawLine(x0, y0, x1, y1, 1, clr);
	};
	for (float x = start_x; x <= end_x + minor * 0.5f; x += minor) {
		int idx = (int)floor(x / minor + 0.5f);
		int mod = idx % divs;
		if (mod < 0)
			mod += divs;
		bool is_major = (mod == 0);
		Color clr = is_major ? conf.grid_major_clr : conf.grid_minor_clr;
		draw_grid_line(vec3(x, 0, start_z), vec3(x, 0, end_z), clr);
	}
	for (float z = start_z; z <= end_z + minor * 0.5f; z += minor) {
		int idx = (int)floor(z / minor + 0.5f);
		int mod = idx % divs;
		if (mod < 0)
			mod += divs;
		bool is_major = (mod == 0);
		Color clr = is_major ? conf.grid_major_clr : conf.grid_minor_clr;
		draw_grid_line(vec3(start_x, 0, z), vec3(end_x, 0, z), clr);
	}
}








EditRendererBase::EditRendererBase() {
	SetFrame(BlackFrame());
	WantFocus();
	
}

EditRendererV1::EditRendererV1() {}

void EditRendererV1::PaintObject(Draw& d, const GeomObjectState& os, const mat4& view, const Frustum& frustum) {
	GeomObject& go = *os.obj;
	if (!go.is_visible)
		return;
	Size sz = GetSize();
	Color clr = White();
	int lw = 1;
	bool z_cull = view_mode == VIEWMODE_PERSPECTIVE;
	
	mat4 o_world = (QuatMat(os.orientation) * Translate(os.position) * Scale(os.scale)).GetInverse();
	mat4 o_view = view * o_world;
	
	if (go.IsModel()) {
		if (!go.mdl)
			return;
		
		const Model& mdl = *go.mdl;
		for (const Mesh& mesh : mdl.meshes) {
			int tri_count = mesh.indices.GetCount() / 3;
			const auto* tri_idx = mesh.indices.Begin();
			for(int i = 0; i < tri_count; i++) {
				const Vertex& v0 = mesh.vertices[tri_idx[0]];
				const Vertex& v1 = mesh.vertices[tri_idx[1]];
				const Vertex& v2 = mesh.vertices[tri_idx[2]];
				
				bool b0 = frustum.Intersects(v0.position.Splice());
				bool b1 = frustum.Intersects(v1.position.Splice());
				bool b2 = frustum.Intersects(v2.position.Splice());
				
				if (b0 || b1) DrawLine(sz, d, o_view, v0, v1, lw, clr, z_cull);
				if (b1 || b2) DrawLine(sz, d, o_view, v1, v2, lw, clr, z_cull);
				if (b2 || b0) DrawLine(sz, d, o_view, v2, v0, lw, clr, z_cull);
				
				tri_idx += 3;
			}
		}
		
	}
	else if (go.IsOctree()) {
		Octree& o = go.octree_ptr ? *go.octree_ptr : go.octree.octree;
		OctreeFrustumIterator iter = o.GetFrustumIterator(frustum);
		vec3 fx_pos = vec3(0);
		quat fx_ori = Identity<quat>();
		Vector<GeomPointcloudEffectTransform*> effects;
		go.GetPointcloudEffects(effects);
		auto apply_local = [](vec3& pos, quat& ori, const vec3& lpos, const quat& lori) {
			pos = pos + VectorTransform(lpos, ori);
			ori = ori * lori;
		};
		for (GeomPointcloudEffectTransform* fx : effects) {
			if (!fx || !fx->enabled)
				continue;
			apply_local(fx_pos, fx_ori, fx->position, fx->orientation);
		}
		
		while (iter) {
			const OctreeNode& n = *iter;
			
			for (const auto& one_obj : n.objs) {
				
				const OctreeObject& obj = *one_obj;
				vec3 pos = obj.GetPosition();
				vec3 world = VectorTransform(pos, fx_ori) + fx_pos;
				world = VectorTransform(world * os.scale, os.orientation) + os.position;
				
				if (!frustum.Intersects(world))
					continue;
				
				vec3 cam_pos = VecMul(view, world);
				
				float x = (cam_pos[0] + 1) * 0.5 * sz.cx;
				float y = (-cam_pos[1] + 1) * 0.5 * sz.cy;
				
				d.DrawRect(x, y, 1, 1, clr);
			}
			
			iter++;
		}
	}
	else if (go.IsCamera()) {
		float fov_deg = 90.0f;
		float scale = 0.8f;
		if (ctx && ctx->state) {
			GeomCamera& ref_cam = ctx->state->GetProgram();
			fov_deg = ref_cam.fov;
			scale = Max(0.2f, ref_cam.scale * 0.5f);
		}
		DrawCameraGizmo(sz, d, view, os.position, os.orientation, fov_deg, scale, CameraColor(go), z_cull);
	}
	if (GeomEditableMesh* mesh = go.FindEditableMesh()) {
		if (!mesh->points.IsEmpty() || !mesh->lines.IsEmpty() || !mesh->faces.IsEmpty()) {
			const Vector<float>* weights = nullptr;
			if (ctx && ctx->show_weights && !ctx->weight_bone.IsEmpty()) {
				if (GeomSkinWeights* sw = go.FindSkinWeights()) {
					int wi = sw->weights.Find(ctx->weight_bone);
					if (wi >= 0)
						weights = &sw->weights[wi];
				}
			}
			DrawEditableMeshOverlay(sz, d, view, os, *mesh, z_cull, weights, ctx && ctx->show_weights);
		}
	}
	if (Geom2DLayer* layer = go.Find2DLayer()) {
		if (!layer->shapes.IsEmpty())
			Draw2DLayerOverlay(sz, d, view, os, *layer, z_cull);
	}
	if (GeomSkeleton* sk = go.FindSkeleton()) {
		for (auto& sub : sk->val.sub) {
			if (IsVfsType(sub, AsTypeHash<GeomBone>()))
				DrawSkeletonRecursive(sz, d, view, os.position, os.orientation, os.scale, sub,
					ctx ? ctx->selected_bone : nullptr, z_cull);
		}
	}
}

void EditRendererV1::Paint(Draw& d) {
	Size sz = GetSize();
	if (!ctx || !ctx->state || !ctx->conf)
		return;
	d.DrawRect(sz, ctx->conf->background_clr);
	
	GeomWorldState& state = *ctx->state;
	GeomScene& scene = state.GetActiveScene();
	GeomCamera& camera = GetGeomCamera();
	
	Camera cam;
	camera.LoadCamera(view_mode, cam, sz);
	Frustum frustum = cam.GetFrustum();
	mat4 view = cam.GetViewMatrix();
	bool z_cull = view_mode == VIEWMODE_PERSPECTIVE;

	mat4 cam_world = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	DrawGroundGrid(sz, d, proj, cam_world, camera.position, *ctx->conf, z_cull);
	
	/*if (view_mode == VIEWMODE_PERSPECTIVE) {
		mat4 world = cam.GetWorldMatrix();
		mat4 proj = cam.GetProjectionMatrix();
		//proj.SetPerspectiveRH_NO(DEG2RAD(120/2), (float)sz.cx / sz.cy, 0.1, 100.0);
		view = proj * world;
	}*/
	
	if (cam_src == CAMSRC_VIDEOIMPORT_FOCUS ||
		cam_src == CAMSRC_VIDEOIMPORT_PROGRAM) {
		if (!ctx->anim || !ctx->video)
			return;
		int frame_i = ctx->anim->position;
		if (frame_i < 0 || frame_i >= ctx->video->uncam.frames.GetCount())
			return;
		UncameraFrame& frame = ctx->video->uncam.frames[frame_i];
		
		GeomObjectState os;
		VfsValue temp_node;
		GeomObject& go = temp_node.CreateExt<GeomObject>();
		go.type = GeomObject::O_OCTREE;
		os.obj = &go;
		os.position = vec3(0);
		os.orientation = Identity<quat>();
		os.scale = vec3(1);
		go.octree_ptr = &frame.otree;
		PaintObject(d, os, view, frustum);
	}
	if (ctx->anim && ctx->anim->is_playing) {
		for (GeomObjectState& os : state.objs) {
			PaintObject(d, os, view, frustum);
		}
	}
	else {
		GeomObjectCollection iter(scene);
		GeomObjectState os;
		for (GeomObject& go : iter) {
			os.obj = &go;
			os.position = vec3(0);
			os.orientation = Identity<quat>();
			os.scale = vec3(1);
			if (GeomTransform* tr = go.FindTransform()) {
				os.position = tr->position;
				os.orientation = tr->orientation;
				os.scale = tr->scale;
			}
			PaintObject(d, os, view, frustum);
		}
	}
	auto draw_frustum = [&](const vec3& pos, const quat& orient, float fov_deg, float scale, Color clr) {
		Vector<vec3> corners;
		{
			Camera cam;
			float aspect = (float)sz.cx / (float)sz.cy;
			cam.SetPerspective(fov_deg, aspect, 0.1, 3.0);
			cam.SetWorld(pos, orient, scale);
			Frustum frustum = cam.GetFrustum();
			corners.SetCount(8);
			frustum.GetCorners(corners.Begin());
		}
		DrawRect(sz, d, view, pos, Size(2,2), clr, z_cull);
		int lw = 1;
		DrawLine(sz, d, view, corners[0], corners[1], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[2], corners[3], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[4], corners[5], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[6], corners[7], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[0], corners[2], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[1], corners[3], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[4], corners[6], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[5], corners[7], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[0], corners[4], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[1], corners[5], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[2], corners[6], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[3], corners[7], lw, clr, z_cull);
	};

	GeomCamera& program = state.GetProgram();
	GeomCamera& focus = state.GetFocus();
	if (state.focus_mode == 1) {
		GeomObject* foc = state.FindObjectByKey(state.focus_object_key);
		if (foc && foc->IsCamera() && foc->is_visible) {
			vec3 pos = vec3(0);
			quat ori = Identity<quat>();
			if (GeomTransform* tr = foc->FindTransform()) {
				pos = tr->position;
				ori = tr->orientation;
			}
			else if (const GeomObjectState* os = state.FindObjectStateByKey(state.focus_object_key)) {
				pos = os->position;
				ori = os->orientation;
			}
			draw_frustum(pos, ori, program.fov, program.scale, Color(255, 255, 172));
		}
	}
	if (state.focus_mode == 2 && state.program_visible)
		draw_frustum(program.position, program.orientation, program.fov, program.scale, Color(220, 120, 120));
	if (state.focus_mode == 3 && state.focus_visible)
		draw_frustum(focus.position, focus.orientation, focus.fov, focus.scale, Color(120, 220, 120));
	
	// Overlay: active camera axes and forward vector
	{
		vec3 fwd = VectorTransform(VEC_FWD, camera.orientation);
		vec3 right = VectorTransform(VEC_RIGHT, camera.orientation);
		vec3 up = VectorTransform(VEC_UP, camera.orientation);
		String info = Format("cam fwd=(%.2f %.2f %.2f) right=(%.2f %.2f %.2f) up=(%.2f %.2f %.2f)",
			fwd[0], fwd[1], fwd[2], right[0], right[1], right[2], up[0], up[1], up[2]);
		d.DrawText(4, 4, info, StdFont().Bold(), LtGray());
	}
	
	// Draw red-green-blue unit-vectors
	if (1) {
		vec3 axes[3] = {vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)};
		Color clr[3] = {LtRed(), LtGreen(), LtBlue()};
		int pad = 20;
		Point pt0(pad, sz.cy - pad);
		float len = 20;
		mat4 view_orient = QuatMat(MatQuat(view));
		for(int i = 0; i < 3; i++) {
			vec3 orig = VecMul(view_orient, vec3(0));
			vec3 ax = VecMul(view_orient, axes[i]);
			vec3 diff = ax - orig;
				diff[1] *= -1; // 3d space top is +1 and bottom is -1 and 2d space top is 0 and bottom is sz.cy
			diff.Normalize();
			if (view_mode != VIEWMODE_PERSPECTIVE)
				diff *= -1;
			Point pt1(pt0.x - diff[0] * len, pt0.y - diff[1] * len);
			d.DrawLine(pt0, pt1, 1, clr[i]);
		}
	}
	//LOG("");
	
}

EditRendererV2::EditRendererV2() {}

namespace {

struct SoftSurface {
	Size sz;
	ImageBuffer ib;
	Vector<float> zbuf;
	
	SoftSurface(Size s, Color bg) : sz(s), ib(s), zbuf(s.cx * s.cy, 1e9f) {
		for (int y = 0; y < sz.cy; y++) {
			RGBA* line = ib[y];
			for (int x = 0; x < sz.cx; x++) {
				line[x] = bg;
			}
		}
	}
	
	void SetPixel(int x, int y, float z, const Color& c) {
		if (x < 0 || y < 0 || x >= sz.cx || y >= sz.cy)
			return;
		int idx = y * sz.cx + x;
		if (z >= zbuf[idx])
			return;
		zbuf[idx] = z;
		ib[y][x] = c;
	}
	
	void DrawLine2D(int x0, int y0, int x1, int y1, const Color& c) {
		int dx = abs(x1 - x0);
		int sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0);
		int sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;
		while (true) {
			if (x0 >= 0 && y0 >= 0 && x0 < sz.cx && y0 < sz.cy)
				ib[y0][x0] = c;
			if (x0 == x1 && y0 == y1)
				break;
			int e2 = 2 * err;
			if (e2 >= dy) {
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx) {
				err += dx;
				y0 += sy;
			}
		}
	}
};

struct TriProj {
	vec2 p;
	float z = 0;
};

struct GridLineDump : Moveable<GridLineDump> {
	vec3 a_world;
	vec3 b_world;
	vec3 a_world_clipped;
	vec3 b_world_clipped;
	vec3 a_cam;
	vec3 b_cam;
	vec3 a_ndc;
	vec3 b_ndc;
	vec2 a_clip;
	vec2 b_clip;
	bool culled = false;
	bool clipped = false;
};

float Edge2D(const vec2& a, const vec2& b, const vec2& c) {
	return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

}

void EditRendererV2::Paint(Draw& d) {
	Size sz = GetSize();
	if (!ctx || !ctx->state || !ctx->conf)
		return;
	
	GeomWorldState& state = *ctx->state;
	GeomScene& scene = state.GetActiveScene();
	GeomCamera& camera = GetGeomCamera();
	
	Camera cam;
	camera.LoadCamera(view_mode, cam, sz);
	Frustum frustum = cam.GetFrustum();
	mat4 view = cam.GetViewMatrix();
	mat4 cam_world = cam.GetWorldMatrix();
	mat4 proj = cam.GetProjectionMatrix();
	bool z_cull = view_mode == VIEWMODE_PERSPECTIVE;
	
	SoftSurface surf(sz, ctx->conf->background_clr);
	Vector<GridLineDump> grid_dumps;
	bool dump_grid = !ctx->conf->dump_grid_path.IsEmpty();
	auto draw_grid_line = [&](const vec3& a, const vec3& b, const Color& clr) {
		GridLineDump dump;
		dump.a_world = a;
		dump.b_world = b;
		vec3 a_world = a;
		vec3 b_world = b;
		vec4 ap4 = proj * (cam_world * a_world.Embed());
		vec4 bp4 = proj * (cam_world * b_world.Embed());
		float u1 = 0.0f;
		float u2 = 1.0f;
		if (!ClipLineClipSpace(ap4, bp4, &u1, &u2)) {
			dump.culled = true;
			if (dump_grid) grid_dumps.Add(dump);
			return;
		}
		if (ap4[3] == 0 || bp4[3] == 0) {
			dump.culled = true;
			if (dump_grid) grid_dumps.Add(dump);
			return;
		}
		vec3 ap = ap4.Splice() / ap4[3];
		vec3 bp = bp4.Splice() / bp4[3];
		dump.a_world_clipped = a_world + (b_world - a_world) * u1;
		dump.b_world_clipped = a_world + (b_world - a_world) * u2;
		dump.a_cam = VecMul(cam_world, a_world);
		dump.b_cam = VecMul(cam_world, b_world);
		dump.a_ndc = ap;
		dump.b_ndc = bp;
		vec2 a2(ap[0], ap[1]);
		vec2 b2(bp[0], bp[1]);
		dump.a_clip = a2;
		dump.b_clip = b2;
		if (!ClipLineNdc(a2, b2)) {
			dump.clipped = false;
			if (dump_grid) grid_dumps.Add(dump);
			return;
		}
		dump.clipped = true;
		dump.a_clip = a2;
		dump.b_clip = b2;
		if (dump_grid) grid_dumps.Add(dump);
		int x0 = (int)floor((a2[0] + 1) * 0.5 * (float)(sz.cx - 1) + 0.5f);
		int y0 = (int)floor((-a2[1] + 1) * 0.5 * (float)(sz.cy - 1) + 0.5f);
		int x1 = (int)floor((b2[0] + 1) * 0.5 * (float)(sz.cx - 1) + 0.5f);
		int y1 = (int)floor((-b2[1] + 1) * 0.5 * (float)(sz.cy - 1) + 0.5f);
		surf.DrawLine2D(x0, y0, x1, y1, clr);
	};
	float grid_major = ctx->conf->grid_major_step;
	int grid_divs = ctx->conf->grid_minor_divs;
	if (grid_major <= 0.0001f)
		grid_major = 1.0f;
	if (grid_divs < 1)
		grid_divs = 1;
	float grid_minor = grid_major / (float)grid_divs;
	float grid_extent = ctx->conf->grid_extent;
	if (grid_extent < grid_major)
		grid_extent = grid_major;
	if (ctx->conf->show_grid) {
		float major = grid_major;
		int divs = grid_divs;
		float minor = grid_minor;
		float extent = grid_extent;
		if (extent < major)
			extent = major;
		float start_x = floor((camera.position[0] - extent) / minor) * minor;
		float end_x = ceil((camera.position[0] + extent) / minor) * minor;
		float start_z = floor((camera.position[2] - extent) / minor) * minor;
		float end_z = ceil((camera.position[2] + extent) / minor) * minor;
		for (float x = start_x; x <= end_x + minor * 0.5f; x += minor) {
			int idx = (int)floor(x / minor + 0.5f);
			int mod = idx % divs;
			if (mod < 0)
				mod += divs;
			bool is_major = (mod == 0);
			Color clr = is_major ? ctx->conf->grid_major_clr : ctx->conf->grid_minor_clr;
			draw_grid_line(vec3(x, 0, start_z), vec3(x, 0, end_z), clr);
		}
		for (float z = start_z; z <= end_z + minor * 0.5f; z += minor) {
			int idx = (int)floor(z / minor + 0.5f);
			int mod = idx % divs;
			if (mod < 0)
				mod += divs;
			bool is_major = (mod == 0);
			Color clr = is_major ? ctx->conf->grid_major_clr : ctx->conf->grid_minor_clr;
			draw_grid_line(vec3(start_x, 0, z), vec3(end_x, 0, z), clr);
		}
	}
	if (dump_grid && !ctx->conf->dump_grid_done) {
		ctx->conf->dump_grid_done = true;
		FileOut out(ctx->conf->dump_grid_path);
		if (out.IsOpen()) {
			auto wvec3 = [&](const vec3& v) {
				out << v[0] << " " << v[1] << " " << v[2];
			};
			auto wvec2 = [&](const vec2& v) {
				out << v[0] << " " << v[1];
			};
			out << "size " << sz.cx << " " << sz.cy << "\n";
			out << "cam_pos "; wvec3(camera.position); out << "\n";
			out << "cam_orient " << camera.orientation[0] << " "
			    << camera.orientation[1] << " "
			    << camera.orientation[2] << " "
			    << camera.orientation[3] << "\n";
			out << "cam_scale " << camera.scale << " fov " << camera.fov << "\n";
			out << "grid major " << grid_major << " minor " << grid_minor
			    << " extent " << grid_extent << " divs " << grid_divs << "\n";
			out << "world\n";
			for (int r = 0; r < 4; r++) {
				for (int c = 0; c < 4; c++)
					out << cam_world[r][c] << (c == 3 ? '\n' : ' ');
			}
			out << "proj\n";
			for (int r = 0; r < 4; r++) {
				for (int c = 0; c < 4; c++)
					out << proj[r][c] << (c == 3 ? '\n' : ' ');
			}
			out << "view\n";
			for (int r = 0; r < 4; r++) {
				for (int c = 0; c < 4; c++)
					out << view[r][c] << (c == 3 ? '\n' : ' ');
			}
			out << "lines " << grid_dumps.GetCount() << "\n";
			for (const auto& ln : grid_dumps) {
				out << "line ";
				wvec3(ln.a_world); out << " ";
				wvec3(ln.b_world); out << " ";
				wvec3(ln.a_world_clipped); out << " ";
				wvec3(ln.b_world_clipped); out << " ";
				wvec3(ln.a_cam); out << " ";
				wvec3(ln.b_cam); out << " ";
				wvec3(ln.a_ndc); out << " ";
				wvec3(ln.b_ndc); out << " ";
				wvec2(ln.a_clip); out << " ";
				wvec2(ln.b_clip); out << " ";
				out << (ln.culled ? 1 : 0) << " " << (ln.clipped ? 1 : 0) << "\n";
			}
		}
	}
	
	auto draw_line_ndc = [&](const vec3& a, const vec3& b, const Color& clr) {
		vec2 a2(a[0], a[1]);
		vec2 b2(b[0], b[1]);
		if (!ClipLineNdc(a2, b2))
			return;
		int x0 = (int)floor((a2[0] + 1) * 0.5 * (float)(sz.cx - 1) + 0.5f);
		int y0 = (int)floor((-a2[1] + 1) * 0.5 * (float)(sz.cy - 1) + 0.5f);
		int x1 = (int)floor((b2[0] + 1) * 0.5 * (float)(sz.cx - 1) + 0.5f);
		int y1 = (int)floor((-b2[1] + 1) * 0.5 * (float)(sz.cy - 1) + 0.5f);
		surf.DrawLine2D(x0, y0, x1, y1, clr);
	};
	auto draw_triangle = [&](const vec4& c0, const vec4& c1, const vec4& c2, const Color& clr) {
		if (wireframe_only) {
			vec4 e0 = c0, e1 = c1;
			if (ClipLineClipSpace(e0, e1) && e0[3] != 0 && e1[3] != 0) {
				vec3 n0 = e0.Splice() / e0[3];
				vec3 n1 = e1.Splice() / e1[3];
				draw_line_ndc(n0, n1, clr);
			}
			e0 = c1; e1 = c2;
			if (ClipLineClipSpace(e0, e1) && e0[3] != 0 && e1[3] != 0) {
				vec3 n0 = e0.Splice() / e0[3];
				vec3 n1 = e1.Splice() / e1[3];
				draw_line_ndc(n0, n1, clr);
			}
			e0 = c2; e1 = c0;
			if (ClipLineClipSpace(e0, e1) && e0[3] != 0 && e1[3] != 0) {
				vec3 n0 = e0.Splice() / e0[3];
				vec3 n1 = e1.Splice() / e1[3];
				draw_line_ndc(n0, n1, clr);
			}
			return;
		}
		if (c0[3] == 0 || c1[3] == 0 || c2[3] == 0)
			return;
		vec3 n0 = c0.Splice() / c0[3];
		vec3 n1 = c1.Splice() / c1[3];
		vec3 n2 = c2.Splice() / c2[3];
		if (n0[2] < -1 && n1[2] < -1 && n2[2] < -1)
			return;
		if (n0[2] > 1 && n1[2] > 1 && n2[2] > 1)
			return;
		
		TriProj tp[3];
		vec3 ndc[3] = {n0, n1, n2};
		for (int i = 0; i < 3; i++) {
			tp[i].p[0] = (ndc[i][0] + 1.0f) * 0.5f * (float)(sz.cx - 1);
			tp[i].p[1] = (-ndc[i][1] + 1.0f) * 0.5f * (float)(sz.cy - 1);
			tp[i].z = ndc[i][2];
		}
		
		float area = Edge2D(tp[0].p, tp[1].p, tp[2].p);
		if (area == 0)
			return;
		int minx = (int)floor(min(tp[0].p[0], min(tp[1].p[0], tp[2].p[0])));
		int maxx = (int)ceil(max(tp[0].p[0], max(tp[1].p[0], tp[2].p[0])));
		int miny = (int)floor(min(tp[0].p[1], min(tp[1].p[1], tp[2].p[1])));
		int maxy = (int)ceil(max(tp[0].p[1], max(tp[1].p[1], tp[2].p[1])));
		minx = max(minx, 0);
		miny = max(miny, 0);
		maxx = min(maxx, sz.cx - 1);
		maxy = min(maxy, sz.cy - 1);
		for (int y = miny; y <= maxy; y++) {
			for (int x = minx; x <= maxx; x++) {
				vec2 p((float)x + 0.5f, (float)y + 0.5f);
				float w0 = Edge2D(tp[1].p, tp[2].p, p);
				float w1 = Edge2D(tp[2].p, tp[0].p, p);
				float w2 = Edge2D(tp[0].p, tp[1].p, p);
				if ((area > 0 && (w0 < 0 || w1 < 0 || w2 < 0)) ||
				    (area < 0 && (w0 > 0 || w1 > 0 || w2 > 0)))
					continue;
				w0 /= area;
				w1 /= area;
				w2 /= area;
				float z = w0 * tp[0].z + w1 * tp[1].z + w2 * tp[2].z;
				surf.SetPixel(x, y, z, clr);
			}
		}
	};
	
	auto paint_model = [&](const GeomObjectState& os, const Model& mdl) {
		mat4 o_world = Translate(os.position) * QuatMat(os.orientation) * Scale(os.scale);
		mat4 o_view = view * o_world;
		vec3 light_dir = vec3(0.4f, 0.7f, 0.5f);
		light_dir.Normalize();
		for (const Mesh& mesh : mdl.meshes) {
			const auto* tri_idx = mesh.indices.Begin();
			int tri_count = mesh.indices.GetCount() / 3;
			for (int i = 0; i < tri_count; i++) {
				const Vertex& v0 = mesh.vertices[tri_idx[0]];
				const Vertex& v1 = mesh.vertices[tri_idx[1]];
				const Vertex& v2 = mesh.vertices[tri_idx[2]];
				vec3 n = Cross(v1.position.Splice() - v0.position.Splice(),
				               v2.position.Splice() - v0.position.Splice());
				if (n.GetLength() == 0) {
					tri_idx += 3;
					continue;
				}
				n.Normalize();
				vec3 n_world = VectorTransform(n, os.orientation);
				float diff = max(0.0f, Dot(n_world, light_dir));
				float intensity = 0.2f + diff * 0.8f;
				int r = (int)Clamp(200.0f * intensity, 0.0f, 255.0f);
				int g = (int)Clamp(200.0f * intensity, 0.0f, 255.0f);
				int b = (int)Clamp(200.0f * intensity, 0.0f, 255.0f);
				Color clr(r, g, b);
				vec4 c0 = o_view * v0.position;
				vec4 c1 = o_view * v1.position;
				vec4 c2 = o_view * v2.position;
				draw_triangle(c0, c1, c2, clr);
				tri_idx += 3;
			}
		}
	};
	
	if (ctx->anim && ctx->anim->is_playing) {
		for (GeomObjectState& os : state.objs) {
			GeomObject& go = *os.obj;
			if (!go.is_visible)
				continue;
			if (go.IsModel() && go.mdl)
				paint_model(os, *go.mdl);
		}
	}
	else {
		GeomObjectCollection iter(scene);
		GeomObjectState os;
		for (GeomObject& go : iter) {
			if (!go.is_visible)
				continue;
			if (go.IsModel() && go.mdl) {
				os.obj = &go;
				os.position = vec3(0);
				os.orientation = Identity<quat>();
				os.scale = vec3(1);
				if (GeomTransform* tr = go.FindTransform()) {
					os.position = tr->position;
					os.orientation = tr->orientation;
					os.scale = tr->scale;
				}
				paint_model(os, *go.mdl);
			}
		}
	}
	
	d.DrawImage(0, 0, surf.ib);
	// Camera gizmos + overlays (match V1 behavior)
	auto draw_camera_gizmos = [&] {
		auto draw_one = [&](const vec3& pos, const quat& ori, float fov_deg, float scale, Color clr) {
			DrawCameraGizmo(sz, d, view, pos, ori, fov_deg, scale, clr, z_cull);
		};
		if (ctx->anim && ctx->anim->is_playing) {
			for (GeomObjectState& os : state.objs) {
				if (!os.obj || !os.obj->IsCamera() || !os.obj->is_visible)
					continue;
				float fov_deg = 90.0f;
				float scale = 0.8f;
				if (ctx && ctx->state) {
					GeomCamera& ref_cam = ctx->state->GetProgram();
					fov_deg = ref_cam.fov;
					scale = Max(0.2f, ref_cam.scale * 0.5f);
				}
				draw_one(os.position, os.orientation, fov_deg, scale, CameraColor(*os.obj));
			}
		}
		else {
			GeomObjectCollection iter(scene);
			for (GeomObject& go : iter) {
				if (!go.IsCamera() || !go.is_visible)
					continue;
				vec3 pos = vec3(0);
				quat ori = Identity<quat>();
				if (GeomTransform* tr = go.FindTransform()) {
					pos = tr->position;
					ori = tr->orientation;
				}
				float fov_deg = 90.0f;
				float scale = 0.8f;
				if (ctx && ctx->state) {
					GeomCamera& ref_cam = ctx->state->GetProgram();
					fov_deg = ref_cam.fov;
					scale = Max(0.2f, ref_cam.scale * 0.5f);
				}
				draw_one(pos, ori, fov_deg, scale, CameraColor(go));
			}
		}
	};
	draw_camera_gizmos();
	// Skeleton overlay
	if (ctx && ctx->state) {
		if (ctx->anim && ctx->anim->is_playing) {
			for (GeomObjectState& os : state.objs) {
				if (!os.obj || !os.obj->is_visible)
					continue;
				if (GeomSkeleton* sk = os.obj->FindSkeleton()) {
					for (auto& sub : sk->val.sub) {
						if (IsVfsType(sub, AsTypeHash<GeomBone>()))
							DrawSkeletonRecursive(sz, d, view, os.position, os.orientation, os.scale, sub,
								ctx ? ctx->selected_bone : nullptr, z_cull);
					}
				}
			}
		}
		else {
			GeomObjectCollection iter(scene);
			GeomObjectState os;
			for (GeomObject& go : iter) {
				if (!go.is_visible)
					continue;
				if (!go.FindSkeleton())
					continue;
				os.obj = &go;
				os.position = vec3(0);
				os.orientation = Identity<quat>();
				os.scale = vec3(1);
				if (GeomTransform* tr = go.FindTransform()) {
					os.position = tr->position;
					os.orientation = tr->orientation;
					os.scale = tr->scale;
				}
				GeomSkeleton* sk = go.FindSkeleton();
				for (auto& sub : sk->val.sub) {
					if (IsVfsType(sub, AsTypeHash<GeomBone>()))
						DrawSkeletonRecursive(sz, d, view, os.position, os.orientation, os.scale, sub,
							ctx ? ctx->selected_bone : nullptr, z_cull);
				}
			}
		}
	}

	auto draw_frustum = [&](const vec3& pos, const quat& orient, float fov_deg, float scale, Color clr) {
		Vector<vec3> corners;
		{
			Camera cam;
			float aspect = (float)sz.cx / (float)sz.cy;
			cam.SetPerspective(fov_deg, aspect, 0.1, 3.0);
			cam.SetWorld(pos, orient, scale);
			Frustum frustum = cam.GetFrustum();
			corners.SetCount(8);
			frustum.GetCorners(corners.Begin());
		}
		DrawRect(sz, d, view, pos, Size(2,2), clr, z_cull);
		int lw = 1;
		DrawLine(sz, d, view, corners[0], corners[1], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[2], corners[3], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[4], corners[5], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[6], corners[7], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[0], corners[2], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[1], corners[3], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[4], corners[6], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[5], corners[7], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[0], corners[4], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[1], corners[5], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[2], corners[6], lw, clr, z_cull);
		DrawLine(sz, d, view, corners[3], corners[7], lw, clr, z_cull);
	};

	GeomCamera& program = state.GetProgram();
	GeomCamera& focus = state.GetFocus();
	if (state.focus_mode == 1) {
		GeomObject* foc = state.FindObjectByKey(state.focus_object_key);
		if (foc && foc->IsCamera() && foc->is_visible) {
			vec3 pos = vec3(0);
			quat ori = Identity<quat>();
			if (GeomTransform* tr = foc->FindTransform()) {
				pos = tr->position;
				ori = tr->orientation;
			}
			else if (const GeomObjectState* os = state.FindObjectStateByKey(state.focus_object_key)) {
				pos = os->position;
				ori = os->orientation;
			}
			draw_frustum(pos, ori, program.fov, program.scale, Color(255, 255, 172));
		}
	}
	if (state.focus_mode == 2 && state.program_visible)
		draw_frustum(program.position, program.orientation, program.fov, program.scale, Color(220, 120, 120));
	if (state.focus_mode == 3 && state.focus_visible)
		draw_frustum(focus.position, focus.orientation, focus.fov, focus.scale, Color(120, 220, 120));

	{
		vec3 fwd = VectorTransform(VEC_FWD, camera.orientation);
		vec3 right = VectorTransform(VEC_RIGHT, camera.orientation);
		vec3 up = VectorTransform(VEC_UP, camera.orientation);
		String info = Format("cam fwd=(%.2f %.2f %.2f) right=(%.2f %.2f %.2f) up=(%.2f %.2f %.2f)",
			fwd[0], fwd[1], fwd[2], right[0], right[1], right[2], up[0], up[1], up[2]);
		d.DrawText(4, 4, info, StdFont().Bold(), LtGray());
	}
	if (1) {
		vec3 axes[3] = {vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)};
		Color clr[3] = {LtRed(), LtGreen(), LtBlue()};
		int pad = 20;
		Point pt0(pad, sz.cy - pad);
		float len = 20;
		mat4 view_orient = QuatMat(MatQuat(view));
		for(int i = 0; i < 3; i++) {
			vec3 orig = VecMul(view_orient, vec3(0));
			vec3 ax = VecMul(view_orient, axes[i]);
			vec3 diff = ax - orig;
			diff[1] *= -1;
			diff.Normalize();
			if (view_mode != VIEWMODE_PERSPECTIVE)
				diff *= -1;
			Point pt1(pt0.x - diff[0] * len, pt0.y - diff[1] * len);
			d.DrawLine(pt0, pt1, 1, clr[i]);
		}
	}
	if (ctx->anim && ctx->anim->is_playing) {
		for (GeomObjectState& os : state.objs) {
			if (!os.obj || !os.obj->is_visible)
				continue;
			if (GeomEditableMesh* mesh = os.obj->FindEditableMesh()) {
				if (!mesh->points.IsEmpty() || !mesh->lines.IsEmpty() || !mesh->faces.IsEmpty()) {
					const Vector<float>* weights = nullptr;
					if (ctx && ctx->show_weights && !ctx->weight_bone.IsEmpty()) {
						if (GeomSkinWeights* sw = os.obj->FindSkinWeights()) {
							int wi = sw->weights.Find(ctx->weight_bone);
							if (wi >= 0)
								weights = &sw->weights[wi];
						}
					}
					DrawEditableMeshOverlay(sz, d, view, os, *mesh, z_cull, weights, ctx && ctx->show_weights);
				}
			}
		}
	}
	else {
		GeomObjectCollection iter(scene);
		GeomObjectState os;
		for (GeomObject& go : iter) {
			if (!go.is_visible)
				continue;
			if (!go.FindEditableMesh())
				continue;
			os.obj = &go;
			os.position = vec3(0);
			os.orientation = Identity<quat>();
			os.scale = vec3(1);
			if (GeomTransform* tr = go.FindTransform()) {
				os.position = tr->position;
				os.orientation = tr->orientation;
				os.scale = tr->scale;
			}
			const Vector<float>* weights = nullptr;
			if (ctx && ctx->show_weights && !ctx->weight_bone.IsEmpty()) {
				if (GeomSkinWeights* sw = go.FindSkinWeights()) {
					int wi = sw->weights.Find(ctx->weight_bone);
					if (wi >= 0)
						weights = &sw->weights[wi];
				}
			}
			DrawEditableMeshOverlay(sz, d, view, os, *go.FindEditableMesh(), z_cull, weights, ctx && ctx->show_weights);
		}
	}
}

void EditRendererBase::LeftDown(Point p, dword keyflags) {
	GeomCamera& camera = GetGeomCamera();
	if (WhenInput)
		WhenInput("mouseDown", p, keyflags, 0);
	
	cap_mouse_pos = p;
	is_captured_mouse = true;
	cap_begin_pos = camera.position;
	cap_begin_orientation = camera.orientation;
	
	bool is_shift = keyflags & K_SHIFT;
	bool is_ctrl = keyflags & K_CTRL;
	if (is_shift)
		cap_mode = CAPMODE_MOVE_YZ;
	else if (is_ctrl)
		cap_mode = CAPMODE_ROTATE;
	else
		cap_mode = CAPMODE_MOVE_XY;
	
	SetCapture();
	
	SetFocus();
}

void EditRendererBase::LeftUp(Point p, dword keyflags) {
	if (WhenInput)
		WhenInput("mouseUp", p, keyflags, 0);
	is_captured_mouse = false;
	
	ReleaseCapture();
}

void EditRendererBase::MouseMove(Point p, dword keyflags) {
	GeomCamera& camera = GetGeomCamera();
	if (WhenInput)
		WhenInput("mouseMove", p, keyflags, 0);
	
	if (is_captured_mouse) {
		Point diff = p - cap_mouse_pos;
		if (!ctx || !ctx->conf)
			return;
		float s = ctx->conf->mouse_move_sensitivity * camera.scale;
		switch (cap_mode) {
			case CAPMODE_MOVE_XY: MoveRel(vec3(-diff.x * s, diff.y * s, 0)); break;
			case CAPMODE_MOVE_YZ: MoveRel(vec3(-diff.x * s, 0, -diff.y * s * SCALAR_FWD_Z)); break;
			case CAPMODE_ROTATE: RotateRel(vec3(-diff.x * s, -diff.y * s, 0)); break;
			default: break;
		}
	}
}

void EditRendererBase::Move(const vec3& v) {
	GeomCamera& camera = GetGeomCamera();
	
	switch (view_mode) {
		
	case VIEWMODE_YZ:
		camera.position += VecMul(YRotation(M_PI/2), v);
		break;
		
	case VIEWMODE_XZ:
		camera.position += VecMul(XRotation(-M_PI/2), v);
		break;
		
	case VIEWMODE_XY:
		camera.position += VecMul(YRotation(0), v);
		break;
		
	case VIEWMODE_PERSPECTIVE:
		camera.position += VecMul(QuatMat(camera.orientation), v);
		break;
		
	}
	
	WhenChanged();
}

void EditRendererBase::MoveRel(const vec3& v) {
	GeomCamera& camera = GetGeomCamera();
	
	switch (view_mode) {
		
	case VIEWMODE_YZ:
		camera.position = cap_begin_pos + VecMul(YRotation(M_PI/2), v);
		break;
		
	case VIEWMODE_XZ:
		camera.position = cap_begin_pos + VecMul(XRotation(-M_PI/2), v);
		break;
		
	case VIEWMODE_XY:
		camera.position = cap_begin_pos + VecMul(YRotation(0), v);
		break;
		
	case VIEWMODE_PERSPECTIVE:
		camera.position = cap_begin_pos + VecMul(QuatMat(camera.orientation), v);
		break;
		
	}
	
	WhenChanged();
}

void EditRendererBase::Rotate(const axes3& v) {
	GeomCamera& camera = GetGeomCamera();
	
	camera.orientation = MatQuat(QuatMat(camera.orientation) * AxesMat(v));
	
	WhenChanged();
}

void EditRendererBase::RotateRel(const axes3& v) {
	GeomCamera& camera = GetGeomCamera();
	
	camera.orientation = MatQuat(QuatMat(cap_begin_orientation) * AxesMat(v));
	
	WhenChanged();
}

void EditRendererBase::MouseWheel(Point p, int zdelta, dword keyflags) {
	if (WhenInput)
		WhenInput("mouseWheel", p, keyflags, zdelta);
	const double scale = 0.75;
	GeomCamera& camera = GetGeomCamera();
	
	if (zdelta < 0) {
		camera.scale /= scale;
	}
	else {
		camera.scale *= scale;
	}
	
	WhenChanged();
}

void EditRendererBase::RightDown(Point p, dword keyflags) {
	if (WhenInput)
		WhenInput("mouseRightDown", p, keyflags, 0);
	if (WhenMenu)
		MenuBar::Execute(WhenMenu, GetMousePos());
}

GeomCamera& EditRendererBase::GetGeomCamera() const {
	switch (cam_src) {
		
	case CAMSRC_FOCUS:
	case CAMSRC_VIDEOIMPORT_FOCUS:
		ASSERT(ctx && ctx->state);
		return ctx->state->GetFocus();
		break;
		
	case CAMSRC_PROGRAM:
	case CAMSRC_VIDEOIMPORT_PROGRAM:
		ASSERT(ctx && ctx->state);
		return ctx->state->GetProgram();
		break;
		
	}
	Panic("Invalid view mode in EditRendererBase");
	NEVER();
}

bool EditRendererBase::Key(dword key, int count) {
	if (WhenInput)
		WhenInput("keyDown", Point(0, 0), 0, (int)key);
	GeomCamera& camera = GetGeomCamera();
	float step = camera.scale * 0.1;
	
	bool is_shift = key & K_SHIFT;
	bool is_ctrl = key & K_CTRL;
	bool is_release = key & K_UP;
	key &= 0xFFFF | K_DELTA;
	
	if (is_shift) {
		if (key == K_LEFT) {
			Move(VEC_LEFT * step);
			return true;
		}
		else if (key == K_RIGHT) {
			Move(VEC_RIGHT * step);
			return true;
		}
		else if (key == K_UP) {
			Move(VEC_FWD * step);
			return true;
		}
		else if (key == K_DOWN) {
			Move(VEC_BWD * step);
			return true;
		}
	}
	else if (is_ctrl) {
		step = 0.1;
		if (key == K_LEFT) {
			Rotate(VEC_ROT_LEFT * step);
			return true;
		}
		else if (key == K_RIGHT) {
			Rotate(VEC_ROT_RIGHT * step);
			return true;
		}
		else if (key == K_UP) {
			Rotate(VEC_ROT_UP * step);
			return true;
		}
		else if (key == K_DOWN) {
			Rotate(VEC_ROT_DOWN * step);
			return true;
		}
	}
	else {
		if (key == K_LEFT) {
			Move(VEC_LEFT * step);
			return true;
		}
		else if (key == K_RIGHT) {
			Move(VEC_RIGHT * step);
			return true;
		}
		else if (key == K_UP) {
			Move(VEC_UP * step);
			return true;
		}
		else if (key == K_DOWN) {
			Move(VEC_DOWN * step);
			return true;
		}
	}
	
	return false;
}


END_UPP_NAMESPACE

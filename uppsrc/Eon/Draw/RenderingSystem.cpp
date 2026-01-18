#include "Draw.h"

#if defined(flagDEBUG) && defined(flagDEBUG_GFX)
#include <Geometry/Frustum.h>
#endif

NAMESPACE_UPP


bool Renderable::Initialize(const WorldState& ws) {
	Engine& e = GetEngine();
	RenderingSystemPtr rend = e.TryGet<RenderingSystem>();
	if (rend)
		rend->AddRenderable(this);
	
	return true;
}

void Renderable::Uninitialize() {
	Engine& e = GetEngine();
	RenderingSystemPtr rend = e.TryGet<RenderingSystem>();
	if (rend)
		rend->RemoveRenderable(this);
}

void Renderable::Visit(Vis& v) {
	TODO//e % color % offset % alpha_multiplier;
}



void RenderingSystem::AddViewable(ViewablePtr v) {
	ASSERT(v);
	VectorFindAdd(views, v);
}

void RenderingSystem::RemoveViewable(ViewablePtr v) {
	ASSERT(v);
	VectorRemoveKey(views, v);
}

void RenderingSystem::AddRenderable(RenderablePtr b) {
	ASSERT(b);
	VectorFindAdd(rends, b);
}

void RenderingSystem::RemoveRenderable(RenderablePtr b) {
	ASSERT(b);
	VectorRemoveKey(rends, b);
}

void RenderingSystem::AddModel(ModelComponentPtr m) {
	ASSERT(m);
	VectorFindAdd(models, m);
	LOG("RenderingSystem::AddModel: added model, total models=" << models.GetCount());
}

void RenderingSystem::RemoveModel(ModelComponentPtr m) {
	ASSERT(m);
	VectorRemoveKey(models, m);
}

void RenderingSystem::AddCamera(CameraBase& c) {
	VectorFindAdd(cams, &c);
}

void RenderingSystem::RemoveCamera(CameraBase& c) {
	VectorRemoveKey(cams, &c);
}


void RenderingSystem::Update(double dt) {
	
}

#if defined(flagDEBUG) && defined(flagDEBUG_GFX)
void ExtractFrustum(Frustum& result, const mat4& view, const mat4& proj)
{
    mat4 vp = proj * view;

    const float* clip = (const float*)vp.data;

    // Right
    result.planes[Frustum::RIGHT].normal[0] = clip[3] - clip[0];
    result.planes[Frustum::RIGHT].normal[1] = clip[7] - clip[4];
    result.planes[Frustum::RIGHT].normal[2] = clip[11] - clip[8];
    result.planes[Frustum::RIGHT].distance   = clip[15] - clip[12];

    // Left
    result.planes[Frustum::LEFT].normal[0] = clip[3] + clip[0];
    result.planes[Frustum::LEFT].normal[1] = clip[7] + clip[4];
    result.planes[Frustum::LEFT].normal[2] = clip[11] + clip[8];
    result.planes[Frustum::LEFT].distance   = clip[15] + clip[12];

    // Bottom
    result.planes[Frustum::BOTTOM].normal[0] = clip[3] + clip[1];
    result.planes[Frustum::BOTTOM].normal[1] = clip[7] + clip[5];
    result.planes[Frustum::BOTTOM].normal[2] = clip[11] + clip[9];
    result.planes[Frustum::BOTTOM].distance   = clip[15] + clip[13];

    // Top
    result.planes[Frustum::TOP].normal[0] = clip[3] - clip[1];
    result.planes[Frustum::TOP].normal[1] = clip[7] - clip[5];
    result.planes[Frustum::TOP].normal[2] = clip[11] - clip[9];
    result.planes[Frustum::TOP].distance   = clip[15] - clip[13];

    // Far
    result.planes[Frustum::FAR].normal[0] = clip[3] - clip[2];
    result.planes[Frustum::FAR].normal[1] = clip[7] - clip[6];
    result.planes[Frustum::FAR].normal[2] = clip[11] - clip[10];
    result.planes[Frustum::FAR].distance   = clip[15] - clip[14];

    // Near
    result.planes[Frustum::NEAR].normal[0] = clip[3] + clip[2];
    result.planes[Frustum::NEAR].normal[1] = clip[7] + clip[6];
    result.planes[Frustum::NEAR].normal[2] = clip[11] + clip[10];
    result.planes[Frustum::NEAR].distance   = clip[15] + clip[14];

    for (int i = 0; i < 6; i++) {
        float length = result.planes[i].normal.GetLength();
        result.planes[i].normal /= length;
        result.planes[i].distance /= length;
    }
}

void RenderingSystem::RenderDebug(GfxDataState& state)
{
    if (cams.IsEmpty()) {
        GFXLOG("RenderDebug: No cameras found.");
        return;
    }

    GFXLOG("--- GFXDEBUG: Frame Summary ---");

    CameraBase* cam = cams[0];
    
    Frustum frustum;
    mat4 view_matrix = cam->view_stereo[0];
    mat4 proj_matrix = cam->proj_stereo[0];
    ExtractFrustum(frustum, view_matrix, proj_matrix);

    int visible_models = 0;
    int culled_models = 0;
    int total_meshes = 0;
    int total_vertices = 0;
    int total_indices = 0;

    GFXLOG("Total models: " << models.GetCount());

    for (ModelComponentPtr& m : models) {
        Model* model = m->GetModel();
        if (!model || model->IsEmpty()) continue;

        Transform* transform = m->val.owner->Find<Transform>();
        if (!transform) continue;
        
        mat4 world_matrix = transform->GetMatrix();
        
        vec3 min_pt(FLT_MAX), max_pt(-FLT_MAX);
        for (const auto& mesh : model->meshes) {
            vec3 mesh_min, mesh_max;
            mesh.GetMinMax(mesh_min, mesh_max);
            for(int i = 0; i < 3; i++) {
                min_pt[i] = Upp::min(min_pt[i], mesh_min[i]);
                max_pt[i] = Upp::max(max_pt[i], mesh_max[i]);
            }
        }
        
        vec3 world_min(FLT_MAX), world_max(-FLT_MAX);
        vec3 corners[8] = {
            {min_pt[0], min_pt[1], min_pt[2]}, {max_pt[0], min_pt[1], min_pt[2]},
            {min_pt[0], max_pt[1], min_pt[2]}, {max_pt[0], max_pt[1], min_pt[2]},
            {min_pt[0], min_pt[1], max_pt[2]}, {max_pt[0], min_pt[1], max_pt[2]},
            {min_pt[0], max_pt[1], max_pt[2]}, {max_pt[0], max_pt[1], max_pt[2]}
        };
        
        for(int i = 0; i < 8; i++) {
            vec3 world_pt = (corners[i].Embed() * world_matrix).Splice();
            for(int j = 0; j < 3; j++) {
                world_min[j] = Upp::min(world_min[j], world_pt[j]);
                world_max[j] = Upp::max(world_max[j], world_pt[j]);
            }
        }
        
        AABB world_aabb;
        world_aabb.position = (world_min + world_max) * 0.5f;
        world_aabb.size = (world_max - world_min) * 0.5f;

        bool is_visible = frustum.Intersects(world_aabb);

        if (is_visible) {
            visible_models++;
            total_meshes += model->meshes.GetCount();
            for (const auto& mesh : model->meshes) {
                total_vertices += mesh.vertices.GetCount();
                total_indices += mesh.indices.GetCount();
            }

            // Screen coverage
            vec2 min_screen(FLT_MAX, FLT_MAX), max_screen(-FLT_MAX, -FLT_MAX);
            
            mat4 vp = proj_matrix * view_matrix;
            
            for (int i = 0; i < 8; ++i) {
                vec3 world_pt = (corners[i].Embed() * world_matrix).Splice();
                vec4 clip_space = world_pt.Embed() * vp;
                if (clip_space[3] == 0) continue;
                vec3 ndc = clip_space.Splice() / clip_space[3];

                vec2 screen_pos;
                screen_pos[0] = (ndc[0] + 1.0f) * 0.5f * state.resolution[0];
                screen_pos[1] = (1.0f - ndc[1]) * 0.5f * state.resolution[1];

                min_screen[0] = Upp::min(min_screen[0], screen_pos[0]);
                min_screen[1] = Upp::min(min_screen[1], screen_pos[1]);
                max_screen[0] = Upp::max(max_screen[0], screen_pos[0]);
                max_screen[1] = Upp::max(max_screen[1], screen_pos[1]);
            }
            
            float screen_area = (max_screen[0] - min_screen[0]) * (max_screen[1] - min_screen[1]);

            // Average color
            vec3 avg_color(0,0,0);
            int mesh_count = model->meshes.GetCount();
            if (mesh_count > 0) {
                for (const auto& mesh : model->meshes) {
                    if (mesh.material >= 0 && mesh.material < model->materials.GetCount()) {
                        avg_color += model->materials[mesh.material].params->diffuse;
                    }
                }
                avg_color /= (float)mesh_count;
            }

            GFXLOG("Model '" << m->val.owner->id << "': VISIBLE"
                   << ", Pos: " << transform->data.position
                   << ", BBox: " << world_min << " -> " << world_max
                   << ", ScreenCov: " << screen_area << "px"
                   << ", AvgColor: " << avg_color);
        } else {
            culled_models++;
            GFXLOG("Model '" << m->val.owner->id << "': CULLED"
                   << ", Pos: " << transform->data.position);
        }
    }

    GFXLOG("Visible models: " << visible_models);
    GFXLOG("Culled models: " << culled_models);
    GFXLOG("Total meshes (visible): " << total_meshes);
    GFXLOG("Total vertices (visible): " << total_vertices);
    GFXLOG("Total indices (visible): " << total_indices);
    GFXLOG("--- GFXDEBUG: End Frame Summary ---");
}
#endif

void RenderingSystem::Render(GfxDataState& state) {
	static int render_count = 0;
	if ((render_count++ % 60) == 0) {
		LOG("RenderingSystem::Render: models=" << models.GetCount() << " cameras=" << cams.GetCount());
	}

#if defined(flagDEBUG) && defined(flagDEBUG_GFX)
    RenderDebug(state);
#endif

	for (ModelComponentPtr& m : models) {

		m->Load(state);

	}

	for (CameraBase* cb : cams) {
		if (calib.is_enabled) {
			cb->calib = calib;
			cb->UpdateCalibration();
		}

		cb->Load(state);
	}
}

bool RenderingSystem::Initialize(const WorldState& ws) {
	return true;
}

bool RenderingSystem::Start() {
	return true;
}

void RenderingSystem::Stop() {
	
}

void RenderingSystem::Uninitialize() {
	ASSERT_(rends.IsEmpty(), "RenderingSystem must be added to Engine before EntityStore");
	
}

bool RenderingSystem::Arg(String key, Value value) {
	
	if (key == "dummy")
		is_dummy = value.ToString() == "true";
	
	return true;
}

void RenderingSystem::CalibrationEvent(GeomEvent& ev) {
	
	if (ev.type == EVENT_HOLO_CALIB) {
		calib.is_enabled = true;
		
		switch (ev.n) {
			case HOLO_CALIB_FOV:		calib.fov += ev.fvalue; break;
			case HOLO_CALIB_SCALE:		calib.scale += ev.fvalue; break;
			case HOLO_CALIB_EYE_DIST:	calib.eye_dist += ev.fvalue; break;
			case HOLO_CALIB_X:			calib.position[0] += ev.fvalue; break;
			case HOLO_CALIB_Y:			calib.position[1] += ev.fvalue; break;
			case HOLO_CALIB_Z:			calib.position[2] += ev.fvalue; break;
			case HOLO_CALIB_YAW:		calib.axes[0] += ev.fvalue; break;
			case HOLO_CALIB_PITCH:		calib.axes[1] += ev.fvalue; break;
			case HOLO_CALIB_ROLL:		calib.axes[2] += ev.fvalue; break;
			default: Panic("invalid holographic calibration subtype");
		}
		
		calib.Dump();
	}
	
}



END_UPP_NAMESPACE

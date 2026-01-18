#include "Eon07.h"
#include <Eon/Interaction/Player.h>
#include <Eon/Draw/ToolboxSystem.h>
#include <Eon/Draw/PaintingSystem.h>
#include <Eon/Draw/ShootingSystem.h>
#include <Eon/Draw/ThrowingSystem.h>
#include <Eon/Draw/PaintStrokeSystem.h>
#include <SoftRend/SoftRend.h>

NAMESPACE_UPP

namespace {

RGBA MakeRGBA(int r, int g, int b, int a) {
	RGBA c;
	c.r = (byte)r;
	c.g = (byte)g;
	c.b = (byte)b;
	c.a = (byte)a;
	return c;
}

vec4 ToVec4(RGBA c) {
	return vec4(c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f);
}

RGBA SampleTexture(const ByteImage& img, vec2 uv) {
	if (img.sz.cx == 0 || img.sz.cy == 0) return MakeRGBA(0,0,0,0);
	int x = (int)(uv[0] * img.sz.cx) % img.sz.cx;
	int y = (int)(uv[1] * img.sz.cy) % img.sz.cy;
	if (x < 0) x += img.sz.cx;
	if (y < 0) y += img.sz.cy;
	
	const byte* p = img.GetIter(x, y);
	if (!p) return MakeRGBA(0,0,0,0);
	
	return MakeRGBA(p[0], p[1], p[2], p[3]);
}

RGBA SampleCube(const ByteImage& img, vec3 dir) {
	if (img.faces.GetCount() != 6) return MakeRGBA(255, 0, 255, 255); // Magenta for error

	float absX = fabs(dir[0]);
	float absY = fabs(dir[1]);
	float absZ = fabs(dir[2]);

	int face;
	float u, v;

	if (absX >= absY && absX >= absZ) {
		// x-major
		if (dir[0] > 0) {
			face = 0; // +x
			u = -dir[2] / absX;
			v = -dir[1] / absX;
		} else {
			face = 1; // -x
			u = dir[2] / absX;
			v = -dir[1] / absX;
		}
	} else if (absY >= absX && absY >= absZ) {
		// y-major
		if (dir[1] > 0) {
			face = 2; // +y
			u = dir[0] / absY;
			v = dir[2] / absY;
		} else {
			face = 3; // -y
			u = dir[0] / absY;
			v = -dir[2] / absY;
		}
	} else {
		// z-major
		if (dir[2] > 0) {
			face = 4; // +z
			u = dir[0] / absZ;
			v = -dir[1] / absZ;
		} else {
			face = 5; // -z
			u = -dir[0] / absZ;
			v = -dir[1] / absZ;
		}
	}

	// Remap u,v from [-1, 1] to [0, 1]
	u = (u + 1.0f) * 0.5f;
	v = (v + 1.0f) * 0.5f;

	return SampleTexture(img.faces[face], vec2(u, v));
}


struct PbrVertex : SoftShaderBase {
	void Process(VertexShaderArgs& args) override {
		mat4 mvp = args.va->view * args.va->model;
		args.v.position = args.v.position * mvp;
	}
};

struct PbrFragment : SoftShaderBase {
	void Process(FragmentShaderArgs& args) override {
		vec3 lightDir = args.fa->light_dir.GetNormalized();
		vec3 normal = args.normal.GetNormalized();
		float diff = max(normal.Dot(lightDir), 0.0f);
		
		vec3 baseColor = args.fa->iIsDiffuse ? ToVec4(SampleTexture(*args.GetTexture(args.fa->iDiffuse), args.tex_coord)).Splice() : vec3(0.8f, 0.8f, 0.8f);
		
		// Simple ambient + diffuse
		vec3 color = baseColor * (0.2f + 0.8f * diff);
		
		// Emissive
		if (args.fa->iIsEmissive) {
			color += ToVec4(SampleTexture(*args.GetTexture(args.fa->iEmissive), args.tex_coord)).Splice();
		}
		
		args.frag_color_out = vec4(color[0], color[1], color[2], 1.0f);
	}
};


struct SkyboxVertex : SoftShaderBase {
	void Process(VertexShaderArgs& args) override {
		mat4 mvp = args.va->view * args.va->model;
		
		// Use original position (unit cube/sphere) as direction
		vec3 dir = args.v.position.Splice();
		
		// Transform position
		args.v.position = args.v.position * mvp;
		
		// Pass direction via normal
		args.v.normal = dir;
	}
};

struct SkyboxFragment : SoftShaderBase {
	void Process(FragmentShaderArgs& args) override {
		vec3 dir = args.normal.GetNormalized();
		
		ConstByteImage* skyTex = args.GetTexture(args.fa->iCubeDisplay);
		if (skyTex && !skyTex->IsEmpty() && skyTex->faces.GetCount() == 6) {
			args.frag_color_out = ToVec4(SampleCube(*skyTex, dir));
		} else {
			float t = 0.5f * (dir[1] + 1.0f);
			args.frag_color_out = ToVec4(MakeRGBA(
				(int)(255 * (1.0f - t) + 100 * t),
				(int)(255 * (1.0f - t) + 100 * t),
				(int)(255 * (1.0f - t) + 255 * t),
				255
			));
		}
	}
};

void RegisterShaders() {
	static bool done = false;
	if (done) return;
	done = true;
	
	SoftShaderLibrary::AddShaderClass<PbrVertex>(GVar::VERTEX_SHADER, "pbr_vertex");
	SoftShaderLibrary::AddShaderClass<PbrFragment>(GVar::FRAGMENT_SHADER, "pbr_fragment");
	SoftShaderLibrary::AddShaderClass<SkyboxVertex>(GVar::VERTEX_SHADER, "skybox_vertex");
	SoftShaderLibrary::AddShaderClass<SkyboxFragment>(GVar::FRAGMENT_SHADER, "skybox_fragment");
}

} // namespace

void Run07lDemoroomFakespatialX11Sw3d(Engine& eng, int method) {
	RegisterShaders();

	eng.GetAdd<InteractionSystem>();
	eng.GetAdd<RenderingSystem>();
	eng.GetAdd<EventSystem>();
	eng.GetAdd<ModelCache>();
	eng.GetAdd<PhysicsSystem>();
	eng.GetAdd<PlayerBodySystem>();
	eng.GetAdd<ToolboxSystemBase>();
	eng.GetAdd<PaintStrokeSystemBase>();
	eng.GetAdd<PaintingInteractionSystemBase>();
	eng.GetAdd<ShootingInteractionSystemBase>();
	eng.GetAdd<ThrowingInteractionSystemBase>();

	auto sys = eng.GetAdd<Eon::ScriptLoader>();
	sys->SetEagerChainBuild(true);

	switch(method) {
	case 1:
	case 2:
		LOG(Format("warning: Run07lDemoroomFakespatialX11Sw3d: method %d not implemented yet", method));
	case 0:
		sys->PostLoadFile(ShareDirFile("eon/tests/07l_demoroom_fakespatial_x11_sw3d.eon"));
		break;
	default:
		throw Exc(Format("Run07lDemoroomFakespatialX11Sw3d: unknown method %d", method));
	}
}

END_UPP_NAMESPACE
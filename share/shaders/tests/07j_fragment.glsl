
// from https://www.shadertoy.com/view/XsfXWX

#define saturate(x) clamp(x,0.0,1.0)

vec3 PBR_HDRCubemap(vec3 sampleDir, float LOD_01)
{
    vec3 linearGammaColor_sharp = pow(texture( iCubeDiffuse, sampleDir ).rgb,vec3(2.2));
    vec3 linearGammaColor_blur  = pow(texture( iCubeIrradiance, sampleDir ).rgb,vec3(1));
    vec3 linearGammaColor = mix(linearGammaColor_sharp,linearGammaColor_blur,saturate(LOD_01));
    return linearGammaColor;
}



float G1V ( float dotNV, float k ) {
	return 1.0 / (dotNV*(1.0 - k) + k);
}

float GGX(vec3 N, vec3 V, vec3 L, float roughness, float F0) {
    	float alpha = roughness*roughness;
	vec3 H = normalize (V + L);

	float dotNL = clamp (dot (N, L), 0.0, 1.0);
	float dotNV = clamp (dot (N, V), 0.0, 1.0);
	float dotNH = clamp (dot (N, H), 0.0, 1.0);
	float dotLH = clamp (dot (L, H), 0.0, 1.0);

	float D, vis;
	float F;

	// NDF : GGX
	float alphaSqr = alpha*alpha;
	float pi = 3.1415926535;
	float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0;
	D = alphaSqr / (pi * denom * denom);

	// Fresnel (Schlick)
	float dotLH5 = pow (1.0 - dotLH, 5.0);
	F = F0 + (1.0 - F0)*(dotLH5);

	// Visibility term (G) : Smith with Schlick's approximation
	float k = alpha / 2.0;
	vis = G1V (dotNL, k) * G1V (dotNV, k);

	return /*dotNL */ D * F * vis;
}

uniform vec4 iModelColor;



void mainImage(out vec4 fragColor, in vec2 fragCoord)

{

	float intensity = dot(normalize(vNormal), iLightDir);

	intensity = (1.0 + intensity) * 0.5;

	

	if (iIsDiffuse == false) {

		fragColor = iModelColor * intensity;

		fragColor.a = 1.0;

		return;

	}

	

	vec2 tex_coord = vTexCoord.xy / vTexCoord.z;

	vec3 ray = normalize(vPosition - iCameraPos);

	vec3 normal = normalize(vNormal);

	

	// material

	float metallic = 0.9;

	float spec_value = iIsSpecular ? texture(iSpecular, tex_coord).r : 0.5;

	float roughness = 0.5;

	float fresnel_pow = mix(5.0, 3.5, metallic);

	vec3 light_color = vec3(1.0);

	vec3 clr = texture(iDiffuse, tex_coord).rgb;

	vec3 color_mod = iModelColor.rgb;

	

	// IBL

	vec3 ibl_diffuse = clr;

	vec3 ibl_reflection = iIsCubeIrradiance ? texture(iCubeIrradiance, reflect(ray, normal)).rgb : vec3(0.1);

	

	// fresnel

	float fresnel = max(1.0 - dot(normal, -ray), 0.0);

	fresnel = pow(fresnel, fresnel_pow);    

	

	// reflection        

	vec3 refl = iIsCubeDiffuse ? texture(iCubeDiffuse, reflect(ray, normal)).rgb : ibl_reflection;

	refl = mix(refl, ibl_reflection, (1.0 - fresnel) * roughness);

	refl = mix(refl, ibl_reflection, roughness);

	

	// specular

	vec3 light = iLightDir;

	vec3 spec = light_color * GGX(normal, -ray, light, roughness * 0.7, 0.2);

	refl -= spec;

	

	// diffuse

	vec3 diff = ibl_diffuse * spec_value;

	diff = mix(diff * color_mod, refl, fresnel);        

	

	vec3 color = mix(diff, refl * color_mod, metallic) + spec;

	fragColor = vec4(color * intensity, 1.0);

}







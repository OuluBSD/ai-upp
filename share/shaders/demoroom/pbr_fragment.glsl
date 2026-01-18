
#define saturate(x) clamp((x), 0.0, 1.0)

float G1V(float dotNV, float k)
{
	return 1.0 / (dotNV * (1.0 - k) + k);
}

float geometricOcclusion(float NdotL, float NdotV, float alphaRoughness)
{
	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(alphaRoughness * alphaRoughness + (1.0 - alphaRoughness * alphaRoughness) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(alphaRoughness * alphaRoughness + (1.0 - alphaRoughness * alphaRoughness) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

float microfacetDistribution(float NdotH, float alphaRoughness)
{
	float roughnessSq = alphaRoughness * alphaRoughness;
	float f = (NdotH * roughnessSq - NdotH) * NdotH + 1.0;
	return roughnessSq / (3.1415926535 * f * f);
}

vec3 specularReflection(vec3 reflectance0, vec3 reflectance90, float VdotH)
{
	return reflectance0 + (reflectance90 - reflectance0) * pow(saturate(1.0 - VdotH), 5.0);
}

vec3 diffuseTerm(vec3 diffuseColor)
{
	return diffuseColor / 3.1415926535;
}

vec3 getIBLContribution(float perceptualRoughness, float NdotV, vec3 diffuseColor, vec3 specularColor, vec3 n, vec3 reflection)
{
	float lod = perceptualRoughness * 5.0;
	vec2 brdf = texture(iBrdfSpecular, vec2(NdotV, 1.0 - perceptualRoughness)).rg;
	vec3 diffuseLight = texture(iCubeIrradiance, n).rgb;
	vec3 specularLight = textureLod(iCubeDiffuse, reflection, lod).rgb;
	vec3 diffuse = diffuseLight * diffuseColor;
	vec3 specular = specularLight * (specularColor * brdf.x + brdf.y);
	return diffuse + specular;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec3 baseColor = iIsDiffuse ? texture(iDiffuse, vTexCoord.xy).rgb : vec3(1.0);
	vec3 mrSample = iIsSpecular ? texture(iSpecular, vTexCoord.xy).rgb : vec3(0.0, 1.0, 0.0);
	vec3 emissive = iIsEmissive ? texture(iEmissive, vTexCoord.xy).rgb : vec3(0.0);
	float occlusion = iIsAmbient ? texture(iAmbient, vTexCoord.xy).r : 1.0;

	float metallic = saturate(mrSample.b);
	float perceptualRoughness = clamp(mrSample.g, 0.04, 1.0);
	float alphaRoughness = perceptualRoughness * perceptualRoughness;

	vec3 n = normalize(vNormal);
	if (iIsNormals) {
		vec3 nm = texture(iNormals, vTexCoord.xy).xyz * 2.0 - 1.0;
		n = normalize(mix(n, nm, 1.0));
	}

	vec3 v = normalize(iCameraPos - vPosition);
	vec3 l = normalize(iLightDir);
	vec3 h = normalize(l + v);
	vec3 reflection = -normalize(reflect(v, n));

	float NdotL = saturate(dot(n, l));
	float NdotV = saturate(dot(n, v)) + 0.001;
	float NdotH = saturate(dot(n, h));
	float VdotH = saturate(dot(v, h));

	vec3 f0 = vec3(0.04);
	vec3 diffuseColor = baseColor * (vec3(1.0) - f0) * (1.0 - metallic);
	vec3 specularColor = mix(f0, baseColor, metallic);
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
	float reflectance90 = saturate(reflectance * 25.0);

	vec3 F = specularReflection(specularColor, vec3(reflectance90), VdotH);
	float G = geometricOcclusion(NdotL, NdotV, alphaRoughness);
	float D = microfacetDistribution(NdotH, alphaRoughness);

	vec3 diffuseContrib = (vec3(1.0) - F) * diffuseTerm(diffuseColor);
	vec3 specContrib = F * G * D / max(4.0 * NdotL * NdotV, 0.001);
	vec3 color = NdotL * (diffuseContrib + specContrib);

	color += getIBLContribution(perceptualRoughness, NdotV, diffuseColor, specularColor, n, reflection);
	color = mix(color, color * occlusion, 1.0);
	color += emissive;

	fragColor = vec4(color, 1.0);
}

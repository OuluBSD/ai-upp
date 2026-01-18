
void mainVertex(out vec4 pos_out)
{
	vec4 world = iModel * vec4(iPos.xyz, 1.0);
	vPosition = world.xyz;
	vNormal = normalize(mat3(iModel) * iNormal);
	vTexCoord = vec3(iTexCoord, 0.0);
	pos_out = iView * world;
}

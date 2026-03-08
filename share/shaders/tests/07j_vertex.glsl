void mainVertex(out vec4 pos_out)
{
	pos_out = iView * iModel * iPos;
	
	vNormal = normalize(mat3(iModel) * iNormal);
	vPosition = (iModel * iPos).xyz;
	vTexCoord = vec3(iTexCoord, 1.0);
}
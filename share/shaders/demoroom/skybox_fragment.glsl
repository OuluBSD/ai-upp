
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	fragColor = vec4(texture(iCubeDisplay, vTexCoord).rgb, 1.0);
}

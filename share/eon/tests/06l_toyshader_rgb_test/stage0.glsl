// Simple pass-through shader to display the texture
// This should show the RGB test image with correct colors

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;

    // Sample the texture
    vec4 col = texture(iChannel0, uv);

    fragColor = col;
}

void mainVR( out vec4 fragColor, in vec2 fragCoord, in vec3 fragRayOri, in vec3 fragRayDir )
{
    mainImage(fragColor, fragCoord);
}

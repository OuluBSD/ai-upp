// Stage 0: Initial image generation
// Creates an initial image to be used as input for the buffer stage

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;

    // Create a base pattern
    vec3 color = 0.5 + 0.5*cos(iTime + uv.xyx + vec3(0,2,4));

    fragColor = vec4(color, 1.0);
}
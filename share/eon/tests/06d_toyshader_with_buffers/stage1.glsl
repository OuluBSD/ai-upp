// Stage 1: Buffer processing
// Takes the initial image and applies buffer-based effects

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy / iResolution.xy;

    // Sample from the buffer (using stage color input from stage 0)
    vec4 buf0 = texture(iStageColor0, uv);

    // Apply some buffer-based effects
    vec3 processed = 0.5 * buf0.rgb + 0.5 * cos(iTime + uv.xyx + vec3(0,2,4));

    fragColor = vec4(processed, 1.0);
}
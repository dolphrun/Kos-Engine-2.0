R"(
#version 460 core
uniform sampler2D bloomTexture;
uniform float bloomStrength;
uniform float bloomThreshold;

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    vec3 bloom = texture(bloomTexture, texCoord).rgb;
    float brightness = dot(bloom, vec3(0.2126, 0.7152, 0.0722));
    
    if(brightness <= bloomThreshold)discard;
    fragColor = vec4(bloom * bloomStrength, 1.0);
}
)"
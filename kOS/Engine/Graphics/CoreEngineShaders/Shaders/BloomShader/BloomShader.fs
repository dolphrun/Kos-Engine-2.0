R"(
#version 460 core
uniform sampler2D bloomTexture;
uniform float bloomStrength;

in vec2 texCoord;
out vec4 fragColor;

void main()
{
    vec3 bloom = texture(bloomTexture, texCoord).rgb;
    fragColor = vec4(bloom * bloomStrength, 1.0);
}
)"
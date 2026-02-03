R"(
#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
in float Noise;
uniform sampler2D screenTexture;

void main()
{ 
    FragColor = texture(screenTexture, TexCoords);
    float x = (TexCoords.x + 4.0) * (TexCoords.y + 4.0);
    vec3 grain = vec3(mod((mod(x, 13.0) + 1.0), 13.0) * (mod(x, 123.0) + 1.0) * 0.01 - 0.005) * Noise;
    FragColor.rgb += grain;
}
)"
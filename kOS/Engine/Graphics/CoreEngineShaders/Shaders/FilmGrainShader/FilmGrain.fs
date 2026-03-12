R"(
#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
in float Noise;
in float Time;
uniform sampler2D screenTexture;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}


void main()
{ 
    FragColor = texture(screenTexture, TexCoords);
    float x = (TexCoords.x + 4.0) * (TexCoords.y + 4.0);
    float noise = rand(TexCoords + fract(Time)) * 2.0 - 1.0; // range -1 to 1
    FragColor.rgb += noise * Noise;
}
)"
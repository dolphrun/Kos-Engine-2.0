R"(
#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform float iNoise;
uniform float iTime;
out vec2 TexCoords;

out float Noise;
out float Time;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y,0.0, 1.0);
    Noise=iNoise;
    Time=iTime;
    TexCoords = aTexCoords;
}  
)"
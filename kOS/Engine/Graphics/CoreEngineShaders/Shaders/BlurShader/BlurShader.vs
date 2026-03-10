R"(
#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
uniform vec2 iResoVal;  
uniform float iRadius;

out vec2 TexCoords;
out vec2 ResoVal;
out float Radius;


void main()
{
    gl_Position = vec4(aPos.x, aPos.y,0.0, 1.0);

    ResoVal=iResoVal;
    Radius=iRadius;
    TexCoords = aTexCoords;
}  
)"
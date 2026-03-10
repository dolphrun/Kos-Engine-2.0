R"(
#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
uniform vec2 iResolution;  
uniform float iIntensity;
uniform float iExtent;
uniform vec3 iVignetteColor;
out vec2 TexCoords;
out vec2 Resolution;
out float Intensity;
out float Extent;
out vec3 VignetteColor;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y,0.0, 1.0);
    Resolution=iResolution;
    Intensity=iIntensity;
    Extent=iExtent;
    TexCoords = aTexCoords;
    VignetteColor=iVignetteColor;
}  
)"
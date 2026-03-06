R"(

#version 460 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;

out vec4 vColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(inPos, 1.0);
    vColor = inColor;
}

)"
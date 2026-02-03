R"( #version 460 core


    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;

    out vec2 TexCoords;

    uniform mat4 transformation;
    uniform mat4 projection;
    uniform mat4 view;

void main()
{
    
    
    mat4 model = transformation;

    mat4 matrix = projection * view * model;

    gl_Position = matrix * vec4(aPos, 1.0);

    TexCoords = aTexCoord;
}

)"
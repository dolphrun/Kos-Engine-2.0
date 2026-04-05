R"(
#version 460 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 texCO;

out vec2 texCoords;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 point;
uniform vec2 scale;
uniform vec3 rotation;  // radians

void main()
{
    texCoords = texCO;

    float cx = cos(rotation.x);
    float sx = sin(rotation.x);
    float cy = cos(rotation.y);
    float sy = sin(rotation.y);
    float cz = cos(rotation.z);
    float sz = sin(rotation.z);

    mat3 rotX = mat3(
        vec3(1,  0,   0),   // col 0
        vec3(0,  cx, sx),   // col 1
        vec3(0, -sx, cx)    // col 2
    );

    mat3 rotY = mat3(
        vec3( cy, 0, -sy),  // col 0
        vec3(  0, 1,   0),  // col 1
        vec3( sy, 0,  cy)   // col 2
    );

    mat3 rotZ = mat3(
        vec3( cz, sz, 0),   // col 0
        vec3(-sz, cz, 0),   // col 1
        vec3(  0,  0, 1)    // col 2
    );

    mat3 rotationMatrix = rotZ * rotY * rotX;
    // Then scale in local axes
    vec3 scaledVertex =vec3(vertex.xy * scale, vertex.z);

    // Then translate
    vec3 worldPos = point + rotationMatrix * scaledVertex;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
)"
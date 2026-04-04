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
        1, 0, 0,
        0, cx, -sx,
        0, sx, cx
    );

    mat3 rotY = mat3(
        cy, 0, sy,
        0, 1, 0,
        -sy, 0, cy
    );

    mat3 rotZ = mat3(
        cz, -sz, 0,
        sz,  cz, 0,
        0,   0,  1
    );

    mat3 rotationMatrix = rotZ * rotY * rotX;

    // Apply rotation first (object space)
    vec3 rotatedVertex = rotationMatrix * vertex;

    // Then scale in local axes
    vec3 scaledVertex = vec3(rotatedVertex.xy * scale, rotatedVertex.z);

    // Then translate
    vec3 worldPos = point + scaledVertex;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
)"
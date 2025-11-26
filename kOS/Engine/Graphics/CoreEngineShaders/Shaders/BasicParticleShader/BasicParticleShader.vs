R"(
#version 460 core

layout(location = 0) in vec3 inPos;       // base quad vertex (-0.5..0.5)
layout(location = 1) in vec2 texCoords;
layout(location = 2) in vec3 instancePos; // per-particle center
layout(location = 3) in vec2 instanceScale;
layout(location = 4) in vec4 instanceColor;
layout(location = 5) in vec3 instanceRot; // per-particle rotation (around view axis)
layout(location = 6) in int textureID;
layout(location = 7) in float particleType;

out vec4 vColor;
out flat int vTexture;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;
uniform float uShaderType;
out float shaderType;

void main()
{

if (particleType > 0.f) //three dimesional rotation
{
        // Extract full camera basis from view matrix
    vec3 cameraRight   = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp      = vec3(view[0][1], view[1][1], view[2][1]);
    vec3 cameraForward = -vec3(view[0][2], view[1][2], view[2][2]); // negative because view looks down -Z
    
    // QUAD local vertex before rotation
    vec3 local = vec3(inPos.xy * instanceScale, 0.0);
    
    // Build rotation matrix from Euler angles
    float cx = cos(instanceRot.x); float sx = sin(instanceRot.x);
    float cy = cos(instanceRot.y); float sy = sin(instanceRot.y);
    float cz = cos(instanceRot.z); float sz = sin(instanceRot.z);
    
    // Rotation matrices
    mat3 rotX = mat3(
        1,  0,   0,
        0,  cx, -sx,
        0,  sx,  cx
    );
    mat3 rotY = mat3(
         cy, 0, sy,
         0,  1, 0,
        -sy, 0, cy
    );
    mat3 rotZ = mat3(
        cz, -sz, 0,
        sz,  cz, 0,
        0,    0, 1
    );
    
    mat3 rot3D = rotZ * rotY * rotX;
    
    // Rotate quad in local space
    vec3 rotated = rot3D * local;
    
    // Transform to world space using camera billboard basis
    vec3 worldPos = instancePos
        + cameraRight   * rotated.x
        + cameraUp      * rotated.y
        + cameraForward * rotated.z;  // Use camera forward, not world Z
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    vColor     = instanceColor;
    vTexture   = textureID;
    TexCoords  = texCoords;
    shaderType = uShaderType;
}
else // two dimensional rotation
{
    // Extract camera right and up vectors from view matrix
        vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
        vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);

        // Local rotation (around Z/view axis)o.o
        float c = cos(instanceRot.x);
        float s = sin(instanceRot.x);
        mat2 rot = mat2(c, -s, s, c);

        // Apply local rotation and scale to quad vertex
        vec2 rotated = rot * (inPos.xy * instanceScale);

        // Build the final world position from the camera basis
        vec3 worldPos = instancePos 
                      + cameraRight * rotated.x
                      + cameraUp * rotated.y;

        gl_Position = projection * view * vec4(worldPos, 1.0);
        vColor = instanceColor;
        vTexture = textureID;   
        TexCoords = texCoords;
        shaderType=uShaderType;
}
    
}
)"
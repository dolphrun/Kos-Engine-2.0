/*
 FILENAME: GBuffPBRShader.vs
 AUTHOR(S): Gabe (100%)
 @version 460 core
 All content � 2025 DigiPen Institute of Technology Singapore. All
 rights reserved.
 */

R"(
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBinormal;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4  aWeights;
    
out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 ReflectDir;
out mat3 tangentToWorld;
out float shaderType;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;
uniform int isRigged;

const int MAX_BONES = 200;
uniform mat4 bones[MAX_BONES];

//Debug con
uniform float uShaderType;

void main()
{
    mat4 boneTransform =
        bones[aBoneIDs[0]] * aWeights[0] +
        bones[aBoneIDs[1]] * aWeights[1] +
        bones[aBoneIDs[2]] * aWeights[2] +
        bones[aBoneIDs[3]] * aWeights[3];

    if (isRigged == 0)
    {
      boneTransform = mat4(1.f);
    }
    

    vec4 skinnedPos    = boneTransform * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(boneTransform) * aNormal;

    vec4 worldPos = model * skinnedPos;

    //ReflectDir=reflect( normalize(cameraPosition - aPos), normalize(aNormal));

    ReflectDir = reflect(normalize(cameraPosition - worldPos.xyz),
                         normalize(skinnedNormal));

    mat4 MV = view * model; // Model-View transform matrix
    mat3 N = mat3(vec3(MV[0]), vec3(MV[1]), vec3(MV[2])); // Normal transform matrix
    Normal = normalize(N* skinnedNormal);

    vec3 tang     = normalize(vec3(aTangent));
    vec3 binormal = normalize(vec3(aBinormal));

     tangentToWorld = mat3(
        tang.x,     binormal.x, Normal.x,
        tang.y,     binormal.y, Normal.y,
        tang.z,     binormal.z, Normal.z
    );

    
    //vec4 VertexPositionInView = MV * vec4(aPos, 1.0f);

    vec4 VertexPositionInView = view * worldPos;
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * VertexPositionInView; 

    TexCoords = aTexCoords;  
    shaderType=uShaderType;
}
)"
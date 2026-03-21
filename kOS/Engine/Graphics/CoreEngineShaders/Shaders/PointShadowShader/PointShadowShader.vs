R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4  aWeights;

uniform mat4 model;
uniform int isRigged;

const int MAX_BONES = 200;
uniform mat4 bones[MAX_BONES];
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
    gl_Position = model * skinnedPos;
}
)"
R"(
#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gReflect;
layout (location = 4) out vec4 gMaterial;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 ReflectDir;
in mat3 tangentToWorld;
in float shaderType;
in flat int vTexture;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_roughness1;
uniform sampler2D textures[32];

struct Material 
{
    float reflectivity;
};
uniform Material material;
uniform int entityID=-1;
in vec4 vColor;
void main()
{          
            if(vTexture < 32){
                vec4 texColor = texture(textures[vTexture], TexCoords) * vColor;
                gAlbedoSpec.rgb = texColor.rgb;
                gMaterial.b=shaderType;
            } else{
                
                gAlbedoSpec.rgb = vColor.rgb;
                gMaterial.b=shaderType;
            }
}
)"
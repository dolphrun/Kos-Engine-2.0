R"(
#version 460 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 ReflectDir;
in mat3 tangentToWorld;
in float shaderType;
in flat int vTexture;

uniform sampler2D textures[32];

struct Material 
{
    float reflectivity;
};
uniform Material material;
uniform int entityID=-1;
in vec4 vColor;

out vec4 FragColor;

void main()
{          
    //Use frag color, bring it over to another pass..?
    FragColor =  vColor;
}
)"
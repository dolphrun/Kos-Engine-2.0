R"(
#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec3 colOffset;
uniform sampler2D screenTexture;

void main()
{ 
    vec2 centerOffset = TexCoords - 0.5;
    FragColor.rgb =vec3(texture(screenTexture, TexCoords+centerOffset *colOffset.r).r,
                        texture(screenTexture, TexCoords+centerOffset *colOffset.g).g,
                        texture(screenTexture, TexCoords+centerOffset *colOffset.b).b);
    FragColor.a = texture(screenTexture, TexCoords).a;
}
)"
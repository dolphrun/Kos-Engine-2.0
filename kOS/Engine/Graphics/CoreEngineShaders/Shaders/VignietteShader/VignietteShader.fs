R"(
#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec2 Resolution;
in float Intensity;
in float Extent;
uniform sampler2D screenTexture;
in vec3 VignetteColor;

void main()
{ 

    vec4 texColor =texture(screenTexture, TexCoords); 
    vec2 uv = gl_FragCoord.xy / Resolution.xy;
   
    uv *=  1.0 - uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
    
    float vig = uv.x*uv.y * Intensity; // multiply with sth for intensity
    
    vig = pow(vig, Extent); // change pow for modifying the extend of the  vignette
    // Blend the vignette with the texture
    texColor.rgb = mix(VignetteColor, texColor.rgb, vig);

    // Output the final color
    FragColor = texColor;
}
)"
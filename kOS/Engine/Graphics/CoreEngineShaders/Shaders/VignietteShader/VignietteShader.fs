/*
 FILENAME: VignetteShader.fs
 AUTHOR(S): Keith (100%)
 @version 460 core
 BRIEF: Fragment shader for rendering text with customizable color.
 
 \ParamType TexCoord Input texture coordinates for the character texture.
 \ParamType character A uniform sampler2D representing the character texture.
 \ParamType  textColor A uniform vec3 specifying the RGB color of the text.
 
 This shader takes the texture coordinates, samples the character 
 texture at the specified coordinates, and colors the character based 
 on the provided text color.

 All content � 2024 DigiPen Institute of Technology Singapore. All
 rights reserved.
 */
#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;
in vec2 Resolution;
in float Intensity;
in float Extent;
uniform sampler2D screenTexture;

void main()
{ 

    vec4 texColor =texture(screenTexture, TexCoords); 
    vec2 uv = gl_FragCoord.xy / Resolution.xy;
   
    uv *=  1.0 - uv.yx;   //vec2(1.0)- uv.yx; -> 1.-u.yx; Thanks FabriceNeyret !
    
    float vig = uv.x*uv.y * Intensity; // multiply with sth for intensity
    
    vig = pow(vig, Extent); // change pow for modifying the extend of the  vignette
    // Blend the vignette with the texture
    texColor.rgb *= vig; // Apply the vignette effect to the texture's RGB channels

    // Output the final color
    FragColor = texColor;
}
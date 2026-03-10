R"(
#version 460 core
in vec2 TexCoords;
in vec2 ResoVal;
in float Radius;
out vec4 FragColor;
uniform sampler2D txr;

void main()
{
    float x, y, rr = Radius * Radius, w, w0, wsum = 0.0;
    vec2  d = vec2(1.0 / ResoVal.x, 1.0 / ResoVal.y);
    vec4  col = vec4(0.0);
    w0 = 0.5135 / pow(Radius, 0.96);
    for (x = -Radius; x <= Radius; x++)
    for (y = -Radius; y <= Radius; y++)
        {
        w     = w0 * exp(-(x*x + y*y) / (2.0 * rr));
        col  += texture(txr, TexCoords + vec2(x, y) * d) * w;
        wsum += w;
        }
    FragColor = col / wsum;
}
)"
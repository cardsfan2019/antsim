#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{    
    ivec2 textureSize2d = textureSize(texture1, 0);
    vec4 color = texture(texture1, TexCoords);
    FragColor = color;
}
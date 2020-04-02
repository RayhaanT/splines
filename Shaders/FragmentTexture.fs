#version 400 core
out vec4 FragColour;

in vec2 texCoords;

uniform sampler2D text;

void main()
{
    vec4 textureSample = texture(text, texCoords);
    if(textureSample.a < 0.1)
        discard;
    FragColour = textureSample;
}
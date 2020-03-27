#version 400 core

out vec4 FragColour;

in vec3 Normal;
in vec3 FragPos;
in vec3 LightPos;

uniform vec3 colour;

void main()
{
    //Colour
    vec3 ambient = colour;
    FragColour = vec4(ambient, 1);
}
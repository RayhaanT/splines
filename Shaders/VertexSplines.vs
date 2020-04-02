#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;

out vec3 FragPos;
out vec2 texCoords;

void main()
{
    gl_Position = model * vec4(aPos, 1);
    //Find fragment's position in view coords by multiplying by model and view only
    FragPos = vec3(view * model * vec4(aPos, 1));
    texCoords = aTexCoord;
}
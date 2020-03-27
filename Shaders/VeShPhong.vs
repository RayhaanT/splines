#version 400 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos;

out vec3 FragPos;

void main()
{
    gl_Position = model * vec4(aPos, 1);
    //Find fragment's position in view coords by multiplying by model and view only
    FragPos = vec3(view * model * vec4(aPos, 1));
}
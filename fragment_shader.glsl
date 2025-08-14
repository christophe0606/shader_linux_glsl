#version 330 core

out vec4 FragColor;  // Declare an output variable
uniform vec3 uColor;  // our uniform

void main()
{
   FragColor = vec4(uColor,1.0);
}
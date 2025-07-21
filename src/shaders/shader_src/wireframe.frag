#version 330 core

out vec4 FragColor; 

in vec3 FragPos;    
in vec3 Normal;     
in vec3 LightPos;   

uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    FragColor = vec4(1.0,0.1,1.0,1.0);
}

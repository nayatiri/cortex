#version 330 core

layout(location = 0) in vec3 aPosition; 
layout(location = 1) in vec3 aNormal;   

out vec3 fragNormal; 
out vec3 fragPosition;

uniform mat4 model; 
uniform mat4 view;  
uniform mat4 projection; 

void main()
{
    fragNormal = normalize(mat3(model) * aNormal); 
    fragPosition = vec3(model * vec4(aPosition, 1.0));
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
}

#version 330 core

layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 2) in vec3 aNormal;   // Vertex normal

out vec3 FragPos;                       // Position of the fragment
out vec3 Normal;                        // Normal of the fragment
out vec3 LightPos;                      // Position of the light

uniform mat4 model;                     // Model matrix
uniform mat4 view;                      // View matrix
uniform mat4 projection;                // Projection matrix
uniform vec3 lightPosition;             // Light position

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    //Normal = mat3(transpose(inverse(model))) * aNormal;
    Normal = aNormal;

    LightPos = lightPosition; // Pass light position to fragment shader

    gl_Position = projection * view * model * vec4(aPos, 1.0); // Final vertex position
}

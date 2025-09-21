#version 330 core

layout(location = 0) in vec3 aPos;     // Vertex position (x, y, z)
layout(location = 1) in vec2 aTexCoord; // Texture coordinate (u, v)
layout(location = 2) in vec3 aNormals; // Texture coordinate (u, v)
layout(location = 3) in vec3 aTangents; // Texture coordinate (u, v)

out vec2 TexCoord;
out vec4 FragLightSpacePos;
out vec4 FragWorldPos;
out mat3 TBN;

uniform mat4 model;                     
uniform mat4 view;                      
uniform mat4 projection;
uniform mat4 light_space_matrix;

void main() {
    FragLightSpacePos = light_space_matrix *  model *  vec4(aPos, 1.0);
    FragWorldPos = model * vec4(aPos, 1.0);

    vec3 T = normalize(vec3(model * vec4(aTangents, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormals, 0.0)));
    vec3 B = cross(N, T); 

    TBN = mat3(T, B, N);
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
    TexCoord = aTexCoord;
}
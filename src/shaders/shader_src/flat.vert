#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormals;
layout(location = 3) in vec4 aTangents;

out vec2 TexCoord;
out vec4 FragLightSpacePos;
out vec4 FragWorldPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_space_matrix;
uniform mat3 normalMatrix;

void main() {
    FragLightSpacePos = light_space_matrix * model * vec4(aPos, 1.0);
    FragWorldPos = model * vec4(aPos, 1.0);
    
    vec3 T = normalize(normalMatrix * aTangents.xyz);
    vec3 N = normalize(normalMatrix * aNormals);

    vec3 B = normalize(cross(N, T) * aTangents.w);

    TBN = mat3(T, B, N); // Columns: tangent, bitangent, normal

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
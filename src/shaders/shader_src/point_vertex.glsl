#version 450 core

layout(location = 0) in vec3 aPos;     // Vertex position (x, y, z)

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 a_point_location;
uniform float a_point_radius;

layout(location = 1) out float point_size;
layout(location = 2) out vec4 point_location;

void main() {
point_size = a_point_radius;
point_location = model*view*projection*vec4(a_point_location, 1.0);
vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
gl_Position = vec4(vertices[gl_VertexID],0,1);
}

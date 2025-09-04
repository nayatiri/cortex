#version 450 core

out vec4 FragColor;

layout(location = 1) in float point_size;
layout(location = 2) in vec4 point_location;


void main() {
     FragColor = vec4(1.0,0.0,1.0,0.5);
}
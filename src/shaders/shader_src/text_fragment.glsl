#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D fontAtlas;

void main() {
     vec4 color = texture(fontAtlas, TexCoord); // or .a
     FragColor = color;
    //FragColor = vec4(1.0,0.0,1.0,1.0);
}
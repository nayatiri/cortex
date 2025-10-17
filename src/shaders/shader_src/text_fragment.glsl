#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D fontAtlas;
uniform vec3 text_color;

void main() {
     vec4 color = texture(fontAtlas, TexCoord); // or .a
     FragColor = mix(vec4(text_color,1.0), vec4(0.0,0.0,0.0,0.0), 1-color.r);
     if(FragColor.r < 0.05)
     	discard;
}
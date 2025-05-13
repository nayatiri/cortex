#version 330 core

in vec3 fragNormal; 
in vec3 fragPosition; 

out vec4 color; 

uniform vec3 surfaceColor; 
uniform vec3 lightPosition;
uniform vec3 viewPosition; 

void main()
{
    vec3 normal = normalize(fragNormal);

    vec3 lightDir = normalize(lightPosition - fragPosition);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 finalColor = surfaceColor * diff;

    color = vec4(finalColor, 1.0);

}

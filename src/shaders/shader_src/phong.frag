#version 330 core

out vec4 FragColor; 

in vec3 FragPos;    
in vec3 Normal;     
in vec3 LightPos;   

uniform vec3 viewPos;
uniform vec3 lightColor = vec3(0.8,0.8,0.8);
uniform vec3 objectColor = vec3(0.5,0.8,0.2);

void main()
{

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float shininess = 32.0; // Shininess factor
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor;

    // Combine results
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}

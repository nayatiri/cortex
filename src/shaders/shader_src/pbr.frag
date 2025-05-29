#version 330 core

in vec2 TexCoord;
in vec4 FragLightSpacePos;

out vec3 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uDepthMap;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if (projCoords.z > 1.0)
    return 0.0;
    
    projCoords = projCoords * 0.5 + 0.5;
   
    float closestDepth = texture(uDepthMap, projCoords.xy).r; 
   
    float currentDepth = projCoords.z;
   
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  

void main() {
    FragColor = texture(uTexture, TexCoord).rgb * (ShadowCalculation(FragLightSpacePos));
}
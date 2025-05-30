#version 330 core

in vec2 TexCoord;
in vec4 FragLightSpacePos;

out vec3 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uDepthMap;

uniform float ambient = 0.3;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 1.0;
    float closestDepth = texture(uDepthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.005;
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}


void main() {
     float shadow = ShadowCalculation(FragLightSpacePos);
     vec3 texColor = texture(uTexture, TexCoord).rgb;

     FragColor = texColor * (ambient + (1-shadow));
}
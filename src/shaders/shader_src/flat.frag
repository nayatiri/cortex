#version 330 core

in vec2 TexCoord;
in vec4 FragLightSpacePos;
in vec4 FragWorldPos;
in mat3 TBN;

out vec3 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uDepthMap;
uniform sampler2D uNormalMap;

uniform bool use_full_pbr;
uniform vec3 lightPosition;
uniform float ambient = 0.3;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If outside the shadow map
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 1.0;
    }

    float closestDepth = texture(uDepthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.001;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(uDepthMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(uDepthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return 1.0 - shadow;
}

float pbr_normal_component() {

    if(!use_full_pbr) {return 1.0;}

    vec3 lightDir = normalize(lightPosition - FragWorldPos.xyz);

    vec3 tangentNormal = texture(uNormalMap, TexCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0); // [-1,1]

    vec3 worldNormal = normalize(TBN * tangentNormal);

    float diff = max(dot(worldNormal, lightDir), 0.0);

    return diff;

}

void main() {
     float shadow      = ShadowCalculation(FragLightSpacePos);
     float normal_bias = pbr_normal_component();
     vec3  texColor    = texture(uTexture, TexCoord).rgb;
     
     FragColor = (texColor * ambient * normal_bias) + (texColor * shadow * normal_bias);

     //FragColor = texture(uNormalMap, TexCoord).rgb;

}
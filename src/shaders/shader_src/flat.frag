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

    // If outside the shadow map
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 1.0;
    }

    float closestDepth = texture(uDepthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.05; // try 0.05 first

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

void main() {
     float shadow = ShadowCalculation(FragLightSpacePos);	
     vec3 projCoords = FragLightSpacePos.xyz / FragLightSpacePos.w;
     vec3 texColor = texture(uTexture, TexCoord).rgb;
     
     //projCoords = projCoords * 0.5 + 0.5;
     //FragColor = vec3(texture(uDepthMap, projCoords.xy).r);

     FragColor = texColor * (ambient + shadow);
     
}
#version 330 core

in vec2 TexCoord;
in vec4 FragLightSpacePos;
in vec4 FragWorldPos;
in mat3 TBN;

out vec3 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uDepthMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicRoughnessMap; 

uniform bool use_normal_map;
uniform bool useMetallicRoughness;
uniform vec3 lightPosition;
uniform float lightIntensity;
uniform vec3 cameraPosition;
uniform float ambient = 0.1;

const float M_PI = 3.14159265359;

vec3 pbr_lighting(vec3 albedo, float metallic, float roughness, vec3 normal, vec3 lightDir, vec3 viewDir, float shadow) {

//skidding sim
    vec3 lightColor = vec3(1.0);
    vec3 halfDir = normalize(lightDir + viewDir);

    // Cook-Torrance BRDF components

    // Normal Distribution Function (GGX/Trowbridge-Reitz)
    float NdotH = max(dot(normal, halfDir), 0.0);
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    float D = alpha2 / (M_PI * denom * denom);

    // Geometry Shadowing (Smith GGX)
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float G = min(1.0, min(
        (2.0 * NdotH * NdotV) / NdotH,
        (2.0 * NdotH * NdotL) / NdotH
    ));
    // Simplified Smith
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    G = G_L * G_V;

    // Fresnel (Schlick approx)
    vec3 F0 = mix(vec3(0.04), albedo, metallic); // non-metal F0 ~ 0.04, metal uses albedo
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - dot(halfDir, viewDir), 5.0);

    // Cook-Torrance specular
    vec3 numerator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001; // prevent divide by zero
    vec3 specular = numerator / denominator;

    // Energy conservation: diffuse decreases as metallic increases
    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metallic;

    // Final lighting
    vec3 diffuse = kD * albedo / M_PI;
    vec3 radiance = lightColor * lightIntensity;

    return (diffuse + specular) * radiance * NdotL * shadow;
}

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

vec3 getPBRNormal()
{
    if (!use_normal_map) {
        return vec3(0.0, 1.0, 0.0); // fallback upward normal ik it shit
    }

    vec3 tangentNormal = texture(uNormalMap, TexCoord).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);
    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 albedo = texture(uTexture, TexCoord).rgb;
    float shadow = ShadowCalculation(FragLightSpacePos);
    vec3 worldNormal = getPBRNormal();

    vec3 lightDir = normalize(lightPosition - FragWorldPos.xyz);
    vec3 viewDir = normalize(cameraPosition - FragWorldPos.xyz);

    float metallic = 0.0;
    float roughness = 0.0;
    if(useMetallicRoughness) {
        vec4 metallicRoughness = texture(uMetallicRoughnessMap, TexCoord);
        metallic = metallicRoughness.b;
        roughness = metallicRoughness.g;
    } else {

    metallic = 0.0;
    roughness = 0.4;
    }
    
    vec3 ambientLight = albedo * ambient;

    vec3 directLight = vec3(0.0);
    if (use_normal_map) {
        directLight = pbr_lighting(albedo, metallic, roughness, worldNormal, lightDir, viewDir, shadow);
    } else {
    //lambertian diffuse
        float diff = max(dot(worldNormal, lightDir), 0.0);
        directLight = albedo * diff * shadow;
    }

    FragColor = ambientLight + directLight;
}
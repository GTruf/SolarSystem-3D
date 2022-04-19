#version 460 core

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D mainDiffuseTexture;
uniform sampler2D cloudsNormalMap;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float farPlane;
uniform float ambientFactor;
uniform float bias; // For shadows
// uniform bool isNearbyPlanetaryRing; // Not used in the scene, but if desired, it can be implemented as in shader planet.fs

out vec4 fragColor;

// https://www.youtube.com/watch?v=yn5UJzMqxj0
float SampleShadowMap(vec2 coords, float compare) {
    return step(compare, texture(shadowMap, coords).r);
}

float SampleShadowMapLinear(vec2 coords, float compare, vec2 texelSize) {
    vec2 pixelPos = coords / texelSize + vec2(0.5);
    vec2 fracPart = fract(pixelPos);
    vec2 startTexel = (pixelPos - fracPart) * texelSize;

    float blTexel = SampleShadowMap(startTexel, compare);
    float brTexel = SampleShadowMap(startTexel + vec2(texelSize.x, 0.0), compare);
    float tlTexel = SampleShadowMap(startTexel + vec2(0.0, texelSize.y), compare);
    float trTexel = SampleShadowMap(startTexel + texelSize, compare);

    float mixA = mix(blTexel, tlTexel, fracPart.y);
    float mixB = mix(brTexel, trTexel, fracPart.y);

    return mix(mixA, mixB, fracPart.x);
}

void ApplyPCF(out float shadow, vec3 projCoords, float currentDepth) {
    const float NUM_SAMPLES = 2.0; // Change this (lower to increase fps or higher to increase softening)
    const float SAMPLES_START = (NUM_SAMPLES - 1.0) / 2.0;
    const float NUM_SAMPLES_SQUARED = NUM_SAMPLES * NUM_SAMPLES;

    shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(float y = -SAMPLES_START; y <= SAMPLES_START; y += 1.0) {
        for(float x = -SAMPLES_START; x <= SAMPLES_START; x += 1.0) {
            shadow += SampleShadowMapLinear(projCoords.xy + vec2(x, y) * texelSize, currentDepth - bias, texelSize);
        }
    }

    shadow /= NUM_SAMPLES_SQUARED;
}

float CalculateShadow(vec4 fragPosLightSpace) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    float shadow;
    ApplyPCF(shadow, projCoords, currentDepth);
    return shadow;
}

void main() {
    vec3 diffuseColor = texture(mainDiffuseTexture, fs_in.TexCoords).rgb;

    vec3 normal = texture(cloudsNormalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

    float NdotL = dot(normal, lightDir);
    vec3 color = diffuseColor;

    float ambientAlpha = smoothstep(-0.15, 0.25, NdotL);
    float ambientMult = mix(0.01, ambientFactor, ambientAlpha);
    vec3 ambient = ambientMult * color;

    vec3 diffuse;

    float diff = max(dot(lightDir, normal), 0.0);
    diffuse = diff * color;

    float shadow = CalculateShadow(fs_in.FragPosLightSpace);

    if (shadow < 0.05)
        ambient *= 0.1;

    vec3 lighting = ambient + shadow * diffuse;
    fragColor = vec4(lighting, 1.0f);
}
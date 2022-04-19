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
uniform sampler2D cloudTexture;
uniform sampler2D nightTexture;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D ringDiffuse;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 starGlowTint;

uniform float farPlane;
uniform float ambientFactor;
uniform float bias; // For shadows
uniform float yRotation; // For fake cloud shadows

uniform bool hasNightTexture;
uniform bool hasSpecularMap;
uniform bool hasSpecular;
uniform bool hasClouds;
uniform bool isNearbyPlanetaryRing;
uniform bool isUseSphereIntersect; // To avoid the ring shadow while behind a planet (parent planet with rings)

uniform vec3 parentPlanetCenter; // Center of parent planet with planetary ring in eye space
uniform float parentPlanetRadiusSquared;

uniform vec3 ringCenter; // Center of disk in eye space
uniform vec3 ringNormal; // Disk plane normal in eye space
uniform vec2 ringInnerOuterRadiuses; // x = Inner, y = Outer

out vec4 fragColor;

void swap(out float left, out float right) {
    float temp = left;
    left = right;
    right = temp;
}

bool solveQuadratic(float a, float b, float c, out float x0, out float x1) {
    float discr = b * b - 4.0 * a * c;

    if (discr < 0)
        return false;

    else if (discr == 0) {
        x0 = x1 = - 0.5 * b / a;
    }
    else {
        float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }

    if (x0 > x1)
        swap(x0, x1);

    return true;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool intersectSphere(vec3 dir) {
    float t0, t1;

    // Analytic solution
    vec3 L = fs_in.FragPos - parentPlanetCenter;
    float a = dot(dir, dir);
    float b = 2 * dot(dir, L);
    float c = dot(L, L) - parentPlanetRadiusSquared;

    if (!solveQuadratic(a, b, c, t0, t1))
        return false;

    if (t0 > t1)
        swap(t0, t1);

    if (t0 < 0) {
        t0 = t1; // If t0 is negative, let's use t1 instead
        if (t0 < 0) { // Both t0 and t1 are negative
            return false;
        }
    }

    return true;
}

bool intersectPlane(vec3 n, vec3 p0, vec3 l0, vec3 l, out float t) {
    // Assuming vectors are all normalized
    float denom = dot(n, l);
    if (denom > 1e-6) {
        vec3 p0l0 = p0 - l0;
        t = dot(p0l0, n) / denom;
        return (t >= 0);
    }

    return false;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
bool intersectDisk(vec3 n, vec3 p0, float radius, vec3 l0, vec3 l, out float intersectSquared) {
    float t = 0;
    if (intersectPlane(n, p0, l0, l, t)) {
        vec3 p = l0 + l * t;
        vec3 v = p - p0;
        float d2 = dot(v, v);
        intersectSquared = sqrt(d2);
        return d2 <= radius * radius;
    }

    return false;
}

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
    const float NUM_SAMPLES = 3.0; // Change this (lower to increase fps or higher to increase softening)
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
    vec3 lightDir = lightPos - fs_in.FragPos;
    vec3 lightDirNorm = normalize(lightDir);

    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (isNearbyPlanetaryRing) {
        if (isUseSphereIntersect && intersectSphere(normalize(lightPos - fs_in.FragPos))) // Behind the parent planet with rings (to avoid shadow from the ring)
            return 0.0;

        float intersectSquared;
        float NdotL = dot(ringNormal, lightDirNorm);
        vec3 correctRingNormal = ringNormal;

        if (NdotL < 0.0)
            correctRingNormal = -ringNormal;

        if (intersectDisk(correctRingNormal, ringCenter, ringInnerOuterRadiuses.y, fs_in.FragPos, lightDirNorm, intersectSquared)) {
            if (intersectSquared > ringInnerOuterRadiuses.x) {
                // If some planet obscures the ring
                if (shadow > 0.0 && length(lightPos - ringCenter) - closestDepth * farPlane > ringInnerOuterRadiuses.y) {
                    // PCF won't work, because physically in the place where the penumbra from the PCF should be, there will be a shadow from the ring, and not from the planet
                    // ApplyPCF(shadow, projCoords, currentDepth);
                    return 1.0 - shadow;
                }

                // Very high quality shadow from the ring with alpha blending
                float u = (intersectSquared - ringInnerOuterRadiuses.x) / (ringInnerOuterRadiuses.y - ringInnerOuterRadiuses.x);
                vec4 ringColor = texture(ringDiffuse, vec2(u, 0));
                return 1.0 - (ringColor.r + ringColor.g + ringColor.b) * ringColor.a;
            }
        }
    }

    ApplyPCF(shadow, projCoords, currentDepth);
    return shadow;
}

void main() {
    vec3 diffuseColor, specular;

    diffuseColor = texture(mainDiffuseTexture, fs_in.TexCoords).rgb;

    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

    float NdotL = dot(normal, lightDir);

    if (hasClouds) {
        vec2 cloudTexCoord = fs_in.TexCoords - vec2(yRotation / 360.0, 0);
        vec3 cloudColor = texture(cloudTexture, cloudTexCoord).rgb;
        diffuseColor -= cloudColor * 0.5;
    }

    if (hasNightTexture) {
        vec3 nightColor = texture(nightTexture, fs_in.TexCoords).rgb;
        float dayNightAlpha = smoothstep(-0.15, 0.15, NdotL);
        diffuseColor = mix(nightColor, diffuseColor, dayNightAlpha);
    }

    vec3 ambient = ambientFactor * diffuseColor;

    float diff = max(dot(lightDir, normal), 0.0);
    diffuseColor *= diff;

    float spec;
    if (hasSpecular) {
        vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        float specularMix = smoothstep(-0.08, 0.08, NdotL);
        spec = mix(0.0, spec, specularMix) * 0.675;
    }

    if (hasSpecularMap) {
        vec4 specularMapColor = texture(specularMap, fs_in.TexCoords);
        specular = specularMapColor.rrr * specularMapColor.a * spec * starGlowTint;
    }
    else {
        specular = spec * starGlowTint;
    }

    float shadow = CalculateShadow(fs_in.FragPosLightSpace);

    if (shadow < 0.05)
        ambient *= 0.1;

    vec3 lighting;
    if (hasSpecular)
        lighting = ambient + shadow * (diffuseColor + specular);
    else
        lighting = ambient + shadow * diffuseColor;

    fragColor = vec4(lighting, 1.0);
}

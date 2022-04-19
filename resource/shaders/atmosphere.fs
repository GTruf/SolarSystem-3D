//
// Atmospheric scattering fragment shader
//
// Author: Sean O'Neil
//
// Copyright (c) 2004 Sean O'Neil
//

#version 460 core

in vec3 fWorldPosition;
in vec3 fPosition;
in mat3 modelMat3;
in vec4 fragPosLightSpace;

uniform sampler2D ringDiffuse;
uniform sampler2D shadowMap;
uniform float farPlane;
uniform float bias; // For shadows
uniform float earthSizeCoefficient;
uniform bool isUseToneMapping;
uniform bool isNearbyPlanetaryRing;
uniform bool isUseSphereIntersect;

uniform vec3 ringParentPlanetCenter; // Center of parent planet with planetary ring in eye space
uniform float ringParentPlanetRadiusSquared;

uniform vec3 ringCenter; // Center of disk in eye space
uniform vec3 ringNormal; // Disk plane normal in eye space
uniform vec2 ringInnerOuterRadiuses; // x = Inner, y = Outer

uniform vec3 camPosition;
uniform vec3 lightPos;
uniform vec3 C_R; // Main atmosphere color from front side [Основной цвет атмосферы с лицевой стороны]
uniform vec3 mieTint; // mie tint (backlit tint)
uniform float innerRadius;
uniform float outerRadius;

const float PI = 3.14159265359;
const float MAX = 10000.;
const float E = 12.3;        // Exposure
const float K_R = 0.0639999; // Coef rayleigh
const float K_M = 0.0031;    // Coef mie
const float G_M = -0.717998; // The Mie phase asymmetry factor

const int numOutScatter = 10;
const float fNumOutScatter = 10.0;
const int numInScatter = 4;
const float fNumInScatter = 4.0;

uniform float SCALE_H_FACTOR;
uniform float SCALE_L_FACTOR = 1.0;

float SCALE_H = SCALE_H_FACTOR / (outerRadius - innerRadius);
float SCALE_L = SCALE_L_FACTOR / (outerRadius - innerRadius);

out vec4 fragColor;

vec3 rayDirection(vec3 camPos) {
    vec3 ray = normalize(modelMat3 * fPosition - camPos);
    return ray;
}

vec2 rayIntersection(vec3 p, vec3 dir, float radius) {
    float b = dot(p, dir);
    float c = dot(p, p) - radius * radius;

    float d = b * b - c;

    if (d < 0.0)
        return vec2(MAX, -MAX);

    d = sqrt(d);

    float near = -b - d;
    float far = -b + d;

    return vec2(near, far);
}

// Mie
// g : ( -0.75, -0.999 )
//      3 * ( 1 - g^2 )               1 + c^2
// F = ----------------- * -------------------------------
//      2 * ( 2 + g^2 )     ( 1 + g^2 - 2 * g * c )^(3/2)
float miePhase(float g, float c, float cc) {
    float gg = g * g;

    float a = (1.0 - gg) * (1.0 + cc);

    float b = 1.0 + gg - 2.0 * g * c;
    b *= sqrt(b);
    b *= 2.0 + gg;

    return 1.5 * a / b;
}

float rayleighPhase(float cc) {
    return 0.75 * (1.0 + cc);
}

float density(vec3 p) {
    return exp(-(length(p) - innerRadius) * SCALE_H);
}

float optic(vec3 p, vec3 q) {
    vec3 step = (q - p) / fNumOutScatter;
    vec3 v = p + step * 0.5;

    float sum = 0.0;
    for(int i = 0; i < numOutScatter; i++) {
        sum += density(v);
        v += step;
    }
    sum *= length(step) * SCALE_L;
    return sum;
}

vec3 colorInScatter(vec3 o, vec3 dir, vec2 e, vec3 l) {
    float len = (e.y - e.x) / fNumInScatter;
    vec3 step = dir * len;
    vec3 p = o + dir * e.x;
    vec3 v = p + dir * (len * 0.5);


    vec3 sum = vec3(0.0);
    for (int i = 0; i < numInScatter; i++) {
        vec2 f = rayIntersection(v, l, outerRadius);
        vec3 u = v + l * f.y;
        float n = (optic(p, v) + optic(v, u)) * (PI * 4.0);
        sum += density(v) * exp(-n * (K_R * C_R + K_M));
        v += step;
    }

    sum *= len * SCALE_L;
    float c = dot(dir, -l);
    float cc = c * c;
    return sum * (K_R * C_R * rayleighPhase(cc) + K_M * miePhase(G_M, c, cc) * mieTint) * E;
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 acesFilm(const vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d ) + e), 0.0, 1.0);
}

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
    vec3 L = fWorldPosition - ringParentPlanetCenter;
    float a = dot(dir, dir);
    float b = 2 * dot(dir, L);
    float c = dot(L, L) - ringParentPlanetRadiusSquared;

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

float CalculateShadow(vec4 fragPosLightSpace) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(lightPos - fWorldPosition);

    // Check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    if (isNearbyPlanetaryRing) {
        if (isUseSphereIntersect && intersectSphere(normalize(lightPos - modelMat3 * fPosition)))
            return 1.0;

        float intersectSquared;
        float NdotL = dot(ringNormal, lightDir);
        vec3 correctRingNormal = ringNormal;

        if (NdotL < 0.0)
            correctRingNormal = -ringNormal;

        if (intersectDisk(correctRingNormal, ringCenter, ringInnerOuterRadiuses.y, fWorldPosition, lightDir, intersectSquared)) {
            if (intersectSquared > ringInnerOuterRadiuses.x) {
                // If some planet obscures the ring
                if (shadow > 0.0 && length(lightPos - ringCenter) - (closestDepth - bias) * farPlane > ringInnerOuterRadiuses.y) {
                    return shadow;
                }

                float u = (intersectSquared - ringInnerOuterRadiuses.x) / (ringInnerOuterRadiuses.y - ringInnerOuterRadiuses.x);
                vec4 ringColor = texture(ringDiffuse, vec2(u, 0));
                return (ringColor.r + ringColor.g + ringColor.b) * ringColor.a;
            }
        }
    }

    return shadow;
}

float criticalLengthFactor() {
    if (earthSizeCoefficient <= 1.0)
        return 60.0;
    else if (earthSizeCoefficient > 1.0 && earthSizeCoefficient <= 3.0)
        return 120.0;
    else if (earthSizeCoefficient > 3.0 && earthSizeCoefficient <= 5.0)
        return 245.0;
    else
        return 600.;
}

void main() {
    float shadow = CalculateShadow(fragPosLightSpace);

    if (shadow > 0.0)
        discard;

    vec3 eye = camPosition;
    float eyeLength = length(eye);
    float eyeCriticalLengthFactor = criticalLengthFactor();

    if (eyeLength > eyeCriticalLengthFactor) {
        float reductionFactor = eyeCriticalLengthFactor / eyeLength;
        eye *= reductionFactor; // Keep the vector length within eyeCriticalLengthFactor to avoid artifacts (even logarithmic z-buffer doesn't help)
    }

    vec3 dir = rayDirection(eye);
    vec3 l = normalize(lightPos);
    vec2 e = rayIntersection(eye, dir, outerRadius);

    if (e.x > e.y)
        discard;

    vec2 f = rayIntersection(eye, dir, innerRadius);
    e.y = min(e.y, f.x);

    vec3 I = colorInScatter(eye, dir, e, l);

    fragColor = vec4(I /** (1.0 - shadow)*/, 1.0);

    if (isUseToneMapping)
        fragColor.rgb = acesFilm(fragColor.rgb);
}


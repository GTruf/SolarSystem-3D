#version 460 core

uniform sampler2D lensTexture;
uniform vec3 color;

uniform bool isPlanetaryRingInView;

uniform vec3 cameraPosition;
uniform vec3 ringCenter; // Center of disk in eye space
uniform vec3 ringNormal; // Disk plane normal in eye space
uniform vec2 ringInnerOuterRadiuses; // x = Inner, y = Outer
uniform sampler2D ringDiffuse;

in vec3 fCenter;
in vec2 fPosition;
in vec2 fUV;
in float fIntensity;

out vec4 fragColor;

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

void main() {
    float intensity = fIntensity;

    if (isPlanetaryRingInView) {
        float NdotL = dot(ringNormal, fCenter - ringCenter);
        vec3 correctRingNormal = ringNormal;
        if (NdotL < 0.0)
        correctRingNormal = -ringNormal;

        float intersectSquared;

        if (intersectDisk(correctRingNormal, ringCenter, ringInnerOuterRadiuses.y, cameraPosition, normalize(fCenter - cameraPosition), intersectSquared)) {
            if (intersectSquared > ringInnerOuterRadiuses.x) {
                float u = (intersectSquared - ringInnerOuterRadiuses.x) / (ringInnerOuterRadiuses.y - ringInnerOuterRadiuses.x);
                vec4 ringColor = texture(ringDiffuse, vec2(u, 0));
                intensity *= 1 - ringColor.a * 0.65;
            }
        }
    }

    vec4 texColor = texture(lensTexture, fUV) * intensity;
    fragColor = vec4(texColor.rgb * color, texColor.a);
}
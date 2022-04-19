#version 460 core

vec3 mod2893(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod2893(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute3(vec4 x) {
    return mod2893(((x * 34.0) + 1.0)*x);
}

vec4 taylorInvSqrt3(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0);
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i = floor(v + dot(v, C.yyy) );
    vec3 x0 = v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod2893(i);
    vec4 p = permute3( permute3( permute3(
    i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
    + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
    + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0 / 7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p, 7 * 7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j, N)

    vec4 x = x_ * ns.x + ns.yyyy;
    vec4 y = y_ * ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0) * 2.0 + 1.0;
    vec4 s1 = floor(b1) * 2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalize gradients
    vec4 norm = taylorInvSqrt3(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot(m*m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
}

float noise(vec3 position, int octaves, float frequency, float persistence) {
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        total += snoise(position * frequency) * amplitude;
        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}

float absNoise(vec3 position, int octaves, float frequency, float persistence) {
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        total += abs(snoise(position * frequency)) * amplitude;
        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}

float ridgedNoise(vec3 position, int octaves, float frequency, float persistence) {
    float total = 0.0;
    float maxAmplitude = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < octaves; i++) {
        total += ((1.0 - abs(snoise(position * frequency))) * 2.0 - 1.0) * amplitude;
        frequency *= 2.0;
        maxAmplitude += amplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}

float squaredNoise(vec3 position, int octaves, float frequency, float persistence) {
    float n = noise(position, octaves, frequency, persistence);
    return n * n;
}

float cubedNoise(vec3 position, int octaves, float frequency, float persistence) {
    float n = noise(position, octaves, frequency, persistence);
    return n * n * n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

in vec3 fPosition;
uniform vec3 centerDir;
uniform vec3 shiftStarColor;
uniform vec3 colorMult;
uniform sampler2D colorMap;
uniform float uColorMap;
uniform float starRadiusInKilometers;
uniform bool isVisible;

out vec4 fragColor;

void main() {
    vec3 position = fPosition;
    vec3 sPosition = position * starRadiusInKilometers;

    // Sunspots
    float s = 0.3;
    float frequency = 0.00001;
    float t1 = snoise(sPosition * frequency) - s;
    float t2 = snoise((sPosition + starRadiusInKilometers) * frequency) - s;
    float ss = (max(t1, 0.0) * max(t2, 0.0)) * 2.0;

    float n = (noise(position, 4, 40.0, 0.7) + 1.0) * 0.5;
    float totalNoise = n - ss;

    vec3 colorByTemperature = texture(colorMap, vec2(uColorMap, 0)).rgb;

    float theta = 1.0 - dot(centerDir, fPosition) * 0.7;

    fragColor = vec4(colorByTemperature * (shiftStarColor * colorMult + totalNoise - 0.5 - theta), isVisible);
}

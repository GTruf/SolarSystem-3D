#version 460 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in float aOffset;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 center;
uniform vec2 dims;
uniform float intensity;

// Output
out vec3 fCenter;
out vec2 fPosition;
out vec2 fUV;
out float fIntensity;

void main() {
    fCenter = center;
    fUV = aTexCoords;
    // Fixed size billboard
    // Get the screen-space position of the center
    gl_Position = projection * view * vec4(center, 1.0);
    gl_Position /= gl_Position.w;
    vec2 centerPos = gl_Position.xy;
    vec2 offsetVec = vec2(0.0) - centerPos;
    float distance = length(offsetVec);
    // Decrease intensity with distance
    fIntensity = max(0.0, 1.0 - distance / 1.0) * intensity;
    // Decrease intensity when it gets too close
    fIntensity *= min(1.0, distance * 2.0);

    // Rotate the vertices
    vec2 offsetDir = offsetVec / distance;
    float angle = acos(dot(vec2(1.0f, 0.0f), offsetDir));
    if (offsetDir.y < 0.0f)
        angle = -angle;
    fPosition.x = aPos.x * cos(angle) - aPos.y * sin(angle);
    fPosition.y = aPos.x * sin(angle) + aPos.y * cos(angle);

    // Move the vertex in screen space.
    gl_Position.xy += fPosition * dims + offsetVec * aOffset * aOffset * 0.5;
}